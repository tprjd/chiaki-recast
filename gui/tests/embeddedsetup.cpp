#include "keyboardmousebindings.h"
#if CHIAKI_GUI_ENABLE_STEAM_SHORTCUT
#include "steamtools.h"
#endif
#include <QKeyEvent>
#include <QPushButton>
#include <QLabel>
#include <QDialog>
#include <QtTest>
#include <SDL.h>
#include <QApplication>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QScopeGuard>
#include <QUuid>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QFileDialog>
#include <QTemporaryDir>
#include <QClipboard>
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
#include "applicationcontroller.h"
#include <QTcpServer>
#include <atomic>
#include <QSemaphore>
#ifdef Q_OS_WIN
#include <stdlib.h>
#endif
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

struct DecodedBuffers
{
    std::atomic<int> allocated = 0;
    std::atomic<int> released = 0;
    struct Buffer { AVBufferRef *original; DecodedBuffers *counts; };
    static int allocate(AVCodecContext *context, AVFrame *frame, int flags)
    {
        const int result = avcodec_default_get_buffer2(context, frame, flags);
        if (result < 0) return result;
        auto *counts = static_cast<DecodedBuffers *>(context->opaque);
        for (auto &buffer : frame->buf) {
            if (!buffer) continue;
            auto *owner = new Buffer{buffer, counts};
            auto *tracked = av_buffer_create(buffer->data, buffer->size, [](void *opaque, uint8_t *) {
                auto *owner = static_cast<Buffer *>(opaque);
                av_buffer_unref(&owner->original);
                ++owner->counts->released;
                delete owner;
            }, owner, 0);
            if (!tracked) { delete owner; return AVERROR(ENOMEM); }
            buffer = tracked;
            ++counts->allocated;
        }
        return result;
    }
};

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
    void finish(QByteArray json, NetworkError error = NoError, int status = 200)
    {
        if (isFinished()) return;
        payload = std::move(json);
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, status);
        if (error != NoError) setError(error, "credential-bearing-network-error");
        setFinished(true);
        emit readyRead();
        emit finished();
    }
    bool deferAbort = false;
    void abort() override { if (!deferAbort) finish({}, OperationCanceledError); }
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

struct RegistrationTransport
{
    struct Request {
        QPointer<QObject> owner;
        ChiakiRegistCb callback;
        void *user;
        ChiakiLog *log;
        QString address;
        QString onlineId;
        ChiakiTarget target;
        bool broadcast;
        uint32_t pin;
        uint32_t consolePin;
    };
    QList<Request> requests;
    ChiakiErrorCode startResult = CHIAKI_ERR_SUCCESS;
    RegistrationRequests boundary()
    {
        RegistrationRequests transport;
        transport.start = [this](ChiakiRegist *, ChiakiLog *log, const ChiakiRegistInfo *info, ChiakiRegistCb callback, void *user) {
            requests.append({static_cast<QmlRegist *>(user), callback, user, log, QString::fromUtf8(info->host),
                QString::fromUtf8(info->psn_online_id), info->target, info->broadcast, info->pin, info->console_pin});
            return startResult;
        };
        transport.stop = [](ChiakiRegist *) {};
        transport.fini = [](ChiakiRegist *) {};
        return transport;
    }
    PsnRequests guardedPsn(PsnRequests requests = {})
    {
        requests.startSession = [](ChiakiSession *) {
            QTest::qFail("Unexpected native session start in a pairing fixture", __FILE__, __LINE__);
            return CHIAKI_ERR_CANCELED;
        };
        return requests;
    }
    void complete(int index, bool success, uint8_t lastMacByte = 0x55)
    {
        const auto request = requests.at(index);
        if (!request.owner) return;
        ChiakiRegisteredHost host = {};
        host.target = request.target;
        memcpy(host.server_mac, "\x00\x11\x22\x33\x44", 5);
        host.server_mac[5] = lastMacByte;
        memcpy(host.server_nickname, "Paired console", 15);
        memcpy(host.rp_regist_key, "registration-key", 16);
        memset(host.rp_key, 0x42, sizeof(host.rp_key));
        host.console_pin = request.consolePin;
        ChiakiRegistEvent event{success ? CHIAKI_REGIST_EVENT_TYPE_FINISHED_SUCCESS : CHIAKI_REGIST_EVENT_TYPE_FINISHED_FAILED, &host};
        request.callback(&event, request.user);
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

    DiscoveryRequests controlledDiscovery()
    {
        return {[](ChiakiDiscoveryService *, ChiakiDiscoveryServiceOptions *, ChiakiLog *) { return CHIAKI_ERR_SUCCESS; },
            [](ChiakiDiscoveryService *) {}, [](ChiakiLog *, ChiakiDiscovery *, const char *, uint64_t, bool) {
                QTest::qFail("Unexpected wake packet in a controlled discovery fixture", __FILE__, __LINE__);
                return CHIAKI_ERR_CANCELED;
            }};
    }

    PsnRequests controlledPsn(QList<QPointer<PsnReply>> &requests)
    {
        PsnRequests transport;
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **, size_t *, ChiakiLog *) {
            QTest::qFail("Unexpected PSN list in a wake fixture", __FILE__, __LINE__);
            return CHIAKI_ERR_CANCELED;
        };
        transport.connect = [](StreamSession *, const QString &, bool) {
            QTest::qFail("Unexpected PSN worker in a wake fixture", __FILE__, __LINE__);
            return CHIAKI_ERR_CANCELED;
        };
        transport.startSession = [](ChiakiSession *) {
            QTest::qFail("Unexpected native start in a wake fixture", __FILE__, __LINE__);
            return CHIAKI_ERR_CANCELED;
        };
        return transport;
    }

    QVariantMap settingRequest(QWebEngineView *view, const QString &name, const QVariant &value, const QString &generation, const QString &method = "setGeneralSetting")
    {
        return setupRequest(view, method, QJsonArray{name, QJsonValue::fromVariant(value), generation});
    }

    QVariantMap setupRequest(QWebEngineView *view, const QString &method, const QJsonArray &args)
    {
        const QString arguments = QString::fromUtf8(QJsonDocument(args).toJson(QJsonDocument::Compact));
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

    bool click(QWebEngineView *view, const QString &selector)
    {
        const auto args = QString::fromUtf8(QJsonDocument(QJsonArray{selector}).toJson(QJsonDocument::Compact));
        const auto source = QString(R"((() => {
            const field = document.querySelector(%1[0]);
            if (!field || field.disabled) return false;
            field.click(); return true;
        })())").arg(args);
        QElapsedTimer timeout;
        timeout.start();
        while (timeout.elapsed() < 5000) {
            if (script(view, source).toBool()) return true;
            QTest::qWait(10);
        }
        return false;
    }

    bool fill(QWebEngineView *view, const QString &selector, const QString &value)
    {
        const auto args = QString::fromUtf8(QJsonDocument(QJsonArray{selector, value}).toJson(QJsonDocument::Compact));
        return script(view, QString(R"((() => {
            const [selector, value] = %1, field = document.querySelector(selector);
            if (!field) return false;
            field.value = value;
            field.dispatchEvent(new Event(field.tagName === 'SELECT' ? 'change' : 'input', {bubbles:true}));
            return true;
        })())").arg(args)).toBool();
    }

    QVariantMap consoleRequest(QWebEngineView *view, const QString &method, const QJsonArray &arguments)
    {
        const auto args = QString::fromUtf8(QJsonDocument(arguments).toJson(QJsonDocument::Compact));
        script(view, QString(R"(
            window.consoleReply = null;
            { const transport = window.qt.webChannelTransport, receive = transport.onmessage;
              new QWebChannel(transport, channel => channel.objects.setup.%1(...%2, reply => {
                window.consoleReply = reply; transport.onmessage = receive;
              })); }
        )").arg(method, args));
        QElapsedTimer timeout;
        timeout.start();
        while (timeout.elapsed() < 5000) {
            const auto reply = script(view, "window.consoleReply").toMap();
            if (!reply.isEmpty()) return reply;
            QTest::qWait(10);
        }
        return {};
    }

    QObject *nativeObject(const QString &name)
    {
        const auto find = [&](auto &&self, QQuickItem *item) -> QObject * {
            if (item->objectName() == name) return item;
            if (auto *object = item->findChild<QObject *>(name)) return object;
            for (auto *child : item->childItems())
                if (auto *object = self(self, child)) return object;
            return nullptr;
        };
        for (auto *window : QGuiApplication::allWindows()) {
            if (window->objectName() == name) return window;
            if (auto *quick = qobject_cast<QQuickWindow *>(window))
                if (auto *object = find(find, quick->contentItem())) return object;
        }
        return nullptr;
    }

private slots:
    void startupNativeConnection_data()
    {
        QTest::addColumn<QString>("mode");
        for (const auto &mode : {"local-ready", "local-wake", "psn-ready", "psn-late-list", "psn-ps4", "psn-ps4-ambiguous", "psn-cancel", "psn-profile", "psn-removed", "psn-replaced"})
            QTest::newRow(mode) << QString::fromLatin1(mode);
    }

    void startupNativeConnection()
    {
        QFETCH(QString, mode);
        const bool local = mode.startsWith("local");
        const bool ps5 = !mode.startsWith("psn-ps4");
        const bool success = mode == "local-ready" || mode == "local-wake" || mode == "psn-ready" || mode == "psn-late-list" || mode == "psn-ps4";
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            for (const auto &name : {profile, profile + "-next"})
                QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + name).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetHardwareDecoder({});
        settings.SetCodecLocalPS5(CHIAKI_CODEC_H264);
        settings.SetCodecRemotePS5(CHIAKI_CODEC_H264);
        settings.SetRenderBackend(RenderBackend::OpenGL);
        settings.SetPlaceboPreset(PlaceboPreset::Fast);
        settings.SetWindowType(WindowType::AdjustableResolution);
        settings.SetStreamGeometry(QRect(80, 80, 960, 640));
        settings.SetPsnAuthToken("startup-access-secret");
        settings.SetPsnRefreshToken("startup-refresh-secret");
        settings.SetPsnAccountId("AQAAAAAAAAA=");
        settings.SetPsnAuthTokenExpiry(QDateTime::currentDateTime().addSecs(3600).toString(settings.GetTimeFormat()));
        ChiakiRegisteredHost registered = {};
        registered.target = ps5 ? CHIAKI_TARGET_PS5_1 : CHIAKI_TARGET_PS4_10;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "Startup console");
        strcpy(registered.rp_regist_key, "12345678");
        RegisteredHost paired(registered);
        paired.SetConsolePin(paired, {});
        settings.AddRegisteredHost(paired);
        if (mode == "psn-ps4-ambiguous") {
            auto second = registered;
            second.server_mac[5] = 0x66;
            strcpy(second.server_nickname, "Other PS4");
            settings.AddRegisteredHost(RegisteredHost(second));
        }
        settings.SetAutoConnectHost(QByteArray::fromHex("001122334455"));
        QList<QPointer<PsnReply>> requests;
        QSemaphore release, listRelease;
        std::atomic<int> lists = 0;
        std::atomic<int> workers = 0;
        int starts = 0, wakes = 0;
        bool selectedRoute = false;
        auto transport = controlledPsn(requests);
        transport.listDevices = [&](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            if (mode == "psn-late-list") { ++lists; listRelease.acquire(); }
            *devices = static_cast<ChiakiHolepunchDeviceInfo *>(calloc(1, sizeof(ChiakiHolepunchDeviceInfo)));
            *count = 1;
            strcpy((*devices)[0].device_name, "Startup console");
            memset((*devices)[0].device_uid, 0x12, sizeof((*devices)[0].device_uid));
            (*devices)[0].remoteplay_enabled = true;
            return CHIAKI_ERR_SUCCESS;
        };
        transport.connect = [&](StreamSession *, const QString &duid, bool remotePs5) {
            if (ps5) selectedRoute = remotePs5 && duid == QString("12").repeated(32);
            else selectedRoute = !remotePs5 && duid == QString("41").repeated(32);
            ++workers;
            release.acquire();
            return CHIAKI_ERR_SUCCESS;
        };
        transport.startSession = [&](ChiakiSession *session) {
            ++starts;
            if (!success) return CHIAKI_ERR_CANCELED;
            if (local) selectedRoute = session->connect_info.host_addrinfos
                && QHostAddress(session->connect_info.host_addrinfos->ai_addr) == QHostAddress::LocalHost && session->connect_info.ps5;
            return chiaki_thread_create(&session->session_thread, [](void *user) -> void * {
                auto *session = static_cast<ChiakiSession *>(user);
                ChiakiEvent connected = {};
                connected.type = CHIAKI_EVENT_CONNECTED;
                session->event_cb(&connected, session->event_cb_user);
                chiaki_mutex_lock(&session->state_mutex);
                while (!session->should_stop) chiaki_cond_wait(&session->state_cond, &session->state_mutex);
                chiaki_mutex_unlock(&session->state_mutex);
                ChiakiEvent event = {};
                event.type = CHIAKI_EVENT_QUIT;
                event.quit.reason = CHIAKI_QUIT_REASON_STOPPED;
                session->event_cb(&event, session->event_cb_user);
                return nullptr;
            }, session);
        };
        auto discovery = controlledDiscovery();
        discovery.wakeup = [&](ChiakiLog *, ChiakiDiscovery *, const char *, uint64_t, bool) { ++wakes; return CHIAKI_ERR_SUCCESS; };
        Settings startupSettings(profile);
        QmlNativeServices services(&startupSettings, transport, {}, {}, discovery);
        auto *backend = services.backend();
        ApplicationController controller(&services);
        const auto releaseOnExit = qScopeGuard([&] { release.release(); listRelease.release(); });
        auto &setup = *controller.setupWindow();
        setup.view()->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
        setup.resize(1280, 720);
        setup.show();
        const auto attempt = backend->connectionAttempt().value("id").toString();
        DiscoveryHost host = {};
        host.ps5 = true;
        host.host_id = "001122334455";
        host.host_name = "Startup console";
        host.host_addr = "127.0.0.1";
        host.state = mode == "local-wake" ? CHIAKI_DISCOVERY_HOST_STATE_STANDBY : CHIAKI_DISCOVERY_HOST_STATE_READY;
        if (local) discover(backend, {host});
        else {
            if (mode == "psn-late-list") QTRY_COMPARE(lists.load(), 1);
            else QTRY_VERIFY(!backend->hosts().isEmpty());
            bool accelerated = false;
            for (auto *timer : backend->findChildren<QTimer *>())
                if (timer->isActive() && timer->interval() == 5000) { timer->start(1); accelerated = true; }
            QVERIFY(accelerated);
            if (mode == "psn-late-list") {
                QTest::qWait(40);
                listRelease.release();
            }
        }
        if (mode == "psn-ps4-ambiguous") {
            QTest::qWait(40);
            QCOMPARE(workers.load(), 0);
            QCOMPARE(starts, 0);
            QVERIFY(!backend->qmlSession());
            QVERIFY(!backend->qmlWindow());
            setup.view()->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
            setup.view()->reload();
            QVERIFY(click(setup.view(), "[data-stop-auto-connect]"));
            QTRY_COMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("cancelled"));
            return;
        }
        QTRY_VERIFY(backend->qmlSession());
        QTRY_VERIFY(backend->qmlWindow());
        QPointer<QmlMainWindow> native = backend->qmlWindow();
        QPointer<StreamSession> session = backend->qmlSession();
        QCOMPARE(backend->connectionAttempt().value("id").toString(), attempt);
        QVERIFY(!script(setup.view(), "!!document.querySelector('.connect-console')").toBool());
        if (mode == "local-wake") {
            QCOMPARE(starts, 0);
            QCOMPARE(wakes, 1);
            host.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
            discover(backend, {host});
        }
        if (!local) {
            QTRY_COMPARE(workers.load(), 1);
            QCOMPARE(starts, 0);
            if (mode == "psn-cancel") QVERIFY(backend->stopAutoConnect(attempt));
            else if (mode == "psn-profile") backend->qmlSettings()->setCurrentProfile(profile + "-next");
            else if (mode == "psn-removed" || mode == "psn-replaced") {
                backend->nativeSettings()->RemoveRegisteredHost(paired.GetServerMAC());
                if (mode == "psn-replaced") {
                    registered.server_mac[5] = 0x66;
                    backend->nativeSettings()->AddRegisteredHost(RegisteredHost(registered));
                }
            }
            release.release();
        }
        if (success) {
            QTRY_COMPARE(starts, 1);
            QVERIFY(selectedRoute);
            QCOMPARE(workers.load(), local ? 0 : 1);
            QTRY_VERIFY(!setup.isVisible());
            QTRY_VERIFY(backend->connectionAttempt().value("cancelable").toBool());
            QVERIFY(backend->stopAutoConnect(attempt));
        }
        QTRY_VERIFY(!session);
        native->dismissStreamView();
        QTRY_VERIFY(!native);
        QTRY_VERIFY(setup.isVisible());
        if (!success) QCOMPARE(starts, 0);
        setup.view()->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
        setup.view()->reload();
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('.connect-console')").toBool());
        QCOMPARE(starts, success ? 1 : 0);
        QVERIFY(requests.isEmpty());
    }

    void wakeFailures_data()
    {
        QTest::addColumn<QString>("mode");
        for (const auto &mode : {"transport", "timeout", "duplicate", "removed", "hidden", "profile"})
            QTest::newRow(mode) << QString::fromLatin1(mode);
    }

    void wakeFailures()
    {
        QFETCH(QString, mode);
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            for (const auto &name : {profile, profile + "-next"})
                QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + name).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        ChiakiRegisteredHost registered = {};
        registered.target = CHIAKI_TARGET_PS5_1;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "Sleeping console");
        strcpy(registered.rp_regist_key, "12345678");
        const RegisteredHost paired(registered);
        settings.AddRegisteredHost(paired);
        QList<QPointer<PsnReply>> requests;
        int wakes = 0;
        auto discovery = controlledDiscovery();
        discovery.wakeup = [&](ChiakiLog *, ChiakiDiscovery *, const char *, uint64_t, bool) {
            ++wakes;
            return mode == "transport" && wakes == 1 ? CHIAKI_ERR_NETWORK : CHIAKI_ERR_SUCCESS;
        };
        QmlNativeServices services(&settings, controlledPsn(requests), {}, {}, discovery);
        auto *backend = services.backend();
        SetupWindow setup(backend);
        setup.resize(1280, 720);
        setup.show();
        DiscoveryHost host = {};
        host.ps5 = true;
        host.host_id = "001122334455";
        host.host_name = "Sleeping console";
        host.host_addr = "192.0.2.10";
        host.state = CHIAKI_DISCOVERY_HOST_STATE_STANDBY;
        discover(backend, {host});
        QVERIFY(click(setup.view(), "[data-wake-console]"));
        QTRY_COMPARE(wakes, 1);
        if (mode == "timeout") {
            bool accelerated = false;
            for (auto *timer : backend->findChildren<QTimer *>())
                if (timer->isActive() && timer->interval() == 25000) { timer->start(1); accelerated = true; }
            QVERIFY(accelerated);
        } else if (mode == "duplicate") discover(backend, {host, host});
        else if (mode == "removed") settings.RemoveRegisteredHost(paired.GetServerMAC());
        else if (mode == "hidden") settings.AddHiddenHost(HiddenHost(paired.GetServerMAC(), "Sleeping console"));
        else if (mode == "profile") backend->qmlSettings()->setCurrentProfile(profile + "-next");
        QTRY_COMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("failure"));
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-psn-connection] h2')?.textContent.length > 0").toBool());
        if (mode == "transport" || mode == "timeout") {
            QVERIFY(click(setup.view(), "[data-connection-retry]"));
            QTRY_COMPARE(wakes, 2);
            QVERIFY(click(setup.view(), "[data-connection-cancel]"));
            QTRY_COMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("cancelled"));
        }
        host.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
        discover(backend, {host});
        QTest::qWait(30);
        QVERIFY(backend->connectionAttempt().value("phase") != "woken");
        QVERIFY(!backend->qmlSession());
        QVERIFY(!backend->qmlWindow());
        QVERIFY(requests.isEmpty());
    }

    void setupReminders()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(true);
        settings.SetAddSteamShortcutAsk(true);
        QList<QPointer<PsnReply>> http;
        RegistrationTransport registration;
        PsnRequests requests = registration.guardedPsn();
        requests.networkManager = [&](QObject *parent) { return new PsnNetwork(http, parent); };
        requests.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = nullptr; *count = 0; return CHIAKI_ERR_SUCCESS;
        };
        requests.connect = [](StreamSession *, const QString &, bool) { return CHIAKI_ERR_CANCELED; };
        QmlNativeServices services(&settings, requests, {}, registration.boundary(), controlledDiscovery());
        SetupWindow setup(services.backend());
        setup.resize(1280, 900);
        setup.show();
#if CHIAKI_GUI_ENABLE_STEAM_SHORTCUT
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-reminder]')?.getAttribute('data-reminder')").toString(), QString("steam"));
        QVERIFY(click(setup.view(), "[data-reminder-later]"));
        QVERIFY(settings.GetAddSteamShortcutAsk());
#endif
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-reminder]')?.getAttribute('data-reminder')").toString(), QString("psn"));
        QVERIFY(click(setup.view(), "[data-reminder-never]"));
        QTRY_VERIFY(!settings.GetRemotePlayAsk());
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-reminder]') === null").toBool());
        setup.view()->reload();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings') !== null").toBool());
        QVERIFY(script(setup.view(), "document.querySelector('[data-reminder]') === null").toBool());
        Settings reopened(profile);
        QVERIFY(!reopened.GetRemotePlayAsk());
        QVERIFY(reopened.GetAddSteamShortcutAsk());
        QVERIFY(http.isEmpty());
    }

    void steamShortcutWorkflow()
    {
#if !CHIAKI_GUI_ENABLE_STEAM_SHORTCUT
        QSKIP("Steam shortcuts are not enabled in this build");
#else
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
        });
        QTemporaryDir files;
        QVERIFY(files.isValid());
        const auto write = [&](const QString &path, const QByteArray &bytes) {
            QVERIFY(QDir().mkpath(QFileInfo(path).absolutePath()));
            QFile file(path); QVERIFY(file.open(QIODevice::WriteOnly)); QCOMPARE(file.write(bytes), bytes.size());
        };
        const QString base = files.path();
        const QString shortcuts = base + "/userdata/1/config/shortcuts.vdf";
        const QString controller = base + "/steamapps/common/Steam Controller Configs/1/config/configset_controller_neptune.vdf";
        write(base + "/config/loginusers.vdf", "\"users\"\n{\n\"76561197960265729\"\n{\n\"MostRecent\" \"1\"\n}\n}\n");
        write(controller, "\"controller_mappings\"\n{\n}\n");
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(true);
        QList<QPointer<PsnReply>> http;
        RegistrationTransport registration;
        PsnRequests requests = registration.guardedPsn();
        requests.networkManager = [&](QObject *parent) { return new PsnNetwork(http, parent); };
        requests.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = nullptr; *count = 0; return CHIAKI_ERR_SUCCESS;
        };
        requests.connect = [](StreamSession *, const QString &, bool) { return CHIAKI_ERR_CANCELED; };
        QmlNativeServices services(&settings, requests, {}, registration.boundary(), controlledDiscovery());
        SetupWindow setup(services.backend());
        setup.resize(1280, 900); setup.show();
        QVERIFY(click(setup.view(), "[data-reminder-now]"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-steam-form]')").toBool());
        const auto choose = [&](bool cancel) {
            QTimer completion;
            completion.setInterval(10);
            connect(&completion, &QTimer::timeout, &completion, [&] {
                for (auto *widget : QApplication::topLevelWidgets()) if (auto *picker = qobject_cast<QFileDialog *>(widget)) {
                    if (!picker->isVisible()) continue;
                    completion.stop();
                    if (!cancel) picker->selectFile(base);
                    QMetaObject::invokeMethod(picker, "done", Qt::DirectConnection, Q_ARG(int, cancel ? QDialog::Rejected : QDialog::Accepted));
                    return;
                }
            });
            completion.start();
            QVERIFY(click(setup.view(), "[data-steam-folder]"));
            QTRY_VERIFY(!completion.isActive());
        };
        choose(true);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-steam-path]').textContent.includes('default Steam folder')").toBool());
        choose(false);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-steam-path]').textContent").toString(), QDir::toNativeSeparators(base));
        QVERIFY(fill(setup.view(), "[data-steam-name]", "  "));
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-steam-create]').disabled").toBool());
        QVERIFY(!QFile::exists(shortcuts));
        QVERIFY(fill(setup.view(), "[data-steam-name]", "  Synthetic shortcut  "));
        QVERIFY(fill(setup.view(), "[data-steam-options]", "  --profile=synthetic  "));
        script(setup.view(), "document.querySelector('[data-steam-create]').click();document.querySelector('[data-steam-create]').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-steam-result]')?.getAttribute('data-steam-result')").toString(), QString("success"));
        QVERIFY(!settings.GetAddSteamShortcutAsk());
        Settings reopened(profile); QVERIFY(!reopened.GetAddSteamShortcutAsk());
        QStringList errors;
        SteamTools tools([](const QString &) {}, [&](const QString &error) { errors.append(error); }, base);
        auto entries = tools.parseShortcuts();
        QCOMPARE(entries.size(), 1);
        QCOMPARE(entries[0].getAppName(), QString("Synthetic shortcut"));
        QCOMPARE(entries[0].getLaunchOptions(), QString("--profile=synthetic"));
        QCOMPARE(QDir(base + "/userdata/1/config/grid").entryList({"*.png"}, QDir::Files).size(), 5);
        QFile config(controller); QVERIFY(config.open(QIODevice::ReadOnly));
        const auto configBytes = config.readAll(); config.close();
        QVERIFY(configBytes.contains("3049833406") && configBytes.contains("synthetic shortcut"));
        QVERIFY(click(setup.view(), "[data-steam-close]"));
        QTRY_VERIFY(script(setup.view(), "!document.querySelector('[data-steam-form]')").toBool());
        QVERIFY(click(setup.view(), "[data-steam-open]"));
        choose(false);
        QVERIFY(fill(setup.view(), "[data-steam-name]", "Renamed shortcut"));
        QVERIFY(fill(setup.view(), "[data-steam-options]", "--profile=synthetic"));
        QVERIFY(click(setup.view(), "[data-steam-create]"));
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-steam-result]')?.getAttribute('data-steam-result')").toString(), QString("success"));
        entries = tools.parseShortcuts(); QCOMPARE(entries.size(), 1); QCOMPARE(entries[0].getAppName(), QString("Renamed shortcut"));
        QVERIFY(!QDir(QFileInfo(shortcuts).absolutePath()).entryList({"shortcuts.vdf.*.bak"}, QDir::Files).isEmpty());
        QVERIFY(click(setup.view(), "[data-steam-close]"));
        services.backend()->qmlSettings()->setAddSteamShortcutAsk(true);
        QVERIFY(QFile::remove(controller));
        QVERIFY(click(setup.view(), "[data-steam-open]"));
        choose(false);
        QVERIFY(fill(setup.view(), "[data-steam-name]", "Partial write"));
        QVERIFY(fill(setup.view(), "[data-steam-options]", "--profile=other"));
        QVERIFY(click(setup.view(), "[data-steam-create]"));
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-steam-result]')?.getAttribute('data-steam-result')").toString(), QString("failure"));
        QVERIFY(settings.GetAddSteamShortcutAsk());
        QCOMPARE(tools.parseShortcuts().size(), 2);
        QVERIFY(script(setup.view(), "document.querySelector('[data-steam-form]').textContent.includes('Some files may already have changed')").toBool());
        write(controller, "\"controller_mappings\"\n{\n}\n");
        QVERIFY(click(setup.view(), "[data-steam-create]"));
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-steam-result]')?.getAttribute('data-steam-result')").toString(), QString("success"));
        QCOMPARE(tools.parseShortcuts().size(), 2);
        QVERIFY(!settings.GetAddSteamShortcutAsk());
        QVERIFY(errors.isEmpty()); QVERIFY(http.isEmpty());
