#include <QtTest>
#include <QApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QUuid>
#include <QScopeGuard>
#include <QQuickItem>
#include "qmlnativeservices.h"
#include "qmlmainwindow.h"
#include "qmlbackend.h"

class NativeServicesTest : public QObject
{
    Q_OBJECT
private slots:
    void streamOnlyRoot()
    {
        const QString profile = QUuid::createUuid().toString(QUuid::WithoutBraces);
        const auto removeSettings = qScopeGuard([&] {
            QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile).clear();
        });
        Settings settings(profile);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetWindowType(WindowType::AdjustableResolution);
        settings.SetGeometry(QRect(40, 40, 720, 480));
        settings.SetStreamGeometry(QRect(80, 80, 1024, 640));
        settings.SetRenderBackend(qEnvironmentVariable("CHIAKI_INPUT_TEST_RENDERER") == "vulkan"
            ? RenderBackend::Vulkan : RenderBackend::OpenGL);
        QmlNativeServices services(&settings);
        services.backend()->stopAutoConnect();
        QmlMainWindow window(services.backend(), services.engine());
        window.show();
        QCOMPARE(window.size(), QSize(1024, 640));
        QCOMPARE(settings.GetGeometry(), QRect(40, 40, 720, 480));
        window.requestActivate();
        QTRY_VERIFY(window.isExposed());
        const auto findProgress = [](auto &&self, QQuickItem *item) -> QQuickItem * {
            if (item->objectName() == "streamLoadingView") return item;
            for (auto *child : item->childItems())
                if (auto *found = self(self, child)) return found;
            return nullptr;
        };
        QQuickItem *progress = nullptr;
        for (auto *candidate : QGuiApplication::allWindows()) {
            auto *quick = qobject_cast<QQuickWindow *>(candidate);
            if (!quick) continue;
            progress = findProgress(findProgress, quick->contentItem());
            if (progress) break;
        }
        QVERIFY2(progress && progress->isVisible(), "A native stream window must show connection progress without a setup root");
        QQuickItem *root = progress;
        while (root && !root->property("streamSettingsOpen").isValid()) root = root->parentItem();
        QVERIFY(root);
        QSignalSpy dismissed(&window, &QmlMainWindow::streamViewDismissed);
        QVERIFY(QMetaObject::invokeMethod(root, "showMainView"));
        QCOMPARE(dismissed.count(), 1);
        QVERIFY(window.isVisible());
        settings.SetHardwareDecoder({});
        StreamSessionConnectInfo info(&settings, CHIAKI_TARGET_PS5_1, "127.0.0.1", "Test console",
            QByteArray(CHIAKI_SESSION_AUTH_SIZE, '\0'), QByteArray(16, '\0'), {}, {}, false, false, false, false);
        auto *backend = services.backend();
        backend->createSession(info);
        QPointer<StreamSession> session = backend->qmlSession();
        QVERIFY(session);
        session->LoginPINRequested(false);
        const QString request = backend->sessionPinRequest().value("id").toString();
        QVERIFY(!request.isEmpty());
        auto *pinHost = root->findChild<QObject *>("liveConsolePinHost");
        QVERIFY(pinHost && pinHost->property("blocking").toBool());
        QVERIFY(window.streamInput()->blocked());
        QVERIFY(QMetaObject::invokeMethod(root, "openDisplaySettings"));
        QVERIFY(!root->property("streamSettingsOpen").toBool());
        QVERIFY(backend->cancelSessionPin(request));
        session->SessionQuit(CHIAKI_QUIT_REASON_STOPPED, {});
        QVERIFY(!session);
        QCOMPARE(window.size(), QSize(1024, 640));
        QCOMPARE(settings.GetGeometry(), QRect(40, 40, 720, 480));
        QCOMPARE(backend->sessionPinRequest().value("phase").toString(), QStringLiteral("cancelled"));
        QVERIFY(backend->dismissSessionPin(request));
        QVERIFY(QMetaObject::invokeMethod(root, "showMainView"));
        QCOMPARE(dismissed.count(), 2);
        QVERIFY(!pinHost->property("blocking").toBool());
    }
    void windowRecreation()
    {
        QQuickWindow setup;
        setup.resize(160, 90);
        QSignalSpy setupFrames(&setup, &QQuickWindow::frameSwapped);
        setup.show();
        QTRY_VERIFY(!setupFrames.isEmpty());
        const auto setupApi = QQuickWindow::graphicsApi();
        setup.hide();
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
        settings.SetRenderBackend(qEnvironmentVariable("CHIAKI_INPUT_TEST_RENDERER") == "vulkan"
            ? RenderBackend::Vulkan : RenderBackend::OpenGL);
        QmlNativeServices services(&settings);
        auto *backend = services.backend();
        backend->stopAutoConnect();
        QVERIFY(!backend->qmlWindow());
        QQmlComponent observer(services.engine());
        observer.setData("import QtQml; import org.streetpea.chiaking; QtObject { property var host: Chiaki.window; property var state: Chiaki.settings }", {});
        QScopedPointer<QObject> state(observer.create());
        QVERIFY2(state, qPrintable(observer.errorString()));
        auto *savedSettings = backend->qmlSettings();
        savedSettings->setAudioVolume(37);
        QSignalSpy changed(backend, &QmlBackend::windowChanged);
        for (int connection = 0; connection < 2; ++connection) {
            QmlMainWindow window(backend, services.engine());
            QCOMPARE(QQuickWindow::graphicsApi(), setupApi);
            QCOMPARE(window.nativeGraphicsApi(), settings.GetRenderBackend() == RenderBackend::Vulkan
                && !qEnvironmentVariableIsSet("CHIAKI_FORCE_VULKAN_FALLBACK")
                ? QSGRendererInterface::Vulkan : QSGRendererInterface::OpenGL);
            QCOMPARE(backend->qmlWindow(), &window);
            QCOMPARE(savedSettings->audioVolume(), 37);
            QCOMPARE(state->property("host").value<QObject *>(), &window);
            window.show();
            QTRY_VERIFY(window.isExposed());
            window.showNormal();
            window.resize(960, 640);
            QTRY_COMPARE(window.size(), QSize(960, 640));
            window.show();
            QCOMPARE(window.size(), QSize(960, 640));
            window.hide();
        }
        const auto framesBeforeReturn = setupFrames.count();
        setup.show();
        QTRY_VERIFY(setupFrames.count() > framesBeforeReturn);
        QVERIFY(!backend->qmlWindow());
        QVERIFY(!state->property("host").value<QObject *>());
        QCOMPARE(backend->qmlSettings(), savedSettings);
        QCOMPARE(changed.count(), 4);
        settings.SetHardwareDecoder({});
        auto streamingWindow = std::make_unique<QmlMainWindow>(backend, services.engine());
        StreamSessionConnectInfo info(&settings, CHIAKI_TARGET_PS5_1, "127.0.0.1", "Test console",
            QByteArray(CHIAKI_SESSION_AUTH_SIZE, '\0'), QByteArray(16, '\0'), {}, {}, false, false, false, false);
        backend->createSession(info);
        QPointer<StreamSession> session = backend->qmlSession();
        QVERIFY(session);
        session->SessionQuit(CHIAKI_QUIT_REASON_STOPPED, {});
        QVERIFY(!session);
        QVERIFY(!backend->connectionAttempt().value("cleanupPending").toBool());
        backend->createSession(info);
        session = backend->qmlSession();
        QVERIFY(session);
        streamingWindow.reset();
        QVERIFY(!backend->qmlSession());
        QVERIFY(!session);
        backend->createSession(info);
        QVERIFY(!backend->qmlSession());
        QCOMPARE(backend->connectionAttempt().value("phase").toString(), QStringLiteral("failure"));
        settings.SetWindowType(WindowType::AdjustableResolution);
        settings.SetGeometry(QRect(40, 40, 720, 480));
        settings.SetStreamGeometry(QRect(80, 80, 1024, 640));
        auto direct = std::make_unique<QmlMainWindow>(backend, services.engine(), info);
        direct->show();
        QVERIFY(direct->directStream());
        QCOMPARE(direct->size(), QSize(1024, 640));
        QCOMPARE(settings.GetGeometry(), QRect(40, 40, 720, 480));
        QTimer timeout;
        timeout.setSingleShot(true);
        connect(&timeout, &QTimer::timeout, qApp, [] { QCoreApplication::exit(1); });
        timeout.start(2000);
        QTimer::singleShot(0, direct.get(), [backend] {
            backend->qmlSession()->SessionQuit(CHIAKI_QUIT_REASON_STOPPED, {});
        });
        QCOMPARE(QCoreApplication::exec(), 0);
        direct.reset();
        Settings nextProfile(profile + "-next");
        nextProfile.SetAudioVolume(61);
        savedSettings->setCurrentProfile(profile + "-next");
        QVERIFY(!backend->qmlWindow());
        QCOMPARE(backend->qmlSettings(), savedSettings);
        QCOMPARE(savedSettings->audioVolume(), 61);
        savedSettings->setAudioVolume(62);
        Settings reloaded(profile + "-next");
        QCOMPARE(reloaded.GetAudioVolume(), 62);
    }
};

int main(int argc, char **argv)
{
    if (chiaki_lib_init() != CHIAKI_ERR_SUCCESS) return 1;
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Chiaki native services tests");
    QCoreApplication::setApplicationName("Chiaki native services tests");
    NativeServicesTest test;
    return QTest::qExec(&test, argc, argv);
}
#include "nativeservices.moc"
