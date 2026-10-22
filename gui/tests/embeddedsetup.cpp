#include <QtTest>
#include <SDL.h>
#include <QApplication>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QScopeGuard>
#include <QUuid>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QScreen>
#include <QRegularExpression>
#include <QProcess>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

static QString windowProcess(HWND window)
{
    DWORD processId = 0;
    GetWindowThreadProcessId(window, &processId);
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (!process) return {};
    wchar_t path[32768];
    DWORD size = 32768;
    const bool found = QueryFullProcessImageNameW(process, 0, path, &size);
    CloseHandle(process);
    return found ? QString::fromWCharArray(path, size).section('\\', -1) : QString();
}

static bool protectedWindowVisible()
{
    bool found = false;
    EnumWindows([](HWND window, LPARAM data) -> BOOL {
        if (IsWindowVisible(window) && windowProcess(window).compare("PickerHost.exe", Qt::CaseInsensitive) == 0) {
            *reinterpret_cast<bool *>(data) = true;
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&found));
    return found;
}

static bool inputDesktopAvailable()
{
    const auto desktopName = [](HDESK desktop) {
        wchar_t name[256];
        DWORD needed = 0;
        return GetUserObjectInformationW(desktop, UOI_NAME, name, sizeof(name), &needed)
            ? QString::fromWCharArray(name) : QString();
    };
    HDESK input = OpenInputDesktop(0, FALSE, DESKTOP_READOBJECTS);
    if (!input) return false;
    const auto inputName = desktopName(input);
    const auto threadName = desktopName(GetThreadDesktop(GetCurrentThreadId()));
    CloseDesktop(input);
    qInfo() << "Graphics desktop requirement input/thread" << inputName << threadName
        << "foreground HWND" << quintptr(GetForegroundWindow());
    return !inputName.isEmpty() && inputName == threadName && GetForegroundWindow();
}
#endif
#ifdef CHIAKI_HAVE_WEBENGINE
#include <QtWebEngineQuick>
#endif
#include <memory>
#include "qmlnativeservices.h"
#include "setupwindow.h"
#include "setupbridge.h"
#include "psnrequests.h"
#include "psnsigninwindow.h"
#include <QWebEngineProfile>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QWebEngineCookieStore>
#include <QNetworkCookie>
#include <QProcess>
#include <QWebEngineUrlRequestInfo>
#include <QWebEngineUrlRequestInterceptor>
#ifdef main
#undef main
#endif

class PsnReply : public QNetworkReply
{
    QByteArray payload;
    qint64 offset = 0;
public:
    PsnReply(const QNetworkRequest &request, QObject *parent) : QNetworkReply(parent)
    {
        setRequest(request);
        setUrl(request.url());
        open(QIODevice::ReadOnly | QIODevice::Unbuffered);
    }
    void finish(QByteArray json, NetworkError error = NoError)
    {
        if (isFinished()) return;
        payload = std::move(json);
        if (error != NoError) setError(error, "credential-bearing-network-error");
        setFinished(true);
        emit readyRead();
        emit finished();
    }
    void abort() override { finish({}, OperationCanceledError); }
    qint64 bytesAvailable() const override { return payload.size() - offset + QNetworkReply::bytesAvailable(); }
protected:
    qint64 readData(char *data, qint64 size) override
    {
        const auto count = qMin(size, qint64(payload.size()) - offset);
        if (!count) return -1;
        memcpy(data, payload.constData() + offset, size_t(count));
        offset += count;
        return count;
    }
};

class PsnNetwork : public QNetworkAccessManager
{
    QList<QPointer<PsnReply>> &requests;
public:
    PsnNetwork(QList<QPointer<PsnReply>> &requests, QObject *parent) : QNetworkAccessManager(parent), requests(requests) {}
protected:
    QNetworkReply *createRequest(Operation, const QNetworkRequest &request, QIODevice *) override
    {
        auto *reply = new PsnReply(request, this);
        requests.append(reply);
        return reply;
    }
};

class PsnBrowserOpener : public QObject
{
    Q_OBJECT
public:
    QUrl url;
public slots:
    void open(const QUrl &value) { url = value; }
};

class PsnPageRequests : public QWebEngineUrlRequestInterceptor
{
public:
    using QWebEngineUrlRequestInterceptor::QWebEngineUrlRequestInterceptor;
    QAtomicInteger<int> count = 0;
    void interceptRequest(QWebEngineUrlRequestInfo &request) override
    {
        count.fetchAndAddRelaxed(1);
        request.block(true);
    }
};

class EmbeddedSetupTest : public QObject
{
    Q_OBJECT
    QVariant script(QWebEngineView *view, const QString &source)
    {
        auto result = std::make_shared<QVariant>();
        auto finished = std::make_shared<bool>(false);
        view->page()->runJavaScript(source, [result, finished](const QVariant &value) { *result = value; *finished = true; });
        QElapsedTimer timeout;
        timeout.start();
        while (!*finished && timeout.elapsed() < 5000) QTest::qWait(10);
        return *result;
    }

    QVariantMap cookieMetadata(const QString &path, const QByteArray &name)
    {
        QProcess probe;
        probe.start(QStringLiteral(CHIAKI_TEST_PYTHON), {"-c",
            "import sqlite3,sys,json,time; from pathlib import Path; "
            "db=sqlite3.connect(Path(sys.argv[1]).as_uri()+'?mode=ro',uri=True); "
            "r=db.execute('SELECT COUNT(*),SUM(is_persistent),MAX(expires_utc) FROM cookies WHERE name=?',(sys.argv[2],)).fetchone(); "
            "print(json.dumps(dict(count=r[0],persistent=r[1] or 0,future=bool(r[2] and r[2]/1000000-11644473600>time.time()))))",
            QDir(path).filePath("Cookies"), QString::fromUtf8(name)});
        if (!probe.waitForFinished(5000) || probe.exitCode() != 0) return {};
        return QJsonDocument::fromJson(probe.readAllStandardOutput()).toVariant().toMap();
    }

    void discover(QmlBackend *backend, const QList<DiscoveryHost> &hosts)
    {
        // This is the queued completion boundary used by the native discovery callback.
        QVERIFY(QMetaObject::invokeMethod(backend->discoveryManager(), "DiscoveryServiceHosts",
            Qt::QueuedConnection, Q_ARG(QList<DiscoveryHost>, hosts)));
    }

    QVariantMap settingRequest(QWebEngineView *view, const QString &name, const QVariant &value, const QString &generation, const QString &method = "setGeneralSetting")
    {
        const QString arguments = QString::fromUtf8(QJsonDocument(QJsonArray{name, QJsonValue::fromVariant(value), generation}).toJson(QJsonDocument::Compact));
        // A second real WebChannel client checks rejected public commands, then restores the UI client's receiver.
        script(view, QString(R"(
            window.settingsReply = null;
            { const transport = window.qt.webChannelTransport, receive = transport.onmessage;
              new QWebChannel(transport, channel => channel.objects.setup.%2(...%1, reply => {
                window.settingsReply = reply; transport.onmessage = receive;
              })); }
        )").arg(arguments, method));
        QElapsedTimer timeout;
        timeout.start();
        while (timeout.elapsed() < 5000) {
            const auto reply = script(view, "window.settingsReply").toMap();
            if (!reply.isEmpty()) return reply;
            QTest::qWait(10);
        }
        return {};
    }

private slots:
    void init()
    {
#ifdef Q_OS_WIN
        if (QString::fromLatin1(QTest::currentTestFunction()).startsWith("psn") && QGuiApplication::platformName() != "offscreen") {
            QVERIFY(inputDesktopAvailable());
            QVERIFY(!protectedWindowVisible());
        }
#endif
    }

    void psnAccountPresentation()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        QmlNativeServices services(&settings);
        services.backend()->stopAutoConnect();
        SetupWindow setup(services.backend());
        setup.resize(1280, 900);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-psn-account]') !== null").toBool());
        script(setup.view(), "document.querySelector('[data-psn-account]').click()");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.tagName").toString(), QStringLiteral("H2"));
        QVERIFY(script(setup.view(), "const e=new KeyboardEvent('keydown',{key:'ArrowLeft',bubbles:true,cancelable:true});document.querySelector('[data-psn-redirect]').dispatchEvent(e);!e.defaultPrevented").toBool());
        const QDir evidence(qEnvironmentVariable("CHIAKI_EMBEDDED_EVIDENCE", "."));
        const auto capture = [&](const QString &name, int width, int height, qreal zoom, bool form) {
            setup.resize(width, height);
            setup.view()->setZoomFactor(zoom);
            QTRY_VERIFY(script(setup.view(), "document.documentElement.scrollWidth <= innerWidth").toBool());
            script(setup.view(), form ? "document.querySelector('form').scrollIntoView({block:'start'})"
                : "document.querySelector('.account-panel').scrollIntoView({block:'start'})");
            QTest::qWait(80);
            QVERIFY(setup.view()->grab().save(evidence.filePath(name)));
        };
        capture("account-1280.png", 1280, 900, 1, false);
        capture("account-390.png", 390, 720, 1, false);
        capture("account-390-form.png", 390, 720, 1, true);
        capture("account-200-percent.png", 800, 900, 2, true);
        PsnBrowserOpener opener;
        QDesktopServices::setUrlHandler("https", &opener, "open");
        const auto resetOpener = qScopeGuard([] { QDesktopServices::unsetUrlHandler("https"); });
        script(setup.view(), "document.querySelector('[data-psn-redirect]').value='temporary-private-url';document.querySelector('[data-psn-redirect]').dispatchEvent(new Event('input',{bubbles:true}));document.querySelector('[data-psn-external]').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-redirect]').value").toString(), QString());
        script(setup.view(), "document.querySelector('[data-psn-redirect]').dispatchEvent(new KeyboardEvent('keydown',{key:'Escape',bubbles:true,cancelable:true}))");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.account-panel') === null && document.activeElement.hasAttribute('data-psn-account')").toBool());
    }

    void psnEmbeddedIsolation()
    {
        const QString profileName = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profileName).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profileName);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(true);
        settings.SetAddSteamShortcutAsk(false);
        QList<QPointer<PsnReply>> requests;
        PsnRequests transport;
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = nullptr;
            *count = 0;
            return CHIAKI_ERR_SUCCESS;
        };
        QmlNativeServices services(&settings, transport);
        services.backend()->stopAutoConnect();
        QString retainedPath;
        QByteArray retainedName;
        for (bool complete : {false, true}) {
            PsnSignInWindow signIn(services.backend());
            auto *page = signIn.view()->page();
            auto *profile = page->profile();
            QVERIFY(!page->webChannel());
            QVERIFY(!profile->isOffTheRecord());
            QCOMPARE(profile->storageName(), QStringLiteral("psn-token"));
            QVERIFY(profile->persistentCookiesPolicy() != QWebEngineProfile::NoPersistentCookies);
            QCOMPARE(script(signIn.view(), "typeof window.qt").toString(), QStringLiteral("undefined"));
            QSignalSpy added(profile->cookieStore(), &QWebEngineCookieStore::cookieAdded);
            QSignalSpy removed(profile->cookieStore(), &QWebEngineCookieStore::cookieRemoved);
            QSignalSpy cacheCleared(profile, &QWebEngineProfile::clearHttpCacheCompleted);
            const auto cookieName = QUuid::createUuid().toByteArray(QUuid::WithoutBraces);
            const auto hasCookie = [&] {
                for (const auto &arguments : added)
                    if (qvariant_cast<QNetworkCookie>(arguments.at(0)).name() == cookieName) return true;
                return false;
            };
            QNetworkCookie cookie(cookieName, "private-fixture");
            cookie.setSecure(true);
            cookie.setExpirationDate(QDateTime::currentDateTime().addDays(1));
            profile->cookieStore()->setCookie(cookie, QUrl("https://fixture.invalid"));
            QTRY_VERIFY(hasCookie());
            if (complete) {
                retainedPath = profile->persistentStoragePath();
                retainedName = cookieName;
                qInfo() << "Owned PSN cookie profile" << profile->persistentStoragePath()
                    << "fixture name" << cookieName << "before success" << cookieMetadata(retainedPath, cookieName);
                auto *http = new PsnPageRequests(profile);
                profile->setUrlRequestInterceptor(http);
                QSignalSpy loadFinished(page, &QWebEnginePage::loadFinished);
                page->setUrl(QUrl("https://remoteplay.dl.playstation.net/remoteplay/redirect?code=embedded-fixture"));
                QTRY_COMPARE(requests.size(), 1);
                QTRY_VERIFY(loadFinished.count() > 0);
                QVERIFY(!loadFinished.last().at(0).toBool());
                QCOMPARE(http->count.loadRelaxed(), 0);
                requests[0]->finish(R"({"access_token":"embedded-access","refresh_token":"embedded-refresh","expires_in":3600})");
                QTRY_COMPARE(requests.size(), 2);
                requests[1]->finish(R"({"user_id":"1"})");
                QTRY_COMPARE(settings.GetPsnAccountId(), QStringLiteral("AQAAAAAAAAA="));
                QTRY_VERIFY(cacheCleared.count() > 0);
                QCOMPARE(removed.count(), 0);
            } else {
                signIn.close();
                QTRY_VERIFY(cacheCleared.count() > 0);
                QTRY_VERIFY(removed.count() > 0);
            }
        }
        // The profile flushes persistent cookies on destruction; post-cache replay emits no cookies here.
        QTRY_COMPARE(cookieMetadata(retainedPath, retainedName).value("count").toInt(), 1);
        const auto retained = cookieMetadata(retainedPath, retainedName);
        QCOMPARE(retained.value("persistent").toInt(), 1);
        QVERIFY(retained.value("future").toBool());
        qInfo() << "Owned PSN cookie after success and profile destruction" << retained;
    }

    void psnRefreshCredentials()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetPsnAuthToken("saved-access");
        settings.SetPsnRefreshToken("saved-refresh");
        settings.SetPsnAccountId("AQAAAAAAAAA=");
        settings.SetPsnAuthTokenExpiry(QDateTime::currentDateTime().addSecs(-60).toString(settings.GetTimeFormat()));
        QList<QPointer<PsnReply>> requests;
        PsnRequests transport;
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = nullptr;
            *count = 0;
            return CHIAKI_ERR_SUCCESS;
        };
        QmlNativeServices services(&settings, transport);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        SetupWindow setup(backend);
        setup.show();
        QTRY_COMPARE(requests.size(), 1);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-psn-account]') !== null").toBool());
        script(setup.view(), "document.querySelector('[data-psn-account]').click()");
        requests[0]->finish(R"({"access_token":"invalid-access","expires_in":-1})");
        QTest::qWait(30);
        QCOMPARE(settings.GetPsnAuthToken(), QStringLiteral("saved-access"));
        QCOMPARE(settings.GetPsnRefreshToken(), QStringLiteral("saved-refresh"));
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-status]').textContent").toString(), QStringLiteral("PSN credentials expired"));
        backend->refreshPsnToken();
        QTRY_COMPARE(requests.size(), 2);
        requests[1]->finish({}, QNetworkReply::ProtocolInvalidOperationError);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-status]').textContent").toString(), QStringLiteral("PSN credentials expired"));
        QVERIFY(!QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson().contains("credential-bearing-network-error"));
        backend->refreshPsnToken();
        QTRY_COMPARE(requests.size(), 3);
        requests[2]->finish(R"({"access_token":"renewed-access","refresh_token":"renewed-refresh","expires_in":3600})");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-status]').textContent").toString(), QStringLiteral("PSN remote access is ready"));
        QCOMPARE(settings.GetPsnAuthToken(), QStringLiteral("renewed-access"));
        QCOMPARE(settings.GetPsnRefreshToken(), QStringLiteral("renewed-refresh"));
    }

    void psnSignIn()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            for (const auto &name : {profile, profile + "-next"})
                QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + name).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(true);
        settings.SetAddSteamShortcutAsk(false);
        QList<QPointer<PsnReply>> requests;
        PsnRequests transport;
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = nullptr;
            *count = 0;
            return CHIAKI_ERR_SUCCESS;
        };
        QmlNativeServices services(&settings, transport);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        SetupWindow setup(backend);
        setup.show();
        QTRY_COMPARE(script(setup.view(), "document.querySelector('h1')?.textContent").toString(), QStringLiteral("Consoles"));
        script(setup.view(), "document.querySelector('[data-psn-account]')?.click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-psn-redirect]') !== null").toBool());
        PsnBrowserOpener opener;
        QDesktopServices::setUrlHandler("https", &opener, "open");
        const auto resetOpener = qScopeGuard([] { QDesktopServices::unsetUrlHandler("https"); });
        script(setup.view(), "document.querySelector('[data-psn-external]').click()");
        QTRY_COMPARE(opener.url.host(), QStringLiteral("auth.api.sonyentertainmentnetwork.com"));
        QCOMPARE(opener.url.scheme(), QStringLiteral("https"));
        QCOMPARE(opener.url.path(), QStringLiteral("/2.0/oauth/authorize"));
        QCOMPARE(QUrlQuery(opener.url).queryItemValue("redirect_uri"), QStringLiteral("https://remoteplay.dl.playstation.net/remoteplay/redirect"));
        QVERIFY(!QUrlQuery(opener.url).queryItemValue("duid").isEmpty());
        const auto submit = [&](const QString &url) {
            if (backend->psnAuthAttempt().value("phase") != "exchanging" && backend->psnAuthAttempt().value("phase") != "account")
                QTRY_VERIFY(!script(setup.view(), "document.querySelector('[data-psn-redirect]').disabled").toBool());
            const auto literal = QString::fromUtf8(QJsonDocument(QJsonArray{url}).toJson(QJsonDocument::Compact));
            script(setup.view(), "document.querySelector('[data-psn-redirect]').value=" + literal + "[0];document.querySelector('[data-psn-redirect]').dispatchEvent(new Event('input',{bubbles:true}));document.querySelector('[data-psn-submit]').form.requestSubmit()");
        };
        submit("https://remoteplay.dl.playstation.net/remoteplay/redirect?code=fixture-code");
        QTRY_COMPARE(requests.size(), 1);
        QCOMPARE(backend->psnAuthAttempt().value("phase").toString(), QStringLiteral("exchanging"));
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-redirect]').value").toString(), QString());
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-psn-submit]').disabled").toBool());
        requests[0]->finish(R"({"access_token":"fixture-access-secret","refresh_token":"fixture-refresh-secret","expires_in":3600})");
        QTRY_COMPARE(requests.size(), 2);
        QTRY_VERIFY(script(setup.view(), "document.body.textContent.includes('Retrieving your Account ID')").toBool());
        requests[1]->finish(R"({"user_id":"1"})");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-status]').textContent").toString(), QStringLiteral("PSN remote access is ready"));
        QCOMPARE(settings.GetPsnAccountId(), QStringLiteral("AQAAAAAAAAA="));
        QCOMPARE(settings.GetPsnAuthToken(), QStringLiteral("fixture-access-secret"));
        QCOMPARE(settings.GetPsnRefreshToken(), QStringLiteral("fixture-refresh-secret"));
        QVERIFY(!settings.GetRemotePlayAsk());
        Settings persisted(profile);
        QCOMPARE(persisted.GetPsnAccountId(), QStringLiteral("AQAAAAAAAAA="));
        const auto snapshot = QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson();
        QVERIFY(!snapshot.contains("fixture-access-secret"));
        QVERIFY(!snapshot.contains("fixture-refresh-secret"));
        QVERIFY(!snapshot.contains("fixture-code"));
        QVERIFY(!snapshot.contains("AQAAAAAAAAA="));
        QVERIFY(script(setup.view(), "localStorage.length === 0 && sessionStorage.length === 0").toBool());
        QCOMPARE(script(setup.view(), "document.querySelector('[data-psn-redirect]').type").toString(), QStringLiteral("password"));
        submit("https://remoteplay.dl.playstation.net.evil.invalid/remoteplay/redirect?code=private-code");
        QTRY_COMPARE(backend->psnAuthAttempt().value("phase").toString(), QStringLiteral("failure"));
        QCOMPARE(requests.size(), 2);
        submit("https://remoteplay.dl.playstation.net/remoteplay/redirect?code=one&code=two");
        QTRY_VERIFY(backend->psnAuthAttempt().value("inputRejected").toBool());
        QCOMPARE(requests.size(), 2);
        submit("https://remoteplay.dl.playstation.net/remoteplay/redirect?code=retry");
        QTRY_COMPARE(requests.size(), 3);
        const auto attempt = backend->psnAuthAttempt().value("id");
        submit("https://remoteplay.dl.playstation.net/remoteplay/redirect?code=duplicate");
        QTest::qWait(30);
        QCOMPARE(requests.size(), 3);
        QCOMPARE(backend->psnAuthAttempt().value("id"), attempt);
        requests[2]->finish({}, QNetworkReply::AuthenticationRequiredError);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[role=alert]')?.textContent.includes('expired')").toBool());
        QVERIFY(!QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson().contains("credential-bearing-network-error"));
        submit("https://remoteplay.dl.playstation.net/remoteplay/redirect?code=incomplete");
        QTRY_COMPARE(requests.size(), 4);
        requests[3]->finish(R"({"access_token":"discarded-access","expires_in":3600})");
        QTRY_COMPARE(backend->psnAuthAttempt().value("phase").toString(), QStringLiteral("failure"));
        QCOMPARE(settings.GetPsnAuthToken(), QStringLiteral("fixture-access-secret"));
        submit("https://remoteplay.dl.playstation.net/remoteplay/redirect?code=late");
        QTRY_COMPARE(requests.size(), 5);
        requests[4]->finish(R"({"access_token":"late-access","refresh_token":"late-refresh","expires_in":3600})");
        QTRY_COMPARE(requests.size(), 6);
        const auto oldGeneration = QString::number(backend->profileGeneration());
        backend->qmlSettings()->setCurrentProfile(profile + "-next");
        backend->setDiscoveryEnabled(false);
        if (requests[5]) requests[5]->finish(R"({"user_id":"2"})");
        QTest::qWait(30);
        QVERIFY(backend->nativeSettings()->GetPsnAuthToken().isEmpty());
        QVERIFY(backend->nativeSettings()->GetPsnAccountId().isEmpty());
        QVERIFY(!setup.bridge()->submitPsnRedirect(oldGeneration, "https://remoteplay.dl.playstation.net/remoteplay/redirect?code=old-profile").value("accepted").toBool());
        QCOMPARE(requests.size(), 6);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-redirect]').value").toString(), QString());
        script(setup.view(), "document.querySelector('[data-psn-redirect]').value='temporary-private-url';document.querySelector('[data-psn-redirect]').dispatchEvent(new Event('input',{bubbles:true}));document.querySelector('[data-psn-close]').click()");
        script(setup.view(), "document.querySelector('[data-psn-account]').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-redirect]').value").toString(), QString());
    }

    void psnAccountStatus()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        QmlNativeServices services(&settings);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        SetupWindow setup(backend);
        setup.show();
        QTRY_COMPARE(script(setup.view(), "document.querySelector('h1')?.textContent").toString(), QStringLiteral("Consoles"));
        script(setup.view(), "document.querySelector('[data-psn-account]')?.click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-status]')?.textContent").toString(), QStringLiteral("Not signed in"));
        backend->qmlSettings()->setPsnAuthToken("fixture-access-secret");
        backend->qmlSettings()->setPsnRefreshToken("fixture-refresh-secret");
        backend->qmlSettings()->setPsnAccountId("AQAAAAAAAAA=");
        backend->qmlSettings()->setPsnAuthTokenExpiry(QDateTime::currentDateTime().addSecs(-60).toString(settings.GetTimeFormat()));
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-status]')?.textContent").toString(), QStringLiteral("PSN credentials expired"));
        backend->qmlSettings()->setPsnAuthTokenExpiry(QDateTime::currentDateTime().addSecs(3600).toString(settings.GetTimeFormat()));
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-status]')?.textContent").toString(), QStringLiteral("PSN remote access is ready"));
        const auto snapshot = QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson();
        QVERIFY(!snapshot.contains("fixture-access-secret"));
        QVERIFY(!snapshot.contains("fixture-refresh-secret"));
        QVERIFY(!snapshot.contains("AQAAAAAAAAA="));
        QVERIFY(script(setup.view(), "localStorage.length === 0 && sessionStorage.length === 0").toBool());
    }

    void graphicsCoexistence()
    {
#ifdef Q_OS_WIN
        QVERIFY2(inputDesktopAvailable(), "Graphics check requires the active unlocked input desktop and an available foreground window");
#endif
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetPlaceboPreset(PlaceboPreset::Fast);
        settings.SetManualHost(ManualHost(-1, "private.example", false, {}));
        QmlNativeServices services(&settings);
        services.backend()->stopAutoConnect();
        services.backend()->setDiscoveryEnabled(false);
        SetupWindow setup(services.backend());
        setup.show();
        QTRY_COMPARE(script(setup.view(), "document.querySelector('h1')?.textContent").toString(), QStringLiteral("Consoles"));
        const QDir evidence(qEnvironmentVariable("CHIAKI_EMBEDDED_EVIDENCE", "."));
        QWebEngineView diagnostics;
        diagnostics.resize(900, 720);
        diagnostics.show();
        diagnostics.load(QUrl("chrome://gpu"));
        QString gpu;
        QTRY_VERIFY_WITH_TIMEOUT((gpu = script(&diagnostics,
            "document.body.innerText + [...document.querySelectorAll('*')].map(e=>e.shadowRoot?.textContent??'').join('\\n')")
            .toString()).contains("Graphics Feature Status"), 15000);
        QFile gpuFile(evidence.filePath("chromium-gpu.txt"));
        QVERIFY(gpuFile.open(QIODevice::WriteOnly));
        gpuFile.write(gpu.toUtf8());
        gpuFile.close();
        QVERIFY2(gpu.contains(QRegularExpression("Compositing\\s*:?\\s*Hardware accelerated")), qPrintable(gpu));
        diagnostics.close();
        auto *backend = services.backend();
        const auto globalApi = QQuickWindow::graphicsApi();
        const RenderBackend modes[] = {RenderBackend::OpenGL, RenderBackend::Vulkan, RenderBackend::Vulkan, RenderBackend::Vulkan};
        for (int pass = 0; pass < 4; ++pass) {
            const bool fallback = pass == 3;
            if (fallback) qputenv("CHIAKI_FORCE_VULKAN_FALLBACK", "1");
            const auto resetFallback = qScopeGuard([] { qunsetenv("CHIAKI_FORCE_VULKAN_FALLBACK"); });
            backend->nativeSettings()->SetRenderBackend(modes[pass]);
            backend->nativeSettings()->SetPlaceboPreset(PlaceboPreset::Fast);
            backend->nativeSettings()->SetStreamGeometry(QRect(80, 80, 960, 640));
            setup.show();
            setup.view()->setZoomFactor(1);
            {
                QmlMainWindow native(backend, services.engine());
                QCOMPARE(native.videoPreset(), QmlMainWindow::VideoPreset::Fast);
                QCOMPARE(QQuickWindow::graphicsApi(), globalApi);
                QCOMPARE(native.nativeGraphicsApi(), modes[pass] == RenderBackend::Vulkan && !fallback
                    ? QSGRendererInterface::Vulkan : QSGRendererInterface::OpenGL);
                native.show();
                QTRY_VERIFY(native.isExposed());
                setup.hide();
                native.raise();
                native.requestActivate();
                QTRY_VERIFY(native.isActive());
#ifdef Q_OS_WIN
                const auto nativeWindow = reinterpret_cast<HWND>(native.winId());
                qInfo() << "Graphics observation native HWND" << quintptr(nativeWindow)
                    << "foreground HWND" << quintptr(GetForegroundWindow())
                    << "foreground process" << windowProcess(GetForegroundWindow());
                QVERIFY2(inputDesktopAvailable(), "The active input desktop changed during the graphics check");
                QVERIFY2(!protectedWindowVisible(), "Windows Security owns a visible protected window");
                QVERIFY(SetWindowPos(nativeWindow, HWND_TOPMOST, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE));
                const auto restoreStacking = qScopeGuard([&] {
                    SetWindowPos(nativeWindow, HWND_NOTOPMOST, 0, 0, 0, 0,
                        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                });
                QTRY_VERIFY(([&] {
                    RECT client;
                    if (!IsWindowVisible(nativeWindow) || !GetClientRect(nativeWindow, &client)
                        || client.right != qRound(native.width() * native.devicePixelRatio())
                        || client.bottom != qRound(native.height() * native.devicePixelRatio())) return false;
                    for (int y : {client.bottom / 4, 3 * client.bottom / 4})
                        for (int x : {client.right / 4, 3 * client.right / 4}) {
                            POINT point{x, y};
                            if (!ClientToScreen(nativeWindow, &point)
                                || GetAncestor(WindowFromPoint(point), GA_ROOT) != nativeWindow) return false;
                        }
                    return !protectedWindowVisible();
                })());
#endif
                AVFrame *frame = av_frame_alloc();
                QVERIFY(frame);
                frame->format = AV_PIX_FMT_YUV420P;
                frame->width = 320;
                frame->height = 180;
                frame->colorspace = AVCOL_SPC_SMPTE170M;
                frame->color_range = AVCOL_RANGE_MPEG;
                QVERIFY(av_frame_get_buffer(frame, 32) >= 0);
                for (int plane = 0; plane < 3; ++plane)
                    for (int y = 0; y < (plane ? 90 : 180); ++y)
                        memset(frame->data[plane] + y * frame->linesize[plane], plane == 0 ? 81 : plane == 1 ? 90 : 240, plane ? 160 : 320);
                native.presentFrame({frame, 0.0, 1.0 / 60.0, false}, 0);
                QTRY_VERIFY(native.hasVideo());
                emit backend->error("GPU coexistence", "Native overlay rendered");
                QImage rendered;
                int bluePixels = 0, redPixels = 0;
                QElapsedTimer captureTimer;
                captureTimer.start();
                qint64 lastCaptureMs = 0;
                const auto preserveCapture = qScopeGuard([&] {
                    rendered.save(evidence.filePath(QString("native-overlay-%1.png").arg(pass)));
                    qInfo() << "Native rendered pass" << pass << "blue overlay pixels" << bluePixels
                        << "native frame pixels" << redPixels << "last capture elapsed ms" << lastCaptureMs;
                });
                QTRY_VERIFY_WITH_TIMEOUT(([&] {
                    const auto position = native.mapToGlobal(QPoint(0, 0));
                    rendered = native.screen()->grabWindow(0, position.x(), position.y(), native.width(), native.height()).toImage();
                    bluePixels = redPixels = 0;
                    for (int y = 0; y < rendered.height(); y += 2)
                        for (int x = 0; x < rendered.width(); x += 2) {
                            const auto color = rendered.pixelColor(x, y);
                            if (color.blue() > color.red() + 40 && color.green() > color.red() + 40) ++bluePixels;
                            if (color.red() > color.green() + 80 && color.red() > color.blue() + 80) ++redPixels;
                        }
                    lastCaptureMs = captureTimer.elapsed();
                    return bluePixels > 500 && redPixels > 5000;
                })(), 15000);
                QVERIFY(rendered.save(evidence.filePath(QString("native-overlay-%1.png").arg(pass))));
                native.resize(900, 600);
                QTRY_COMPARE(native.size(), QSize(900, 600));
                native.hide();
            }
            QVERIFY(!backend->qmlWindow());
            setup.show();
            setup.raise();
            setup.activateWindow();
            script(setup.view(), "document.querySelector('input').value='';document.querySelector('input').dispatchEvent(new Event('input',{bubbles:true}))");
            discover(backend, {});
            QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 1);
            script(setup.view(), "document.querySelector('.discovery').click()");
            QTRY_VERIFY(backend->discoveryEnabled());
            QTRY_VERIFY(!script(setup.view(), "document.querySelector('.discovery').disabled").toBool());
            script(setup.view(), "document.querySelector('.discovery').click()");
            QTRY_VERIFY(!backend->discoveryEnabled());
            QVERIFY(setup.view()->grab().save(evidence.filePath(QString("setup-restored-%1.png").arg(pass))));
        }
    }

    void videoAudioSettings()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto cleanup = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile + "-next").clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetAudioOutDevice("Missing output <device>");
        settings.SetAudioInDevice("Missing input <device>");
        QmlNativeServices services(&settings);
        auto *backend = services.backend();
        auto *native = backend->qmlSettings();
        backend->stopAutoConnect();
        backend->setDiscoveryEnabled(false);
        SetupWindow setup(backend);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "[...document.querySelectorAll('.category-nav button')].some(b=>b.textContent==='Stream')").toBool());
        const auto category = [&](const QString &name) {
            script(setup.view(), QString("[...document.querySelectorAll('.category-nav button')].find(b=>b.textContent==='%1').click()").arg(name));
        };
        const auto change = [&](const QString &key, const QString &value) {
            QTRY_VERIFY(script(setup.view(), QString("document.getElementById('%1')?.disabled === false").arg(key)).toBool());
            script(setup.view(), QString("{const field=document.getElementById('%1');field.value='%2';field.dispatchEvent(new Event('change',{bubbles:true}));}").arg(key, value));
            QTRY_VERIFY(script(setup.view(), QString("document.getElementById('%1')?.disabled === false").arg(key)).toBool());
        };
        category("Stream");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#resolutionLocalPS5')?.options.length").toInt(), 4);
        QVERIFY(script(setup.view(), "!document.querySelector('#codecLocalPS4') && !document.querySelector('#codecRemotePS4')").toBool());
        change("bitrateLocalPS4", "20");
        QTRY_COMPARE(settings.GetBitrateLocalPS4(), 20000u);
        change("resolutionLocalPS4", "1");
        QTRY_COMPARE(settings.GetResolutionLocalPS4(), CHIAKI_VIDEO_RESOLUTION_PRESET_360p);
        QCOMPARE(settings.GetBitrateLocalPS4(), 0u);
        change("bitrateLocalPS4", "2");
        change("resolutionRemotePS4", "2");
        change("bitrateRemotePS4", "6");
        change("resolutionLocalPS5", "3");
        change("bitrateLocalPS5", "10");
        change("resolutionRemotePS5", "4");
        change("bitrateRemotePS5", "15");
        for (const auto &suffix : {"LocalPS4", "RemotePS4", "LocalPS5", "RemotePS5"}) change(QString("fps") + suffix, "30");
        change("codecLocalPS5", "0");
        change("codecRemotePS5", "2");
        QCOMPARE(settings.GetResolutionRemotePS4(), CHIAKI_VIDEO_RESOLUTION_PRESET_540p);
        QCOMPARE(settings.GetResolutionLocalPS5(), CHIAKI_VIDEO_RESOLUTION_PRESET_720p);
        QCOMPARE(settings.GetResolutionRemotePS5(), CHIAKI_VIDEO_RESOLUTION_PRESET_1080p);
        QCOMPARE(settings.GetBitrateLocalPS4(), 2000u);
        QCOMPARE(settings.GetBitrateRemotePS4(), 6000u);
        QCOMPARE(settings.GetBitrateLocalPS5(), 10000u);
        QCOMPARE(settings.GetBitrateRemotePS5(), 15000u);
        QCOMPARE(settings.GetFPSLocalPS4(), CHIAKI_VIDEO_FPS_PRESET_30);
        QCOMPARE(settings.GetFPSRemotePS4(), CHIAKI_VIDEO_FPS_PRESET_30);
        QCOMPARE(settings.GetFPSLocalPS5(), CHIAKI_VIDEO_FPS_PRESET_30);
        QCOMPARE(settings.GetFPSRemotePS5(), CHIAKI_VIDEO_FPS_PRESET_30);
        QCOMPARE(settings.GetCodecLocalPS5(), CHIAKI_CODEC_H264);
        QCOMPARE(settings.GetCodecRemotePS5(), CHIAKI_CODEC_H265_HDR);

        category("Video");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#decoder')").toBool());
        change("decoder", "none");
        QTRY_VERIFY(settings.GetHardwareDecoder().isEmpty());
        change("windowType", "1");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#customResolutionWidth')").toBool());
        change("customResolutionWidth", "2560");
        change("customResolutionHeight", "1440");
        script(setup.view(), "document.querySelector('#useZeroCopy').click()");
        QTRY_VERIFY(!settings.GetUseZeroCopy());
        script(setup.view(), "document.querySelector('#fullscreenDoubleClick').click()");
        QTRY_VERIFY(settings.GetFullscreenDoubleClickEnabled());
        script(setup.view(), "document.querySelector('#hideCursor').click()");
        QTRY_VERIFY(!settings.GetHideCursor());
        QCOMPARE(settings.GetCustomResolutionWidth(), 2560u);
        QCOMPARE(settings.GetCustomResolutionHeight(), 1440u);

        QSignalSpy refreshed(native, &QmlSettings::audioDevicesChanged);
        category("Audio/Wifi");
        QTRY_VERIFY(refreshed.count() >= 1);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#audioOutDevice')?.value").toString(), QStringLiteral("Missing output <device>"));
        QVERIFY(script(setup.view(), "document.querySelector('#audioOutDevice').selectedOptions[0].textContent.includes('(unavailable)') && !document.querySelector('#audioOutDevice device')").toBool());
        const int before = refreshed.count();
        script(setup.view(), "[...document.querySelectorAll('button')].find(b=>b.textContent==='Refresh audio devices').click()");
        QTRY_VERIFY(refreshed.count() > before);
        QCOMPARE(script(setup.view(), "document.querySelector('#audioOutDevice').options.length").toInt(), native->availableAudioOutDevices().size() + 2);
        QCOMPARE(script(setup.view(), "document.querySelector('#audioInDevice').options.length").toInt(), native->availableAudioInDevices().size() + 2);
        change("audioOutDevice", "");
        change("audioInDevice", "");
        QTRY_VERIFY(settings.GetAudioOutDevice().isEmpty());
        QTRY_VERIFY(settings.GetAudioInDevice().isEmpty());
        change("audioBufferSize", "70");
        change("audioVolume", "64");
        script(setup.view(), "document.querySelector('#startMicUnmuted').click()");
        QTRY_VERIFY(settings.GetStartMicUnmuted());