#endif
    }

    void setupReminderOwnership()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            for (const auto &name : {profile, profile + "-next"})
                QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + name).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(true);
        settings.SetAddSteamShortcutAsk(true);
        QList<QPointer<PsnReply>> http;
        RegistrationTransport registration;
        PsnRequests requests = registration.guardedPsn();
        requests.networkManager = [&](QObject *parent) { return new PsnNetwork(http, parent); };
        requests.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = nullptr; *count = 0; return CHIAKI_ERR_SUCCESS;
        };
        requests.connect = [](StreamSession *, const QString &, bool) { return CHIAKI_ERR_CANCELED; };
        QmlNativeServices services(&settings, requests, {}, registration.boundary(), controlledDiscovery());
        SetupWindow setup(services.backend());
        setup.resize(1280, 900); setup.show();
#if CHIAKI_GUI_ENABLE_STEAM_SHORTCUT
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-reminder]')?.getAttribute('data-reminder')").toString(), QString("steam"));
        QTRY_VERIFY(script(setup.view(), "document.activeElement?.hasAttribute('data-reminder-later')").toBool());
        setup.activateWindow(); setup.view()->setFocus();
        QTRY_VERIFY(setup.isActiveWindow());
        auto *receiver = setup.view()->focusProxy(); QVERIFY(receiver);
        QKeyEvent repeat(QEvent::KeyPress, Qt::Key_Yes, Qt::NoModifier, {}, true);
        QCoreApplication::sendEvent(receiver, &repeat);
        QCOMPARE(script(setup.view(), "document.querySelector('[data-reminder]')?.getAttribute('data-reminder')").toString(), QString("steam"));
        QTest::keyClick(receiver, Qt::Key_Yes);
