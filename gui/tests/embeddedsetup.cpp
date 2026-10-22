#include <QtTest>
#include <QApplication>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QScopeGuard>
#include <QUuid>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QScreen>
#include <QRegularExpression>
#include <QTcpServer>
#include <atomic>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <stdlib.h>

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

    void discover(QmlBackend *backend, const QList<DiscoveryHost> &hosts)
    {
        // This is the queued completion boundary used by the native discovery callback.
        QVERIFY(QMetaObject::invokeMethod(backend->discoveryManager(), "DiscoveryServiceHosts",
            Qt::QueuedConnection, Q_ARG(QList<DiscoveryHost>, hosts)));
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
        QTRY_VERIFY(!script(setup.view(), "document.querySelector('.connect-console').disabled").toBool());
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