#if CHIAKI_GUI_ENABLE_SPEEX
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#speechProcessing')").toBool());
        QVERIFY(script(setup.view(), "!document.querySelector('#noiseSuppressLevel')").toBool());
        script(setup.view(), "document.querySelector('#speechProcessing').click()");
        QTRY_VERIFY(settings.GetSpeechProcessingEnabled());
        change("noiseSuppressLevel", "12");
        change("echoSuppressLevel", "40");
        QCOMPARE(settings.GetNoiseSuppressLevel(), 12);
        QCOMPARE(settings.GetEchoSuppressLevel(), 40);
#else
        QVERIFY(script(setup.view(), "!document.querySelector('#speechProcessing')").toBool());
#endif
        change("wifiDroppedNotif", "7");
        change("packetLossReportedMax", "9");
        script(setup.view(), "document.querySelector('#iDROnFECFailureEnabled').click()");
        QTRY_VERIFY(settings.GetIDROnFECFailureEnabled());
        script(setup.view(), "document.querySelector('#showStreamStats').click()");
        QTRY_VERIFY(settings.GetShowStreamStats());
        QCOMPARE(settings.GetAudioBufferSize(), 13440u);
        QCOMPARE(settings.GetAudioVolume(), 64);
        QCOMPARE(settings.GetWifiDroppedNotif(), 7u);
        QVERIFY(qAbs(settings.GetPacketLossReportedMax() - 0.09f) < 0.0001f);
        const QString generation = setup.bridge()->snapshot().value("profileGeneration").toString();
        for (const auto &input : QList<QPair<QString, QVariant>>{{"resolutionLocalPS4", 0}, {"resolutionRemotePS5", 5}, {"fpsLocalPS4", 45},
                 {"bitrateRemotePS4", 1000}, {"bitrateLocalPS5", 100001}, {"codecLocalPS4", 0}, {"codecRemotePS5", 3},
                 {"audioVolume", 129}, {"audioBufferSize", 9601}, {"customResolutionWidth", -1}, {"customResolutionHeight", 10000},
                 {"useZeroCopy", 1}, {"audioVolume", QStringLiteral("64")}, {"audioVolume", 1.5},
                 {"audioOutDevice", QStringLiteral("Unknown")}, {"decoder", QStringLiteral("unsupported")}, {"psnAuthToken", QStringLiteral("forbidden")}}) {
            const auto reply = settingRequest(setup.view(), input.first, input.second, generation, "setMediaSetting");
            QVERIFY2(reply.contains("accepted"), "The real WebChannel media command did not return");
            QVERIFY2(!reply.value("accepted").toBool(), qPrintable(input.first));
        }
        QCOMPARE(settings.GetAudioVolume(), 64);
        QCOMPARE(settings.GetBitrateLocalPS5(), 10000u);
        QVERIFY(settings.GetPsnAuthToken().isEmpty());
        setup.view()->reload();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        category("Audio/Wifi");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#audioBufferSize')?.value").toString(), QStringLiteral("70"));
        QCOMPARE(script(setup.view(), "document.querySelector('#audioVolume').value").toString(), QStringLiteral("64"));
        Settings reopened(profile);
        QCOMPARE(reopened.GetBitrateLocalPS4(), 2000u);
        QCOMPARE(reopened.GetBitrateRemotePS4(), 6000u);
        QCOMPARE(reopened.GetBitrateLocalPS5(), 10000u);
        QCOMPARE(reopened.GetBitrateRemotePS5(), 15000u);
        QCOMPARE(reopened.GetAudioBufferSize(), 13440u);
        QCOMPARE(reopened.GetAudioVolume(), 64);
        native->setCurrentProfile(profile + "-next");
        backend->setDiscoveryEnabled(false);
        const auto stale = settingRequest(setup.view(), "audioVolume", 0, generation, "setMediaSetting");
        QVERIFY(stale.contains("accepted"));
        QVERIFY(!stale.value("accepted").toBool());
        QCOMPARE(backend->nativeSettings()->GetAudioVolume(), 128);
    }

    void videoAudioState()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto cleanup = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile + "-next").clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        QmlNativeServices services(&settings);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        backend->setDiscoveryEnabled(false);
        SetupWindow setup(backend);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#disconnectAction')").toBool());
        script(setup.view(), "[...document.querySelectorAll('.category-nav button')].find(b=>b.textContent==='Audio/Wifi').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#audioVolume')?.value").toString(), QStringLiteral("128"));
        script(setup.view(), "{const field=document.querySelector('#audioVolume');field.value='42';field.dispatchEvent(new Event('input',{bubbles:true}));}");
        backend->qmlSettings()->setCurrentProfile(profile + "-next");
        backend->setDiscoveryEnabled(false);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#audioVolume')?.value").toString(), QStringLiteral("128"));
        QCOMPARE(backend->nativeSettings()->GetAudioVolume(), 128);
        script(setup.view(), "document.querySelector('#audioVolume').dispatchEvent(new Event('change',{bubbles:true}))");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#audioVolume')?.disabled").toBool(), false);
        QCOMPARE(backend->nativeSettings()->GetAudioVolume(), 128);
        backend->qmlSettings()->setAudioVolume(64);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#audioVolume')?.value").toString(), QStringLiteral("64"));
        QCOMPARE(script(setup.view(), "document.querySelector('output[for=audioVolume]').textContent").toString(), QStringLiteral("50% volume"));
        script(setup.view(), "{const field=document.querySelector('#audioVolume');field.value='129';field.dispatchEvent(new Event('input',{bubbles:true}));field.dispatchEvent(new Event('change',{bubbles:true}));}");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.settings-message').textContent").toString(), QStringLiteral("Choose a valid setting value."));
        QCOMPARE(script(setup.view(), "document.querySelector('#audioVolume').value").toString(), QStringLiteral("64"));
        QCOMPARE(backend->nativeSettings()->GetAudioVolume(), 64);
        qInfo() << "Native audio inventory" << backend->qmlSettings()->availableAudioOutDevices().size()
                << backend->qmlSettings()->availableAudioInDevices().size();
        if (!backend->qmlSettings()->availableAudioOutDevices().isEmpty()) {
            const QString name = backend->qmlSettings()->availableAudioOutDevices().first();
            const auto encoded = QString::fromUtf8(QJsonDocument(QJsonArray{name}).toJson(QJsonDocument::Compact));
            script(setup.view(), QString("{const field=document.querySelector('#audioOutDevice');field.value=%1[0];field.dispatchEvent(new Event('change',{bubbles:true}));}").arg(encoded));
            QTRY_COMPARE(backend->nativeSettings()->GetAudioOutDevice(), name);
        }
        if (!backend->qmlSettings()->availableAudioInDevices().isEmpty()) {
            const QString name = backend->qmlSettings()->availableAudioInDevices().first();
            const auto encoded = QString::fromUtf8(QJsonDocument(QJsonArray{name}).toJson(QJsonDocument::Compact));
            QTRY_VERIFY(script(setup.view(), "document.querySelector('#audioInDevice').disabled === false").toBool());
            script(setup.view(), QString("{const field=document.querySelector('#audioInDevice');field.value=%1[0];field.dispatchEvent(new Event('change',{bubbles:true}));}").arg(encoded));
            QTRY_COMPARE(backend->nativeSettings()->GetAudioInDevice(), name);
        }
        const auto search = [&](const QString &term) {
            script(setup.view(), QString("{const field=document.querySelector('.settings-search input');field.value='%1';field.dispatchEvent(new Event('input',{bubbles:true}));}").arg(term));
            QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.settings-results button').length").toInt(), 1);
            script(setup.view(), "document.querySelector('.settings-results button').click()");
        };
        search("PS5 Remote Codec");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("codecRemotePS5"));
        search("Custom resolution width");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("windowType"));