#endif
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-reminder]')?.getAttribute('data-reminder')").toString(), QString("psn"));
        // The release from the first key must leave the next reminder open.
        QVERIFY(settings.GetRemotePlayAsk()); QVERIFY(settings.GetAddSteamShortcutAsk());
        QVERIFY(click(setup.view(), "[data-reminder-now]"));
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-psn-embedded]')?.getClientRects().length > 0").toBool());
        QTRY_COMPARE(script(setup.view(), "document.activeElement?.textContent").toString(), QString("PSN remote access"));
        QVERIFY(click(setup.view(), "[data-psn-close]"));
        QTRY_VERIFY(script(setup.view(), "document.activeElement?.hasAttribute('data-psn-account')").toBool());
        QVERIFY(!setupRequest(setup.view(), "openSetupHelp", {"file:///private"}).value("accepted").toBool());
        QVERIFY(!setupRequest(setup.view(), "openSetupHelp", {"https://example.com"}).value("accepted").toBool());
        PsnBrowserOpener opener;
        QDesktopServices::setUrlHandler("https", &opener, "open");
        const auto resetHandler = qScopeGuard([] { QDesktopServices::unsetUrlHandler("https"); });
        QVERIFY(setupRequest(setup.view(), "openSetupHelp", {"steam"}).value("accepted").toBool());
        QCOMPARE(opener.url, QUrl("https://streetpea.github.io/chiaki-ng/setup/steaming/"));
        QVERIFY(http.isEmpty());
    }

    void steamShortcutGuards()
    {
#if !CHIAKI_GUI_ENABLE_STEAM_SHORTCUT
        QSKIP("Steam shortcuts are not enabled in this build");
#else
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            for (const auto &name : {profile, profile + "-next"})
                QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + name).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        QList<QPointer<PsnReply>> http;
        RegistrationTransport registration;
        PsnRequests requests = registration.guardedPsn();
        requests.networkManager = [&](QObject *parent) { return new PsnNetwork(http, parent); };
        requests.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = nullptr; *count = 0; return CHIAKI_ERR_SUCCESS;
        };
        requests.connect = [](StreamSession *, const QString &, bool) { return CHIAKI_ERR_CANCELED; };
        QmlNativeServices services(&settings, requests, {}, registration.boundary(), controlledDiscovery());
        SetupWindow setup(services.backend());
        setup.resize(1280, 900); setup.show();
        QVERIFY(click(setup.view(), "[data-steam-open]"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-steam-form]')").toBool());
        const auto generation = QString::number(services.backend()->profileGeneration());
        const auto form = setup.bridge()->snapshot().value("steam").toMap().value("form").toString();
        QVERIFY(!setupRequest(setup.view(), "beginSteamShortcut", {generation}).value("accepted").toBool());
        QVERIFY(!setupRequest(setup.view(), "submitSteamShortcut", {form, QJsonObject{{"name", "  "}}, generation}).value("accepted").toBool());
        QVERIFY(!setupRequest(setup.view(), "submitSteamShortcut", {form, QJsonObject{{"steamDir", "C:/unauthorized"}}, generation}).value("accepted").toBool());
        QTemporaryDir files; QVERIFY(files.isValid());
        bool pickerSeen = false;
        QTimer picker;
        picker.setInterval(10);
        connect(&picker, &QTimer::timeout, &picker, [&] {
            for (auto *widget : QApplication::topLevelWidgets()) if (auto *dialog = qobject_cast<QFileDialog *>(widget)) {
                if (!dialog->isVisible()) continue;
                picker.stop(); pickerSeen = true;
                dialog->selectFile(files.path());
                services.backend()->qmlSettings()->setCurrentProfile(profile + "-next");
                services.backend()->setDiscoveryEnabled(false);
                QMetaObject::invokeMethod(dialog, "done", Qt::DirectConnection, Q_ARG(int, QDialog::Accepted));
                return;
            }
        });
        picker.start();
        QVERIFY(click(setup.view(), "[data-steam-folder]"));
        QTRY_VERIFY(pickerSeen);
        QTRY_VERIFY(script(setup.view(), "!document.querySelector('[data-steam-form]')").toBool());
        QVERIFY(!setupRequest(setup.view(), "submitSteamShortcut", {form, QJsonObject{{"name", "Stale"}}, generation}).value("accepted").toBool());
        QVERIFY(QDir(files.path()).entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty());
        QVERIFY(click(setup.view(), "[data-steam-open]"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-steam-form]')").toBool());
        const auto next = setup.bridge()->snapshot().value("steam").toMap().value("form").toString();
        QVERIFY(next != form);
        emit services.backend()->steamShortcutMessage(form, "stale-path-secret", true, true);
        QCOMPARE(setup.bridge()->snapshot().value("steam").toMap().value("phase").toString(), QString("form"));
        services.backend()->qmlSettings()->setStreamerMode(true);
        const auto json = QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson();
        QVERIFY(!json.contains(profile.toUtf8())); QVERIFY(!json.contains(files.path().toUtf8())); QVERIFY(!json.contains("stale-path-secret"));
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-steam-name]').type === 'password'").toBool());
        setup.hide();
        QVERIFY(setup.bridge()->snapshot().value("steam").toMap().value("form").toString().isEmpty());
        setup.show();
        QVERIFY(click(setup.view(), "[data-steam-open]"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-steam-form]')").toBool());
        const auto beforeReload = setup.bridge()->snapshot().value("steam").toMap().value("form").toString();
        setup.view()->reload();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-steam-open]')?.disabled === false").toBool());
        QVERIFY(!setupRequest(setup.view(), "chooseSteamFolder", {beforeReload, QString::number(services.backend()->profileGeneration())}).value("accepted").toBool());
        QVERIFY(click(setup.view(), "[data-steam-open]"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-steam-form]')").toBool());
        // Select an isolated folder before checking cancellation of queued native work.
        QTimer selection;
        selection.setInterval(10);
        connect(&selection, &QTimer::timeout, &selection, [&] {
            for (auto *widget : QApplication::topLevelWidgets()) if (auto *dialog = qobject_cast<QFileDialog *>(widget)) {
                if (!dialog->isVisible()) continue;
                selection.stop(); dialog->selectFile(files.path());
                QMetaObject::invokeMethod(dialog, "done", Qt::DirectConnection, Q_ARG(int, QDialog::Accepted));
                return;
            }
        });
        selection.start();
        QVERIFY(click(setup.view(), "[data-steam-folder]"));
        QTRY_VERIFY(!selection.isActive());
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-steam-path]')?.textContent").toString(), QString("Folder hidden"));
        bool staleIgnored = false, hidden = false;
        QMetaObject::Connection cancel;
        cancel = connect(setup.bridge(), &SetupBridge::stateChanged, &setup, [&](const QVariantMap &state) {
            if (state.value("steam").toMap().value("phase") != "progress") return;
            disconnect(cancel);
            emit services.backend()->steamShortcutMessage(beforeReload, "obsolete completion", true, true);
            staleIgnored = setup.bridge()->snapshot().value("steam").toMap().value("phase") == "progress";
            setup.hide(); hidden = true;
        });
        QVERIFY(click(setup.view(), "[data-steam-create]"));
        QTRY_VERIFY(hidden); QVERIFY(staleIgnored);
        QTest::qWait(80);
        QVERIFY(QDir(files.path()).entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty());
        QVERIFY(setup.bridge()->snapshot().value("steam").toMap().value("form").toString().isEmpty());
        QVERIFY(http.isEmpty());
#endif
    }

    void setupReminderEligibility_data()
    {
        QTest::addColumn<bool>("automatic");
        QTest::addColumn<bool>("complete");
        QTest::addColumn<bool>("steam");
        QTest::newRow("auto-connect") << true << false << true;
        QTest::newRow("complete-credentials") << false << true << false;
        QTest::newRow("incomplete-credentials") << false << false << false;
#if CHIAKI_GUI_ENABLE_STEAM_SHORTCUT
        QTest::newRow("steam-before-complete-psn") << false << true << true;
#endif
    }

    void setupReminderEligibility()
    {
        QFETCH(bool, automatic); QFETCH(bool, complete); QFETCH(bool, steam);
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(true); settings.SetAddSteamShortcutAsk(steam);
        if (complete) {
            settings.SetPsnAuthToken("synthetic-access"); settings.SetPsnRefreshToken("synthetic-refresh");
            settings.SetPsnAuthTokenExpiry(QDateTime::currentDateTime().addDays(1).toString(settings.GetTimeFormat()));
            settings.SetPsnAccountId("AQAAAAAAAAA=");
        }
        if (automatic) {
            ChiakiRegisteredHost host = {};
            host.target = CHIAKI_TARGET_PS5_1;
            memcpy(host.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
            strcpy(host.server_nickname, "Auto console");
            settings.AddRegisteredHost(RegisteredHost(host));
            settings.SetAutoConnectHost(QByteArray::fromHex("001122334455"));
        }
        QList<QPointer<PsnReply>> http;
        RegistrationTransport registration;
        PsnRequests requests = registration.guardedPsn();
        requests.networkManager = [&](QObject *parent) { return new PsnNetwork(http, parent); };
        requests.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = nullptr; *count = 0; return CHIAKI_ERR_SUCCESS;
        };
        requests.connect = [](StreamSession *, const QString &, bool) { return CHIAKI_ERR_CANCELED; };
        QmlNativeServices services(&settings, requests, {}, registration.boundary(), controlledDiscovery());
        SetupWindow setup(services.backend());
        setup.resize(1280, 900); setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        if (automatic) {
            QVERIFY(services.backend()->autoConnect());
            QVERIFY(!setupRequest(setup.view(), "startReminders", {QString::number(services.backend()->profileGeneration())}).value("accepted").toBool());
            QVERIFY(script(setup.view(), "!document.querySelector('[data-reminder]')").toBool());
            QVERIFY(settings.GetRemotePlayAsk()); QVERIFY(settings.GetAddSteamShortcutAsk());
            services.backend()->stopAutoConnect();
        } else if (steam) {
            QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-reminder]')?.getAttribute('data-reminder')").toString(), QString("steam"));
            QVERIFY(settings.GetRemotePlayAsk());
            QVERIFY(click(setup.view(), "[data-reminder-now]"));
            QVERIFY(click(setup.view(), "[data-steam-close]"));
            QTRY_VERIFY(!settings.GetRemotePlayAsk());
            QVERIFY(settings.GetAddSteamShortcutAsk());
            QTRY_VERIFY(script(setup.view(), "!document.querySelector('[data-reminder]') && !document.querySelector('[data-steam-form]')").toBool());
        } else if (complete) {
            QTRY_VERIFY(!settings.GetRemotePlayAsk());
            QVERIFY(script(setup.view(), "!document.querySelector('[data-reminder]')").toBool());
        } else {
            QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-reminder]')?.getAttribute('data-reminder')").toString(), QString("psn"));
            setup.resize(2048, 1700); setup.view()->setZoomFactor(2);
            QTRY_VERIFY(script(setup.view(), "innerWidth >= 1024 && document.documentElement.scrollWidth <= innerWidth").toBool());
            QVERIFY(script(setup.view(), "document.querySelector('dialog').getBoundingClientRect().right <= innerWidth").toBool());
            QVERIFY(click(setup.view(), "[data-reminder-later]"));
            QTRY_VERIFY(script(setup.view(), "!document.querySelector('[data-reminder]')").toBool());
            QVERIFY(settings.GetRemotePlayAsk());
        }
    }

    void controllerSettings()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        PsnRequests transport;
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = nullptr; *count = 0; return CHIAKI_ERR_SUCCESS;
        };
        QmlNativeServices services(&settings, transport, {}, {}, controlledDiscovery());
        services.backend()->stopAutoConnect();
        SetupWindow setup(services.backend());
        setup.resize(1280, 900);
        setup.show();
        QVERIFY(click(setup.view(), ".open-settings"));
        QTRY_VERIFY(script(setup.view(), "Array.from(document.querySelectorAll('.category-nav button')).some(e => e.textContent === 'Controllers')").toBool());
        script(setup.view(), "Array.from(document.querySelectorAll('.category-nav button')).find(e => e.textContent === 'Controllers').click()");
        QVERIFY(click(setup.view(), "#allowJoystickBackgroundEvents"));
        QTRY_VERIFY(!settings.GetAllowJoystickBackgroundEvents());
        QVERIFY(fill(setup.view(), "#dpadTouchIncrement", "0.75"));
        script(setup.view(), "document.querySelector('#dpadTouchIncrement').dispatchEvent(new Event('change',{bubbles:true}))");
        QTRY_COMPARE(settings.GetDpadTouchIncrement(), uint16_t(75));
        Settings reopened(profile);
        QCOMPARE(reopened.GetDpadTouchIncrement(), uint16_t(75));
        QVERIFY(click(setup.view(), "#buttonsByPosition"));
        QTRY_VERIFY(settings.GetButtonsByPosition());
        for (int index = 1; index <= 4; ++index) {
            QVERIFY(fill(setup.view(), QString("#dpadTouchShortcut%1").arg(index), QString::number(index)));
        }
        QTRY_COMPARE(services.backend()->qmlSettings()->dpadTouchShortcut4(), uint(4));
        QVERIFY(fill(setup.view(), "#rumbleHapticsIntensity", "5"));
        QTRY_COMPARE(services.backend()->qmlSettings()->rumbleHapticsIntensity(), 5);
        QVERIFY(fill(setup.view(), "#hapticOverride", "1.7"));
        script(setup.view(), "document.querySelector('#hapticOverride').dispatchEvent(new Event('change',{bubbles:true}))");
        QTRY_VERIFY(qAbs(services.backend()->qmlSettings()->hapticOverride() - 1.7f) < 0.001f);
        const auto generation = QString::number(services.backend()->profileGeneration());
        for (const auto &request : QList<QJsonArray>{{"dpadTouchIncrement", 0, generation}, {"dpadTouchShortcut1", 17, generation},
                {"rumbleHapticsIntensity", 6, generation}, {"hapticOverride", 2.1, generation}, {"buttonsByPosition", 1, generation},
                {"allowJoystickBackgroundEvents", "true", generation}, {"unknown", true, generation}})
            QVERIFY(!setupRequest(setup.view(), "setControllerSetting", request).value("accepted").toBool());
        QVERIFY(click(setup.view(), "#dpadTouchEnabled"));
        QTRY_VERIFY(!services.backend()->qmlSettings()->dpadTouchEnabled());
        QVERIFY(!setupRequest(setup.view(), "setControllerSetting", {"dpadTouchIncrement", 80, generation}).value("accepted").toBool());
        Settings persisted(profile);
        QVERIFY(persisted.GetButtonsByPosition());
        QCOMPARE(persisted.GetDpadTouchIncrement(), uint16_t(75));
    }

    void controllerMapping()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            for (const auto &name : {profile, profile + "-next"})
                QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + name).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        PsnRequests transport;
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = nullptr; *count = 0; return CHIAKI_ERR_SUCCESS;
        };
        QmlNativeServices services(&settings, transport, {}, {}, controlledDiscovery());
        auto *backend = services.backend();
        backend->stopAutoConnect();
        SDL_VirtualJoystickDesc description = {};
        description.version = SDL_VIRTUAL_JOYSTICK_DESC_VERSION;
        description.type = SDL_JOYSTICK_TYPE_GAMECONTROLLER;
        description.naxes = 6;
        description.nbuttons = 16;
        description.nhats = 1;
        description.vendor_id = 0xfafa;
        description.product_id = 0x0016;
        description.name = "Recast setup test controller";
        const int index = SDL_JoystickAttachVirtualEx(&description);
        QVERIFY2(index >= 0, SDL_GetError());
        auto *joystick = SDL_JoystickOpen(index);
        QVERIFY(joystick);
        const auto instance = SDL_JoystickInstanceID(joystick);
        const auto detach = [&] {
            for (int i = 0; i < SDL_NumJoysticks(); ++i)
                if (SDL_JoystickGetDeviceInstanceID(i) == instance) { SDL_JoystickDetachVirtual(i); break; }
        };
        const auto removeController = qScopeGuard([&] { detach(); SDL_JoystickClose(joystick); });
        SetupWindow setup(backend);
        setup.resize(1280, 900);
        setup.show();
        QTRY_VERIFY(!backend->qmlControllers().isEmpty());
        const auto button = [&](int number, bool pressed) { QCOMPARE(SDL_JoystickSetVirtualButton(joystick, number, pressed), 0); SDL_JoystickUpdate(); };
        const auto tap = [&](int number) { button(number, true); QTest::qWait(80); button(number, false); QTest::qWait(80); };
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('.open-settings')").toBool());
        setup.view()->setFocus();
        tap(6);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('h1').textContent").toString(), QStringLiteral("Settings"));
        script(setup.view(), "Array.from(document.querySelectorAll('.category-nav button')).find(e => e.textContent === 'Controllers').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-controller-devices]').textContent.includes('Recast setup test controller')").toBool());
        setup.view()->setFocus();
        script(setup.view(), "document.querySelector('#controllerMappingChange').focus()");
        tap(0);
        QTRY_COMPARE(setup.bridge()->snapshot().value("controllerSetup").toMap().value("phase").toString(), QStringLiteral("select"));
        tap(0);
        QTRY_VERIFY(backend->controllerMappingInProgress());
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.controller-mappings > div').length").toInt(), 23);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('dialog[open]') && !document.querySelector('[data-controller-capture]')").toBool());
        if (qEnvironmentVariableIsSet("CHIAKI_EMBEDDED_EVIDENCE")) {
            QVERIFY(setup.view()->grab().save(QDir(qEnvironmentVariable("CHIAKI_EMBEDDED_EVIDENCE")).filePath("controller-mapping.png")));
            setup.resize(2560, 1800);
            setup.view()->setZoomFactor(2);
            QTRY_VERIFY(script(setup.view(), "devicePixelRatio === 2 && innerWidth === 1280 && document.documentElement.scrollWidth <= innerWidth").toBool());
            QTest::qWait(250);
            QVERIFY(setup.view()->grab().save(QDir(qEnvironmentVariable("CHIAKI_EMBEDDED_EVIDENCE")).filePath("controller-mapping-200-percent.png")));
            setup.view()->setZoomFactor(1);
            setup.resize(1280, 900);
        }
        tap(12);
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("controllerMap1Slot0"));
        tap(14);
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("controllerMap1Slot1"));
        const auto original = backend->currentControllerMapping();
        description.product_id = 0x0017;
        const int unrelated = SDL_JoystickAttachVirtualEx(&description);
        QVERIFY2(unrelated >= 0, SDL_GetError());
        const auto devices = backend->qmlControllers().size();
        QTRY_COMPARE(backend->qmlControllers().size(), devices + 1);
        QCOMPARE(SDL_JoystickDetachVirtual(unrelated), 0);
        QTRY_COMPARE(backend->qmlControllers().size(), devices);
        QVERIFY(backend->controllerMappingInProgress());
        const auto mappingInput = [&](int action, int slot) {
            for (const auto &row : backend->currentControllerMapping()) {
                const auto entry = row.toMap();
                if (entry.value("buttonValue").toInt() == action) return entry.value("physicalButton").toStringList().value(slot);
            }
            return QString();
        };
        QVERIFY(click(setup.view(), "#controllerMap1Slot1"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-controller-capture]')").toBool());
        QVERIFY(click(setup.view(), "#controllerCaptureCancel"));
        QTRY_VERIFY(!script(setup.view(), "!!document.querySelector('[data-controller-capture]')").toBool());
        QCOMPARE(backend->currentControllerMapping(), original);
        QVERIFY(click(setup.view(), "#enableAnalogStickMapping"));
        QTRY_VERIFY(backend->enableAnalogStickMapping());
        QVERIFY(click(setup.view(), "#controllerMap1Slot1"));
        tap(1);
        QTRY_COMPARE(mappingInput(1, 1), QStringLiteral("b1"));
        QVERIFY(mappingInput(2, 0).isEmpty());
        QVERIFY(backend->controllerMappingAltered());
        QTRY_VERIFY(!script(setup.view(), "!!document.querySelector('[data-controller-capture]')").toBool());
        QCOMPARE(script(setup.view(), "document.querySelectorAll('dialog[open]').length").toInt(), 1);
        QVERIFY(click(setup.view(), "#controllerMappingCancel"));
        QTRY_VERIFY(!backend->controllerMappingInProgress());
        QVERIFY(settings.GetControllerMappings().isEmpty());
        QVERIFY(click(setup.view(), "#controllerMappingChange"));
        tap(0);
        QTRY_VERIFY(backend->controllerMappingInProgress());
        QCOMPARE(backend->currentControllerMapping(), original);
        QVERIFY(click(setup.view(), "#controllerMap1Slot1"));
        tap(1);
        QTRY_COMPARE(mappingInput(1, 1), QStringLiteral("b1"));
        QVERIFY(click(setup.view(), "#controllerMappingApply"));
        QTRY_VERIFY(!backend->controllerMappingInProgress());
        QTRY_COMPARE(settings.GetControllerMappings().size(), 1);
        const auto saved = settings.GetControllerMappings();
        QVERIFY(saved.first().contains("a:b1"));
        Settings reopened(profile);
        QCOMPARE(reopened.GetControllerMappings(), saved);
        script(setup.view(), "document.querySelector('#controllerMappingReset').focus()");
        tap(0);
        QTRY_COMPARE(setup.bridge()->snapshot().value("controllerSetup").toMap().value("phase").toString(), QStringLiteral("select"));
        tap(0);
        QTRY_VERIFY(settings.GetControllerMappings().isEmpty());
        QTRY_VERIFY(!backend->controllerMappingInProgress());
        QVERIFY(fill(setup.view(), ".settings-search input", "MIC alternate input"));
        QTRY_VERIFY(script(setup.view(), "document.querySelectorAll('.settings-results button').length === 1").toBool());
        QVERIFY(click(setup.view(), ".settings-results button"));
        tap(0);
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("controllerMap1073741824Slot1"));
        QVERIFY(!script(setup.view(), "!!document.querySelector('[data-controller-capture]')").toBool());
        QVERIFY(click(setup.view(), "#controllerMap1073741824Slot1"));
        setup.view()->reload();
        QTRY_VERIFY(!backend->controllerMappingInProgress());
        QTRY_VERIFY(setup.bridge()->snapshot().value("controllerSetup").toMap().value("operation").toString().isEmpty());
        QVERIFY(click(setup.view(), ".open-settings"));
        script(setup.view(), "Array.from(document.querySelectorAll('.category-nav button')).find(e => e.textContent === 'Controllers').click()");
        QVERIFY(click(setup.view(), "#controllerMappingChange"));
        tap(0);
        QTRY_VERIFY(backend->controllerMappingInProgress());
        QVERIFY(click(setup.view(), "#controllerMap1Slot1"));
        const auto previousOperation = setup.bridge()->snapshot().value("controllerSetup").toMap().value("operation").toString();
        const auto previousGeneration = QString::number(backend->profileGeneration());
        backend->qmlSettings()->setCurrentProfile(profile + "-next");
        backend->setDiscoveryEnabled(false);
        QTRY_VERIFY(!backend->controllerMappingInProgress());
        QTRY_VERIFY(script(setup.view(), "!document.querySelector('dialog[open]') && document.querySelector('[data-controller-status]').textContent.includes('profile changed')").toBool());
        QVERIFY(!setupRequest(setup.view(), "captureControllerMapping", {1, 1, previousOperation, previousGeneration}).value("accepted").toBool());
        QVERIFY(!setupRequest(setup.view(), "controllerMappingAction", {"apply", previousOperation, previousGeneration}).value("accepted").toBool());
        QVERIFY(click(setup.view(), "#controllerMappingChange"));
        tap(0);
        QTRY_VERIFY(backend->controllerMappingInProgress());
        setup.hide();
        QTRY_VERIFY(!backend->controllerMappingInProgress());
        setup.show();
        QVERIFY(click(setup.view(), "#controllerMappingChange"));
        tap(0);
        QTRY_VERIFY(backend->controllerMappingInProgress());
        const auto oldOperation = setup.bridge()->snapshot().value("controllerSetup").toMap().value("operation").toString();
        const auto generation = QString::number(backend->profileGeneration());
        QVERIFY(click(setup.view(), "#controllerMap1Slot1"));
        detach();
        QTRY_VERIFY(!backend->controllerMappingInProgress());
        QTRY_VERIFY(script(setup.view(), "!document.querySelector('dialog[open]') && document.querySelector('[data-controller-status]').textContent.includes('disconnected')").toBool());
        QVERIFY(!setupRequest(setup.view(), "controllerMappingAction", {"apply", oldOperation, generation}).value("accepted").toBool());
        QVERIFY(settings.GetControllerMappings().isEmpty());
    }


    void autoConnectCancellation()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            for (const auto &name : {profile, profile + "-next"})
                QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + name).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        ChiakiRegisteredHost registered = {};
        registered.target = CHIAKI_TARGET_PS5_1;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "Startup console");
        strcpy(registered.rp_regist_key, "12345678");
        settings.AddRegisteredHost(RegisteredHost(registered));
        settings.SetAutoConnectHost(QByteArray::fromHex("001122334455"));
        QList<QPointer<PsnReply>> requests;
        auto transport = controlledPsn(requests);
        auto discovery = controlledDiscovery();
        int wakes = 0;
        discovery.wakeup = [&](ChiakiLog *, ChiakiDiscovery *, const char *, uint64_t, bool) { ++wakes; return CHIAKI_ERR_SUCCESS; };
        QmlNativeServices services(&settings, transport, {}, {}, discovery);
        auto *backend = services.backend();
        SetupWindow setup(backend);
        setup.resize(1280, 720);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-stop-auto-connect]')?.disabled === false").toBool());
        const auto attempt = backend->connectionAttempt().value("id").toString();
        QVERIFY(click(setup.view(), "[data-stop-auto-connect]"));
        QTRY_COMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("cancelled"));
        QCOMPARE(settings.GetAutoConnectHost().GetServerMAC().ToString(), QStringLiteral("001122334455"));
        QVERIFY(click(setup.view(), "[data-connection-back]"));
        DiscoveryHost host = {};
        host.ps5 = true;
        host.host_id = "001122334455";
        host.host_name = "Startup console";
        host.host_addr = "192.0.2.10";
        host.state = CHIAKI_DISCOVERY_HOST_STATE_STANDBY;
        discover(backend, {host});
        QVERIFY(click(setup.view(), "[data-wake-console]"));
        QTRY_COMPARE(wakes, 1);
        const auto manual = backend->connectionAttempt().value("id").toString();
        QVERIFY(manual != attempt);
        QVERIFY(!setupRequest(setup.view(), "stopAutoConnect", {attempt}).value("accepted").toBool());
        backend->stopAutoConnect();
        QCOMPARE(backend->connectionAttempt().value("id").toString(), manual);
        QCOMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("waking"));
        QVERIFY(click(setup.view(), "[data-connection-cancel]"));
        host.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
        discover(backend, {host});
        QTest::qWait(30);
        QCOMPARE(wakes, 1);
        QVERIFY(!backend->qmlWindow());
        QVERIFY(!backend->qmlSession());
        QVERIFY(requests.isEmpty());
    }

    void wakeConsole()
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
        ChiakiRegisteredHost registered = {};
        registered.target = CHIAKI_TARGET_PS5_1;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "Sleeping console");
        strcpy(registered.rp_regist_key, "12345678");
        settings.AddRegisteredHost(RegisteredHost(registered));
        QList<QPointer<PsnReply>> requests;
        RegistrationTransport guarded;
        auto transport = guarded.guardedPsn();
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **, size_t *, ChiakiLog *) {
            QTest::qFail("Unexpected PSN list in wake fixture", __FILE__, __LINE__);
            return CHIAKI_ERR_CANCELED;
        };
        transport.connect = [](StreamSession *, const QString &, bool) {
            QTest::qFail("Unexpected PSN worker in wake fixture", __FILE__, __LINE__);
            return CHIAKI_ERR_CANCELED;
        };
        int wakes = 0;
        bool wakeMatches = false;
        auto discovery = controlledDiscovery();
        discovery.wakeup = [&](ChiakiLog *, ChiakiDiscovery *, const char *address, uint64_t credential, bool ps5) {
            ++wakes;
            wakeMatches = QString::fromUtf8(address) == "192.0.2.10" && credential == 0x12345678 && ps5;
            return CHIAKI_ERR_SUCCESS;
        };
        QmlNativeServices services(&settings, transport, {}, {}, discovery);
        auto *backend = services.backend();
        SetupWindow setup(backend);
        setup.resize(1280, 720);
        setup.show();
        DiscoveryHost host = {};
        host.ps5 = true;
        host.host_id = "001122334455";
        host.host_name = "Sleeping console";
        host.host_addr = "192.0.2.10";
        host.state = CHIAKI_DISCOVERY_HOST_STATE_STANDBY;
        discover(backend, {host});
        QVERIFY(click(setup.view(), "[data-wake-console]"));
        QTRY_COMPARE(wakes, 1);
        QVERIFY(wakeMatches);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-connection-phase]')?.textContent.includes('Waiting for the console to wake')").toBool());
        QVERIFY(!backend->qmlWindow());
        QVERIFY(!backend->qmlSession());
        setup.view()->reload();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-connection-phase]')?.textContent.includes('Waiting for the console to wake')").toBool());
        QCOMPARE(wakes, 1);
        auto other = host;
        other.host_id = "aabbccddeeff";
        other.host_name = "Other console";
        other.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
        discover(backend, {other, host});
        QTest::qWait(40);
        QCOMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("waking"));
        host.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
        discover(backend, {host, other});
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-connection-phase]')?.textContent.includes('Console is awake')").toBool());
        QVERIFY(click(setup.view(), "[data-connection-back]"));
        QTRY_VERIFY(!script(setup.view(), "!!document.querySelector('[data-connection-phase]')").toBool());
        QVERIFY(!backend->qmlSession());
        QVERIFY(requests.isEmpty());
    }

    void autoConnectTarget()
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
        ChiakiRegisteredHost registered = {};
        registered.target = CHIAKI_TARGET_PS5_1;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "Startup console");
        settings.AddRegisteredHost(RegisteredHost(registered));
        QList<QPointer<PsnReply>> requests;
        PsnRequests transport;
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **, size_t *, ChiakiLog *) {
            QTest::qFail("Unexpected PSN list in auto-connect settings", __FILE__, __LINE__);
            return CHIAKI_ERR_CANCELED;
        };
        transport.connect = [](StreamSession *, const QString &, bool) {
            QTest::qFail("Unexpected PSN connection in auto-connect settings", __FILE__, __LINE__);
            return CHIAKI_ERR_CANCELED;
        };
        transport.startSession = [](ChiakiSession *) {
            QTest::qFail("Unexpected native start in auto-connect settings", __FILE__, __LINE__);
            return CHIAKI_ERR_CANCELED;
        };
        QmlNativeServices services(&settings, transport, {}, {}, controlledDiscovery());
        SetupWindow setup(services.backend());
        setup.resize(1280, 720);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-auto-connect-target]') !== null").toBool());
        const auto key = setup.bridge()->snapshot().value("registrations").toList().first().toMap().value("key").toString();
        QVERIFY(fill(setup.view(), "[data-auto-connect-target]", key));
        QTRY_COMPARE(settings.GetAutoConnectHost().GetServerMAC().ToString(), QStringLiteral("001122334455"));
        QVERIFY(services.backend()->connectionAttempt().isEmpty());
        Settings restarted(profile);
        QCOMPARE(restarted.GetAutoConnectHost().GetServerMAC().ToString(), QStringLiteral("001122334455"));
        setup.view()->reload();
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-auto-connect-target]')?.value").toString(), key);
        QVERIFY(fill(setup.view(), "[data-auto-connect-target]", ""));
        QTRY_VERIFY(!settings.GetAutoConnectHost().GetServerMAC().GetValue());
        QVERIFY(requests.isEmpty());
    }

    void remoteNativeConnection()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            for (const auto &name : {profile, profile + "-next"})
                QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + name).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetHardwareDecoder({});
        settings.SetCodecRemotePS5(CHIAKI_CODEC_H264);
        settings.SetRenderBackend(RenderBackend::OpenGL);
        settings.SetPlaceboPreset(PlaceboPreset::Fast);
        settings.SetWindowType(WindowType::AdjustableResolution);
        settings.SetStreamGeometry(QRect(80, 80, 960, 640));
        settings.SetPsnAuthToken("worker-access-secret");
        settings.SetPsnRefreshToken("worker-refresh-secret");
        settings.SetPsnAccountId("AQAAAAAAAAA=");
        settings.SetPsnAuthTokenExpiry(QDateTime::currentDateTime().addSecs(3600).toString(settings.GetTimeFormat()));
        ChiakiRegisteredHost registered = {};
        registered.target = CHIAKI_TARGET_PS5_1;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "Remote office");
        RegisteredHost paired(registered);
        paired.SetConsolePin(paired, {});
        settings.AddRegisteredHost(paired);
        QSemaphore release;
        std::atomic<int> calls = 0;
        std::atomic<ChiakiErrorCode> result = CHIAKI_ERR_HOST_UNREACH;
        std::atomic<int> starts = 0;
        QString connectedDuid;
        bool connectedPs5 = false;
        QList<QPointer<PsnReply>> requests;
        PsnRequests transport;
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = static_cast<ChiakiHolepunchDeviceInfo *>(calloc(1, sizeof(ChiakiHolepunchDeviceInfo)));
            *count = 1;
            strcpy((*devices)[0].device_name, "Remote office");
            memset((*devices)[0].device_uid, 0x12, sizeof((*devices)[0].device_uid));
            (*devices)[0].remoteplay_enabled = true;
            return CHIAKI_ERR_SUCCESS;
        };
        transport.connect = [&](StreamSession *, const QString &duid, bool ps5) {
            connectedDuid = duid;
            connectedPs5 = ps5;
            ++calls;
            release.acquire();
            return result.load();
        };
        transport.startSession = [&](ChiakiSession *) { ++starts; return CHIAKI_ERR_CANCELED; };
        QmlNativeServices services(&settings, transport, {}, {}, controlledDiscovery());
        auto *backend = services.backend();
        backend->stopAutoConnect();
        ApplicationController controller(&services);
        const auto unblock = qScopeGuard([&] { release.release(100); });
        auto *setup = controller.setupWindow();
        setup->show();
        QTRY_VERIFY(script(setup->view(), "document.querySelector('.connect-console')?.disabled === false").toBool());
        const auto key = setup->bridge()->snapshot().value("selectedKey").toString();
        script(setup->view(), "document.querySelector('.connect-console').click();document.querySelector('.connect-console').click()");
        QTRY_COMPARE(calls.load(), 1);
        QCOMPARE(connectedDuid, QStringLiteral("1212121212121212121212121212121212121212121212121212121212121212"));
        QVERIFY(connectedPs5);
        QPointer<StreamSession> firstSession = backend->qmlSession();
        QPointer<QmlMainWindow> native = backend->qmlWindow();
        QVERIFY(firstSession && native);
        QTRY_VERIFY(!setup->isVisible());
        QTRY_VERIFY(nativeObject("connectionCancel"));
        QCOMPARE(nativeObject("connectionRoute")->property("text").toString(), QStringLiteral("Remote connection"));
        release.release();
        QTRY_COMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("failure"));
        QTRY_VERIFY(!firstSession);
        QVERIFY(!backend->connectionAttempt().value("cleanupPending").toBool());
        QCOMPARE(nativeObject("connectionTitle")->property("text").toString(), QStringLiteral("Console is unreachable"));
        auto *retry = nativeObject("connectionRetry");
        QVERIFY(retry && retry->property("enabled").toBool());
        const auto firstAttempt = backend->connectionAttempt().value("id").toString();
        result = CHIAKI_ERR_SUCCESS;
        QVERIFY(QMetaObject::invokeMethod(retry, "clicked"));
        QTRY_COMPARE(calls.load(), 2);
        QPointer<StreamSession> secondSession = backend->qmlSession();
        QVERIFY(secondSession);
        QVERIFY(backend->connectionAttempt().value("id").toString() != firstAttempt);
        QVERIFY(!setup->bridge()->cancelConnection(firstAttempt).value("accepted").toBool());
        auto *cancel = nativeObject("connectionCancel");
        QVERIFY(cancel && cancel->property("enabled").toBool());
        QVERIFY(QMetaObject::invokeMethod(cancel, "clicked"));
        QTRY_COMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("cancelling"));
        QVERIFY(backend->connectionAttempt().value("cleanupPending").toBool());
        QVERIFY(!setup->bridge()->connectConsole(key).value("accepted").toBool());
        release.release();
        QTRY_VERIFY(!secondSession);
        QTRY_COMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("cancelled"));
        QCOMPARE(starts.load(), 0);
        auto *back = nativeObject("connectionBack");
        QVERIFY(back && back->property("enabled").toBool());
        QVERIFY(QMetaObject::invokeMethod(back, "clicked"));
        QTRY_VERIFY(!native);
        QTRY_VERIFY(setup->isVisible());
        QTRY_VERIFY(script(setup->view(), "document.querySelector('.connect-console')?.disabled === false").toBool());
        QCOMPARE(calls.load(), 2);
        script(setup->view(), "document.querySelector('.connect-console').click()");
        QTRY_COMPARE(calls.load(), 3);
        QPointer<StreamSession> staleSession = backend->qmlSession();
        native = backend->qmlWindow();
        QVERIFY(staleSession && native);
        QTRY_VERIFY(!setup->isVisible());
        backend->qmlSettings()->setCurrentProfile(profile + "-next");
        backend->setDiscoveryEnabled(false);
        release.release();
        QTRY_VERIFY(!staleSession);
        QTRY_COMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("failure"));
        QVERIFY(!nativeObject("connectionRetry")->property("enabled").toBool());
        QVERIFY(!backend->connectionAttempt().value("cleanupPending").toBool());
        back = nativeObject("connectionBack");
        QVERIFY(back && back->property("enabled").toBool());
        QVERIFY(QMetaObject::invokeMethod(back, "clicked"));
        QTRY_VERIFY(!native);
        QTRY_VERIFY(setup->isVisible());
        QTRY_COMPARE(script(setup->view(), "document.querySelectorAll('.console-card').length").toInt(), 0);
        QCOMPARE(calls.load(), 3);
        QCOMPARE(requests.size(), 0);
        QVERIFY(!backend->qmlSession());
        QVERIFY(!backend->qmlWindow());
        QCOMPARE(starts.load(), 0);
    }

    void automaticRegistration_data()
    {
        QTest::addColumn<QString>("outcome");
        for (const auto &outcome : {"success", "retry", "cancel", "profile", "removed", "duid", "mac", "ps4", "context", "concurrent"})
            QTest::newRow(outcome) << QString(outcome);
    }

    void automaticRegistration()
    {
        QFETCH(QString, outcome);
        const bool ps5 = outcome != "ps4";
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile + "-next").clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetHardwareDecoder({});
        settings.SetRenderBackend(RenderBackend::OpenGL);
        settings.SetPlaceboPreset(PlaceboPreset::Fast);
        settings.SetWindowType(WindowType::AdjustableResolution);
        const QRect geometry(70, 90, 1100, 720);
        settings.SetStreamGeometry(geometry);
        settings.SetPsnAuthToken("registration-access-secret");
        settings.SetPsnRefreshToken("registration-refresh-secret");
        settings.SetPsnAccountId("AQAAAAAAAAA=");
        settings.SetPsnAuthTokenExpiry(QDateTime::currentDateTime().addSecs(3600).toString(settings.GetTimeFormat()));
        QList<QPointer<PsnReply>> requests;
        std::atomic<int> connects = 0, starts = 0, producers = 0;
        std::atomic<int> deviceUid = 0x12;
        std::atomic<ChiakiErrorCode> workerResult = outcome == "retry" ? CHIAKI_ERR_HOST_UNREACH : CHIAKI_ERR_SUCCESS;
        QSemaphore worker, completion;
        struct Producer {
            ChiakiSession *session = nullptr;
            QSemaphore *completion;
            std::atomic<int> *count;
            ChiakiRegisteredHost host = {};
            static void *run(void *opaque) {
                auto *self = static_cast<Producer *>(opaque);
                self->completion->acquire();
                ChiakiEvent event = {};
                event.type = CHIAKI_EVENT_REGIST;
                event.host = self->host;
                self->session->event_cb(&event, self->session->event_cb_user);
                self->session->event_cb(&event, self->session->event_cb_user);
                event = {};
                event.type = CHIAKI_EVENT_QUIT;
                event.quit.reason = CHIAKI_QUIT_REASON_STOPPED;
                self->session->event_cb(&event, self->session->event_cb_user);
                ++*self->count;
                return nullptr;
            }
        } producer{nullptr, &completion, &producers};
        producer.host.target = CHIAKI_TARGET_PS5_1;
        if (!ps5) producer.host.target = CHIAKI_TARGET_PS4_10;
        memcpy(producer.host.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(producer.host.server_nickname, "Office");
        if (outcome == "mac") producer.host.server_mac[5] = 0x66;
        if (!ps5) strcpy(producer.host.server_nickname, "Other PS4");
        PsnRequests transport;
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [&](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = static_cast<ChiakiHolepunchDeviceInfo *>(calloc(1, sizeof(ChiakiHolepunchDeviceInfo)));
            *count = 1;
            strcpy((*devices)[0].device_name, ps5 ? "Office" : "Other PS5");
            memset((*devices)[0].device_uid, deviceUid.load(), sizeof((*devices)[0].device_uid));
            (*devices)[0].remoteplay_enabled = true;
            return CHIAKI_ERR_SUCCESS;
        };
        transport.connect = [&](StreamSession *, const QString &duid, bool requestedPs5) {
            const auto expected = ps5 ? QStringLiteral("1212121212121212121212121212121212121212121212121212121212121212")
                : QStringLiteral("4141414141414141414141414141414141414141414141414141414141414141");
            if (duid != expected || requestedPs5 != ps5) return CHIAKI_ERR_INVALID_DATA;
            ++connects;
            worker.acquire();
            return workerResult.load();
        };
        transport.startSession = [&](ChiakiSession *session) {
            ++starts;
            if (!session->auto_regist) return CHIAKI_ERR_CANCELED;
            producer.session = session;
            return chiaki_thread_create(&session->session_thread, Producer::run, &producer);
        };
        RegistrationTransport manual;
        QmlNativeServices services(&settings, transport, {}, manual.boundary(), controlledDiscovery());
        auto *backend = services.backend();
        backend->stopAutoConnect();
        ApplicationController controller(&services);
        const auto unblock = qScopeGuard([&] { worker.release(100); completion.release(100); });
        auto *setup = controller.setupWindow();
        setup->resize(1200, 800);
        setup->show();
        DiscoveryHost host = {};
        host.ps5 = ps5;
        host.target = ps5 ? CHIAKI_TARGET_PS5_1 : CHIAKI_TARGET_PS4_10;
        host.host_name = "Office";
        host.host_id = "001122334455";
        host.host_addr = "192.0.2.10";
        host.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
        discover(backend, {host});
        QTRY_COMPARE(script(setup->view(), "document.querySelectorAll('.console-card').length").toInt(), 1);
        QTRY_VERIFY(script(setup->view(), "document.querySelector('[data-automatic-register]')?.disabled === false").toBool());
        const auto initialKey = setup->bridge()->snapshot().value("selectedKey").toString();
        const auto initialProfile = setup->bridge()->snapshot().value("profileGeneration").toString();
        QString manualContext;
        if (outcome == "concurrent")
            manualContext = setupRequest(setup->view(), "openRegistration", {initialKey, initialProfile}).value("context").toString();
        if (outcome == "context") {
            script(setup->view(), "document.querySelector('[data-register-selected]').click()");
            QTRY_COMPARE(script(setup->view(), "document.querySelector('[data-registration-address]')?.value").toString(), QStringLiteral("192.0.2.10"));
            QVERIFY(fill(setup->view(), "[data-registration-account]", "AQAAAAAAAAA="));
            QVERIFY(fill(setup->view(), "[data-registration-pin]", "00123456"));
            host.host_addr = "192.0.2.99";
            discover(backend, {host});
            QTRY_COMPARE(backend->hosts().first().toMap().value("address").toString(), QStringLiteral("192.0.2.99"));
            script(setup->view(), "document.querySelector('[data-registration-submit]').click()");
            QTRY_VERIFY(script(setup->view(), "document.querySelector('.pairing').textContent.includes('The console or profile can have changed.')").toBool());
            QCOMPARE(manual.requests.size(), 0);
            script(setup->view(), "document.querySelector('[data-registration-close]').click()");
            host.host_addr = "192.0.2.10";
            discover(backend, {host, host});
            QTRY_COMPARE(script(setup->view(), "document.querySelectorAll('.console-card:disabled').length").toInt(), 2);
            QVERIFY(!setupRequest(setup->view(), "openRegistration", {initialKey, initialProfile}).value("accepted").toBool());
            QVERIFY(!setupRequest(setup->view(), "autoRegisterConsole", {initialKey, initialProfile}).value("accepted").toBool());
            discover(backend, {host});
            QTRY_VERIFY(script(setup->view(), "document.querySelector('[data-register-selected]') !== null").toBool());
            script(setup->view(), "document.querySelector('[data-register-selected]').click()");
            QTRY_VERIFY(script(setup->view(), "document.querySelector('[data-registration-form]') !== null").toBool());
            QVERIFY(fill(setup->view(), "[data-registration-account]", "AQAAAAAAAAA="));
            QVERIFY(fill(setup->view(), "[data-registration-pin]", "00123456"));
            script(setup->view(), "document.querySelector('[data-registration-submit]').click()");
            QTRY_COMPARE(manual.requests.size(), 1);
            const auto request = manual.requests.first();
            ChiakiRegistEvent event{CHIAKI_REGIST_EVENT_TYPE_FINISHED_SUCCESS, &producer.host};
            request.callback(&event, request.user);
            QTRY_COMPARE(settings.GetRegisteredHosts().size(), 1);
            QTRY_VERIFY(script(setup->view(), "document.querySelector('.pairing').textContent.includes('Console registered')").toBool());
            discover(backend, {});
            backend->refreshPsnToken();
            QTRY_VERIFY(!setup->bridge()->snapshot().value("consoles").toList().isEmpty());
            QTRY_COMPARE(setup->bridge()->snapshot().value("consoles").toList().first().toMap().value("route").toString(), QStringLiteral("psn"));
            const auto remoteKey = setup->bridge()->snapshot().value("selectedKey").toString();
            QVERIFY(!setupRequest(setup->view(), "openRegistration", {remoteKey, initialProfile}).value("accepted").toBool());
            QVERIFY(!setupRequest(setup->view(), "autoRegisterConsole", {remoteKey, initialProfile}).value("accepted").toBool());
            backend->qmlSettings()->setCurrentProfile(profile + "-next");
            QVERIFY(!setupRequest(setup->view(), "openRegistration", {remoteKey, initialProfile}).value("accepted").toBool());
            QVERIFY(!setupRequest(setup->view(), "autoRegisterConsole", {remoteKey, initialProfile}).value("accepted").toBool());
            QVERIFY(!backend->qmlSession() && !backend->qmlWindow());
            QCOMPARE(connects.load(), 0);
            QCOMPARE(starts.load(), 0);
            return;
        }
        QSignalSpy saved(&settings, &Settings::RegisteredHostsUpdated);
        script(setup->view(), "document.querySelector('[data-automatic-register]').click();document.querySelector('[data-automatic-register]').click()");
        QTRY_COMPARE(connects.load(), 1);
        QPointer<QmlMainWindow> native = backend->qmlWindow();
        QPointer<StreamSession> session = backend->qmlSession();
        QVERIFY(native && session);
        QVERIFY(!native->isVisible());
        QVERIFY(!nativeObject("connectionRoute"));
        QVERIFY(setup->isVisible());
        QVERIFY(backend->connectionAttempt().value("registrationOnly").toBool());
        const auto oldAttempt = backend->connectionAttempt().value("id").toString();
        const auto key = setup->bridge()->snapshot().value("selectedKey").toString();
        const auto generation = setup->bridge()->snapshot().value("profileGeneration").toString();
        QVERIFY(!setup->bridge()->autoRegisterConsole(key, generation).value("accepted").toBool());
        if (outcome == "concurrent") {
            QVERIFY(!manualContext.isEmpty());
            QVERIFY(!setupRequest(setup->view(), "submitRegistration", {manualContext,
                QJsonObject{{"address", "192.0.2.10"}, {"account", "AQAAAAAAAAA="}, {"pin", "00123456"}, {"consolePin", ""}, {"target", 1000100}}}).value("accepted").toBool());
            QCOMPARE(manual.requests.size(), 0);
        }
        if (outcome == "cancel" || outcome == "profile") {
            if (outcome == "profile") backend->qmlSettings()->setCurrentProfile(profile + "-next");
            else script(setup->view(), "document.querySelector('[data-connection-cancel]').click()");
            QTRY_VERIFY(backend->connectionAttempt().value("phase") == "cancelling" || backend->connectionAttempt().value("phase") == "failure");
            worker.release();
            QTRY_VERIFY(!session);
            QTRY_VERIFY(!native);
            QCOMPARE(starts.load(), 0);
            QCOMPARE(saved.size(), 0);
            QVERIFY(settings.GetRegisteredHosts().isEmpty());
            QVERIFY(backend->nativeSettings()->GetRegisteredHosts().isEmpty());
            QVERIFY(setup->isVisible());
            QCOMPARE(manual.requests.size(), 0);
            return;
        }
        worker.release();
        if (outcome == "retry") {
            QTRY_VERIFY(!session);
            QTRY_VERIFY(!native);
            QTRY_VERIFY(script(setup->view(), "document.querySelector('[data-connection-retry]')?.disabled === false").toBool());
            QVERIFY(script(setup->view(), "document.querySelector('[data-psn-connection]').textContent.includes('Console is unreachable')").toBool());
            workerResult = CHIAKI_ERR_SUCCESS;
            script(setup->view(), "document.querySelector('[data-connection-retry]').click();document.querySelector('[data-connection-retry]').click()");
            QTRY_COMPARE(connects.load(), 2);
            session = backend->qmlSession();
            native = backend->qmlWindow();
            QVERIFY(session && native && !native->isVisible());
            QVERIFY(backend->connectionAttempt().value("id").toString() != oldAttempt);
            QVERIFY(!setup->bridge()->cancelConnection(oldAttempt).value("accepted").toBool());
            worker.release();
        }
        QTRY_COMPARE(starts.load(), 1);
        QTRY_VERIFY(script(setup->view(), "document.querySelector('[data-connection-cancel]')?.disabled === true").toBool());
        QTRY_COMPARE(script(setup->view(), "document.querySelector('[data-registration-phase]')?.textContent").toString(), QStringLiteral("Registering console"));
        QVERIFY(!setup->bridge()->cancelConnection(backend->connectionAttempt().value("id").toString()).value("accepted").toBool());
        if (outcome == "removed") {
            discover(backend, {});
            QTRY_VERIFY(!backend->connectionAttempt().value("targetValid").toBool());
        } else if (outcome == "duid") {
            deviceUid = 0x34;
            backend->refreshPsnToken();
            QTRY_COMPARE(backend->hosts().first().toMap().value("duid").toString(), QStringLiteral("3434343434343434343434343434343434343434343434343434343434343434"));
        }
        completion.release();
        QTRY_VERIFY(!session);
        QTRY_VERIFY(!native);
        QTRY_COMPARE(producers.load(), 1);
        if (outcome == "removed" || outcome == "duid" || outcome == "mac" || outcome == "ps4") {
            QCOMPARE(saved.size(), 0);
            QVERIFY(settings.GetRegisteredHosts().isEmpty());
            if (outcome == "ps4")
                QTRY_VERIFY(script(setup->view(), "document.querySelector('[data-psn-connection]')?.textContent.includes('PS4 Console Not Main')").toBool());
            QVERIFY(setup->isVisible());
            QCOMPARE(manual.requests.size(), 0);
            return;
        }
        QTRY_VERIFY(script(setup->view(), "document.querySelector('[data-psn-connection]')?.textContent.includes('Console registered')").toBool());
        QCOMPARE(settings.GetRegisteredHosts().size(), 1);
        QCOMPARE(settings.GetRegisteredHosts().first().GetServerNickname(), QStringLiteral("Office"));
        QCOMPARE(saved.size(), 1);
        Settings reopened(profile);
        QCOMPARE(reopened.GetRegisteredHosts().size(), 1);
        QCOMPARE(settings.GetStreamGeometry(), geometry);
        QCOMPARE(requests.size(), 0);
        QVERIFY(!backend->connectionAttempt().value("cleanupPending").toBool());
        QVERIFY(setup->isVisible());
        script(setup->view(), "document.querySelector('[data-connection-back]').click()");
        QTRY_VERIFY(script(setup->view(), "!document.querySelector('[data-psn-connection]')").toBool());
        QCOMPARE(connects.load(), outcome == "retry" ? 2 : 1);
        QCOMPARE(starts.load(), 1);
        QCOMPARE(manual.requests.size(), 0);
        if (outcome == "success") {
            QTRY_VERIFY(script(setup->view(), "document.querySelector('.connect-console')?.disabled === false").toBool());
            script(setup->view(), "document.querySelector('.connect-console').click()");
            QTRY_VERIFY(backend->qmlWindow());
            QPointer<QmlMainWindow> playback = backend->qmlWindow();
            QTRY_VERIFY(playback->isVisible());
            QTRY_COMPARE(starts.load(), 2);
            QTRY_COMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("failure"));
            QTRY_VERIFY(!setup->isVisible());
            QVERIFY(nativeObject("connectionRoute"));
            playback->dismissStreamView();
            QTRY_VERIFY(!playback);
            QTRY_VERIFY(setup->isVisible());
            QVERIFY(!backend->qmlSession());
        }
    }

    void psnRemoteRefreshOwnership()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            for (const auto &name : {profile, profile + "-next"})
                QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + name).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetPsnAuthToken("list-access-secret");
        settings.SetPsnRefreshToken("list-refresh-secret");
        settings.SetPsnAccountId("AQAAAAAAAAA=");
        settings.SetPsnAuthTokenExpiry(QDateTime::currentDateTime().addSecs(3600).toString(settings.GetTimeFormat()));
        for (int i = 0; i < 2; ++i) {
            ChiakiRegisteredHost registered = {};
            registered.target = CHIAKI_TARGET_PS5_1;
            memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
            registered.server_mac[5] += i;
            strcpy(registered.server_nickname, i ? "Remote lounge" : "Remote office");
            settings.AddRegisteredHost(RegisteredHost(registered));
        }
        QSemaphore release;
        std::atomic<int> calls = 0;
        std::atomic<bool> fail = false;
        QList<QPointer<PsnReply>> requests;
        PsnRequests transport;
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [&](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            ++calls;
            release.acquire();
            *devices = nullptr;
            *count = 0;
            if (fail) return CHIAKI_ERR_NETWORK;
            *devices = static_cast<ChiakiHolepunchDeviceInfo *>(calloc(4, sizeof(ChiakiHolepunchDeviceInfo)));
            *count = 4;
            for (int i = 0; i < 4; ++i) {
                strcpy((*devices)[i].device_name, i == 0 ? "Remote office" : i == 1 ? "Remote lounge" : "Unregistered");
                memset((*devices)[i].device_uid, i + 1, sizeof((*devices)[i].device_uid));
                (*devices)[i].remoteplay_enabled = i != 3;
            }
            return CHIAKI_ERR_SUCCESS;
        };
        QmlNativeServices services(&settings, transport, {}, {}, controlledDiscovery());
        const auto unblock = qScopeGuard([&] { release.release(100); });
        auto *backend = services.backend();
        backend->stopAutoConnect();
        SetupWindow setup(backend);
        setup.show();
        QTRY_COMPARE(calls.load(), 1);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-refresh-status]')?.textContent").toString(), QStringLiteral("Finding remote consoles"));
        QVERIFY(script(setup.view(), "document.querySelector('[data-psn-refresh]').disabled").toBool());
        release.release();
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 2);
        script(setup.view(), "[...document.querySelectorAll('.console-card')].find(e=>e.textContent.includes('Remote lounge')).click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.console-card.selected .console-name').textContent").toString(), QStringLiteral("Remote lounge"));
        const auto selected = setup.bridge()->snapshot().value("selectedKey");
        script(setup.view(), "document.querySelector('[data-psn-refresh]').click();document.querySelector('[data-psn-refresh]').click()");
        QTRY_COMPARE(calls.load(), 2);
        QCOMPARE(setup.bridge()->snapshot().value("selectedKey"), selected);
        release.release();
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-refresh-status]').textContent").toString(), QStringLiteral("Remote consoles updated"));
        QCOMPARE(setup.bridge()->snapshot().value("selectedKey"), selected);
        DiscoveryHost local = {};
        local.ps5 = true;
        local.host_name = "Remote office";
        local.host_id = "001122334455";
        local.host_addr = "192.0.2.1";
        local.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
        discover(backend, {local});
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 2);
        QTRY_COMPARE(script(setup.view(), "[...document.querySelectorAll('.console-card')].filter(e=>e.textContent.includes('Remote office')).length").toInt(), 1);
        QCOMPARE(setup.bridge()->snapshot().value("selectedKey"), selected);
        fail = true;
        script(setup.view(), "document.querySelector('[data-psn-refresh]').click()");
        QTRY_COMPARE(calls.load(), 3);
        release.release(2);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-refresh-status]').textContent").toString(), QStringLiteral("Could not refresh remote consoles. Check your connection and try again."));
        QCOMPARE(calls.load(), 4);
        QCOMPARE(setup.bridge()->snapshot().value("selectedKey"), selected);
        QCOMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 2);
        const auto oldProfile = setup.bridge()->snapshot().value("profileGeneration").toString();
        fail = false;
        script(setup.view(), "document.querySelector('[data-psn-refresh]').click()");
        QTRY_COMPARE(calls.load(), 5);
        backend->qmlSettings()->setCurrentProfile(profile + "-next");
        backend->setDiscoveryEnabled(false);
        discover(backend, {});
        release.release();
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 0);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-refresh-status]').textContent").toString(), QStringLiteral("Sign in to PSN before refreshing remote consoles."));
        QVERIFY(!setup.bridge()->selectConsole(selected.toString()).value("accepted").toBool());
        QVERIFY(!setup.bridge()->refreshPsnConsoles(oldProfile).value("accepted").toBool());
        QCOMPARE(requests.size(), 0);
    }

    void psnRemoteTokenConnection()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            for (const auto &name : {profile, profile + "-next"})
                QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + name).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetPsnAuthToken("connection-access-secret");
        settings.SetPsnRefreshToken("connection-refresh-secret");
        settings.SetPsnAccountId("AQAAAAAAAAA=");
        settings.SetPsnAuthTokenExpiry(QDateTime::currentDateTime().addSecs(3600).toString(settings.GetTimeFormat()));
        ChiakiRegisteredHost registered = {};
        registered.target = CHIAKI_TARGET_PS5_1;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "Remote office");
        settings.AddRegisteredHost(RegisteredHost(registered));
        QList<QPointer<PsnReply>> requests;
        PsnRequests transport;
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = static_cast<ChiakiHolepunchDeviceInfo *>(calloc(1, sizeof(ChiakiHolepunchDeviceInfo)));
            *count = 1;
            strcpy((*devices)[0].device_name, "Remote office");
            memset((*devices)[0].device_uid, 0x12, sizeof((*devices)[0].device_uid));
            (*devices)[0].remoteplay_enabled = true;
            return CHIAKI_ERR_SUCCESS;
        };
        QmlNativeServices services(&settings, transport, {}, {}, controlledDiscovery());
        auto *backend = services.backend();
        backend->stopAutoConnect();
        SetupWindow setup(backend);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.connect-console')?.disabled === false").toBool());
        const auto key = setup.bridge()->snapshot().value("selectedKey").toString();
        backend->qmlSettings()->setPsnAuthTokenExpiry(QDateTime::currentDateTime().addSecs(-60).toString(settings.GetTimeFormat()));
        script(setup.view(), "document.querySelector('.connect-console').click();document.querySelector('.connect-console').click()");
        QTRY_COMPARE(requests.size(), 1);
        const auto first = backend->connectionAttempt().value("id").toString();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-connection-cancel]')?.disabled === false").toBool());
        QVERIFY(!setup.bridge()->connectConsole(key).value("accepted").toBool());
        requests[0]->finish({}, QNetworkReply::AuthenticationRequiredError);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-connection-retry]')?.disabled === false").toBool());
        QCOMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("failure"));
        QVERIFY(script(setup.view(), "document.querySelector('[data-psn-connection]').textContent.includes('PSN credentials expired. Sign in again.')").toBool());
        setup.resize(390, 720);
        QTRY_VERIFY(script(setup.view(), "document.documentElement.scrollWidth <= innerWidth").toBool());
        if (qEnvironmentVariableIsSet("CHIAKI_EMBEDDED_EVIDENCE"))
            QVERIFY(setup.view()->grab().save(QDir(qEnvironmentVariable("CHIAKI_EMBEDDED_EVIDENCE")).filePath("remote-connection-error-390.png")));
        script(setup.view(), "document.querySelector('[data-connection-retry]').click()");
        QTRY_COMPARE(requests.size(), 2);
        QVERIFY(backend->connectionAttempt().value("id").toString() != first);
        QVERIFY(!setup.bridge()->cancelConnection(first).value("accepted").toBool());
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-connection-cancel]') !== null").toBool());
        script(setup.view(), "document.querySelector('[data-connection-cancel]').click()");
        QTRY_COMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("cancelled"));
        QVERIFY(!requests[1]);
        QVERIFY(!backend->connectionAttempt().value("cleanupPending").toBool());
        QVERIFY(!backend->qmlSession());
        script(setup.view(), "document.querySelector('[data-connection-back]').click()");
        QTRY_VERIFY(script(setup.view(), "!document.querySelector('[data-psn-connection]')").toBool());
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.connect-console').disabled === false").toBool());
        script(setup.view(), "document.querySelector('.connect-console').click()");
        QTRY_COMPARE(requests.size(), 3);
        backend->qmlSettings()->setCurrentProfile(profile + "-next");
        backend->setDiscoveryEnabled(false);
        QVERIFY(!requests[2]);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-connection-retry]')?.disabled === true").toBool());
        QVERIFY(!setup.bridge()->connectConsole(key).value("accepted").toBool());
        QVERIFY(!backend->qmlSession());
        const auto snapshot = QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson();
        QVERIFY(!snapshot.contains("connection-access-secret"));
        QVERIFY(!snapshot.contains("connection-refresh-secret"));
        QVERIFY(!snapshot.contains("credential-bearing-network-error"));
    }

    void psnRemoteRefresh()
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
        settings.SetPsnAuthToken("refresh-access-secret");
        settings.SetPsnRefreshToken("refresh-secret");
        settings.SetPsnAccountId("AQAAAAAAAAA=");
        settings.SetPsnAuthTokenExpiry(QDateTime::currentDateTime().addSecs(-60).toString(settings.GetTimeFormat()));
        ChiakiRegisteredHost registered = {};
        registered.target = CHIAKI_TARGET_PS5_1;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "Remote office");
        settings.AddRegisteredHost(RegisteredHost(registered));
        QList<QPointer<PsnReply>> requests;
        PsnRequests transport;
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = static_cast<ChiakiHolepunchDeviceInfo *>(calloc(1, sizeof(ChiakiHolepunchDeviceInfo)));
            *count = 1;
            strcpy((*devices)[0].device_name, "Remote office");
            memset((*devices)[0].device_uid, 0x12, sizeof((*devices)[0].device_uid));
            (*devices)[0].remoteplay_enabled = true;
            return CHIAKI_ERR_SUCCESS;
        };
        QmlNativeServices services(&settings, transport, {}, {}, controlledDiscovery());
        auto *backend = services.backend();
        backend->stopAutoConnect();
        SetupWindow setup(backend);
        setup.show();
        QTRY_COMPARE(requests.size(), 1);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-psn-refresh]') !== null").toBool());
        QVERIFY(script(setup.view(), "document.querySelector('[data-psn-refresh]').disabled").toBool());
        requests[0]->finish({}, QNetworkReply::AuthenticationRequiredError);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-refresh-status]').textContent").toString(), QStringLiteral("PSN credentials expired. Sign in again."));
        QTRY_VERIFY(!script(setup.view(), "document.querySelector('[data-psn-refresh]').disabled").toBool());
        script(setup.view(), "document.querySelector('[data-psn-refresh]').click();document.querySelector('[data-psn-refresh]').click()");
        QTRY_COMPARE(requests.size(), 2);
        requests[1]->finish(R"({"access_token":"renewed-access-secret","refresh_token":"renewed-refresh-secret","expires_in":3600})");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.console-card .console-name')?.textContent").toString(), QStringLiteral("Remote office"));
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-refresh-status]').textContent").toString(), QStringLiteral("Remote consoles updated"));
        QTRY_VERIFY(!script(setup.view(), "document.querySelector('.connect-console').disabled").toBool());
        backend->qmlSettings()->setStreamerMode(true);
        const auto snapshot = QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson();
        QVERIFY(!snapshot.contains("access-secret"));
        QVERIFY(!snapshot.contains("refresh-secret"));
        QVERIFY(!snapshot.contains("12121212"));
        setup.resize(800, 600);
        setup.view()->setZoomFactor(2);
        QTRY_VERIFY(script(setup.view(), "document.documentElement.scrollWidth <= innerWidth").toBool());
        if (qEnvironmentVariableIsSet("CHIAKI_EMBEDDED_EVIDENCE"))
            QVERIFY(setup.view()->grab().save(QDir(qEnvironmentVariable("CHIAKI_EMBEDDED_EVIDENCE")).filePath("remote-refresh-200-percent.png")));
    }

    void keyboardMouseSettings()
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
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings') !== null").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "Array.from(document.querySelectorAll('.category-nav button')).some(e => e.textContent === 'Keyboard and mouse')").toBool());
        script(setup.view(), "Array.from(document.querySelectorAll('.category-nav button')).find(e => e.textContent === 'Keyboard and mouse').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#keyboardEnabled')?.disabled === false").toBool());
#ifndef Q_OS_WIN
        QVERIFY(script(setup.view(), "!document.querySelector('#mouseCameraEnabled')").toBool());
        return;
#endif
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#mouseCameraEnabled')?.disabled === false").toBool());
        QVERIFY(script(setup.view(), "document.querySelector('#mouseHorizontalSensitivity').disabled").toBool());
        script(setup.view(), "document.querySelector('#mouseCameraEnabled').click()");
        QTRY_VERIFY(settings.GetMouseCameraEnabled());
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#mouseHorizontalSensitivity')?.disabled === false").toBool());
        QVERIFY(script(setup.view(), "document.querySelector('#keyboardEnabled').disabled && document.querySelector('#mouseTouchEnabled').disabled").toBool());
        const auto numeric = [&](const QString &key, const QString &value) {
            QTRY_VERIFY(script(setup.view(), QString("document.getElementById('%1')?.disabled === false").arg(key)).toBool());
            script(setup.view(), QString("{const e=document.getElementById('%1');e.value='%2';e.dispatchEvent(new Event('input',{bubbles:true}));e.dispatchEvent(new Event('change',{bubbles:true}));}").arg(key, value));
        };
        numeric("mouseHorizontalSensitivity", "2.37");
        QTRY_COMPARE(settings.GetMouseResponse().horizontalSensitivity, 2.37);
        numeric("mouseVerticalSensitivity", "0.75");
        QTRY_COMPARE(settings.GetMouseResponse().verticalSensitivity, 0.75);
        numeric("mouseDeadZoneCompensation", "20");
        QTRY_COMPARE(settings.GetMouseResponse().deadZoneCompensation, 0.2);
        numeric("mouseResponseExponent", "0.55");
        QTRY_COMPARE(settings.GetMouseResponse().responseExponent, 0.55);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#mouseInvertVertical')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('#mouseInvertVertical').click()");
        QTRY_VERIFY(settings.GetMouseResponse().invertVertical);
        QCOMPARE(services.backend()->qmlSettings()->mouseHorizontalSensitivity(), 2.37);
        QCOMPARE(script(setup.view(), "document.querySelector('#mouseHorizontalSensitivity').min").toString(), QStringLiteral("0.05"));
        QCOMPARE(script(setup.view(), "document.querySelector('#mouseHorizontalSensitivity').max").toString(), QStringLiteral("8"));
        QCOMPARE(script(setup.view(), "document.querySelector('#mouseDeadZoneCompensation').max").toString(), QStringLiteral("50"));
        QCOMPARE(script(setup.view(), "document.querySelector('#mouseResponseExponent').min").toString(), QStringLiteral("0.1"));
        numeric("mouseHorizontalSensitivity", "9");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-keyboard-mouse] [role=status]').textContent").toString(), QStringLiteral("Choose a valid setting value."));
        QCOMPARE(settings.GetMouseResponse().horizontalSensitivity, 2.37);
        services.backend()->qmlSettings()->setMouseHorizontalSensitivity(3.25);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#mouseHorizontalSensitivity').value").toString(), QStringLiteral("3.25"));
        Settings restarted(profile);
        QCOMPARE(restarted.GetMouseResponse().horizontalSensitivity, 3.25);
        QCOMPARE(restarted.GetMouseResponse().verticalSensitivity, 0.75);
        QCOMPARE(restarted.GetMouseResponse().deadZoneCompensation, 0.2);
        QCOMPARE(restarted.GetMouseResponse().responseExponent, 0.55);
        QVERIFY(restarted.GetMouseResponse().invertVertical);
        QVERIFY(restarted.GetMouseCameraEnabled());
        services.backend()->qmlSettings()->changeKeyboardMouseBinding(0, 1, Qt::Key_B);
        const auto changedBindings = settings.GetKeyboardMouseBindings();
        const auto legacyBindings = settings.GetControllerMapping();
        const auto originalKeyboard = settings.GetKeyboardEnabled();
        const auto originalTouch = settings.GetMouseTouchEnabled();
        script(setup.view(), "document.querySelector('#keyboardMouseReset').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('dialog').open").toBool());
        QVERIFY(script(setup.view(), "document.activeElement.closest('dialog') !== null").toBool());
        script(setup.view(), "document.querySelector('[data-keyboard-reset-cancel]').click()");
        QTRY_VERIFY(script(setup.view(), "!document.querySelector('dialog').open && document.activeElement.id === 'keyboardMouseReset'").toBool());
        QCOMPARE(settings.GetMouseResponse().horizontalSensitivity, 3.25);
        QVERIFY(settings.GetKeyboardMouseBindings() == changedBindings);
        script(setup.view(), "Array.from(document.querySelectorAll('[role=tab]')).find(e => e.textContent === 'Bindings').click();document.querySelector('#keyboardMouseReset').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('dialog').open").toBool());
        script(setup.view(), "document.querySelector('[data-keyboard-reset-confirm]').click()");
        QTRY_VERIFY(!settings.GetMouseCameraEnabled());
        QCOMPARE(settings.GetMouseResponse().horizontalSensitivity, 1.0);
        QCOMPARE(settings.GetMouseResponse().verticalSensitivity, 1.0);
        QCOMPARE(settings.GetMouseResponse().deadZoneCompensation, 0.0);
        QCOMPARE(settings.GetMouseResponse().responseExponent, 1.0);
        QVERIFY(!settings.GetMouseResponse().invertVertical);
        QVERIFY(settings.GetKeyboardMouseBindings() == DefaultKeyboardMouseBindings());
        QCOMPARE(settings.GetControllerMapping(), legacyBindings);
        QCOMPARE(settings.GetKeyboardEnabled(), originalKeyboard);
        QCOMPARE(settings.GetMouseTouchEnabled(), originalTouch);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#keyboardMouseBinding0_0').textContent.trim()").toString(), QStringLiteral("PrimaryW"));
        QCOMPARE(services.backend()->qmlSettings()->keyboardMouseBindings().at(0).toMap().value("primary").toInt(), int(Qt::Key_W));
    }

    void keyboardMouseCapture()
    {
#ifndef Q_OS_WIN
        QSKIP("Mouse camera mode is available on Windows.");
#endif
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile + "-next").clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetMouseCameraEnabled(true);
        QmlNativeServices services(&settings);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        SetupWindow setup(backend);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.category-nav') !== null").toBool());
        script(setup.view(), "Array.from(document.querySelectorAll('.category-nav button')).find(e => e.textContent === 'Keyboard and mouse').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#mouseCameraEnabled')?.disabled === false").toBool());
        script(setup.view(), "Array.from(document.querySelectorAll('[role=tab]')).find(e => e.textContent === 'Bindings').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.binding-row button').length").toInt(), 44);
        const auto capture = [&](int action, int slot) -> QDialog * {
            script(setup.view(), QString("document.getElementById('keyboardMouseBinding%1_%2').click()").arg(action).arg(slot));
            QElapsedTimer timeout;
            timeout.start();
            while (timeout.elapsed() < 5000) {
                auto *dialog = setup.findChild<QDialog *>("setupBindingCapture");
                if (dialog && dialog->isVisible()) return dialog;
                QTest::qWait(10);
            }
            return nullptr;
        };
        QPointer<QDialog> dialog = capture(0, 1);
        QVERIFY(dialog);
        auto *prompt = dialog->findChild<QLabel *>("setupBindingPrompt");
        QVERIFY(prompt);
        QTRY_VERIFY(prompt->hasFocus());
        QTest::keyPress(prompt, Qt::Key_B);
        QCOMPARE(settings.GetKeyboardMouseBindings()[0][1], 0);
        QTest::keyRelease(prompt, Qt::Key_B);
        QTRY_VERIFY(dialog.isNull());
        QCOMPARE(settings.GetKeyboardMouseBindings()[0][1], int(Qt::Key_B));
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#keyboardMouseBinding0_1').textContent.trim()").toString(), QStringLiteral("AlternativeB"));
        Settings restarted(profile);
        QCOMPARE(restarted.GetKeyboardMouseBindings()[0][1], int(Qt::Key_B));

        dialog = capture(0, 1);
        QVERIFY(dialog);
        prompt = dialog->findChild<QLabel *>("setupBindingPrompt");
        QTest::keyClick(prompt, Qt::Key_F8);
        QVERIFY(dialog->isVisible());
        QVERIFY(dialog->findChild<QLabel *>("setupBindingStatus")->text().contains("F8"));
        QCOMPARE(settings.GetKeyboardMouseBindings()[0][1], int(Qt::Key_B));
        QTest::keyClick(prompt, Qt::Key_Enter);
        QVERIFY(dialog->findChild<QLabel *>("setupBindingStatus")->text().contains("Options"));
        QTest::mouseClick(dialog->findChild<QPushButton *>("setupBindingMove"), Qt::LeftButton);
        QTRY_VERIFY(dialog.isNull());
        QCOMPARE(settings.GetKeyboardMouseBindings()[0][1], int(Qt::Key_Return));
        QCOMPARE(settings.GetKeyboardMouseBindings()[18][0], 0);

        dialog = capture(1, 1);
        QVERIFY(dialog);
        QTest::mouseClick(dialog->findChild<QLabel *>("setupBindingPrompt"), Qt::MiddleButton);
        QTRY_VERIFY(dialog.isNull());
        QCOMPARE(settings.GetKeyboardMouseBindings()[1][1], -int(Qt::MiddleButton));
        dialog = capture(1, 1);
        QVERIFY(dialog);
        QTest::mouseClick(dialog->findChild<QPushButton *>("setupBindingClear"), Qt::LeftButton);
        QTRY_VERIFY(dialog.isNull());
        QCOMPARE(settings.GetKeyboardMouseBindings()[1][1], 0);
        dialog = capture(1, 1);
        QVERIFY(dialog);
        QTest::mouseClick(dialog->findChild<QPushButton *>("setupBindingTab"), Qt::LeftButton);
        QVERIFY(dialog->findChild<QLabel *>("setupBindingStatus")->text().contains("Touchpad"));
        QTest::mouseClick(dialog->findChild<QPushButton *>("setupBindingMove"), Qt::LeftButton);
        QTRY_VERIFY(dialog.isNull());
        QCOMPARE(settings.GetKeyboardMouseBindings()[1][1], int(Qt::Key_Tab));
        QCOMPARE(settings.GetKeyboardMouseBindings()[20][0], 0);

        dialog = capture(2, 1);
        QVERIFY(dialog);
        prompt = dialog->findChild<QLabel *>("setupBindingPrompt");
        QKeyEvent press(QEvent::KeyPress, Qt::Key_Control, Qt::ControlModifier, 29, 0, 0);
        QKeyEvent otherRelease(QEvent::KeyRelease, Qt::Key_Control, Qt::NoModifier, 285, 0, 0);
        QApplication::sendEvent(prompt, &press);
        QApplication::sendEvent(prompt, &otherRelease);
        QCOMPARE(settings.GetKeyboardMouseBindings()[2][1], 0);
        QTest::keyClick(prompt, Qt::Key_Escape);
        QTRY_VERIFY(dialog.isNull());
        QCOMPARE(settings.GetKeyboardMouseBindings()[2][1], 0);
        dialog = capture(2, 1);
        QVERIFY(dialog);
        setup.suspend();
        QTRY_VERIFY(dialog.isNull());
        setup.resume();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#keyboardMouseBinding2_1')?.disabled === false").toBool());
        dialog = capture(2, 1);
        QVERIFY(dialog);
        backend->qmlSettings()->setCurrentProfile(profile + "-next");
        QTRY_VERIFY(dialog.isNull());
        QVERIFY(backend->nativeSettings()->GetKeyboardMouseBindings() == DefaultKeyboardMouseBindings());
    }

    void keyboardMouseGuards()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto cleanup = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile + "-next").clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetMouseCameraEnabled(true);
        QmlNativeServices services(&settings);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        SetupWindow setup(backend);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.category-nav') !== null").toBool());
        script(setup.view(), "Array.from(document.querySelectorAll('.category-nav button')).find(e => e.textContent === 'Keyboard and mouse').click()");
        QTRY_VERIFY(script(setup.view(), "(document.querySelector('#keyboardMouseReset') ?? document.querySelector('#keyboardEnabled'))?.disabled === false").toBool());
        const QString generation = setup.bridge()->snapshot().value("profileGeneration").toString();
        const auto begun = setupRequest(setup.view(), "beginKeyboardMouse", {generation});
        QVERIFY(begun.value("accepted").toBool());
        const auto form = begun.value("form").toString();
        QVERIFY(!form.isEmpty());
        for (const auto &input : QList<QPair<QString, QVariant>>{{"mouseHorizontalSensitivity", -0.05}, {"mouseHorizontalSensitivity", 8.01},
                {"mouseVerticalSensitivity", 0}, {"mouseDeadZoneCompensation", -0.01}, {"mouseDeadZoneCompensation", 0.51},
                {"mouseResponseExponent", 0.09}, {"mouseResponseExponent", 3.01}, {"mouseCameraEnabled", 1}, {"mouseInvertVertical", "true"},
                {"mouseHorizontalSensitivity", "2"}, {"psnAuthToken", "forbidden"}, {"keyboardEnabled", 1}, {"mouseTouchEnabled", 1}}) {
            const auto reply = setupRequest(setup.view(), "setKeyboardMouseSetting", {input.first, QJsonValue::fromVariant(input.second), form, generation});
            QVERIFY(reply.contains("accepted"));
            QVERIFY(!reply.value("accepted").toBool());
        }
        for (const auto &args : QList<QJsonArray>{{-1, 0, false, form, generation}, {0.5, 0, false, form, generation},
                 {22, 0, false, form, generation}, {0, 2, false, form, generation}, {0, 0.5, false, form, generation},
                 {0, 0, true, form, generation}, {0, 0, false, "obsolete", generation}}) {
            const auto reply = setupRequest(setup.view(), "captureKeyboardMouse", args);
            QVERIFY(reply.contains("accepted"));
            QVERIFY(!reply.value("accepted").toBool());
        }
#ifdef Q_OS_WIN
        for (const auto &input : QList<QPair<QString, double>>{{"mouseHorizontalSensitivity", 0.05}, {"mouseHorizontalSensitivity", 8},
                {"mouseVerticalSensitivity", 0.05}, {"mouseVerticalSensitivity", 8}, {"mouseDeadZoneCompensation", 0},
                {"mouseDeadZoneCompensation", 0.5}, {"mouseResponseExponent", 0.1}, {"mouseResponseExponent", 3}}) {
            const auto reply = setupRequest(setup.view(), "setKeyboardMouseSetting", {input.first, input.second, form, generation});
            QVERIFY(reply.value("accepted").toBool());
        }
        QCOMPARE(settings.GetMouseResponse().horizontalSensitivity, 8.0);
        QCOMPARE(settings.GetMouseResponse().verticalSensitivity, 8.0);
        QCOMPARE(settings.GetMouseResponse().deadZoneCompensation, 0.5);
        QCOMPARE(settings.GetMouseResponse().responseExponent, 3.0);
#endif
        const auto nextForm = setupRequest(setup.view(), "beginKeyboardMouse", {generation}).value("form").toString();
        QVERIFY(!nextForm.isEmpty() && nextForm != form);
        QVERIFY(setupRequest(setup.view(), "endKeyboardMouse", {form}).value("accepted").toBool());
        QVERIFY(!setupRequest(setup.view(), "resetKeyboardMouse", {form, generation}).value("accepted").toBool());