#if CHIAKI_GUI_ENABLE_SPEEX
        backend->qmlSettings()->setSpeechProcessing(false);
        search("Noise to suppress");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("speechProcessing"));
#endif
        script(setup.view(), "[...document.querySelectorAll('.category-nav button')].find(b=>b.textContent==='Stream').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#bitrateLocalPS5')").toBool());
        QCOMPARE(script(setup.view(), "document.querySelector('#bitrateLocalPS5').min").toString(), QStringLiteral("2"));
        QCOMPARE(script(setup.view(), "document.querySelector('#bitrateLocalPS5').value").toString(), QStringLiteral("15"));
        script(setup.view(), "{const field=document.querySelector('#bitrateLocalPS5');field.value='25';field.dispatchEvent(new Event('change',{bubbles:true}));}");
        QTRY_COMPARE(backend->nativeSettings()->GetBitrateLocalPS5(), 25000u);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#bitrateLocalPS5').disabled === false").toBool());
        script(setup.view(), "document.querySelector('#bitrateLocalPS5').parentElement.querySelector('button').click()");
        QTRY_COMPARE(backend->nativeSettings()->GetBitrateLocalPS5(), 0u);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#bitrateLocalPS5').value").toString(), QStringLiteral("15"));
        setup.resize(800, 600);
        setup.view()->setZoomFactor(2);
        QTRY_VERIFY(script(setup.view(), "document.documentElement.scrollWidth <= innerWidth").toBool());
        setup.resize(390, 720);
        setup.view()->setZoomFactor(1);
        QTRY_VERIFY(script(setup.view(), "document.documentElement.scrollWidth <= innerWidth").toBool());
        setup.resize(1280, 900);
        setup.view()->grab().save(qEnvironmentVariable("CHIAKI_MEDIA_PREVIEW", "media-settings.png"));
    }

    void videoAudioRestartRead()
    {
        const auto profile = qEnvironmentVariable("CHIAKI_MEDIA_RESTART_PROFILE");
        if (profile.isEmpty()) QSKIP("Launched only by videoAudioRestart with an isolated saved profile");
        Settings settings(profile);
        QCOMPARE(settings.GetBitrateLocalPS4(), 2000u);
        QCOMPARE(settings.GetBitrateRemotePS4(), 6000u);
        QCOMPARE(settings.GetBitrateLocalPS5(), 10000u);
        QCOMPARE(settings.GetBitrateRemotePS5(), 15000u);
        QCOMPARE(settings.GetAudioBufferSize(), 13440u);
        QCOMPARE(settings.GetAudioVolume(), 64);
        QmlNativeServices services(&settings);
        services.backend()->stopAutoConnect();
        services.backend()->setDiscoveryEnabled(false);
        SetupWindow setup(services.backend());
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#disconnectAction')").toBool());
        script(setup.view(), "[...document.querySelectorAll('.category-nav button')].find(b=>b.textContent==='Audio/Wifi').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#audioVolume')?.value").toString(), QStringLiteral("64"));
        QCOMPARE(script(setup.view(), "document.querySelector('#audioBufferSize').value").toString(), QStringLiteral("70"));
    }

    void videoAudioRestart()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto cleanup = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        {
            Settings settings(profile);
            settings.SetDiscoveryEnabled(false);
            QmlNativeServices services(&settings);
            services.backend()->stopAutoConnect();
            services.backend()->setDiscoveryEnabled(false);
            SetupWindow setup(services.backend());
            setup.show();
            QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
            script(setup.view(), "document.querySelector('.open-settings').click()");
            QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#disconnectAction')").toBool());
            script(setup.view(), "[...document.querySelectorAll('.category-nav button')].find(b=>b.textContent==='Stream').click()");
            const auto change = [&](const QString &key, const QString &value) {
                QTRY_VERIFY(script(setup.view(), QString("document.getElementById('%1')?.disabled === false").arg(key)).toBool());
                script(setup.view(), QString("{const field=document.getElementById('%1');field.value='%2';field.dispatchEvent(new Event('change',{bubbles:true}));}").arg(key, value));
                QTRY_VERIFY(script(setup.view(), QString("document.getElementById('%1')?.disabled === false").arg(key)).toBool());
            };
            change("bitrateLocalPS4", "2");
            change("bitrateRemotePS4", "6");
            change("bitrateLocalPS5", "10");
            change("bitrateRemotePS5", "15");
            script(setup.view(), "[...document.querySelectorAll('.category-nav button')].find(b=>b.textContent==='Audio/Wifi').click()");
            change("audioBufferSize", "70");
            change("audioVolume", "64");
            QTRY_COMPARE(settings.GetAudioVolume(), 64);
        }
        QProcess restarted;
        auto environment = QProcessEnvironment::systemEnvironment();
        environment.insert("CHIAKI_MEDIA_RESTART_PROFILE", profile);
        restarted.setProcessEnvironment(environment);
        const auto childLog = qEnvironmentVariable("CHIAKI_MEDIA_RESTART_LOG", "media-restart-child.txt");
        restarted.start(QCoreApplication::applicationFilePath(), {"videoAudioRestartRead", "-o", childLog + ",txt"});
        QVERIFY(restarted.waitForStarted());
        QTRY_COMPARE_WITH_TIMEOUT(restarted.state(), QProcess::NotRunning, 20000);
        QCOMPARE(restarted.exitStatus(), QProcess::NormalExit);
        QCOMPARE(restarted.exitCode(), 0);
    }

    void videoAudioDevices()
    {
        // The dummy driver controls the system audio boundary, not WebChannel or QmlSettings.
        QCOMPARE(SDL_AudioInit("dummy"), 0);
        const auto audioCleanup = qScopeGuard([] { SDL_AudioQuit(); });
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto cleanup = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetAudioOutDevice("Disconnected output");
        settings.SetAudioInDevice("Disconnected input");
        QmlNativeServices services(&settings);
        services.backend()->stopAutoConnect();
        services.backend()->setDiscoveryEnabled(false);
        auto *native = services.backend()->qmlSettings();
        QSignalSpy refreshed(native, &QmlSettings::audioDevicesChanged);
        SetupWindow setup(services.backend());
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#disconnectAction')").toBool());
        script(setup.view(), "[...document.querySelectorAll('.category-nav button')].find(b=>b.textContent==='Audio/Wifi').click()");
        QTRY_VERIFY(refreshed.count() >= 1);
        QVERIFY(!native->availableAudioOutDevices().isEmpty());
        QVERIFY(!native->availableAudioInDevices().isEmpty());
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#audioOutDevice')?.value").toString(), QStringLiteral("Disconnected output"));
        for (const auto &key : {"audioOutDevice", "audioInDevice"}) {
            QTRY_VERIFY(script(setup.view(), QString("document.getElementById('%1')?.disabled === false").arg(key)).toBool());
            script(setup.view(), QString("{const field=document.getElementById('%1');field.selectedIndex=1;field.dispatchEvent(new Event('change',{bubbles:true}));}").arg(key));
            if (QString(key) == "audioOutDevice") QTRY_COMPARE(settings.GetAudioOutDevice(), native->availableAudioOutDevices().first());
            else QTRY_COMPARE(settings.GetAudioInDevice(), native->availableAudioInDevices().first());
        }
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#audioOutDevice').options.length").toInt(), native->availableAudioOutDevices().size() + 1);
        Settings reopened(profile);
        QCOMPARE(reopened.GetAudioOutDevice(), native->availableAudioOutDevices().first());
        QCOMPARE(reopened.GetAudioInDevice(), native->availableAudioInDevices().first());
        const int before = refreshed.count();
        script(setup.view(), "[...document.querySelectorAll('button')].find(b=>b.textContent==='Refresh audio devices').click()");
        QTRY_VERIFY(refreshed.count() > before);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#audioOutDevice').value").toString(), settings.GetAudioOutDevice());
    }

    void videoAudioPreview()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto cleanup = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetAudioOutDevice("Disconnected output");
        QmlNativeServices services(&settings);
        services.backend()->stopAutoConnect();
        services.backend()->setDiscoveryEnabled(false);
        SetupWindow setup(services.backend());
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#disconnectAction')").toBool());
        const QDir evidence(qEnvironmentVariable("CHIAKI_MEDIA_EVIDENCE", "."));
        const auto capture = [&](const QString &category, int width, int height, double zoom, const QString &name) {
            setup.resize(width, height);
            setup.view()->setZoomFactor(zoom);
            script(setup.view(), QString("[...document.querySelectorAll('.category-nav button')].find(b=>b.textContent==='%1').click();window.scrollTo(0,0)").arg(category));
            QTRY_COMPARE(script(setup.view(), "innerWidth").toInt(), qRound(width / zoom));
            QTRY_VERIFY(script(setup.view(), "document.documentElement.scrollWidth <= innerWidth && scrollY === 0").toBool());
            QTest::qWait(100);
            QVERIFY(setup.view()->grab().save(evidence.filePath(name)));
        };
        capture("Video", 1280, 900, 1, "video-1280.png");
        capture("Stream", 1280, 900, 1, "stream-1280.png");
        capture("Audio/Wifi", 390, 720, 1, "audio-390.png");
        capture("Audio/Wifi", 800, 600, 2, "audio-200-percent.png");
    }

    void generalSettings()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        QmlNativeServices services(&settings);
        services.backend()->stopAutoConnect();
        services.backend()->setDiscoveryEnabled(false);
        SetupWindow setup(services.backend());
        setup.show();
        QTRY_COMPARE(script(setup.view(), "document.querySelector('h1')?.textContent").toString(), QStringLiteral("Consoles"));
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#disconnectAction')?.value").toString(), QStringLiteral("2"));
        script(setup.view(), "const field=document.querySelector('#disconnectAction');field.value='1';field.dispatchEvent(new Event('change',{bubbles:true}))");
        QTRY_COMPARE(settings.GetDisconnectAction(), DisconnectAction::AlwaysSleep);
        setup.view()->reload();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#disconnectAction')?.value").toString(), QStringLiteral("1"));
        Settings restarted(profile);
        QCOMPARE(restarted.GetDisconnectAction(), DisconnectAction::AlwaysSleep);
    }

    void settingsValidation()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile + "-next").clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetManualHost(ManualHost(-1, "private.example", false, {}));
        QmlNativeServices services(&settings);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        backend->setDiscoveryEnabled(false);
        SetupWindow setup(backend);
        setup.show();
        QTRY_COMPARE(script(setup.view(), "document.querySelector('h1')?.textContent").toString(), QStringLiteral("Consoles"));
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#audioVideoDisabled')?.value").toString(), QStringLiteral("0"));
        QCOMPARE(script(setup.view(), "document.querySelector('#suspendAction').value").toString(), QStringLiteral("0"));
        QCOMPARE(script(setup.view(), "document.querySelector('#streamMenuShortcut1').options.length").toInt(), 17);
        QVERIFY(script(setup.view(), "{const field=document.querySelector('#disconnectAction');field.focus();const key=new KeyboardEvent('keydown',{key:'ArrowDown',bubbles:true,cancelable:true});field.dispatchEvent(key);document.activeElement===field && !key.defaultPrevented;}").toBool());