#ifdef Q_OS_WIN
        QVERIFY(setupRequest(setup.view(), "setKeyboardMouseSetting", {"mouseHorizontalSensitivity", 2, nextForm, generation}).value("accepted").toBool());
        QCOMPARE(settings.GetMouseResponse().horizontalSensitivity, 2.0);
#endif
        backend->qmlSettings()->setCurrentProfile(profile + "-next");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#keyboardEnabled')?.disabled === false").toBool());
        QVERIFY(!setupRequest(setup.view(), "resetKeyboardMouse", {nextForm, generation}).value("accepted").toBool());
        QVERIFY(!setupRequest(setup.view(), "setKeyboardMouseSetting", {"mouseCameraEnabled", true, nextForm, generation}).value("accepted").toBool());
        QVERIFY(!setupRequest(setup.view(), "captureKeyboardMouse", {0, 0, false, nextForm, generation}).value("accepted").toBool());
        QVERIFY(!backend->nativeSettings()->GetMouseCameraEnabled());
        QVERIFY(backend->nativeSettings()->GetKeyboardMouseBindings() == DefaultKeyboardMouseBindings());
        QCOMPARE(backend->nativeSettings()->GetMouseResponse().horizontalSensitivity, 1.0);
    }

    void keyboardMouseLegacyAndLifetime()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto cleanup = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile + "-next").clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        Settings next(profile + "-next");
        next.SetDiscoveryEnabled(false);
        next.SetMouseCameraEnabled(true);
        MouseResponse nextResponse;
        nextResponse.horizontalSensitivity = 2.25;
        QVERIFY(next.SetMouseResponse(nextResponse));
        QmlNativeServices services(&settings);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        SetupWindow setup(backend);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.category-nav') !== null").toBool());
        script(setup.view(), "Array.from(document.querySelectorAll('.category-nav button')).find(e => e.textContent === 'Keyboard and mouse').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#keyboardEnabled')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('#keyboardEnabled').click()");
        QTRY_VERIFY(!settings.GetKeyboardEnabled());
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#mouseTouchEnabled')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('#mouseTouchEnabled').click()");
        QTRY_VERIFY(!settings.GetMouseTouchEnabled());
        const int button = settings.GetControllerMapping().firstKey();
        script(setup.view(), QString("document.getElementById('keyMapping%1').click()").arg(button));
        QPointer<QDialog> capture;
        QTRY_VERIFY(capture = setup.findChild<QDialog *>("setupBindingCapture"));
        QSignalSpy mappingChanged(backend->qmlSettings(), &QmlSettings::controllerMappingChanged);
        QTest::keyClick(capture->findChild<QLabel *>("setupBindingPrompt"), Qt::Key_B);
        QTRY_VERIFY(capture.isNull());
        QCOMPARE(settings.GetControllerMapping().value(button), Qt::Key_B);
        QCOMPARE(mappingChanged.count(), 1);
        QTRY_COMPARE(script(setup.view(), QString("document.getElementById('keyMapping%1').textContent").arg(button)).toString(), QStringLiteral("B"));
        Settings reopened(profile);
        QCOMPARE(reopened.GetControllerMapping().value(button), Qt::Key_B);
        script(setup.view(), "document.querySelector('#resetAllKeys').click()");
        QTRY_VERIFY(settings.GetControllerMapping().value(button) != Qt::Key_B);
        script(setup.view(), QString("document.getElementById('keyMapping%1').click()").arg(button));
        QTRY_VERIFY(capture = setup.findChild<QDialog *>("setupBindingCapture"));
        QEvent deactivate(QEvent::WindowDeactivate);
        QApplication::sendEvent(capture, &deactivate);
        QTRY_VERIFY(capture.isNull());
        script(setup.view(), QString("document.getElementById('keyMapping%1').click()").arg(button));
        QTRY_VERIFY(capture = setup.findChild<QDialog *>("setupBindingCapture"));
        script(setup.view(), "document.querySelector('.back-consoles').click()");
        QTRY_VERIFY(capture.isNull());
        QVERIFY(!backend->qmlSession());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.category-nav') !== null").toBool());
        script(setup.view(), "Array.from(document.querySelectorAll('.category-nav button')).find(e => e.textContent === 'Keyboard and mouse').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#keyboardEnabled')?.disabled === false").toBool());
        script(setup.view(), QString("document.getElementById('keyMapping%1').click()").arg(button));
        QTRY_VERIFY(capture = setup.findChild<QDialog *>("setupBindingCapture"));
        setup.view()->reload();
        QTRY_VERIFY(capture.isNull());
#ifdef Q_OS_WIN
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.category-nav') !== null").toBool());
        script(setup.view(), "Array.from(document.querySelectorAll('.category-nav button')).find(e => e.textContent === 'Keyboard and mouse').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#mouseCameraEnabled')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('#mouseCameraEnabled').click()");
        QTRY_VERIFY(settings.GetMouseCameraEnabled());
        backend->qmlSettings()->setMouseHorizontalSensitivity(3.5);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#mouseHorizontalSensitivity').value").toString(), QStringLiteral("3.5"));
        script(setup.view(), "document.querySelector('#keyboardMouseReset').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('dialog').open && document.activeElement.hasAttribute('data-keyboard-reset-cancel')").toBool());
        setup.suspend();
        setup.resume();
        QTRY_VERIFY(script(setup.view(), "!document.querySelector('dialog').open && document.querySelector('#keyboardMouseReset')?.disabled === false").toBool());
        QCOMPARE(settings.GetMouseResponse().horizontalSensitivity, 3.5);
        script(setup.view(), "document.querySelector('#keyboardMouseReset').click();document.querySelector('[data-keyboard-reset-confirm]').click()");
        QTRY_VERIFY(!settings.GetMouseCameraEnabled());
        QCOMPARE(settings.GetMouseResponse().horizontalSensitivity, 1.0);
        QVERIFY(!settings.GetKeyboardEnabled());
        QVERIFY(!settings.GetMouseTouchEnabled());
        script(setup.view(), "document.querySelector('#mouseCameraEnabled').click()");
        QTRY_VERIFY(settings.GetMouseCameraEnabled());
        script(setup.view(), "{const e=document.querySelector('#mouseHorizontalSensitivity');window.oldMouseDraft=e;e.value='7.25';e.dispatchEvent(new Event('input',{bubbles:true}));}document.querySelector('#keyboardMouseReset').click();window.oldMouseReset=document.querySelector('[data-keyboard-reset-confirm]');");
        backend->qmlSettings()->setCurrentProfile(profile + "-next");
        QTRY_VERIFY(script(setup.view(), "!document.querySelector('dialog').open").toBool());
        script(setup.view(), "window.oldMouseDraft.dispatchEvent(new Event('change',{bubbles:true}));window.oldMouseReset.click()");
        QTest::qWait(50);
        QVERIFY(backend->nativeSettings()->GetMouseCameraEnabled());
        QCOMPARE(backend->nativeSettings()->GetMouseResponse().horizontalSensitivity, 2.25);