#ifndef CHIAKI_GUI_ENABLE_STEAMDECK_NATIVE
        QVERIFY(script(setup.view(), "!document.querySelector('#steamDeckHaptics') && !document.querySelector('#verticalDeck')").toBool());
#endif
        const auto select = [&](const QString &key, int value) {
            QTRY_VERIFY(script(setup.view(), QString("document.getElementById('%1')?.disabled === false").arg(key)).toBool());
            script(setup.view(), QString("{const field=document.getElementById('%1');field.value='%2';field.dispatchEvent(new Event('change',{bubbles:true}));}").arg(key).arg(value));
        };
        select("suspendAction", 1);
        QTRY_COMPARE(settings.GetSuspendAction(), SuspendAction::Sleep);
        select("audioVideoDisabled", 3);
        QTRY_COMPARE(int(settings.GetAudioVideoDisabled()), 3);
        select("streamMenuShortcut1", 0);
        QTRY_COMPARE(settings.GetStreamMenuShortcut1(), 0u);
        select("streamMenuShortcut2", 16);
        QTRY_COMPARE(settings.GetStreamMenuShortcut2(), 16u);
        select("streamMenuShortcut3", 15);
        QTRY_COMPARE(settings.GetStreamMenuShortcut3(), 15u);
        select("streamMenuShortcut4", 14);
        QTRY_COMPARE(settings.GetStreamMenuShortcut4(), 14u);

        script(setup.view(), "{const field=document.querySelector('#disconnectAction');field.add(new Option('Invalid','99'));field.value='99';field.dispatchEvent(new Event('change',{bubbles:true}));}");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.settings-message')?.textContent").toString(), QStringLiteral("Choose a valid setting value."));
        QCOMPARE(settings.GetDisconnectAction(), DisconnectAction::Ask);
        QCOMPARE(script(setup.view(), "document.querySelector('#disconnectAction').value").toString(), QStringLiteral("2"));
        const QString generation = setup.bridge()->snapshot().value("profileGeneration").toString();
        for (const auto &input : QList<QPair<QString, QVariant>>{{"suspendAction", 2}, {"audioVideoDisabled", -1}, {"streamMenuShortcut1", 17},
                 {"disconnectAction", 0.5}, {"disconnectAction", QStringLiteral("1")}, {"streamerMode", 1}, {"psnAuthToken", QStringLiteral("forbidden")}}) {
            const auto reply = settingRequest(setup.view(), input.first, input.second, generation);
            QVERIFY2(reply.contains("accepted"), "The real WebChannel command did not return");
            QVERIFY(!reply.value("accepted").toBool());
        }
#ifndef CHIAKI_GUI_ENABLE_STEAMDECK_NATIVE
        const auto unsupported = settingRequest(setup.view(), "steamDeckHaptics", true, generation);
        QVERIFY(unsupported.contains("accepted"));
        QVERIFY(!unsupported.value("accepted").toBool());
#endif
        QCOMPARE(settings.GetSuspendAction(), SuspendAction::Sleep);
        QCOMPARE(int(settings.GetAudioVideoDisabled()), 3);
        QCOMPARE(settings.GetStreamMenuShortcut1(), 0u);
        QVERIFY(!settings.GetStreamerMode());
        QVERIFY(settings.GetPsnAuthToken().isEmpty());

        script(setup.view(), "document.querySelector('#streamMenuEnabled').click()");
        QTRY_VERIFY(!settings.GetStreamMenuEnabled());
        QTRY_VERIFY(script(setup.view(), "!document.querySelector('#streamMenuShortcut1')").toBool());
        script(setup.view(), "{const field=document.querySelector('.settings-search input');field.value='shortcut button 2';field.dispatchEvent(new Event('input',{bubbles:true}));}");
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.settings-results button').length").toInt(), 1);
        script(setup.view(), "document.querySelector('.settings-results button').click()");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("streamMenuEnabled"));
        script(setup.view(), "document.querySelector('#streamMenuEnabled').click()");
        QTRY_VERIFY(settings.GetStreamMenuEnabled());
        script(setup.view(), "{const field=document.querySelector('.settings-search input');field.value='suspend';field.dispatchEvent(new Event('input',{bubbles:true}));}");
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.settings-results button').length").toInt(), 1);
        script(setup.view(), "document.querySelector('.settings-results button').click()");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("suspendAction"));
        script(setup.view(), "{const field=document.querySelector('.settings-search input');field.value='private.example';field.dispatchEvent(new Event('input',{bubbles:true}));}");
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.settings-results button').length").toInt(), 0);
        script(setup.view(), "{const field=document.querySelector('.settings-search input');field.value='';field.dispatchEvent(new Event('input',{bubbles:true}));}");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#streamerMode')").toBool());
        script(setup.view(), "document.querySelector('#streamerMode').click()");
        QTRY_VERIFY(settings.GetStreamerMode());
        QVERIFY(!QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson().contains("private.example"));
        script(setup.view(), "document.querySelector('.back-consoles').click()");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.className").toString(), QStringLiteral("open-settings"));
        QTRY_VERIFY(script(setup.view(), "document.body.textContent.includes('Manual console') && !document.body.textContent.includes('private.example')").toBool());
        backend->qmlSettings()->setCurrentProfile(profile + "-next");
        backend->setDiscoveryEnabled(false);
        const auto stale = settingRequest(setup.view(), "disconnectAction", 1, generation);
        QVERIFY(stale.contains("accepted"));
        QVERIFY(!stale.value("accepted").toBool());
        QCOMPARE(backend->nativeSettings()->GetDisconnectAction(), DisconnectAction::Ask);
        QVERIFY(!backend->nativeSettings()->GetStreamerMode());
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#audioVideoDisabled')?.value").toString(), QStringLiteral("0"));
    }

    void settingsRestartRead()
    {
        const auto profile = qEnvironmentVariable("CHIAKI_SETTINGS_RESTART_PROFILE");
        if (profile.isEmpty()) QSKIP("Launched only by settingsRestart with an isolated saved profile");
        Settings settings(profile);
        QCOMPARE(settings.GetDisconnectAction(), DisconnectAction::AlwaysSleep);
        QmlNativeServices services(&settings);
        services.backend()->stopAutoConnect();
        services.backend()->setDiscoveryEnabled(false);
        SetupWindow setup(services.backend());
        setup.show();
        QTRY_COMPARE(script(setup.view(), "document.querySelector('h1')?.textContent").toString(), QStringLiteral("Consoles"));
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#disconnectAction')?.value").toString(), QStringLiteral("1"));
    }

    void settingsRestart()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        {
            Settings settings(profile);
            settings.SetDiscoveryEnabled(false);
            QmlNativeServices services(&settings);
            services.backend()->stopAutoConnect();
            services.backend()->setDiscoveryEnabled(false);
            SetupWindow setup(services.backend());
            setup.show();
            QTRY_COMPARE(script(setup.view(), "document.querySelector('h1')?.textContent").toString(), QStringLiteral("Consoles"));
            QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
            QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#disconnectAction')").toBool());
            script(setup.view(), "{const field=document.querySelector('#disconnectAction');field.value='1';field.dispatchEvent(new Event('change',{bubbles:true}));}");
            QTRY_COMPARE(settings.GetDisconnectAction(), DisconnectAction::AlwaysSleep);
        }
        QProcess restarted;
        auto environment = QProcessEnvironment::systemEnvironment();
        environment.insert("CHIAKI_SETTINGS_RESTART_PROFILE", profile);
        restarted.setProcessEnvironment(environment);
        restarted.start(QCoreApplication::applicationFilePath(), {"settingsRestartRead"});
        QVERIFY(restarted.waitForStarted());
        QTRY_COMPARE_WITH_TIMEOUT(restarted.state(), QProcess::NotRunning, 20000);
        qInfo().noquote() << restarted.readAllStandardOutput();
        qInfo().noquote() << restarted.readAllStandardError();
        QCOMPARE(restarted.exitStatus(), QProcess::NormalExit);
        QCOMPARE(restarted.exitCode(), 0);
    }

    void consoleLibrary()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile + "-next").clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        QmlNativeServices services(&settings);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        backend->setDiscoveryEnabled(false);
        QGuiApplication::setApplicationDisplayName("Chiaki Recast: Private profile");
        SetupWindow setup(services.backend());
        QSignalSpy loaded(setup.view(), &QWebEngineView::loadFinished);
        setup.show();
        QTRY_VERIFY(setup.view()->isVisible());
        QTRY_VERIFY(!loaded.isEmpty());
        QCOMPARE(loaded.last().first().toBool(), true);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('h1')?.textContent").toString(), QStringLiteral("Consoles"));
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.empty h2')?.textContent").toString(), QStringLiteral("No consoles found"));
        setup.view()->setFocus();
        script(setup.view(), "document.querySelector('input').focus()");
        QTest::keyClick(setup.windowHandle(), Qt::Key_PageDown);
        QTRY_VERIFY(script(setup.view(), "document.activeElement.classList.contains('discovery')").toBool());
        DiscoveryHost first = {};
        first.ps5 = true;
        first.host_name = "Office <img src=x onerror=alert(1)>";
        first.host_id = "001122334455";
        first.host_addr = "192.0.2.10";
        first.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
        DiscoveryHost second = first;
        second.ps5 = false;
        second.host_name = "Living room";
        second.host_id = "001122334466";
        second.host_addr = "192.0.2.20";
        discover(backend, {first, second});
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 2);
        setup.resize(1280, 720);
        QTest::qWait(50);
        setup.view()->grab().save(qEnvironmentVariable("CHIAKI_EMBEDDED_PREVIEW", "embedded-library.png"));
        QCOMPARE(script(setup.view(), "document.querySelector('.console-name').textContent").toString(), first.host_name);
        QCOMPARE(script(setup.view(), "document.querySelector('.console-name img') === null").toBool(), true);
        QTRY_VERIFY(script(setup.view(), "[...document.querySelectorAll('.art img')].every(i => i.complete && i.naturalWidth > 0)").toBool());
        const QString key = script(setup.view(), "document.querySelectorAll('.console-card')[1].dataset.consoleKey").toString();
        QVERIFY(!key.isEmpty());
        script(setup.view(), "document.querySelectorAll('.console-card')[1].click()");
        QTRY_COMPARE(setup.bridge()->snapshot().value("selectedKey").toString(), key);
        discover(backend, {second, first});
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.selected .console-name')?.textContent").toString(), second.host_name);
        QCOMPARE(script(setup.view(), "document.querySelector('.selected').dataset.consoleKey").toString(), key);
        setup.view()->setFocus();
        script(setup.view(), "document.querySelector('.console-card').focus()");
        QTest::keyClick(setup.windowHandle(), Qt::Key_Right);
        QTRY_COMPARE(script(setup.view(), "document.activeElement?.dataset.consoleKey").toString(),
            script(setup.view(), "document.querySelectorAll('.console-card')[1].dataset.consoleKey").toString());
        QTest::keyClick(setup.windowHandle(), Qt::Key_PageUp);
        QTRY_COMPARE(script(setup.view(), "document.activeElement.tagName").toString(), QStringLiteral("INPUT"));
        QTest::keyClick(setup.windowHandle(), Qt::Key_PageDown);
        QTRY_COMPARE(script(setup.view(), "document.activeElement.className.includes('console-card')").toBool(), true);
        script(setup.view(), "document.querySelector('.console-card').click()");
        QTRY_COMPARE(setup.bridge()->snapshot().value("selectedKey").toString(), key);
        int row = -1;
        QString nickname;
        QVERIFY(setup.bridge()->resolveConsole(key, row, nickname));
        QCOMPARE(row, 0);
        QCOMPARE(nickname, second.host_name);
        script(setup.view(), "document.querySelector('input').value='PS4';document.querySelector('input').dispatchEvent(new Event('input',{bubbles:true}))");
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 1);
        QCOMPARE(script(setup.view(), "document.querySelector('.console-name').textContent").toString(), second.host_name);
        script(setup.view(), "document.querySelector('input').value='absent';document.querySelector('input').dispatchEvent(new Event('input',{bubbles:true}));document.querySelector('input').focus()");
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 0);
        QTest::keyClick(setup.windowHandle(), Qt::Key_Escape);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('input').value").toString(), QString());
        QTRY_COMPARE(script(setup.view(), "document.activeElement?.dataset.consoleKey").toString(), key);
        setup.view()->reload();
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 2);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.selected')?.dataset.consoleKey").toString(), key);
        script(setup.view(), "document.querySelector('.discovery').click()");
        QTRY_VERIFY(backend->discoveryEnabled());
        QTRY_VERIFY(!script(setup.view(), "document.querySelector('.discovery').disabled").toBool());
        script(setup.view(), "document.querySelector('.discovery').click()");
        QTRY_VERIFY(!backend->discoveryEnabled());
        // Repopulate at the external completion boundary after stopping the real discovery worker.
        discover(backend, {second, first});
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 2);
        const QString currentKey = script(setup.view(), "document.querySelector('.console-card').dataset.consoleKey").toString();
        discover(backend, {second, second});
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card:disabled').length").toInt(), 2);
        QVERIFY(!setup.bridge()->selectConsole(currentKey).value("accepted").toBool());
        discover(backend, {first});
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 1);
        QVERIFY(!setup.bridge()->selectConsole(currentKey).value("accepted").toBool());
        const QString oldProfileKey = script(setup.view(), "document.querySelector('.console-card').dataset.consoleKey").toString();
        backend->qmlSettings()->setCurrentProfile(profile + "-next");
        backend->setDiscoveryEnabled(false);
        discover(backend, {first});
        QTRY_VERIFY(!setup.bridge()->selectConsole(oldProfileKey).value("accepted").toBool());
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.console-card')?.dataset.consoleKey").toString() != oldProfileKey);
        backend->nativeSettings()->SetManualHost(ManualHost(-1, "private.example", false, {}));
        backend->qmlSettings()->setStreamerMode(true);
        QCOMPARE(setup.windowTitle(), QStringLiteral("Chiaki Recast"));
        QTRY_VERIFY(script(setup.view(), "document.body.textContent.includes('Manual console')").toBool());
        const auto json = QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson();
        QVERIFY(!json.contains("private.example"));
        QVERIFY(!json.contains("192.0.2.10"));
        QVERIFY(!json.contains("001122334455"));
        QVERIFY(!json.contains("psnAuthToken"));
        QVERIFY(!json.contains("consolePin"));
        QVERIFY(!script(setup.view(), "window.open('https://example.com') !== null").toBool());
        setup.view()->setUrl(QUrl("https://example.com"));
        QTest::qWait(100);
        QCOMPARE(setup.view()->url(), QUrl("qrc:/setup/index.html"));
        QCOMPARE(script(setup.view(), "document.querySelector('h1').textContent").toString(), QStringLiteral("Consoles"));
        setup.resize(800, 600);
        setup.view()->setZoomFactor(2);
        QTRY_VERIFY(script(setup.view(), "document.documentElement.scrollWidth <= innerWidth").toBool());
        setup.resize(390, 720);
        setup.view()->setZoomFactor(1);
        QTRY_VERIFY(script(setup.view(), "document.documentElement.scrollWidth <= innerWidth").toBool());
        setup.view()->grab().save(qEnvironmentVariable("CHIAKI_EMBEDDED_SCREENSHOT", "embedded-setup.png"));
    }
};
int main(int argc, char **argv)
{
    if (chiaki_lib_init() != CHIAKI_ERR_SUCCESS) return 1;
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#ifdef CHIAKI_HAVE_WEBENGINE
    QtWebEngineQuick::initialize();
#endif
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Chiaki embedded setup tests");
    QCoreApplication::setApplicationName("Chiaki embedded setup tests");
    EmbeddedSetupTest test;
    return QTest::qExec(&test, argc, argv);
}
#include "embeddedsetup.moc"