#endif
    }

    void keyboardMousePresentation()
    {
#ifndef Q_OS_WIN
        QSKIP("Mouse camera mode is available on Windows.");
#endif
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto cleanup = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetMouseCameraEnabled(true);
        QmlNativeServices services(&settings);
        services.backend()->stopAutoConnect();
        SetupWindow setup(services.backend());
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.category-nav') !== null").toBool());
        script(setup.view(), "{const e=document.querySelector('.settings-search input');e.value='Cross Primary';e.dispatchEvent(new Event('input',{bubbles:true}));}");
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.settings-results button').length").toInt(), 1);
        script(setup.view(), "document.querySelector('.settings-results button').click()");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("keyboardMouseBinding4_0"));
        QVERIFY(!setup.findChild<QDialog *>("setupBindingCapture"));
        QVERIFY(script(setup.view(), "document.querySelector('[role=tab][aria-selected=true]').textContent === 'Bindings'").toBool());
        script(setup.view(), "document.querySelector('[role=tab][aria-selected=true]').dispatchEvent(new KeyboardEvent('keydown',{key:'ArrowLeft',bubbles:true,cancelable:true}))");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[role=tab][aria-selected=true]').textContent === 'Mouse response' && document.activeElement.getAttribute('role') === 'tab'").toBool());
        const QDir evidence(qEnvironmentVariable("CHIAKI_KEYBOARD_MOUSE_EVIDENCE", "."));
        const auto screenshot = [&](const QString &name, int width, int height, qreal zoom, bool bindings) {
            setup.resize(width, height);
            setup.view()->setZoomFactor(zoom);
            script(setup.view(), QString("Array.from(document.querySelectorAll('[role=tab]')).find(e => e.textContent === '%1').click()").arg(bindings ? "Bindings" : "Mouse response"));
            script(setup.view(), bindings ? "document.querySelector('[role=tablist]').scrollIntoView({block:'start'})" : "document.querySelector('.settings-content').scrollIntoView({block:'start'})");
            QTRY_VERIFY(script(setup.view(), "document.documentElement.scrollWidth <= innerWidth").toBool());
            QTest::qWait(80);
            QVERIFY(setup.view()->grab().save(evidence.filePath(name)));
        };
        screenshot("response-1280.png", 1280, 900, 1, false);
        screenshot("bindings-390.png", 390, 720, 1, true);
        screenshot("response-200-percent.png", 800, 900, 2, false);
        screenshot("bindings-200-percent.png", 800, 900, 2, true);
        setup.view()->setZoomFactor(1);
        setup.resize(390, 720);
        script(setup.view(), "document.querySelector('#keyboardMouseBinding4_1').click()");
        QPointer<QDialog> capture;
        QTRY_VERIFY(capture = setup.findChild<QDialog *>("setupBindingCapture"));
        QVERIFY(capture->width() <= setup.width());
        const auto before = settings.GetKeyboardMouseBindings();
        script(setup.view(), "window.dispatchEvent(new KeyboardEvent('keydown',{key:'Y',code:'KeyY',bubbles:true}));window.dispatchEvent(new KeyboardEvent('keyup',{key:'Y',code:'KeyY',bubbles:true}));");
        QVERIFY(settings.GetKeyboardMouseBindings() == before);
        QVERIFY(capture->grab().save(evidence.filePath("native-capture-390.png")));
        capture->reject();
        QTRY_VERIFY(capture.isNull());
        script(setup.view(), "document.querySelector('#keyboardMouseReset').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('dialog').open").toBool());
        QVERIFY(setup.view()->grab().save(evidence.filePath("reset-390.png")));
        script(setup.view(), "document.querySelector('dialog').dispatchEvent(new Event('cancel',{cancelable:true}))");
        QTRY_VERIFY(script(setup.view(), "!document.querySelector('dialog').open && document.activeElement.id === 'keyboardMouseReset'").toBool());
    }

    void suspendClosesSignIn()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(true);
        QList<QPointer<PsnReply>> requests;
        PsnRequests transport;
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) {
            *devices = nullptr; *count = 0; return CHIAKI_ERR_SUCCESS;
        };
        QmlNativeServices services(&settings, transport);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        SetupWindow setup(backend);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-psn-account]') !== null").toBool());
        script(setup.view(), "document.querySelector('[data-psn-account]').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-psn-embedded]') !== null").toBool());
        script(setup.view(), "document.querySelector('[data-psn-embedded]').click()");
        QPointer<PsnSignInWindow> signIn;
        QTRY_VERIFY((signIn = setup.findChild<PsnSignInWindow *>("psnSignInWindow")));
        signIn->view()->stop();
        auto *profilePage = signIn->view()->page();
        profilePage->profile()->setUrlRequestInterceptor(new PsnPageRequests(profilePage->profile()));
        profilePage->setUrl(QUrl("https://remoteplay.dl.playstation.net/remoteplay/redirect?code=suspend-fixture"));
        QTRY_COMPARE(requests.size(), 1);
        const auto attempt = backend->psnAuthAttempt().value("id");
        setup.suspend();
        QTRY_VERIFY_WITH_TIMEOUT(!signIn, 2000);
        QTRY_COMPARE(setup.view()->page()->lifecycleState(), QWebEnginePage::LifecycleState::Frozen);
        QCOMPARE(backend->psnAuthAttempt().value("phase").toString(), QStringLiteral("exchanging"));
        requests[0]->finish(R"({"access_token":"suspend-access","refresh_token":"suspend-refresh","expires_in":3600})");
        QTRY_COMPARE(requests.size(), 2);
        requests[1]->finish(R"({"user_id":"1"})");
        QTRY_COMPARE(backend->psnAuthAttempt().value("phase").toString(), QStringLiteral("success"));
        setup.resume();
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-psn-status]')?.textContent").toString(), QStringLiteral("PSN remote access is ready"));
        QCOMPARE(backend->psnAuthAttempt().value("id"), attempt);
        QCOMPARE(requests.size(), 2);
        QCOMPARE(settings.GetPsnAuthToken(), QStringLiteral("suspend-access"));
        QVERIFY(!QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson().contains("suspend-access"));
    }

    void decodedSessionCleanup()
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
        settings.SetHardwareDecoder({});
        settings.SetPlaceboPreset(PlaceboPreset::Fast);
        settings.SetCodecLocalPS5(CHIAKI_CODEC_H264);
        settings.SetCodecRemotePS5(CHIAKI_CODEC_H264);
        settings.SetWindowType(WindowType::AdjustableResolution);
        settings.SetStreamGeometry(QRect(80, 80, 960, 640));
        settings.SetDisconnectAction(DisconnectAction::Ask);
        ChiakiRegisteredHost registered = {};
        registered.target = CHIAKI_TARGET_PS5_1;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "Office console");
        RegisteredHost paired(registered);
        paired.SetConsolePin(paired, {});
        settings.AddRegisteredHost(paired);
        QTcpServer transport;
        QVERIFY(transport.listen(QHostAddress::LocalHost, 9295));
        QFile fixture(QFINDTESTDATA("fixtures/red.h264"));
        QVERIFY(fixture.open(QIODevice::ReadOnly));
        auto packet = fixture.readAll();
        const int packetSize = packet.size();
        packet.append(AV_INPUT_BUFFER_PADDING_SIZE, '\0');
        QmlNativeServices services(&settings);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        auto controller = std::make_unique<ApplicationController>(&services);
        auto *setup = controller->setupWindow();
        setup->show();
        DiscoveryHost host = {};
        host.ps5 = true;
        host.host_name = "Office console";
        host.host_id = "001122334455";
        host.host_addr = "127.0.0.1";
        host.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
        discover(backend, {host});
        for (int pass = 0; pass < 2; ++pass) {
            backend->nativeSettings()->SetRenderBackend(pass ? RenderBackend::Vulkan : RenderBackend::OpenGL);
            QTRY_VERIFY(script(setup->view(), "document.querySelector('.connect-console')?.disabled === false").toBool());
            script(setup->view(), "document.querySelector('.connect-console').focus();document.querySelector('.connect-console').click();document.querySelector('.connect-console').click()");
            QTRY_VERIFY(backend->qmlSession());
            QPointer<StreamSession> session = backend->qmlSession();
            QPointer<QmlMainWindow> native = backend->qmlWindow();
            QVERIFY(native);
            QCOMPARE(native->nativeGraphicsApi(), pass ? QSGRendererInterface::Vulkan : QSGRendererInterface::OpenGL);
            QCOMPARE(native->videoPreset(), QmlMainWindow::VideoPreset::Fast);
            QTRY_VERIFY(!setup->isVisible());
            const auto attempt = backend->connectionAttempt().value("id");
            const auto key = setup->bridge()->snapshot().value("selectedKey").toString();
            QVERIFY(!setup->bridge()->connectConsole(key).value("accepted").toBool());
            QCOMPARE(backend->connectionAttempt().value("id"), attempt);
            ChiakiEvent pin = {};
            pin.type = CHIAKI_EVENT_LOGIN_PIN_REQUEST;
            session->Event(&pin);
            QVERIFY2(!backend->sessionPinRequest().value("id").toString().isEmpty(), "The controlled console must request a native PIN form");
            QTRY_VERIFY(nativeObject("consoleLoginPin"));
            QVERIFY(native->streamInput()->blocked());
            auto *input = nativeObject("consoleLoginPin");
            input->setProperty("text", "0012");
            auto *submit = nativeObject("pinSubmit");
            QVERIFY(submit);
            QVERIFY(QMetaObject::invokeMethod(submit, "clicked"));
            QTRY_COMPARE(backend->sessionPinRequest().value("phase").toString(), QStringLiteral("submitting"));
            QVERIFY(settings.GetRegisteredHost(HostMAC(registered.server_mac)).GetConsolePin().isEmpty());
            pin.login_pin_request.pin_incorrect = true;
            session->Event(&pin);
            QTRY_VERIFY(nativeObject("pinHeading") && nativeObject("pinHeading")->property("text").toString().contains("Incorrect"));
            DecodedBuffers counts;
            const auto finishSession = qScopeGuard([&] { if (backend->qmlSession()) controller.reset(); });
            auto *decoder = session->GetFfmpegDecoder();
            QVERIFY(decoder);
            decoder->codec_context->opaque = &counts;
            decoder->codec_context->get_buffer2 = &DecodedBuffers::allocate;
            QVERIFY(chiaki_ffmpeg_decoder_video_sample_cb(reinterpret_cast<uint8_t *>(packet.data()), packetSize, 0, false, decoder));
            int submittedSamples = 1;
            bool samplesValid = true;
            QTimer samples;
            samples.setInterval(16);
            connect(&samples, &QTimer::timeout, &samples, [&] {
                if (!session) { samples.stop(); return; }
                samplesValid &= chiaki_ffmpeg_decoder_video_sample_cb(reinterpret_cast<uint8_t *>(packet.data()), packetSize, 0, false, decoder);
                ++submittedSamples;
            });
            samples.start();
            QTRY_VERIFY(native->hasVideo());
            ChiakiEvent connected = {};
            connected.type = CHIAKI_EVENT_CONNECTED;
            session->Event(&connected);
            QTRY_COMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("ready"));
            QVERIFY(counts.allocated > counts.released);
            native->menuRequested();
            QTRY_VERIFY(native->streamInput()->blocked());
            QTest::qWait(210);
            native->menuRequested();
            QTRY_VERIFY(!native->streamInput()->blocked());
            samples.stop();
            QVERIFY(samplesValid);
            QVERIFY(submittedSamples >= 10);
            QVERIFY(counts.allocated >= 10);
            QVERIFY(chiaki_ffmpeg_decoder_video_sample_cb(reinterpret_cast<uint8_t *>(packet.data()), packetSize, 0, false, decoder));
            // Keep GUI delivery queued while the production frame worker pulls the decoded sample.
            QThread::msleep(30);
            if (pass) {
                controller.reset();
            } else {
                QVERIFY(!native->close());
                auto *stop = nativeObject("streamDisconnectDialog");
                QVERIFY(stop);
                QTRY_VERIFY(stop->property("opened").toBool());
                auto *dialog = stop->findChild<QObject *>("confirmationDialog");
                QVERIFY(dialog);
                QVERIFY(QMetaObject::invokeMethod(dialog, "choose", Q_ARG(QVariant, QVariant("secondary"))));
                QTRY_VERIFY(!native);
                QTRY_VERIFY(setup->isVisible());
                QVERIFY(backend->discoveryEnabled());
                QTRY_COMPARE(script(setup->view(), "document.querySelector('.discovery').getAttribute('aria-pressed')").toString(), QStringLiteral("true"));
                script(setup->view(), "document.querySelector('.discovery').click()");
                QTRY_VERIFY(!backend->discoveryEnabled());
                discover(backend, {host});
                QTRY_VERIFY(!script(setup->view(), "document.querySelector('.connect-console').disabled").toBool());
                QTRY_COMPARE(script(setup->view(), "document.querySelector('.console-card.selected .console-name')?.textContent").toString(), QStringLiteral("Office console"));
            }
            QVERIFY(!native);
            QVERIFY(!session);
            QVERIFY(!backend->qmlSession());
            QVERIFY(!backend->qmlWindow());
            QCOMPARE(counts.released.load(), counts.allocated.load());
            qInfo() << "Decoded buffer lifetime pass" << pass << "samples" << submittedSamples
                << "allocated/released" << counts.allocated.load() << counts.released.load();
            QTest::qWait(30);
        }
    }

    void nativeInitializationFailure()
    {
        QCOMPARE(QGuiApplication::platformName(), QStringLiteral("offscreen"));
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRenderBackend(RenderBackend::OpenGL);
        ChiakiRegisteredHost registered = {};
        registered.target = CHIAKI_TARGET_PS5_1;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "Office console");
        settings.AddRegisteredHost(RegisteredHost(registered));
        QmlNativeServices services(&settings);
        services.backend()->stopAutoConnect();
        ApplicationController controller(&services);
        auto *setup = controller.setupWindow();
        setup->show();
        DiscoveryHost host = {};
        host.ps5 = true;
        host.host_name = "Office console";
        host.host_id = "001122334455";
        host.host_addr = "127.0.0.1";
        host.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
        discover(services.backend(), {host});
        QTRY_VERIFY(script(setup->view(), "document.querySelector('.connect-console')?.disabled === false").toBool());
        script(setup->view(), "document.querySelector('.connect-console').click()");
        QTRY_VERIFY(script(setup->view(), "document.querySelector('.stream-error')?.textContent.includes('Failed initializing OpenGL backend')").toBool());
        QVERIFY(setup->isVisible());
        QVERIFY(!services.backend()->qmlSession());
        QVERIFY(!services.backend()->qmlWindow());
        script(setup->view(), "document.querySelector('.dismiss-stream-error').click()");
        QTRY_VERIFY(script(setup->view(), "document.querySelector('.stream-error') === null").toBool());
        setup->view()->reload();
        QTRY_VERIFY(script(setup->view(), "document.querySelector('h1')?.textContent === 'Consoles'").toBool());
        QVERIFY(script(setup->view(), "document.querySelector('.stream-error') === null").toBool());
    }

    void localHandoff()
    {
#ifdef Q_OS_WIN
        if (qEnvironmentVariableIsSet("CHIAKI_HANDOFF_REQUIRE_FOREGROUND"))
            QVERIFY2(inputDesktopAvailable(), "Handoff requires the active unlocked input desktop");
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
        settings.SetHardwareDecoder({});
        settings.SetPlaceboPreset(PlaceboPreset::Fast);
        ChiakiRegisteredHost registered = {};
        registered.target = CHIAKI_TARGET_PS5_1;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "Office console");
        settings.AddRegisteredHost(RegisteredHost(registered));
        QmlNativeServices services(&settings);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        ApplicationController controller(&services);
        auto &setup = *controller.setupWindow();
        setup.show();
        DiscoveryHost host = {};
        host.ps5 = true;
        host.host_name = "Office console";
        host.host_id = "001122334455";
        host.host_addr = "127.0.0.1";
        host.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
        discover(backend, {host});
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 1);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.connect-console')?.textContent.trim()").toString(), QStringLiteral("Connect"));
        script(setup.view(), "document.querySelector('.connect-console').click()");
        QTRY_VERIFY(backend->qmlWindow());
        QTRY_VERIFY(backend->qmlWindow()->isExposed());
        QTRY_VERIFY(!setup.isVisible());
        QTRY_COMPARE(setup.view()->page()->lifecycleState(), QWebEnginePage::LifecycleState::Frozen);
        QPointer<QmlMainWindow> native = backend->qmlWindow();
        QPointer<StreamSession> session = backend->qmlSession();
        QVERIFY(session);
        native->dismissStreamView();
        QTest::qWait(30);
        QVERIFY(native);
        QVERIFY(!setup.isVisible());
        QVERIFY(backend->cancelConnection(backend->connectionAttempt().value("id").toString()));
        QTRY_VERIFY(!session);
        QTRY_VERIFY(!native);
        QTRY_VERIFY(setup.isVisible());
        QCOMPARE(setup.view()->page()->lifecycleState(), QWebEnginePage::LifecycleState::Active);
        QVERIFY(backend->discoveryEnabled());
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.discovery').getAttribute('aria-pressed')").toString(), QStringLiteral("true"));
    }

    void manualHandoff()
    {
#ifdef Q_OS_WIN
        if (qEnvironmentVariableIsSet("CHIAKI_HANDOFF_REQUIRE_FOREGROUND"))
            QVERIFY2(inputDesktopAvailable(), "Handoff requires the active unlocked input desktop");
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
        settings.SetHardwareDecoder({});
        settings.SetPlaceboPreset(PlaceboPreset::Fast);
        ChiakiRegisteredHost registered = {};
        registered.target = CHIAKI_TARGET_PS5_1;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "Office console");
        settings.AddRegisteredHost(RegisteredHost(registered));
        QmlNativeServices services(&settings);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        ApplicationController controller(&services);
        auto &setup = *controller.setupWindow();
        setup.show();
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-add-manual]')").toBool());
        script(setup.view(), "document.querySelector('[data-add-manual]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-manual-address]')").toBool());
        QVERIFY(fill(setup.view(), "[data-manual-address]", "127.0.0.1"));
        const auto registrationKey = script(setup.view(), "document.querySelector('[data-manual-registration]').options[1].value").toString();
        QVERIFY(fill(setup.view(), "[data-manual-registration]", registrationKey));
        script(setup.view(), "document.querySelector('[data-save-manual]').click()");
        QTRY_COMPARE(settings.GetManualHosts().size(), 1);
        QVERIFY(settings.GetManualHosts().first().GetRegistered());
        script(setup.view(), "document.querySelector('[data-close-management]').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 1);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.connect-console')?.textContent.trim()").toString(), QStringLiteral("Connect"));
        script(setup.view(), "document.querySelector('.connect-console').click()");
        QTRY_VERIFY(backend->qmlWindow());
        QTRY_VERIFY(backend->qmlWindow()->isExposed());
        QTRY_VERIFY(!setup.isVisible());
        QTRY_COMPARE(setup.view()->page()->lifecycleState(), QWebEnginePage::LifecycleState::Frozen);
        QPointer<QmlMainWindow> native = backend->qmlWindow();
        QPointer<StreamSession> session = backend->qmlSession();
        QVERIFY(session);
        native->dismissStreamView();
        QTest::qWait(30);
        QVERIFY(native);
        QVERIFY(!setup.isVisible());
        QVERIFY(backend->cancelConnection(backend->connectionAttempt().value("id").toString()));
        QTRY_VERIFY(!session);
        QTRY_VERIFY(!native);
        QTRY_VERIFY(setup.isVisible());
        QCOMPARE(setup.view()->page()->lifecycleState(), QWebEnginePage::LifecycleState::Active);
        QVERIFY(backend->discoveryEnabled());
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.discovery').getAttribute('aria-pressed')").toString(), QStringLiteral("true"));
    }

    void init()
    {
#ifdef Q_OS_WIN
        if (QString::fromLatin1(QTest::currentTestFunction()).startsWith("psn") && QGuiApplication::platformName() != "offscreen") {
            QVERIFY(inputDesktopAvailable());
            QVERIFY(!protectedWindowVisible());
        }
#endif
    }

    void registrationHandoff()
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
        settings.SetHardwareDecoder({});
        settings.SetPlaceboPreset(PlaceboPreset::Fast);
        const int manualId = settings.SetManualHost(ManualHost(-1, "127.0.0.1", false, {}));
        RegistrationTransport registration;
        QmlNativeServices services(&settings, {}, {}, registration.boundary(), controlledDiscovery());
        auto *backend = services.backend();
        backend->stopAutoConnect();
        ApplicationController controller(&services);
        auto &setup = *controller.setupWindow();
        setup.resize(1280, 720);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-register-selected]')").toBool());
        script(setup.view(), "document.querySelector('[data-register-selected]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-registration-form]')").toBool());
        QVERIFY(fill(setup.view(), "[data-registration-address]", "127.0.0.1"));
        QVERIFY(fill(setup.view(), "[data-registration-account]", "AQAAAAAAAAA="));
        QVERIFY(fill(setup.view(), "[data-registration-pin]", "00123456"));
        script(setup.view(), "document.querySelector('[data-registration-submit]').click()");
        QTRY_COMPARE(registration.requests.size(), 1);
        setup.view()->reload();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-register-new]')?.textContent.includes('View registration progress')").toBool());
        QCOMPARE(registration.requests.size(), 1);
        script(setup.view(), "document.querySelector('[data-register-new]').click()");
        QTRY_VERIFY(script(setup.view(), "document.body.textContent.includes('Registering console')").toBool());
        QCOMPARE(registration.requests.size(), 1);
        registration.complete(0, true);
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-registration-connect]')").toBool());
        QCOMPARE(settings.GetManualHosts().size(), 1);
        QVERIFY(settings.GetManualHost(manualId).GetRegistered());
        QCOMPARE(settings.GetRegisteredHosts().first().GetConsolePin(), QString());
        script(setup.view(), "document.querySelector('[data-registration-connect]').click()");
        QTRY_VERIFY(backend->qmlWindow());
        QTRY_VERIFY(backend->qmlWindow()->isExposed());
        QTRY_VERIFY(!setup.isVisible());
        QPointer<QmlMainWindow> native = backend->qmlWindow();
        QPointer<StreamSession> session = backend->qmlSession();
        QVERIFY(session);
        native->dismissStreamView();
        QTest::qWait(30);
        QVERIFY(native);
        QVERIFY(!setup.isVisible());
        QVERIFY(backend->cancelConnection(backend->connectionAttempt().value("id").toString()));
        QTRY_VERIFY(!session);
        QTRY_VERIFY(!native);
        QTRY_VERIFY(setup.isVisible());
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-registration-connect]')").toBool());
        QCOMPARE(registration.requests.size(), 1);
        script(setup.view(), "document.querySelector('[data-registration-close]').click()");
        QTRY_VERIFY(script(setup.view(), "document.activeElement.hasAttribute('data-register-new')").toBool());
    }

    void registrationForm()
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
        RegistrationTransport registration;
        QmlNativeServices services(&settings, registration.guardedPsn(), {}, registration.boundary(), controlledDiscovery());
        services.backend()->stopAutoConnect();
        SetupWindow setup(services.backend());
        setup.resize(1280, 720);
        setup.show();
        QTRY_COMPARE(script(setup.view(), "document.querySelector('h1')?.textContent").toString(), QStringLiteral("Consoles"));
        QVERIFY2(script(setup.view(), "document.querySelector('[data-register-new]') !== null").toBool(), "The library must offer guided registration.");
        script(setup.view(), "document.querySelector('[data-register-new]').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-registration-form]') !== null").toBool());
        QCOMPARE(script(setup.view(), "document.querySelector('[data-registration-target]').options.length").toInt(), 4);
        const auto capture = [&](const QString &name) {
            const auto directory = qEnvironmentVariable("CHIAKI_EMBEDDED_EVIDENCE");
            if (directory.isEmpty()) return;
            script(setup.view(), "document.querySelector('.pairing-panel').scrollIntoView({block:'start'})");
            QTest::qWait(30);
            QVERIFY(setup.view()->grab().save(QDir(directory).filePath(name + ".png")));
        };
        capture("registration-1280");
        QVERIFY(script(setup.view(), "document.querySelector('[data-registration-submit]').disabled").toBool());
        QVERIFY(fill(setup.view(), "[data-registration-account]", "not-base64"));
        QVERIFY(fill(setup.view(), "[data-registration-pin]", "00123456"));
        QVERIFY(script(setup.view(), "document.querySelector('[data-registration-submit]').disabled").toBool());
        QVERIFY(fill(setup.view(), "[data-registration-account]", "AQAAAAAAAAA="));
        QTRY_VERIFY(!script(setup.view(), "document.querySelector('[data-registration-submit]').disabled").toBool());
        for (const auto &value : {"1234", "1234567", "123456789", "abcdefgh"}) {
            QVERIFY(fill(setup.view(), "[data-registration-pin]", value));
            QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-registration-submit]').disabled").toBool());
        }
        QVERIFY(fill(setup.view(), "[data-registration-pin]", "00123456"));
        QVERIFY(fill(setup.view(), "[data-registration-console-pin]", "007"));
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-registration-submit]').disabled").toBool());
        QVERIFY(fill(setup.view(), "[data-registration-console-pin]", "0007"));
        QTRY_VERIFY(!script(setup.view(), "document.querySelector('[data-registration-submit]').disabled").toBool());
        QCOMPARE(script(setup.view(), "document.querySelector('[data-registration-pin]').value").toString(), QStringLiteral("00123456"));
        QCOMPARE(script(setup.view(), "document.querySelector('[data-registration-console-pin]').value").toString(), QStringLiteral("0007"));
        QVERIFY(fill(setup.view(), "[data-registration-target]", "800"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-registration-online-id]')").toBool());
        QVERIFY(fill(setup.view(), "[data-registration-online-id]", "LegacyUser"));
        QTRY_VERIFY(!script(setup.view(), "document.querySelector('[data-registration-submit]').disabled").toBool());
        services.backend()->qmlSettings()->setStreamerMode(true);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-registration-address]').type").toString(), QStringLiteral("password"));
        QCOMPARE(script(setup.view(), "document.querySelector('[data-registration-online-id]').type").toString(), QStringLiteral("password"));
        QCOMPARE(registration.requests.size(), 0);
        setup.resize(1024, 720);
        QTRY_VERIFY(script(setup.view(), "innerWidth >= 1024 && document.documentElement.scrollWidth <= innerWidth").toBool());
        capture("registration-1024");
        setup.resize(2560, 1440);
        setup.view()->setZoomFactor(2);
        QTRY_VERIFY(script(setup.view(), "innerWidth >= 1024 && document.documentElement.scrollWidth <= innerWidth").toBool());
        capture("registration-200-supported");
        setup.view()->setZoomFactor(1);
        setup.resize(1280, 720);
        QTRY_VERIFY(script(setup.view(), "innerWidth >= 1024 && document.documentElement.scrollWidth <= innerWidth").toBool());
        script(setup.view(), "document.querySelector('[data-registration-close]').scrollIntoView({block:'center'});document.querySelector('[data-registration-close]').click()");
        QTRY_VERIFY(script(setup.view(), "document.activeElement.hasAttribute('data-register-new')").toBool());
    }

    void registrationProgress()
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
        RegistrationTransport registration;
        QmlNativeServices services(&settings, registration.guardedPsn(), {}, registration.boundary(), controlledDiscovery());
        services.backend()->stopAutoConnect();
        SetupWindow setup(services.backend());
        setup.resize(1280, 720);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-register-new]') !== null").toBool());
        script(setup.view(), "document.querySelector('[data-register-new]').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-registration-form]') !== null").toBool());
        script(setup.view(), R"(
            for (const [selector,value] of [['[data-registration-account]','AQAAAAAAAAA='],['[data-registration-pin]','00123456'],['[data-registration-console-pin]','0007']]) {
              const input=document.querySelector(selector);input.value=value;input.dispatchEvent(new Event('input',{bubbles:true}));
            }
        )");
        QTRY_VERIFY(!script(setup.view(), "document.querySelector('[data-registration-submit]').disabled").toBool());
        script(setup.view(), "document.querySelector('[data-registration-submit]').click()");
        QTRY_VERIFY2(script(setup.view(), "document.body.textContent.includes('Registering console')").toBool(), "Accepted native registration must display progress.");
        QCOMPARE(registration.requests.size(), 1);
        QCOMPARE(registration.requests[0].pin, uint32_t(123456));
        QVERIFY(registration.requests[0].broadcast);
        const auto context = setup.bridge()->snapshot().value("registration").toMap().value("context").toString();
        const auto duplicate = consoleRequest(setup.view(), "submitRegistration", {context, QJsonObject{{"address", "255.255.255.255"}, {"target", 1000100}, {"account", "AQAAAAAAAAA="}, {"pin", "00123456"}}});
        QVERIFY(!duplicate.value("accepted").toBool());
        QCOMPARE(registration.requests.size(), 1);
        chiaki_log(registration.requests[0].log, CHIAKI_LOG_INFO, "Regist waiting for response");
        chiaki_log(registration.requests[0].log, CHIAKI_LOG_VERBOSE, "registration-key-secret");
        script(setup.view(), "document.querySelector('[data-registration-close]').click()");
        script(setup.view(), "document.querySelector('[data-register-new]').click()");
        QTRY_VERIFY(script(setup.view(), "document.body.textContent.includes('Registering console')").toBool());
        QCOMPARE(registration.requests.size(), 1);
        registration.complete(0, false);
        QTRY_COMPARE(services.backend()->registrationAttempt().value("phase").toString(), QStringLiteral("failure"));
        QVERIFY(settings.GetRegisteredHosts().isEmpty());
        // Reopening progress does not replay the request. Failure retains valid details for retry.
        QTRY_VERIFY(script(setup.view(), "document.body.textContent.includes('Registration failed')").toBool());
        script(setup.view(), "document.querySelector('[data-registration-edit]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-registration-form]')").toBool());
        QVERIFY(fill(setup.view(), "[data-registration-address]", "127.0.0.1"));
        QVERIFY(fill(setup.view(), "[data-registration-account]", "AQAAAAAAAAA="));
        QVERIFY(fill(setup.view(), "[data-registration-pin]", "00123456"));
        QVERIFY(fill(setup.view(), "[data-registration-console-pin]", "0007"));
        script(setup.view(), "document.querySelector('[data-registration-submit]').click()");
        QTRY_COMPARE(registration.requests.size(), 2);
        QVERIFY(!registration.requests[1].broadcast);
        QCOMPARE(registration.requests[1].consolePin, uint32_t(7));
        chiaki_log(registration.requests[1].log, CHIAKI_LOG_INFO, "Regist waiting for response");
        registration.complete(1, true);
        registration.complete(1, true, 0x66);
        QTRY_VERIFY(script(setup.view(), "document.body.textContent.includes('Console registered')").toBool());
        QCOMPARE(settings.GetRegisteredHosts().size(), 1);
        QCOMPARE(settings.GetManualHosts().size(), 1);
        QVERIFY(settings.GetManualHosts().first().GetRegistered());
        QCOMPARE(settings.GetRegisteredHosts().first().GetConsolePin(), QStringLiteral("0007"));
        Settings persisted(profile);
        QCOMPARE(persisted.GetRegisteredHosts().first().GetConsolePin(), QStringLiteral("0007"));
        QVERIFY(script(setup.view(), "document.querySelector('[data-registration-connect]') !== null").toBool());
        script(setup.view(), "document.querySelector('[data-registration-diagnostics]').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-registration-log]')?.textContent.includes('waiting for response')").toBool());
        auto snapshot = QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson();
        QVERIFY(!snapshot.contains("registration-key"));
        QVERIFY(!snapshot.contains("0007"));
        services.backend()->qmlSettings()->setStreamerMode(true);
        QTRY_VERIFY(script(setup.view(), "document.body.textContent.includes('Diagnostics are hidden in streamer mode.')").toBool());
        QVERIFY(!script(setup.view(), "document.querySelector('[data-registration-log]') !== null").toBool());
        snapshot = QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson();
        QVERIFY(!snapshot.contains("127.0.0.1"));
        QVERIFY(!snapshot.contains("Regist waiting"));
    }

    void registrationLookup()
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
        RegistrationTransport registration;
        QList<QPointer<PsnReply>> requests;
        PsnRequests transport;
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) { *devices = nullptr; *count = 0; return CHIAKI_ERR_SUCCESS; };
        QmlNativeServices services(&settings, registration.guardedPsn(transport), {}, registration.boundary(), controlledDiscovery());
        services.backend()->stopAutoConnect();
        SetupWindow setup(services.backend());
        setup.resize(1280, 720);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-register-new]') !== null").toBool());
        script(setup.view(), "document.querySelector('[data-register-new]').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-registration-form]') !== null").toBool());
        QVERIFY2(script(setup.view(), "document.querySelector('[data-registration-lookup]') !== null").toBool(), "Registration must offer native public Account ID lookup.");
        QVERIFY(fill(setup.view(), "[data-registration-address]", "pair.example"));
        QVERIFY(fill(setup.view(), "[data-registration-pin]", "00123456"));
        QVERIFY(fill(setup.view(), "[data-registration-console-pin]", "0007"));
        script(setup.view(), "document.querySelector('[data-registration-lookup]').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-lookup-explanation]')?.textContent.includes('psn.flipscreen.games')").toBool());
        QVERIFY(script(setup.view(), "document.querySelector('[data-lookup-explanation]').textContent.includes('anyone') && document.querySelector('[data-lookup-explanation]').textContent.includes('credentials')").toBool());
        QCOMPARE(requests.size(), 0);
        QVERIFY(fill(setup.view(), "[data-lookup-username]", "  Public User+  "));
        script(setup.view(), "document.querySelector('[data-lookup-submit]').click()");
        QTRY_COMPARE(requests.size(), 1);
        QCOMPARE(QUrlQuery(requests[0]->url()).queryItemValue("username", QUrl::FullyDecoded), QStringLiteral("Public User+"));
        QVERIFY(requests[0]->request().rawHeader("Authorization").isEmpty());
        QCOMPARE(requests[0]->url().host(), QStringLiteral("psn.flipscreen.games"));
        const auto route = services.backend()->accountLookupAttempt().value("id").toString();
        requests[0]->finish({}, QNetworkReply::NoError, 503);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-lookup-error]')?.textContent.includes('could not be reached')").toBool());
        const QList<QByteArray> invalid = {"unreadable", "{}", R"({"encoded_id":42})", R"({"encoded_id":"invalid"})", R"({"encoded_id":"AQAAAAAAAAB="})"};
        for (const auto &reply : invalid) {
            const int count = requests.size();
            script(setup.view(), "document.querySelector('[data-lookup-submit]').click()");
            QTRY_COMPARE(requests.size(), count + 1);
            QVERIFY(script(setup.view(), "document.querySelector('[data-lookup-submit]').disabled").toBool());
            script(setup.view(), "document.querySelector('[data-lookup-submit]').click()");
            QCOMPARE(requests.size(), count + 1);
            requests.last()->finish(reply);
            QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-lookup-error]')").toBool());
        }
        script(setup.view(), "document.querySelector('[data-lookup-submit]').click()");
        QTRY_COMPARE(requests.size(), 7);
        requests.last()->finish(R"({"encoded_id":"EjRWeJCrze8="})");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-registration-account]')?.value").toString(), QStringLiteral("EjRWeJCrze8="));
        QCOMPARE(script(setup.view(), "document.querySelector('[data-registration-address]').value").toString(), QStringLiteral("pair.example"));
        QCOMPARE(script(setup.view(), "document.querySelector('[data-registration-pin]').value").toString(), QStringLiteral("00123456"));
        QCOMPARE(script(setup.view(), "document.querySelector('[data-registration-console-pin]').value").toString(), QStringLiteral("0007"));
        QTRY_VERIFY(script(setup.view(), "document.activeElement.hasAttribute('data-registration-account')").toBool());
        QVERIFY(settings.GetPsnAccountId().isEmpty());
        QVERIFY(settings.GetPsnAuthToken().isEmpty());
        QVERIFY(settings.GetPsnRefreshToken().isEmpty());
        QCOMPARE(registration.requests.size(), 0);
        QVERIFY(script(setup.view(), "localStorage.length === 0 && sessionStorage.length === 0").toBool());
        // An external reply can arrive after an abort. It cannot complete a replacement route.
        script(setup.view(), "document.querySelector('[data-registration-lookup]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-lookup-username]')").toBool());
        QVERIFY(fill(setup.view(), "[data-lookup-username]", "old-user"));
        script(setup.view(), "document.querySelector('[data-lookup-submit]').click()");
        QTRY_COMPARE(requests.size(), 8);
        auto obsolete = requests.last();
        obsolete->deferAbort = true;
        script(setup.view(), "document.querySelector('[data-lookup-cancel]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-registration-account]')").toBool());
        script(setup.view(), "document.querySelector('[data-registration-lookup]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-lookup-username]')").toBool());
        QVERIFY(fill(setup.view(), "[data-lookup-username]", "new-user"));
        script(setup.view(), "document.querySelector('[data-lookup-submit]').click()");
        QTRY_COMPARE(requests.size(), 9);
        obsolete->finish(R"({"encoded_id":"AAAAAAAAAAA="})");
        QTest::qWait(30);
        QVERIFY(script(setup.view(), "!!document.querySelector('[data-lookup-username]')").toBool());
        requests.last()->finish(R"({"encoded_id":"AgAAAAAAAAA="})");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-registration-account]')?.value").toString(), QStringLiteral("AgAAAAAAAAA="));
        services.backend()->qmlSettings()->setStreamerMode(true);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-registration-account]').type").toString(), QStringLiteral("password"));
        const auto snapshot = QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson();
        QVERIFY(!snapshot.contains("AgAAAAAAAAA="));
        QVERIFY(!snapshot.contains("new-user"));
        QVERIFY(!snapshot.contains("0007"));
    }

    void registrationLookupTimeout()
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
        RegistrationTransport registration;
        QList<QPointer<PsnReply>> requests;
        PsnRequests transport;
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) { *devices = nullptr; *count = 0; return CHIAKI_ERR_SUCCESS; };
        QmlNativeServices services(&settings, registration.guardedPsn(transport), {}, registration.boundary(), controlledDiscovery());
        services.backend()->stopAutoConnect();
        SetupWindow setup(services.backend());
        setup.resize(1280, 720);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-register-new]') !== null").toBool());
        script(setup.view(), "document.querySelector('[data-register-new]').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-registration-form]') !== null").toBool());
        script(setup.view(), "document.querySelector('[data-registration-lookup]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-lookup-username]')").toBool());
        QVERIFY(fill(setup.view(), "[data-lookup-username]", "timeout-user"));
        script(setup.view(), "document.querySelector('[data-lookup-submit]').click()");
        QTRY_COMPARE(requests.size(), 1);
        QTRY_VERIFY_WITH_TIMEOUT(script(setup.view(), "document.querySelector('[data-lookup-error]')?.textContent.includes('timed out')").toBool(), 35000);
        script(setup.view(), "document.querySelector('[data-lookup-submit]').click()");
        QTRY_COMPARE(requests.size(), 2);
        requests.last()->deferAbort = true;
        const auto late = requests.last();
        services.backend()->qmlSettings()->setCurrentProfile(profile + "-next");
        services.backend()->setDiscoveryEnabled(false);
        late->finish(R"({"encoded_id":"EjRWeJCrze8="})");
        QTest::qWait(30);
        QVERIFY(services.backend()->nativeSettings()->GetPsnAccountId().isEmpty());
        QVERIFY(!script(setup.view(), "!!document.querySelector('[data-registration-account]')").toBool());
        QVERIFY(script(setup.view(), "document.querySelector('[data-lookup-submit]').disabled").toBool());
        QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile + "-next").clear();
    }

    void registrationAccountSources()
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
        settings.SetPsnAccountId("AQAAAAAAAAA=");
        RegistrationTransport registration;
        QList<QPointer<PsnReply>> requests;
        PsnRequests transport;
        transport.networkManager = [&requests](QObject *parent) { return new PsnNetwork(requests, parent); };
        transport.listDevices = [](const char *, ChiakiHolepunchConsoleType, ChiakiHolepunchDeviceInfo **devices, size_t *count, ChiakiLog *) { *devices = nullptr; *count = 0; return CHIAKI_ERR_SUCCESS; };
        QmlNativeServices services(&settings, registration.guardedPsn(transport), {}, registration.boundary(), controlledDiscovery());
        services.backend()->stopAutoConnect();
        SetupWindow setup(services.backend());
        setup.resize(1280, 720);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-register-new]') !== null").toBool());
        script(setup.view(), "document.querySelector('[data-register-new]').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-registration-form]') !== null").toBool());
        QVERIFY(fill(setup.view(), "[data-registration-address]", "original-console.example"));
        QVERIFY(fill(setup.view(), "[data-registration-pin]", "00123456"));
        QVERIFY(fill(setup.view(), "[data-registration-console-pin]", "0007"));
        script(setup.view(), "document.querySelector('[data-registration-saved]').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-registration-account]')?.value").toString(), QStringLiteral("AQAAAAAAAAA="));
        QCOMPARE(requests.size(), 0);
        script(setup.view(), "document.querySelector('[data-registration-sign-in]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('.pairing [data-psn-redirect]')").toBool());
        QVERIFY(script(setup.view(), "document.querySelector('.pairing .psn-account').textContent.includes('Sign-in saves PSN credentials')").toBool());
        QVERIFY(fill(setup.view(), ".pairing [data-psn-redirect]", "https://remoteplay.dl.playstation.net/remoteplay/redirect?code=pairing-sign-in"));
        script(setup.view(), "document.querySelector('.pairing [data-psn-submit]').click()");
        QTRY_COMPARE(requests.size(), 1);
        requests[0]->finish(R"({"access_token":"pairing-access-secret","refresh_token":"pairing-refresh-secret","expires_in":3600})");
        QTRY_COMPARE(requests.size(), 2);
        requests[1]->finish(R"({"user_id":"2"})");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-registration-account]')?.value").toString(), QStringLiteral("AgAAAAAAAAA="));
        QCOMPARE(script(setup.view(), "document.querySelector('[data-registration-address]').value").toString(), QStringLiteral("original-console.example"));
        QCOMPARE(script(setup.view(), "document.querySelector('[data-registration-pin]').value").toString(), QStringLiteral("00123456"));
        QCOMPARE(script(setup.view(), "document.querySelector('[data-registration-console-pin]').value").toString(), QStringLiteral("0007"));
        QTRY_VERIFY(script(setup.view(), "document.activeElement.hasAttribute('data-registration-account')").toBool());
        script(setup.view(), "document.querySelector('[data-registration-sign-in]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('.pairing [data-psn-redirect]')").toBool());
        QVERIFY(fill(setup.view(), ".pairing [data-psn-redirect]", "https://remoteplay.dl.playstation.net/remoteplay/redirect?code=obsolete-sign-in"));
        script(setup.view(), "document.querySelector('.pairing [data-psn-submit]').click()");
        QTRY_COMPARE(requests.size(), 3);
        script(setup.view(), "document.querySelector('.pairing [data-psn-close]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-registration-account]')").toBool());
        script(setup.view(), "document.querySelector('[data-registration-sign-in]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('.pairing [data-psn-redirect]')").toBool());
        requests[2]->finish(R"({"access_token":"late-access-secret","refresh_token":"late-refresh-secret","expires_in":3600})");
        QTRY_COMPARE(requests.size(), 4);
        requests[3]->finish(R"({"user_id":"3"})");
        QTRY_COMPARE(services.backend()->psnAuthAttempt().value("phase").toString(), QStringLiteral("success"));
        QTest::qWait(30);
        QVERIFY(script(setup.view(), "!!document.querySelector('.pairing [data-psn-redirect]')").toBool());
        script(setup.view(), "document.querySelector('.pairing [data-psn-close]').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-registration-account]')?.value").toString(), QStringLiteral("AgAAAAAAAAA="));
        const auto snapshot = QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson();
        QVERIFY(!snapshot.contains("pairing-access-secret"));
        QVERIFY(!snapshot.contains("late-access-secret"));
        QVERIFY(!snapshot.contains("AgAAAAAAAAA="));
        QVERIFY(!snapshot.contains("00123456"));
        QVERIFY(!snapshot.contains("0007"));
        script(setup.view(), "document.querySelector('[data-registration-close]').click()");
        script(setup.view(), "document.querySelector('[data-register-new]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-registration-form]')").toBool());
        QCOMPARE(script(setup.view(), "document.querySelector('[data-registration-pin]').value").toString(), QString());
        QCOMPARE(script(setup.view(), "document.querySelector('[data-registration-console-pin]').value").toString(), QString());
    }

    void registrationAddressCorrection()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto cleanup = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        const int originalId = settings.SetManualHost(ManualHost(-1, "original.example", false, {}));
        RegistrationTransport registration;
        QmlNativeServices services(&settings, registration.guardedPsn(), {}, registration.boundary(), controlledDiscovery());
        services.backend()->stopAutoConnect();
        SetupWindow setup(services.backend());
        setup.resize(1280, 720);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-register-selected]')").toBool());
        script(setup.view(), "document.querySelector('[data-register-selected]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-registration-form]')").toBool());
        QVERIFY(fill(setup.view(), "[data-registration-address]", "corrected.example"));
        QVERIFY(fill(setup.view(), "[data-registration-account]", "AQAAAAAAAAA="));
        QVERIFY(fill(setup.view(), "[data-registration-pin]", "00123456"));
        script(setup.view(), "document.querySelector('[data-registration-submit]').click()");
        QTRY_COMPARE(registration.requests.size(), 1);
        registration.complete(0, false);
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-registration-edit]')").toBool());
        script(setup.view(), "document.querySelector('[data-registration-edit]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-registration-form]')").toBool());
        QCOMPARE(script(setup.view(), "document.querySelector('[data-registration-address]').value").toString(), QStringLiteral("corrected.example"));
        QVERIFY(fill(setup.view(), "[data-registration-pin]", "00123456"));
        script(setup.view(), "document.querySelector('[data-registration-submit]').click()");
        QTRY_COMPARE(registration.requests.size(), 2);
        registration.complete(1, true);
        QTRY_COMPARE(settings.GetManualHosts().size(), 2);
        QVERIFY(!settings.GetManualHost(originalId).GetRegistered());
        QCOMPARE(settings.GetManualHost(originalId).GetHost(), QStringLiteral("original.example"));
    }

    void registrationNativeValidation()
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
        RegistrationTransport registration;
        QmlNativeServices services(&settings, registration.guardedPsn(), {}, registration.boundary(), controlledDiscovery());
        services.backend()->stopAutoConnect();
        SetupWindow setup(services.backend());
        setup.resize(1280, 720);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-register-new]')").toBool());
        const auto generation = QString::number(services.backend()->profileGeneration());
        const auto context = consoleRequest(setup.view(), "openRegistration", {"", generation}).value("context").toString();
        QVERIFY(!context.isEmpty());
        const QJsonObject valid{{"address", "console.example"}, {"target", 1000100}, {"account", "AQAAAAAAAAA="}, {"pin", "00123456"}, {"consolePin", "0007"}};
        for (const auto &input : QList<QPair<QString, QJsonValue>>{{"address", "https://private.example/"}, {"address", "192.0.2.999"},
                {"address", "console:9295"}, {"address", ""}, {"target", 123}, {"target", 1000100.5}, {"account", ""}, {"account", "AQAAAAAAAAB="},
                {"pin", "1234"}, {"pin", "123456789"}, {"pin", 12345678}, {"consolePin", "123"}}) {
            auto fields = valid;
            fields[input.first] = input.second;
            const auto reply = consoleRequest(setup.view(), "submitRegistration", {context, fields});
            QVERIFY2(!reply.value("accepted").toBool(), qPrintable(input.first));
        }
        QCOMPARE(registration.requests.size(), 0);
        for (const int target : {800, 900, 1000, 1000100}) {
            const auto next = consoleRequest(setup.view(), "openRegistration", {"", generation}).value("context").toString();
            auto fields = valid;
            fields["target"] = target;
            if (target == 800) fields["account"] = "LegacyUser";
            const auto reply = consoleRequest(setup.view(), "submitRegistration", {next, fields});
            QVERIFY(reply.value("accepted").toBool());
            QCOMPARE(int(registration.requests.last().target), target);
            QCOMPARE(registration.requests.last().onlineId, target == 800 ? QStringLiteral("LegacyUser") : QString());
            registration.complete(registration.requests.size() - 1, false);
            QTRY_VERIFY(!services.backend()->registrationBusy());
        }
        registration.startResult = CHIAKI_ERR_NETWORK;
        const auto failedContext = consoleRequest(setup.view(), "openRegistration", {"", generation}).value("context").toString();
        QVERIFY(!consoleRequest(setup.view(), "submitRegistration", {failedContext, valid}).value("accepted").toBool());
        QCOMPARE(services.backend()->registrationAttempt().value("phase").toString(), QStringLiteral("failure"));
        QVERIFY(!services.backend()->registrationBusy());
        QVERIFY(settings.GetRegisteredHosts().isEmpty());
    }

    void registrationStaleTargets_data()
    {
        QTest::addColumn<QString>("change");
        for (const auto &change : {"manualAddress", "manualRemoved", "discoveryAddress", "discoveryDuplicate", "profile"})
            QTest::newRow(change) << QString::fromLatin1(change);
    }
    void registrationStaleTargets()
    {
        QFETCH(QString, change);
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            for (const auto &name : {profile, profile + "-next"})
                QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + name).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        const bool discovery = change.startsWith("discovery");
        const int manualId = discovery ? -1 : settings.SetManualHost(ManualHost(-1, "192.0.2.10", false, {}));
        RegistrationTransport registration;
        QmlNativeServices services(&settings, registration.guardedPsn(), {}, registration.boundary(), controlledDiscovery());
        auto *backend = services.backend();
        backend->stopAutoConnect();
        SetupWindow setup(backend);
        DiscoveryHost discovered = {};
        discovered.ps5 = true;
        discovered.host_id = "001122334455";
        discovered.host_addr = "192.0.2.10";
        discovered.host_name = "Unregistered console";
        discovered.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
        if (discovery) discover(backend, {discovered});
        setup.resize(1280, 720);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-register-selected]')").toBool());
        script(setup.view(), "document.querySelector('[data-register-selected]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-registration-form]')").toBool());
        QCOMPARE(script(setup.view(), "document.querySelector('[data-registration-address]').value").toString(), QStringLiteral("192.0.2.10"));
        QVERIFY(fill(setup.view(), "[data-registration-account]", "AQAAAAAAAAA="));
        QVERIFY(fill(setup.view(), "[data-registration-pin]", "00123456"));
        script(setup.view(), "document.querySelector('[data-registration-submit]').click()");
        QTRY_COMPARE(registration.requests.size(), 1);
        if (change == "manualAddress") settings.SetManualHost(ManualHost(manualId, "192.0.2.20", false, {}));
        else if (change == "manualRemoved") settings.RemoveManualHost(manualId);
        else if (change == "discoveryAddress") { discovered.host_addr = "192.0.2.20"; discover(backend, {discovered}); QTRY_COMPARE(backend->hosts().first().toMap().value("address").toString(), QStringLiteral("192.0.2.20")); }
        else if (change == "discoveryDuplicate") { discover(backend, {discovered, discovered}); QTRY_COMPARE(backend->hosts().size(), 2); }
        else { backend->qmlSettings()->setCurrentProfile(profile + "-next"); backend->setDiscoveryEnabled(false); }
        registration.complete(0, true);
        QTRY_VERIFY(!backend->registrationBusy());
        QCOMPARE(backend->registrationAttempt().value("phase").toString(), QStringLiteral("failure"));
        QVERIFY(backend->nativeSettings()->GetRegisteredHosts().isEmpty());
        QVERIFY(settings.GetRegisteredHosts().isEmpty());
        QVERIFY(!script(setup.view(), "!!document.querySelector('[data-registration-connect]')").toBool());
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

    void colorMappingSettings()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
            QSettings(QCoreApplication::organizationName(), "pl_render_params").clear();
        });
        QSettings(QCoreApplication::organizationName(), "pl_render_params").clear();
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        QmlNativeServices services(&settings);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        backend->setDiscoveryEnabled(false);
        SetupWindow setup(backend);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled===false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.category-nav')!==null").toBool());
        script(setup.view(), "[...document.querySelectorAll('.category-nav button')].find(e=>e.textContent==='Renderer').click()");
        const auto change = [&](const QString &key, double value) {
            QTRY_VERIFY(script(setup.view(), QString("document.getElementById('%1')?.disabled===false").arg(key)).toBool());
            script(setup.view(), QString("{const field=document.getElementById('%1');field.value='%2';field.dispatchEvent(new Event('change',{bubbles:true}));}").arg(key).arg(value));
        };
        change("videoPreset", 5);
        QTRY_COMPARE(int(settings.GetPlaceboPreset()), 5);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#placeboColorMappingEnabled')!==null").toBool());
        QCOMPARE(script(setup.view(), "document.querySelector('#placeboColorMappingEnabled').checked").toBool(), true);
        change("placeboColorMappingPreset", 0);
        QTRY_COMPARE(int(settings.GetPlaceboColorMappingPreset()), 0);
        script(setup.view(), "document.querySelector('#openPlaceboColorMapping').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#placeboGamutMappingFunction')?.options.length").toInt(), 10);
        QSignalSpy changed(backend->qmlSettings(), &QmlSettings::placeboChanged);
        change("placeboGamutMappingPerceptualDeadzone", 0.42);
        QTRY_VERIFY(qAbs(settings.GetPlaceboGamutMappingPerceptualDeadzone() - 0.42f) < 0.001f);
        QVERIFY(changed.count() > 0);
        const auto generation = setup.bridge()->snapshot().value("profileGeneration").toString();
        change("placeboGamutMappingFunction", 2);
        QTRY_VERIFY(script(setup.view(), "!document.querySelector('#placeboGamutMappingPerceptualDeadzone') && !!document.querySelector('#placeboGamutMappingSoftClipDesat')").toBool());
        change("placeboGamutMappingSoftClipDesat", 0.55);
        QTRY_VERIFY(qAbs(settings.GetPlaceboGamutMappingSoftClipDesat() - 0.55f) < 0.001f);
        change("placeboGamutMappingFunction", 3);
        change("placeboGamutMappingColorimetricGamma", 2.25);
        QTRY_VERIFY(qAbs(settings.GetPlaceboGamutMappingColorimetricGamma() - 2.25f) < 0.001f);
        change("placeboGamutMappingLut3dSizeI", 64);
        QTRY_COMPARE(settings.GetPlaceboGamutMappingLut3dSizeI(), 64);
        QCOMPARE(script(setup.view(), "document.querySelector('#placeboToneMappingFunction').options.length").toInt(), 12);
        change("placeboToneMappingKneeMinimum", 0.23);
        QTRY_VERIFY(qAbs(settings.GetPlaceboToneMappingKneeMinimum() - 0.23f) < 0.001f);
        change("placeboToneMappingFunction", 4);
        QTRY_VERIFY(script(setup.view(), "!document.querySelector('#placeboToneMappingKneeMinimum') && !!document.querySelector('#placeboToneMappingKneeDefault') && !!document.querySelector('#placeboToneMappingKneeOffset')").toBool());
        change("placeboToneMappingKneeOffset", 1.25);
        QTRY_VERIFY(qAbs(settings.GetPlaceboToneMappingKneeOffset() - 1.25f) < 0.001f);
        change("placeboToneMappingFunction", 6);
        change("placeboToneMappingReinhardContrast", 0.65);
        QTRY_VERIFY(qAbs(settings.GetPlaceboToneMappingReinhardContrast() - 0.65f) < 0.001f);
        change("placeboToneMappingFunction", 7);
        change("placeboToneMappingLinearKnee", 0.45);
        QTRY_VERIFY(qAbs(settings.GetPlaceboToneMappingLinearKnee() - 0.45f) < 0.001f);
        change("placeboToneMappingFunction", 10);
        change("placeboToneMappingExposure", 2.5);
        QTRY_VERIFY(qAbs(settings.GetPlaceboToneMappingExposure() - 2.5f) < 0.001f);
        change("placeboToneMappingMetadata", 4);
        QTRY_COMPARE(int(settings.GetPlaceboToneMappingMetadata()), 4);
        change("placeboToneMappingToneLutSize", 512);
        QTRY_COMPARE(settings.GetPlaceboToneMappingToneLutSize(), 512);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#placeboGamutMappingGamutExpansionEnabled')?.disabled===false").toBool());
        script(setup.view(), "document.querySelector('#placeboGamutMappingGamutExpansionEnabled').click()");
        QTRY_VERIFY(settings.GetPlaceboGamutMappingGamutExpansionEnabled());
        const auto previous = settings.GetPlaceboValues();
        for(const auto &input : QList<QPair<QString, QVariant>>{{"placeboColorMappingEnabled", 1}, {"placeboColorMappingPreset", 3},
            {"placeboGamutMappingFunction", 10}, {"placeboGamutMappingFunction", QStringLiteral("1")},
            {"placeboGamutMappingPerceptualDeadzone", 0.5}, {"placeboGamutMappingSoftClipDesat", 0.5},
            {"placeboGamutMappingColorimetricGamma", 10.01}, {"placeboGamutMappingLut3dSizeI", 1025}, {"placeboGamutMappingLut3dSizeC", 0.5},
            {"placeboGamutMappingLut3dTricubicEnabled", 1}, {"placeboToneMappingFunction", 12},
            {"placeboToneMappingKneeMinimum", 0.2}, {"placeboToneMappingReinhardContrast", 0.4},
            {"placeboToneMappingExposure", -0.1}, {"placeboToneMappingMetadata", 5}, {"placeboToneMappingToneLutSize", 4097},
            {"placeboToneMappingContrastRecovery", 2.01}, {"placeboToneMappingContrastSmoothness", 0.9}}) {
            QVERIFY(!settingRequest(setup.view(), input.first, input.second, generation, "setRendererSetting").value("accepted").toBool());
        }
        QVERIFY(!settingRequest(setup.view(), "placeboToneMappingExposure", 3, generation + "-old", "setRendererSetting").value("accepted").toBool());
        QCOMPARE(settings.GetPlaceboValues(), previous);
        backend->qmlSettings()->setPlaceboToneMappingContrastRecovery(0.57);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#placeboToneMappingContrastRecovery')?.value").toString(), QStringLiteral("0.57"));
        const auto search = [&](const QString &query) {
            script(setup.view(), QString("{const field=document.querySelector('.settings-search input');field.value='%1';field.dispatchEvent(new Event('input',{bubbles:true}));}").arg(query));
            QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.settings-results button').length").toInt(), 1);
            script(setup.view(), "document.querySelector('.settings-results button').click()");
        };
        search("perceptual deadzone");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("placeboGamutMappingFunction"));
        change("placeboGamutMappingFunction", 1);
        QTRY_COMPARE(int(settings.GetPlaceboGamutMappingFunction()), 1);
        search("perceptual deadzone");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("placeboGamutMappingPerceptualDeadzone"));
        script(setup.view(), "document.querySelector('#backRenderer').click()");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("openPlaceboColorMapping"));
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#placeboColorMappingEnabled')?.disabled===false").toBool());
        script(setup.view(), "document.querySelector('#placeboColorMappingEnabled').click()");
        QTRY_VERIFY(!settings.GetPlaceboColorMappingEnabled());
        QVERIFY(!settingRequest(setup.view(), "placeboColorMappingPreset", 1, generation, "setRendererSetting").value("accepted").toBool());
        QVERIFY(!settingRequest(setup.view(), "placeboGamutMappingFunction", 2, generation, "setRendererSetting").value("accepted").toBool());
        search("perceptual deadzone");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("placeboColorMappingEnabled"));
        script(setup.view(), "document.querySelector('#placeboColorMappingEnabled').click()");
        QTRY_VERIFY(settings.GetPlaceboColorMappingEnabled());
        change("placeboColorMappingPreset", 2);
        QTRY_COMPARE(int(settings.GetPlaceboColorMappingPreset()), 2);
        search("perceptual deadzone");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("placeboColorMappingPreset"));
        change("placeboColorMappingPreset", 0);
        QTRY_COMPARE(int(settings.GetPlaceboColorMappingPreset()), 0);
        change("videoPreset", 2);
        QTRY_COMPARE(int(settings.GetPlaceboPreset()), 2);
        QVERIFY(!settingRequest(setup.view(), "placeboColorMappingEnabled", false, generation, "setRendererSetting").value("accepted").toBool());
        search("perceptual deadzone");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("videoPreset"));
        change("videoPreset", 5);
        QTRY_COMPARE(int(settings.GetPlaceboPreset()), 5);
        search("perceptual deadzone");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("placeboGamutMappingPerceptualDeadzone"));
        script(setup.view(), "window.dispatchEvent(new KeyboardEvent('keydown',{key:'Escape',bubbles:true}))");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("openPlaceboColorMapping"));
        setup.view()->reload();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled===false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('.settings-search input')").toBool());
        search("perceptual deadzone");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#placeboGamutMappingPerceptualDeadzone')?.value").toString(), QStringLiteral("0.42"));
        QSettings(QCoreApplication::organizationName(), "pl_render_params").sync();
        QProcess restarted;
        auto environment = QProcessEnvironment::systemEnvironment();
        environment.insert("CHIAKI_COLOR_RESTART_PROFILE", profile);
        restarted.setProcessEnvironment(environment);
        restarted.start(QCoreApplication::applicationFilePath(), {"colorMappingRestartRead", "-o",
            qEnvironmentVariable("CHIAKI_EMBEDDED_LAYOUT_DIR", QDir::tempPath()) + "/color-restart.txt,txt"});
        QVERIFY(restarted.waitForStarted());
        QTRY_COMPARE_WITH_TIMEOUT(restarted.state(), QProcess::NotRunning, 20000);
        QCOMPARE(restarted.exitStatus(), QProcess::NormalExit);
        QCOMPARE(restarted.exitCode(), 0);
    }

    void colorMappingLayout()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
            QSettings(QCoreApplication::organizationName(), "pl_render_params").clear();
        });
        QSettings(QCoreApplication::organizationName(), "pl_render_params").clear();
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetPlaceboPreset(PlaceboPreset::Custom);
        settings.SetPlaceboGamutMappingPerceptualDeadzone(0.42);
        settings.SetPlaceboColorMappingPreset(PlaceboColorMappingPreset::None);
        QmlNativeServices services(&settings);
        services.backend()->stopAutoConnect();
        services.backend()->setDiscoveryEnabled(false);
        SetupWindow setup(services.backend());
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled===false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('.category-nav')").toBool());
        script(setup.view(), "[...document.querySelectorAll('.category-nav button')].find(e=>e.textContent==='Color mapping').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#placeboGamutMappingFunction')").toBool());
        const auto capture = [&](int width, int height, double zoom, const QString &name) {
            setup.resize(width, height);
            setup.view()->setZoomFactor(zoom);
            QTRY_COMPARE(script(setup.view(), "innerWidth").toInt(), qRound(setup.view()->width() / zoom));
            QTRY_VERIFY(script(setup.view(), "document.documentElement.scrollWidth<=innerWidth && [...document.querySelectorAll('.settings-content input,.settings-content select,.settings-content button')].every(e=>{const r=e.getBoundingClientRect();return r.left>=0 && r.right<=innerWidth})").toBool());
            script(setup.view(), "document.querySelector('.settings-content h2').scrollIntoView()");
            QTRY_VERIFY(script(setup.view(), "['.settings-content h2','#placeboGamutMappingFunction'].every(s=>{const r=document.querySelector(s).getBoundingClientRect();return r.top>=0 && r.bottom<=innerHeight})").toBool());
            script(setup.view(), "window.__colorPainted=false;requestAnimationFrame(()=>requestAnimationFrame(()=>window.__colorPainted=true))");
            QTRY_VERIFY(script(setup.view(), "window.__colorPainted===true").toBool());
            QTest::qWait(100);
            QVERIFY(setup.view()->grab().save(qEnvironmentVariable("CHIAKI_EMBEDDED_LAYOUT_DIR", QDir::tempPath()) + "/" + name));
        };
        capture(1280, 720, 1, "color-1280.png");
        capture(900, 600, 1, "color-900.png");
        capture(800, 900, 2, "color-200-percent.png");
        capture(390, 720, 1, "color-390.png");
    }

    void colorMappingRestartRead()
    {
        const auto profile = qEnvironmentVariable("CHIAKI_COLOR_RESTART_PROFILE");
        if(profile.isEmpty()) QSKIP("Launched only by colorMappingSettings with isolated saved settings");
        Settings settings(profile);
        QCOMPARE(settings.GetPlaceboGamutMappingLut3dSizeI(), 64);
        QCOMPARE(settings.GetPlaceboToneMappingToneLutSize(), 512);
        QVERIFY(qAbs(settings.GetPlaceboToneMappingExposure() - 2.5f) < 0.001f);
        QmlNativeServices services(&settings);
        services.backend()->stopAutoConnect();
        services.backend()->setDiscoveryEnabled(false);
        SetupWindow setup(services.backend());
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled===false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('.settings-search input')").toBool());
        script(setup.view(), "{const field=document.querySelector('.settings-search input');field.value='perceptual deadzone';field.dispatchEvent(new Event('input',{bubbles:true}));}");
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.settings-results button').length").toInt(), 1);
        script(setup.view(), "document.querySelector('.settings-results button').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#placeboGamutMappingPerceptualDeadzone')?.value").toString(), QStringLiteral("0.42"));
        QCOMPARE(script(setup.view(), "document.querySelector('#placeboToneMappingExposure').value").toString(), QStringLiteral("2.5"));
    }

    void rendererSettings()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
            QSettings(QCoreApplication::organizationName(), "pl_render_params").clear();
        });
        QSettings(QCoreApplication::organizationName(), "pl_render_params").clear();
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        QmlNativeServices services(&settings, {}, [](const QString &, const QStringList &) { return false; });
        auto *backend = services.backend();
        backend->stopAutoConnect();
        backend->setDiscoveryEnabled(false);
        SetupWindow setup(backend);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled === false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "[...document.querySelectorAll('.category-nav button')].some(e=>e.textContent==='Renderer')").toBool());
        const auto category = [&](const QString &name) {
            script(setup.view(), QString("[...document.querySelectorAll('.category-nav button')].find(e=>e.textContent==='%1').click()").arg(name));
        };
        const auto change = [&](const QString &key, double value) {
            QTRY_VERIFY(script(setup.view(), QString("document.getElementById('%1')?.disabled===false").arg(key)).toBool());
            script(setup.view(), QString("{const field=document.getElementById('%1');field.value='%2';field.dispatchEvent(new Event('change',{bubbles:true}));}").arg(key).arg(value));
        };
        category("Display");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#displayTargetPrim')?.options.length").toInt(), 18);
        QCOMPARE(script(setup.view(), "document.querySelector('#displayTargetTrc').options.length").toInt(), 17);
        change("displayTargetPrim", 12);
        QTRY_COMPARE(settings.GetDisplayTargetPrim(), 12);
        change("displayTargetTrc", 12);
        QTRY_COMPARE(settings.GetDisplayTargetTrc(), 12);
        change("targetPeakChoice", 1);
        QTRY_COMPARE(settings.GetDisplayTargetPeak(), 1000);
        change("displayTargetPeak", 2000);
        QTRY_COMPARE(settings.GetDisplayTargetPeak(), 2000);
        change("targetContrastChoice", 1);
        QTRY_COMPARE(settings.GetDisplayTargetContrast(), -1);
        QVERIFY(script(setup.view(), "!document.querySelector('#displayTargetContrast')").toBool());
        change("targetContrastChoice", 2);
        QTRY_COMPARE(settings.GetDisplayTargetContrast(), 1000);
        change("displayTargetContrast", 5000);
        QTRY_COMPARE(settings.GetDisplayTargetContrast(), 5000);
        category("Renderer");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#videoPreset')?.value").toString(), QStringLiteral("2"));
        QVERIFY(script(setup.view(), "!document.querySelector('#placeboUpscaler') && document.querySelector('[data-renderer-runtime]').textContent.includes('No native stream')").toBool());
        const auto generation = setup.bridge()->snapshot().value("profileGeneration").toString();
        QVERIFY(!settingRequest(setup.view(), "placeboUpscaler", 11, generation, "setRendererSetting").value("accepted").toBool());
        change("videoPreset", 5);
        QTRY_COMPARE(int(settings.GetPlaceboPreset()), 5);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#placeboUpscaler')?.options.length").toInt(), 14);
        change("queueDepthLimit", 5);
        QTRY_COMPARE(settings.GetQueueDepthLimit(), 5);
        change("rendererBackend", 1);
        QTRY_COMPARE(int(settings.GetRenderBackend()), 1);
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-renderer-restart]')").toBool());
        QVERIFY(!setup.bridge()->snapshot().value("rendererStatus").toMap().contains("runtimeBackend"));
        change("placeboUpscaler", 13);
        QTRY_COMPARE(int(settings.GetPlaceboUpscaler()), 13);
        change("placeboAntiringingStrength", 0.42);
        QTRY_VERIFY(qAbs(settings.GetPlaceboAntiringingStrength() - 0.42f) < 0.001f);
        change("placeboDebandPreset", 1);
        QTRY_COMPARE(int(settings.GetPlaceboDebandPreset()), 1);
        QTRY_VERIFY(script(setup.view(), "!document.querySelector('#placeboDebandIterations')").toBool());
        change("placeboDebandPreset", 0);
        QTRY_COMPARE(int(settings.GetPlaceboDebandPreset()), 0);
        change("placeboDebandIterations", 4);
        QTRY_COMPARE(settings.GetPlaceboDebandIterations(), 4);
        change("placeboSigmoidSlope", 7.5);
        QTRY_VERIFY(qAbs(settings.GetPlaceboSigmoidSlope() - 7.5f) < 0.001f);
        change("placeboColorAdjustmentTemperature", 1.234);
        QTRY_VERIFY(qAbs(settings.GetPlaceboColorAdjustmentTemperature() - 1.234f) < 0.001f);
        change("placeboPeakDetectionPreset", 0);
        QTRY_COMPARE(int(settings.GetPlaceboPeakDetectionPreset()), 0);
        change("placeboPeakDetectionPeakPercentile", 98.765);
        QTRY_VERIFY(qAbs(settings.GetPlaceboPeakDetectionPeakPercentile() - 98.765f) < 0.001f);
        for(const auto &input : QList<QPair<QString, QVariant>>{{"queueDepthLimit", 6}, {"rendererBackend", -1}, {"videoPreset", 6},
            {"displayTargetPrim", 18}, {"displayTargetTrc", 17}, {"displayTargetPeak", 9}, {"placeboAntiringingStrength", 1.01},
            {"placeboDebandIterations", 0.5}, {"placeboSigmoidSlope", 20.1}, {"placeboColorAdjustmentTemperature", 5.287},
            {"placeboPeakDetectionPeakPercentile", -1}, {"vSyncEnabled", 1}, {"rendererBackend", QStringLiteral("0")}, {"psnAuthToken", QStringLiteral("forbidden")}}) {
            const auto reply = settingRequest(setup.view(), input.first, input.second, generation, "setRendererSetting");
            QVERIFY(reply.contains("accepted"));
            QVERIFY(!reply.value("accepted").toBool());
        }
        QSignalSpy changed(backend->qmlSettings(), &QmlSettings::placeboChanged);
        backend->qmlSettings()->setPlaceboDebandThreshold(4.5);
        QVERIFY(changed.count() > 0);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#placeboDebandThreshold')?.value").toString(), QStringLiteral("4.5"));
        setup.view()->reload();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled===false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        category("Renderer");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#placeboUpscaler')?.value").toString(), QStringLiteral("13"));
        Settings restarted(profile);
        QCOMPARE(restarted.GetQueueDepthLimit(), 5);
        QCOMPARE(restarted.GetDisplayTargetPeak(), 2000);
        QCOMPARE(int(restarted.GetRenderBackend()), 1);
        QCOMPARE(int(restarted.GetPlaceboUpscaler()), 13);
        PsnBrowserOpener opener;
        QDesktopServices::setUrlHandler("https", &opener, "open");
        const auto resetOpener = qScopeGuard([] { QDesktopServices::unsetUrlHandler("https"); });
        script(setup.view(), "document.querySelector('#openRendererDocumentation').click()");
        QTRY_COMPARE(opener.url, QUrl("https://libplacebo.org/options/"));
        script(setup.view(), "document.querySelector('#copyRendererDocumentationUrl').click()");
        QTRY_COMPARE(QGuiApplication::clipboard()->text(), QStringLiteral("https://libplacebo.org/options/"));
    }

    void rendererFiles()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto cleanup = qScopeGuard([&] {
            for(const auto &name : {profile, profile + "-next"})
                QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + name).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
            QSettings(QCoreApplication::organizationName(), "pl_render_params").clear();
        });
        QSettings(QCoreApplication::organizationName(), "pl_render_params").clear();
        QTemporaryDir files;
        QVERIFY(files.isValid());
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetPlaceboPreset(PlaceboPreset::Custom);
        settings.SetPlaceboDebandIterations(7);
        QmlNativeServices services(&settings);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        backend->setDiscoveryEnabled(false);
        SetupWindow setup(backend);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled===false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#disconnectAction')").toBool());
        script(setup.view(), "[...document.querySelectorAll('.category-nav button')].find(e=>e.textContent==='Renderer').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#exportPlaceboSettings')").toBool());
        const auto pick = [&](const QString &action, const QString &path, bool cancel = false, const std::function<void()> &before = {}) {
            QTimer completion;
            completion.setInterval(10);
            connect(&completion, &QTimer::timeout, &completion, [&] {
                for(auto *widget : QApplication::topLevelWidgets()) if(auto *picker = qobject_cast<QFileDialog *>(widget)) {
                    if(!picker->isVisible()) continue;
                    completion.stop();
                    if(before) before();
                    if(!cancel) picker->selectFile(path);
                    // Deliver the native picker result, including OS/file races, without replacing the bridge or file service.
                    QMetaObject::invokeMethod(picker, "done", Qt::DirectConnection, Q_ARG(int, cancel ? QDialog::Rejected : QDialog::Accepted));
                    return;
                }
            });
            completion.start();
            QTRY_VERIFY(script(setup.view(), QString("document.getElementById('%1')?.disabled===false").arg(action)).toBool());
            script(setup.view(), QString("document.getElementById('%1').click()").arg(action));
            QTRY_VERIFY(!completion.isActive());
        };
        const auto output = files.filePath("renderer.ini");
        pick("exportPlaceboSettings", output);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.settings-message')?.textContent").toString(), QStringLiteral("Renderer settings exported."));
        QSettings exported(output, QSettings::IniFormat);
        QCOMPARE(exported.value("placebo_settings/deband_iterations").toInt(), 7);
        QVERIFY(!exported.contains("settings/psn_auth_token"));
        const auto previous = settings.GetPlaceboValues();
        QFile exportedFile(output);
        QVERIFY(exportedFile.open(QIODevice::ReadOnly));
        const auto exportedBytes = exportedFile.readAll();
        exportedFile.close();
        pick("exportPlaceboSettings", output, true);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.settings-message')?.textContent").toString(), QStringLiteral("Export cancelled."));
        QVERIFY(QFile::setPermissions(output, QFile::ReadOwner | QFile::ReadGroup | QFile::ReadOther));
        const auto resetPermissions = qScopeGuard([&] { QFile::setPermissions(output, QFile::ReadOwner | QFile::WriteOwner); });
        pick("exportPlaceboSettings", output);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.settings-message')?.textContent.includes('could not be exported')").toBool());
        QVERIFY(exportedFile.open(QIODevice::ReadOnly));
        QCOMPARE(exportedFile.readAll(), exportedBytes);
        exportedFile.close();
        QVERIFY(QFile::setPermissions(output, QFile::ReadOwner | QFile::WriteOwner));
        QCOMPARE(settings.GetPlaceboValues(), previous);
        const auto invalid = files.filePath("invalid.ini");
        { QFile file(invalid); QVERIFY(file.open(QIODevice::WriteOnly)); file.write("[placebo_settings]\nversion=0\nupscaler=not-a-filter\n"); }
        pick("importPlaceboSettings", invalid);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.settings-message')?.textContent.includes('could not be imported')").toBool());
        QCOMPARE(settings.GetPlaceboValues(), previous);
        pick("importPlaceboSettings", {}, true);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.settings-message')?.textContent").toString(), QStringLiteral("Import cancelled."));
        QCOMPARE(settings.GetPlaceboValues(), previous);
        const auto badExport = files.filePath("missing/renderer.ini");
        pick("exportPlaceboSettings", badExport);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.settings-message')?.textContent.includes('could not be exported')").toBool());
        QCOMPARE(settings.GetPlaceboValues(), previous);
        { QFile file(invalid); QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate)); file.write("[placebo_settings]\nversion=0\nupscaler=fsrcnnx_x2_8_0_4_1\ndeband=yes\ndeband_iterations=3\n"); }
        pick("importPlaceboSettings", invalid);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.settings-message')?.textContent").toString(), QStringLiteral("Renderer settings imported."));
        QCOMPARE(settings.GetPlaceboDebandIterations(), 3);
        QCOMPARE(int(settings.GetPlaceboUpscaler()), 12);
        QTRY_COMPARE(script(setup.view(), "document.querySelector('#placeboDebandIterations')?.value").toString(), QStringLiteral("3"));
        const auto after = settings.GetPlaceboValues();
        pick("importPlaceboSettings", output, false, [&] { backend->qmlSettings()->setCurrentProfile(profile + "-next"); backend->setDiscoveryEnabled(false); });
        QCOMPARE(backend->nativeSettings()->GetPlaceboValues(), after);
        QVERIFY(!QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson().contains(files.path().toUtf8()));
        QVERIFY(!QJsonDocument::fromVariant(setup.bridge()->snapshot()).toJson().contains("fsrcnnx_x2_8_0_4_1"));
    }

    void rendererRestartPolicy_data()
    {
        QTest::addColumn<int>("runtimeBackend");
        QTest::newRow("Vulkan") << 0;
        QTest::newRow("OpenGL") << 1;
    }

    void rendererRestartPolicy()
    {
        QFETCH(int, runtimeBackend);
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto cleanup = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRenderBackend(static_cast<RenderBackend>(runtimeBackend));
        int launches = 0;
        QString launchedProgram;
        QStringList launchedArguments;
        QmlNativeServices services(&settings, {}, [&](const QString &program, const QStringList &arguments) {
            ++launches;
            launchedProgram = program;
            launchedArguments = arguments;
            return false;
        });
        auto *backend = services.backend();
        backend->stopAutoConnect();
        backend->setDiscoveryEnabled(false);
        SetupWindow setup(backend);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled===false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#disconnectAction')").toBool());
        script(setup.view(), "[...document.querySelectorAll('.category-nav button')].find(e=>e.textContent==='Renderer').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#vSyncEnabled')?.disabled===false").toBool());
        script(setup.view(), "document.querySelector('#vSyncEnabled').click()");
        QTRY_VERIFY(!settings.GetVSyncEnabled());
        QTRY_COMPARE(launches, runtimeBackend == 1 ? 1 : 0);
        if(runtimeBackend == 1)
            QTRY_VERIFY(script(setup.view(), "document.querySelector('.settings-message')?.textContent.includes('could not restart')").toBool());
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#rendererBackend')?.disabled===false").toBool());
        script(setup.view(), QString("{const field=document.querySelector('#rendererBackend');field.value='%1';field.dispatchEvent(new Event('change',{bubbles:true}));}").arg(1 - runtimeBackend));
        QTRY_COMPARE(int(settings.GetRenderBackend()), 1 - runtimeBackend);
        QTRY_COMPARE(launches, runtimeBackend == 1 ? 2 : 1);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.settings-message')?.textContent.includes('could not restart')").toBool());
        QCOMPARE(launchedProgram, QCoreApplication::applicationFilePath());
        QCOMPARE(launchedArguments, QCoreApplication::arguments().mid(1));
        QVERIFY(!setup.bridge()->snapshot().value("rendererStatus").toMap().contains("runtimeBackend"));
        const auto generation = setup.bridge()->snapshot().value("profileGeneration").toString();
        const int previous = launches;
        QVERIFY(settingRequest(setup.view(), "rendererBackend", 1 - runtimeBackend, generation, "setRendererSetting").value("accepted").toBool());
        QVERIFY(settingRequest(setup.view(), "vSyncEnabled", false, generation, "setRendererSetting").value("accepted").toBool());
        QVERIFY(!settingRequest(setup.view(), "rendererBackend", 2, generation, "setRendererSetting").value("accepted").toBool());
        QVERIFY(!settingRequest(setup.view(), "rendererBackend", runtimeBackend, generation + "-old", "setRendererSetting").value("accepted").toBool());
        QCOMPARE(launches, previous);
        script(setup.view(), "[...document.querySelectorAll('.settings-content button')].find(e=>e.textContent==='Restart application').click()");
        QTRY_COMPARE(launches, previous + 1);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.settings-message')?.textContent.includes('could not restart')").toBool());
    }

    void rendererLayout()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto cleanup = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
            QSettings(QCoreApplication::organizationName(), "pl_render_params").clear();
        });
        QSettings(QCoreApplication::organizationName(), "pl_render_params").clear();
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        QmlNativeServices services(&settings);
        services.backend()->stopAutoConnect();
        services.backend()->setDiscoveryEnabled(false);
        SetupWindow setup(services.backend());
        setup.resize(1280, 900);
        setup.show();
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.open-settings')?.disabled===false").toBool());
        script(setup.view(), "document.querySelector('.open-settings').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('#disconnectAction')").toBool());
        script(setup.view(), "{const field=document.querySelector('.settings-search input');field.value='deband iterations';field.dispatchEvent(new Event('input',{bubbles:true}));}");
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.settings-results button').length").toInt(), 1);
        script(setup.view(), "document.querySelector('.settings-results button').click()");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("videoPreset"));
        script(setup.view(), "{const field=document.querySelector('#videoPreset');field.value='5';field.dispatchEvent(new Event('change',{bubbles:true}));}");
        QTRY_COMPARE(int(settings.GetPlaceboPreset()), 5);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('#placeboDebandEnabled')?.disabled===false").toBool());
        script(setup.view(), "document.querySelector('#placeboDebandEnabled').click()");
        QTRY_VERIFY(!settings.GetPlaceboDebandEnabled());
        script(setup.view(), "{const field=document.querySelector('.settings-search input');field.value='deband iterations';field.dispatchEvent(new Event('input',{bubbles:true}));}");
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.settings-results button').length").toInt(), 1);
        script(setup.view(), "document.querySelector('.settings-results button').click()");
        QTRY_COMPARE(script(setup.view(), "document.activeElement.id").toString(), QStringLiteral("placeboDebandEnabled"));
        const QDir evidence(qEnvironmentVariable("CHIAKI_EMBEDDED_EVIDENCE", "."));
        const auto capture = [&](const QString &category, int width, int height, qreal zoom, const QString &file) {
            setup.resize(width, height);
            setup.view()->setZoomFactor(zoom);
            script(setup.view(), QString("[...document.querySelectorAll('.category-nav button')].find(e=>e.textContent==='%1').click()").arg(category));
            QTRY_COMPARE(script(setup.view(), "document.querySelector('h2')?.textContent").toString(), category);
            QTRY_VERIFY(script(setup.view(), "document.documentElement.scrollWidth<=innerWidth && [...document.querySelectorAll('.settings-content input,.settings-content select,.settings-content button')].every(e=>{const r=e.getBoundingClientRect();return r.left>=0 && r.right<=innerWidth})").toBool());
            script(setup.view(), "document.querySelector('h2').scrollIntoView()");
            QTest::qWait(100);
            QVERIFY(setup.view()->grab().save(evidence.filePath(file)));
        };
        capture("Renderer", 390, 720, 1, "renderer-390.png");
        capture("Renderer", 800, 900, 2, "renderer-200-percent.png");
        capture("Display", 390, 720, 1, "display-390.png");
        capture("Display", 800, 900, 2, "display-200-percent.png");
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

    void consoleManagementPrivacy()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetStreamerMode(true);
        ChiakiRegisteredHost registered = {};
        registered.target = CHIAKI_TARGET_PS5_1;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "PRIVATE.EXAMPLE");
        settings.AddRegisteredHost(RegisteredHost(registered));
        settings.SetManualHost(ManualHost(-1, "private.example", true, HostMAC(registered.server_mac)));
        settings.AddHiddenHost(HiddenHost(HostMAC(registered.server_mac), "192.0.2.77"));
        QmlNativeServices services(&settings, {}, {}, {}, controlledDiscovery());
        services.backend()->stopAutoConnect();
        SetupWindow setup(services.backend());
        setup.show();
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 1);
        const auto json = QJsonDocument::fromVariant(consoleRequest(setup.view(), "snapshot", {})).toJson();
        QVERIFY(!json.contains("PRIVATE.EXAMPLE"));
        QVERIFY(!json.contains("private.example"));
        QVERIFY(!json.contains("192.0.2.77"));
        QVERIFY(!json.contains("001122334455"));
        QVERIFY(click(setup.view(), ".console-details"));
        QTRY_VERIFY(script(setup.view(), "document.querySelector('dialog').textContent.includes('hidden')").toBool());
        setup.view()->grab().save(qEnvironmentVariable("CHIAKI_DETAILS_PREVIEW", "embedded-details.png"));
        QVERIFY(click(setup.view(), "[data-console-action=deleteManual]"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-confirm-management]')").toBool());
        QVERIFY(!script(setup.view(), "document.querySelector('dialog').textContent.toLowerCase().includes('private.example')").toBool());
        QVERIFY(click(setup.view(), "[data-cancel-management]"));
        QVERIFY(click(setup.view(), "[data-manage-consoles]"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-delete-registration]')").toBool());
        QVERIFY(!script(setup.view(), "document.querySelector('dialog').textContent.includes('192.0.2.77')").toBool());
        QVERIFY(click(setup.view(), "[data-delete-registration]"));
        QVERIFY(!script(setup.view(), "document.querySelector('dialog').textContent.toLowerCase().includes('private.example')").toBool());
        QVERIFY(click(setup.view(), "[data-cancel-management]"));
        QVERIFY(click(setup.view(), ".console-details"));
        QVERIFY(click(setup.view(), "[data-console-action=pin]"));
        QVERIFY(fill(setup.view(), "[data-saved-pin]", "0123"));
        QCOMPARE(script(setup.view(), "document.querySelector('[data-saved-pin]').type").toString(), QStringLiteral("password"));
        QVERIFY(script(setup.view(), "document.querySelector('.reveal input').disabled").toBool());
        QVERIFY(click(setup.view(), "[data-close-management]"));
        QVERIFY(click(setup.view(), "[data-add-manual]"));
        QVERIFY(fill(setup.view(), "[data-manual-address]", "private-input.example"));
        QVERIFY(fill(setup.view(), "[data-manual-registration]", "later"));
        setup.view()->grab().save(qEnvironmentVariable("CHIAKI_MANUAL_WIDE_PREVIEW", "embedded-manual-wide.png"));
        const auto generation = setup.bridge()->snapshot().value("profileGeneration").toString();
        const auto invalid = consoleRequest(setup.view(), "saveManualAddress", {"https://private-input.example", "later", generation});
        QVERIFY(!invalid.value("accepted").toBool());
        QVERIFY(!invalid.value("message").toString().contains("private-input.example"));
        QVERIFY(!script(setup.view(), "document.querySelector('dialog').textContent.includes('private-input.example')").toBool());
    }

    void consoleManagement()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        ChiakiRegisteredHost registered = {};
        registered.target = CHIAKI_TARGET_PS5_1;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "Office console");
        memcpy(registered.rp_regist_key, "PRIVATEKEY", 10);
        RegisteredHost paired(registered);
        paired.SetConsolePin(paired, "0091");
        settings.AddRegisteredHost(paired);
        const auto manualId = settings.SetManualHost(ManualHost(-1, "console.example", true, HostMAC(registered.server_mac)));
        QmlNativeServices services(&settings, {}, {}, {}, controlledDiscovery());
        auto *backend = services.backend();
        backend->stopAutoConnect();
        SetupWindow setup(backend);
        setup.show();
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 1);
        script(setup.view(), "document.querySelector('.console-details').click()");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('dialog')?.textContent.includes('PlayStation 5')").toBool());
        QVERIFY(script(setup.view(), "document.querySelector('dialog').textContent.includes('console.example')").toBool());
        script(setup.view(), "document.querySelector('[data-console-action=pin]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-saved-pin]')").toBool());
        QCOMPARE(script(setup.view(), "document.querySelector('[data-saved-pin]').value").toString(), QString());
        QVERIFY(fill(setup.view(), "[data-saved-pin]", "12"));
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-save-pin]').disabled").toBool());
        const auto key = setup.bridge()->snapshot().value("selectedKey").toString();
        QVERIFY(!consoleRequest(setup.view(), "manageConsole", {"pin", key, "12"}).value("accepted").toBool());
        QVERIFY(fill(setup.view(), "[data-saved-pin]", "0012"));
        script(setup.view(), "document.querySelector('[data-save-pin]').click()");
        QTRY_COMPARE(settings.GetRegisteredHost(HostMAC(registered.server_mac)).GetConsolePin(), QStringLiteral("0012"));
        QTRY_VERIFY(!script(setup.view(), "!!document.querySelector('[data-saved-pin]')").toBool());
        QTRY_VERIFY(script(setup.view(), "document.activeElement.hasAttribute('data-close-management')").toBool());
        const auto json = QJsonDocument::fromVariant(consoleRequest(setup.view(), "snapshot", {})).toJson();
        QVERIFY(!json.contains("\"0012\""));
        QVERIFY(!json.contains("\"0091\""));
        QVERIFY(!json.contains("PRIVATEKEY"));
        QVERIFY(!json.contains("rp_key"));
        script(setup.view(), "document.querySelector('[data-close-management]').click()");
        QTRY_VERIFY(script(setup.view(), "document.activeElement.classList.contains('console-details')").toBool());
        QVERIFY(click(setup.view(), ".console-details"));
        QVERIFY(click(setup.view(), "[data-console-action=deleteManual]"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-cancel-management]')").toBool());
        QTRY_VERIFY(script(setup.view(), "document.activeElement.hasAttribute('data-cancel-management')").toBool());
        script(setup.view(), "document.querySelector('[data-cancel-management]').click()");
        QCOMPARE(settings.GetManualHosts().size(), 1);
        QCOMPARE(settings.GetRegisteredHosts().size(), 1);
        QVERIFY(click(setup.view(), ".console-details"));
        QVERIFY(click(setup.view(), "[data-console-action=deleteManual]"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-confirm-management]')").toBool());
        script(setup.view(), "document.querySelector('[data-confirm-management]').click()");
        QTRY_COMPARE(settings.GetManualHosts().size(), 0);
        QCOMPARE(settings.GetRegisteredHosts().size(), 1);
        QVERIFY(!settings.GetManualHostExists(manualId));
        QTRY_VERIFY(script(setup.view(), "document.activeElement.hasAttribute('data-add-manual')").toBool());
        settings.SetManualHost(ManualHost(-1, "still-saved.example", true, HostMAC(registered.server_mac)));
        script(setup.view(), "document.querySelector('[data-manage-consoles]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-delete-registration]')").toBool());
        script(setup.view(), "document.querySelector('[data-delete-registration]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-cancel-management]')").toBool());
        script(setup.view(), "document.querySelector('[data-cancel-management]').click()");
        QCOMPARE(settings.GetRegisteredHosts().size(), 1);
        QVERIFY(click(setup.view(), "[data-manage-consoles]"));
        QVERIFY(click(setup.view(), "[data-delete-registration]"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-confirm-management]')").toBool());
        script(setup.view(), "document.querySelector('[data-confirm-management]').click()");
        QTRY_COMPARE(settings.GetRegisteredHosts().size(), 0);
        QCOMPARE(settings.GetManualHosts().size(), 1);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('.console-card').textContent.includes('Not registered')").toBool());
    }

    void consoleTargetGuards()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile + "-next").clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        QmlNativeServices services(&settings, {}, {}, {}, controlledDiscovery());
        auto *backend = services.backend();
        backend->stopAutoConnect();
        SetupWindow setup(backend);
        setup.show();
        DiscoveryHost first = {};
        first.ps5 = true;
        first.host_name = "Office <img src=x onerror=alert(1)>";
        first.host_id = "001122334455";
        first.host_addr = "192.0.2.10";
        first.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
        DiscoveryHost second = first;
        second.host_name = "Other console";
        second.host_id = "001122334466";
        second.host_addr = "192.0.2.20";
        discover(backend, {first, second});
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 2);
        script(setup.view(), "document.querySelector('.console-details').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-console-action=hide]')").toBool());
        QVERIFY(!script(setup.view(), "!!document.querySelector('dialog h3 img')").toBool());
        script(setup.view(), "document.querySelector('[data-console-action=hide]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-cancel-management]')").toBool());
        script(setup.view(), "document.querySelector('[data-cancel-management]').click()");
        QCOMPARE(settings.GetHiddenHosts().size(), 0);
        QVERIFY(click(setup.view(), ".console-details"));
        QVERIFY(click(setup.view(), "[data-console-action=hide]"));
        discover(backend, {second, first});
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.console-name').textContent").toString(), second.host_name);
        script(setup.view(), "document.querySelector('[data-confirm-management]').click()");
        QTRY_COMPARE(settings.GetHiddenHosts().size(), 1);
        QCOMPARE(settings.GetHiddenHosts().first().GetMAC().ToString(), QStringLiteral("001122334455"));
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 1);
        script(setup.view(), "document.querySelector('[data-manage-consoles]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-unhide-console]')").toBool());
        script(setup.view(), "document.querySelector('[data-unhide-console]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-cancel-management]')").toBool());
        script(setup.view(), "document.querySelector('[data-cancel-management]').click()");
        QCOMPARE(settings.GetHiddenHosts().size(), 1);
        QVERIFY(click(setup.view(), "[data-manage-consoles]"));
        QVERIFY(click(setup.view(), "[data-unhide-console]"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-confirm-management]')").toBool());
        script(setup.view(), "document.querySelector('[data-confirm-management]').click()");
        QTRY_COMPARE(settings.GetHiddenHosts().size(), 0);
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 2);
        const auto key = script(setup.view(), "document.querySelector('.console-card').dataset.consoleKey").toString();
        QVERIFY(click(setup.view(), ".console-details"));
        QVERIFY(click(setup.view(), "[data-console-action=hide]"));
        discover(backend, {second, second});
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-confirm-management]').disabled").toBool());
        QVERIFY(!consoleRequest(setup.view(), "manageConsole", {"hide", key, ""}).value("accepted").toBool());
        QCOMPARE(settings.GetHiddenHosts().size(), 0);
        discover(backend, {});
        QVERIFY(!consoleRequest(setup.view(), "manageConsole", {"hide", key, ""}).value("accepted").toBool());
        script(setup.view(), "document.querySelector('[data-close-management]').click()");
        const auto id = settings.SetManualHost(ManualHost(-1, "private.example", false, {}));
        QTRY_COMPARE(script(setup.view(), "document.querySelectorAll('.console-card').length").toInt(), 1);
        const auto manualKey = script(setup.view(), "document.querySelector('.console-card').dataset.consoleKey").toString();
        QVERIFY(click(setup.view(), ".console-details"));
        QVERIFY(click(setup.view(), "[data-console-action=deleteManual]"));
        settings.SetManualHost(ManualHost(id, "changed.example", false, {}));
        QVERIFY(!consoleRequest(setup.view(), "manageConsole", {"deleteManual", manualKey, ""}).value("accepted").toBool());
        QCOMPARE(settings.GetManualHosts().size(), 1);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-confirm-management]').disabled").toBool());
        backend->qmlSettings()->setStreamerMode(true);
        const auto json = QJsonDocument::fromVariant(consoleRequest(setup.view(), "snapshot", {})).toJson();
        QVERIFY(!json.contains("private.example"));
        QVERIFY(!json.contains("changed.example"));
        QVERIFY(!json.contains("192.0.2.10"));
        QVERIFY(!json.contains("001122334455"));
        QVERIFY(click(setup.view(), "[data-close-management]"));
        QVERIFY(click(setup.view(), ".console-details"));
        QTRY_VERIFY(script(setup.view(), "document.querySelector('dialog').textContent.includes('hidden')").toBool());
        QVERIFY(!script(setup.view(), "document.body.textContent.includes('changed.example')").toBool());
        QVERIFY(click(setup.view(), "[data-close-management]"));
        QVERIFY(click(setup.view(), "[data-add-manual]"));
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-manual-address]')?.type === 'password'").toBool());
        QVERIFY(fill(setup.view(), "[data-manual-address]", "private-input.example"));
        setup.resize(390, 720);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('dialog').scrollWidth <= document.querySelector('dialog').clientWidth").toBool());
        setup.view()->grab().save(qEnvironmentVariable("CHIAKI_MANUAL_PREVIEW", "embedded-manual.png"));
        setup.resize(900, 600);
        setup.view()->setZoomFactor(2);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('dialog').scrollWidth <= document.querySelector('dialog').clientWidth").toBool());
        const auto oldGeneration = setup.bridge()->snapshot().value("profileGeneration").toString();
        backend->qmlSettings()->setCurrentProfile(profile + "-next");
        backend->setDiscoveryEnabled(false);
        QVERIFY(!consoleRequest(setup.view(), "saveManualAddress", {"private-input.example", "later", oldGeneration}).value("accepted").toBool());
        QVERIFY(!consoleRequest(setup.view(), "manageConsole", {"deleteManual", manualKey, ""}).value("accepted").toBool());
        QCOMPARE(backend->nativeSettings()->GetManualHosts().size(), 0);
    }

    void manualAddressValidation()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile + "-next").clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        ChiakiRegisteredHost registered = {};
        registered.target = CHIAKI_TARGET_PS5_1;
        memcpy(registered.server_mac, "\x00\x11\x22\x33\x44\x55", 6);
        strcpy(registered.server_nickname, "Office console");
        settings.AddRegisteredHost(RegisteredHost(registered));
        QmlNativeServices services(&settings, {}, {}, {}, controlledDiscovery());
        services.backend()->stopAutoConnect();
        SetupWindow setup(services.backend());
        setup.show();
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-add-manual]')").toBool());
        script(setup.view(), "document.querySelector('[data-add-manual]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-manual-address]')").toBool());
        script(setup.view(), "const input=document.querySelector('[data-manual-address]');input.value='https://example.com';input.dispatchEvent(new Event('input',{bubbles:true}));const choice=document.querySelector('[data-manual-registration]');choice.value='later';choice.dispatchEvent(new Event('change',{bubbles:true}));");
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-save-manual]').disabled").toBool());
        QCOMPARE(settings.GetManualHosts().size(), 0);
        const auto generation = setup.bridge()->snapshot().value("profileGeneration").toString();
        for (const auto &invalid : {"", "https://example.com", "192.0.2.1:9295", "example.com/path", "bad host", "-console", "999.999.999.999"}) {
            QVERIFY(fill(setup.view(), "[data-manual-address]", invalid));
            QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-save-manual]').disabled").toBool());
            QVERIFY(!consoleRequest(setup.view(), "saveManualAddress", {invalid, "later", generation}).value("accepted").toBool());
        }
        QVERIFY(fill(setup.view(), "[data-manual-address]", "  console.example  "));
        const auto registrationKey = script(setup.view(), "document.querySelector('[data-manual-registration]').options[1].value").toString();
        QVERIFY(fill(setup.view(), "[data-manual-registration]", registrationKey));
        ChiakiRegisteredHost other = registered;
        memcpy(other.server_mac, "\x00\x00\x00\x00\x00\x01", 6);
        strcpy(other.server_nickname, "Other console");
        settings.AddRegisteredHost(RegisteredHost(other));
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-manual-registration]').options.length").toInt(), 4);
        script(setup.view(), "document.querySelector('[data-save-manual]').click();document.querySelector('[data-save-manual]').click()");
        QTRY_COMPARE(settings.GetManualHosts().size(), 1);
        const auto savedHost = settings.GetManualHosts().first();
        QCOMPARE(savedHost.GetHost(), QStringLiteral("console.example"));
        QVERIFY(savedHost.GetRegistered());
        QCOMPARE(savedHost.GetMAC().ToString(), QStringLiteral("001122334455"));
        Settings reread(profile);
        QCOMPARE(reread.GetManualHosts().first().GetHost(), QStringLiteral("console.example"));
        QVERIFY(click(setup.view(), "[data-close-management]"));
        QVERIFY(click(setup.view(), "[data-add-manual]"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-manual-address]')").toBool());
        QVERIFY(fill(setup.view(), "[data-manual-address]", "2001:db8::42"));
        QVERIFY(fill(setup.view(), "[data-manual-registration]", "later"));
        QTRY_VERIFY(!script(setup.view(), "document.querySelector('[data-save-manual]').disabled").toBool());
        script(setup.view(), "document.querySelector('[data-save-manual]').click()");
        QTRY_COMPARE(settings.GetManualHosts().size(), 2);
        QVERIFY(click(setup.view(), "[data-close-management]"));
        QVERIFY(click(setup.view(), "[data-add-manual]"));
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-manual-address]')").toBool());
        QVERIFY(fill(setup.view(), "[data-manual-address]", "new.example"));
        QVERIFY(fill(setup.view(), "[data-manual-registration]", registrationKey));
        settings.RemoveRegisteredHost(HostMAC(registered.server_mac));
        settings.AddRegisteredHost(RegisteredHost(registered));
        QVERIFY(!consoleRequest(setup.view(), "saveManualAddress", {"new.example", registrationKey, generation}).value("accepted").toBool());
        QCOMPARE(settings.GetManualHosts().size(), 2);
        QCOMPARE(script(setup.view(), "document.querySelector('[data-manual-address]').value").toString(), QStringLiteral("new.example"));
        services.backend()->qmlSettings()->setCurrentProfile(profile + "-next");
        services.backend()->setDiscoveryEnabled(false);
        QVERIFY(!consoleRequest(setup.view(), "saveManualAddress", {"new.example", "later", generation}).value("accepted").toBool());
        QCOMPARE(services.backend()->nativeSettings()->GetManualHosts().size(), 0);
        QTRY_VERIFY(script(setup.view(), "document.querySelector('[data-save-manual]').disabled").toBool());
    }

    void manualAddress()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName()).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        QmlNativeServices services(&settings, {}, {}, {}, controlledDiscovery());
        services.backend()->stopAutoConnect();
        SetupWindow setup(services.backend());
        setup.show();
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-add-manual]')").toBool());
        script(setup.view(), "document.querySelector('[data-add-manual]').click()");
        QTRY_VERIFY(script(setup.view(), "!!document.querySelector('[data-manual-address]')").toBool());
        script(setup.view(), "const input=document.querySelector('[data-manual-address]');input.value='192.0.2.42';input.dispatchEvent(new Event('input',{bubbles:true}));");
        QTRY_VERIFY(!script(setup.view(), "document.querySelector('[data-save-manual]').disabled").toBool());
        script(setup.view(), "document.querySelector('[data-save-manual]').click()");
        QTRY_COMPARE(settings.GetManualHosts().size(), 1);
        QCOMPARE(settings.GetManualHosts().first().GetHost(), QStringLiteral("192.0.2.42"));
        QVERIFY(!settings.GetManualHosts().first().GetRegistered());
        QTRY_COMPARE(script(setup.view(), "document.querySelector('[data-manual-result]')?.textContent").toString(), QStringLiteral("Address saved"));
        script(setup.view(), "document.querySelector('[data-close-management]').click()");
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.console-name')?.textContent").toString(), QStringLiteral("192.0.2.42"));
        setup.view()->reload();
        QTRY_COMPARE(script(setup.view(), "document.querySelector('.console-name')?.textContent").toString(), QStringLiteral("192.0.2.42"));
        QVERIFY(script(setup.view(), "document.querySelector('.connect-console').disabled").toBool());
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
        QmlNativeServices services(&settings, {}, {}, {}, controlledDiscovery());
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
#ifdef Q_OS_WIN
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
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
