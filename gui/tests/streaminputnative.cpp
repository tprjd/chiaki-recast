#include <QtTest>
#include <QApplication>
#include <memory>
#include <algorithm>
#include <QUuid>
#include <QQuickItem>
#include <QQmlComponent>
#include <QQmlEngine>
#include <qpa/qwindowsysteminterface.h>
#include "qmlmainwindow.h"
#include "qmlbackend.h"
#include "qmlnativeservices.h"
#include "qmlsettings.h"
#include "streamsession.h"
#ifdef Q_OS_WIN
#include <windows.h>
#endif

static ChiakiControllerState outgoing;
static bool mouseMotionObserved = false;
extern "C" ChiakiErrorCode __wrap_chiaki_session_set_controller_state(ChiakiSession *, ChiakiControllerState *state)
{
    outgoing = *state;
    mouseMotionObserved |= state->right_x != 0 || state->right_y != 0;
    return CHIAKI_ERR_SUCCESS;
}

struct TestSettingsProfile {
    const QString name = QUuid::createUuid().toString(QUuid::WithoutBraces);
    ~TestSettingsProfile() {
        QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + name).clear();
    }
};

class InputTestWindow : public QmlMainWindow
{
public:
    InputTestWindow(QmlNativeServices &services, bool controlledForeground)
        : QmlMainWindow(services.backend(), services.engine()), controlled_foreground(controlledForeground) {}
protected:
    bool gameplayForeground() const override
    {
        return controlled_foreground ? isActive() : QmlMainWindow::gameplayForeground();
    }
private:
    bool controlled_foreground;
};

class StreamInputNativeTest : public QObject
{
    Q_OBJECT
private:
    void runNative(bool nativeKeys, bool platformKeys = false)
    {
        if (QGuiApplication::platformName() == "offscreen") QSKIP("Requires desktop activation and an OpenGL or Vulkan device.");
        TestSettingsProfile profile;
        Settings settings(profile.name);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetRenderBackend(qEnvironmentVariable("CHIAKI_INPUT_TEST_RENDERER") == "vulkan" ? RenderBackend::Vulkan : RenderBackend::OpenGL);
        settings.SetHardwareDecoder({});
        settings.SetKeyboardEnabled(true);
        QmlNativeServices services(&settings);
        InputTestWindow window(services, platformKeys);
        auto *backend = window.getBackend();
        backend->stopAutoConnect();
        StreamSessionConnectInfo info(&settings, CHIAKI_TARGET_PS5_1, "127.0.0.1", "Test console",
            QByteArray(CHIAKI_SESSION_AUTH_SIZE, '\0'), QByteArray(16, '\0'), {}, {}, false, false, false, false);
        std::unique_ptr<StreamSession> firstSession, replacementSession;
        try {
            firstSession = std::make_unique<StreamSession>(info);
            replacementSession = std::make_unique<StreamSession>(info);
        } catch (const std::exception &error) { QFAIL(error.what()); }
        auto &first = *firstSession;
        auto &replacement = *replacementSession;
        struct SessionBoundary {
            QmlBackend *backend;
            ~SessionBoundary() { backend->session = nullptr; backend->sessionChanged(nullptr); }
        } cleanup{backend};
        const auto changeSession = [backend](StreamSession *session) {
            backend->session = session;
            backend->sessionChanged(session);
        };
        changeSession(&first);
        window.resize(900, 600);
        window.show();
        window.requestActivate();
        QTRY_VERIFY(window.isActive());
        auto *input = window.streamInput();
        auto *root = window.quick_item;
        QVERIFY(root);
        QQuickItem owner(window.quick_window->contentItem());
        owner.setVisible(true);
        if (!nativeKeys && !platformKeys) {
        QQuickItem settingsOpener(window.quick_window->contentItem());
        settingsOpener.forceActiveFocus(Qt::TabFocusReason);
        QVERIFY(QMetaObject::invokeMethod(root, "openDisplaySettings"));
        QVERIFY(input->blocked());
        QVERIFY(root->property("streamSettingsOpen").toBool());
        QVERIFY(QMetaObject::invokeMethod(root, "closeDialog"));
        QVERIFY(!input->blocked());
        QVERIFY(settingsOpener.hasFocus());
        QVERIFY(QMetaObject::invokeMethod(root, "openPlaceboSettings"));
        QVERIFY(QMetaObject::invokeMethod(root, "showPlaceboColorMappingDialog", Q_ARG(QVariant, QVariant())));
        QVERIFY(input->blocked());
        QVERIFY(QMetaObject::invokeMethod(root, "closeDialog"));
        QVERIFY(input->blocked());
        QVERIFY(QMetaObject::invokeMethod(root, "showMainView"));
        QVERIFY(!input->blocked());
        QVERIFY(!root->property("streamSettingsOpen").toBool());
        const auto presentation = input->present(&owner, "display", &owner);
        QVERIFY(input->blocked());
        QCOMPARE(window.cursor().shape(), Qt::ArrowCursor);
        first.touch_state.buttons = CHIAKI_CONTROLLER_BUTTON_CROSS;
        first.SendFeedbackState();
        QCOMPARE(outgoing.buttons, 0u);
        input->dismiss(presentation);
        input->settled(presentation);
        QVERIFY(!input->blocked());
        QCOMPARE(outgoing.buttons, 0u);
        first.touch_state.buttons = 0;
        first.SendFeedbackState();
        first.touch_state.buttons = CHIAKI_CONTROLLER_BUTTON_CROSS;
        first.SendFeedbackState();
        QCOMPARE(outgoing.buttons, uint32_t(CHIAKI_CONTROLLER_BUTTON_CROSS));
        const auto next = input->present(&owner, "pin", nullptr);
        changeSession(&replacement);
        QCOMPARE(input->present(&owner, "pin", nullptr), next);
        QVERIFY(input->blocked());
        replacement.touch_state.buttons = CHIAKI_CONTROLLER_BUTTON_CROSS;
        replacement.DpadSendFeedbackState();
        QCOMPARE(outgoing.buttons, 0u);
        input->dismiss(next);
        input->settled(next);
        replacement.touch_state.buttons = 0;
        replacement.DpadSendFeedbackState();
        replacement.touch_state.buttons = CHIAKI_CONTROLLER_BUTTON_CROSS;
        replacement.DpadSendFeedbackState();
        QCOMPARE(outgoing.buttons, uint32_t(CHIAKI_CONTROLLER_BUTTON_CROSS));
        replacement.touch_state.buttons = 0;
        replacement.SendFeedbackState();
        } else changeSession(&replacement);
#ifdef Q_OS_WIN
        if (nativeKeys || platformKeys) {
            const auto settingsHidden = [&window] {
                const auto windows = QGuiApplication::topLevelWindows();
                return std::none_of(windows.cbegin(), windows.cend(), [&window](QWindow *candidate) {
                    return candidate != &window && candidate->isVisible() && candidate->type() == Qt::Dialog;
                });
            };
            QTRY_VERIFY(settingsHidden());
            settings.SetMouseCameraEnabled(true);
            replacement.connected = true;
            replacement.ConnectedChanged();
            window.requestActivate();
            QTRY_COMPARE(QGuiApplication::focusWindow(), &window);
            QVERIFY(window.gameplayCaptureAvailable());
            if (nativeKeys) {
                SetForegroundWindow(reinterpret_cast<HWND>(window.winId()));
                QTRY_COMPARE(GetForegroundWindow(), reinterpret_cast<HWND>(window.winId()));
            }
            struct ReleaseKeys {
                bool native;
                ~ReleaseKeys() {
                    if (!native) return;
                    for (const WORD code : {WORD(VK_F8), WORD(VK_SPACE), WORD(VK_CONTROL), WORD('Q'), WORD('O'), WORD(VK_F11)}) {
                        INPUT event = {};
                        event.type = INPUT_KEYBOARD;
                        event.ki.wVk = code;
                        event.ki.dwFlags = KEYEVENTF_KEYUP;
                        SendInput(1, &event, sizeof(INPUT));
                    }
                }
            } releaseKeys{nativeKeys};
            Qt::KeyboardModifiers modifiers;
            bool f8Down = false;
            const auto key = [&](WORD code, bool down) -> UINT {
                if (platformKeys) {
                    int mapped = code;
                    if (code == VK_F8) mapped = Qt::Key_F8;
                    if (code == VK_F11) mapped = Qt::Key_F11;
                    if (code == VK_CONTROL) { mapped = Qt::Key_Control; modifiers = down ? Qt::ControlModifier : Qt::NoModifier; }
                    const bool repeat = code == VK_F8 && down && f8Down;
                    if (code == VK_F8) f8Down = down;
                    QWindowSystemInterface::handleKeyEvent<QWindowSystemInterface::SynchronousDelivery>(
                        &window, GetTickCount(), down ? QEvent::KeyPress : QEvent::KeyRelease, mapped, modifiers, {}, repeat);
                    return 1;
                }
                INPUT input = {};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = code;
                input.ki.dwFlags = down ? 0 : KEYEVENTF_KEYUP;
                return SendInput(1, &input, sizeof(INPUT));
            };
            QCOMPARE(key(VK_F8, true), 1u);
            QTRY_VERIFY(window.property("gameplayCaptured").toBool());
            QCOMPARE(key(VK_F8, true), 1u);
            QTest::qWait(30);
            QVERIFY(window.property("gameplayCaptured").toBool());
            QCOMPARE(key(VK_F8, false), 1u);
            if (platformKeys) {
                const auto modifier = [&](WORD code, quint32 scan, bool down, bool obsolete = false) {
                    MSG message = {};
                    message.hwnd = reinterpret_cast<HWND>(window.winId());
                    message.message = down ? WM_KEYDOWN : WM_KEYUP;
                    message.wParam = code;
                    message.lParam = (scan & 0xff) << 16 | (scan & 0xe000 ? 0x01000000 : 0);
                    message.time = GetTickCount() - (obsolete ? 1000 : 0);
                    qintptr result = 0;
                    QWindowSystemInterface::handleNativeEvent(&window, "windows_generic_MSG", &message, &result);
                };
                for (WORD code : {WORD(VK_SHIFT), WORD(VK_CONTROL)}) {
                    const quint32 left = code == VK_SHIFT ? 0x2a : 0x1d;
                    const quint32 right = code == VK_SHIFT ? 0x36 : 0xe01d;
                    const uint32_t button = code == VK_SHIFT ? CHIAKI_CONTROLLER_BUTTON_L3 : CHIAKI_CONTROLLER_BUTTON_MOON;
                    modifier(code, left, true, true);
                    QCOMPARE(outgoing.buttons, 0u);
                    modifier(code, left, true);
                    modifier(code, right, true);
                    modifier(code, left, false);
                    QCOMPARE(outgoing.buttons, button);
                    modifier(code, right, false);
                    QCOMPARE(outgoing.buttons, 0u);
                }
            }
            QCOMPARE(key(VK_SPACE, true), 1u);
            QTRY_COMPARE(outgoing.buttons, uint32_t(CHIAKI_CONTROLLER_BUTTON_CROSS));
            QCOMPARE(key(VK_SPACE, false), 1u);
            QTRY_COMPARE(outgoing.buttons, 0u);
            QCOMPARE(key(VK_CONTROL, true), 1u);
            QCOMPARE(key('Q', true), 1u);
            QTRY_COMPARE(outgoing.buttons, uint32_t(CHIAKI_CONTROLLER_BUTTON_MOON | CHIAKI_CONTROLLER_BUTTON_L1));
            QVERIFY(window.isVisible());
            QCOMPARE(key('Q', false), 1u);
            QCOMPARE(key('O', true), 1u);
            QVERIFY(!input->blocked());
            QVERIFY(window.gameplayCaptured());
            QCOMPARE(key('O', false), 1u);
            QCOMPARE(key(VK_CONTROL, false), 1u);
            QTRY_COMPARE(outgoing.buttons, 0u);
            QCOMPARE(key(VK_F8, true), 1u);
            QTRY_VERIFY(!window.gameplayCaptured());
            QCOMPARE(key(VK_F8, false), 1u);
            QCOMPARE(key(VK_SPACE, true), 1u);
            QTest::qWait(20);
            QCOMPARE(outgoing.buttons, 0u);
            if (nativeKeys) {
                QCOMPARE(key(VK_F8, true), 1u);
                QTRY_VERIFY(window.gameplayCaptured());
                QCOMPARE(key(VK_F8, false), 1u);
                QCOMPARE(outgoing.buttons, 0u);
            }
            QCOMPARE(key(VK_SPACE, false), 1u);
            if (!nativeKeys) {
                QCOMPARE(key(VK_F8, true), 1u);
                QVERIFY(window.gameplayCaptured());
                QCOMPARE(key(VK_F8, false), 1u);
            }
            QWindowSystemInterface::handleKeyEvent<QWindowSystemInterface::SynchronousDelivery>(
                &window, GetTickCount() - 1000, QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
            QCOMPARE(outgoing.buttons, 0u);
            const auto captureOwner = input->present(&owner, "menu");
            QObject secondOwner;
            const auto secondCaptureOwner = input->present(&secondOwner, "dialog");
            QTRY_VERIFY(!window.property("gameplayCaptured").toBool());
            QCOMPARE(outgoing.buttons, 0u);
            input->dismiss(captureOwner);
            input->settled(captureOwner);
            QVERIFY(input->blocked());
            QCOMPARE(key(VK_F8, true), 1u);
            QTest::qWait(20);
            QVERIFY(!window.gameplayCaptured());
            QCOMPARE(key(VK_F8, false), 1u);
            input->dismiss(secondCaptureOwner);
            input->settled(secondCaptureOwner);
            QVERIFY(!window.property("gameplayCaptured").toBool());
            QCOMPARE(key(VK_F8, true), 1u);
            QTRY_VERIFY(window.property("gameplayCaptured").toBool());
            QCOMPARE(key(VK_F8, false), 1u);
            changeSession(&first);
            QVERIFY(!window.gameplayCaptured());
            first.connected = true;
            first.ConnectedChanged();
            QCOMPARE(key(VK_F8, true), 1u);
            QTRY_VERIFY(window.gameplayCaptured());
            QCOMPARE(key(VK_F8, false), 1u);
            replacement.connected = false;
            replacement.ConnectedChanged();
            QVERIFY(window.gameplayCaptured());
            first.connected = false;
            first.ConnectedChanged();
            QTRY_VERIFY(!window.property("gameplayCaptured").toBool());
        }
#endif
        changeSession(nullptr);
        QVERIFY(!input->blocked());
        window.hide();
    }
private:
    void runMouse(bool nativeInput)
    {
#ifndef Q_OS_WIN
        QSKIP("Mouse capture requires Windows.");
#else
        TestSettingsProfile profile;
        Settings settings(profile.name);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetRenderBackend(RenderBackend::OpenGL);
        settings.SetHardwareDecoder({});
        settings.SetMouseCameraEnabled(true);
        settings.SetHideCursor(true);
        QmlNativeServices services(&settings);
        InputTestWindow window(services, !nativeInput);
        auto *backend = window.getBackend();
        backend->stopAutoConnect();
        StreamSessionConnectInfo info(&settings, CHIAKI_TARGET_PS5_1, "127.0.0.1", "Test console",
            QByteArray(CHIAKI_SESSION_AUTH_SIZE, '\0'), QByteArray(16, '\0'), {}, {}, false, false, false, false);
        StreamSession session(info);
        struct SessionBoundary {
            QmlBackend *backend;
            ~SessionBoundary() { backend->session = nullptr; backend->sessionChanged(nullptr); }
        } cleanup{backend};
        session.connected = true;
        backend->session = &session;
        backend->sessionChanged(&session);
        window.resize(900, 600);
        window.show();
        window.requestActivate();
        QTRY_VERIFY(window.isActive());
        const auto hwnd = reinterpret_cast<HWND>(window.winId());
        if (nativeInput) {
            SetForegroundWindow(hwnd);
            QTRY_COMPARE(GetForegroundWindow(), hwnd);
        }
        const auto registration = [] {
            UINT count = 0;
            GetRegisteredRawInputDevices(nullptr, &count, sizeof(RAWINPUTDEVICE));
            std::vector<RAWINPUTDEVICE> devices(count);
            if (count) GetRegisteredRawInputDevices(devices.data(), &count, sizeof(RAWINPUTDEVICE));
            for (const auto &device : devices)
                if (device.usUsagePage == 1 && device.usUsage == 2) return device;
            return RAWINPUTDEVICE{1, 2, RIDEV_REMOVE, nullptr};
        };
        struct RestoreRegistration {
            RAWINPUTDEVICE original;
            ~RestoreRegistration() { RegisterRawInputDevices(&original, 1, sizeof(original)); }
        } restore{registration()};
        QWindow other;
        RAWINPUTDEVICE previous = {1, 2, 0, reinterpret_cast<HWND>(other.winId())};
        QVERIFY(RegisterRawInputDevices(&previous, 1, sizeof(previous)));
        RECT before;
        QVERIFY(GetClipCursor(&before));
        window.has_video = true;
        window.startGameplayCapture(GetTickCount());
        QVERIFY(window.gameplayCaptured());
        QCOMPARE(registration().hwndTarget, hwnd);
        QCOMPARE(registration().dwFlags, DWORD(0));
        RECT client;
        QVERIFY(GetClientRect(hwnd, &client));
        POINT topLeft{client.left, client.top}, bottomRight{client.right, client.bottom};
        QVERIFY(ClientToScreen(hwnd, &topLeft));
        QVERIFY(ClientToScreen(hwnd, &bottomRight));
        RECT clipped;
        QVERIFY(GetClipCursor(&clipped));
        QCOMPARE(clipped.left, topLeft.x);
        QCOMPARE(clipped.right, bottomRight.x);
        QCOMPARE(clipped.top, topLeft.y);
        QCOMPARE(clipped.bottom, bottomRight.y);
        QCOMPARE(window.cursor().shape(), Qt::BlankCursor);
        window.resize(960, 640);
        QCoreApplication::processEvents();
        QVERIFY(window.gameplayCaptured());
        QVERIFY(GetClientRect(hwnd, &client));
        bottomRight = {client.right, client.bottom};
        QVERIFY(ClientToScreen(hwnd, &bottomRight));
        QVERIFY(GetClipCursor(&clipped));
        QCOMPARE(clipped.right, bottomRight.x);
        QCOMPARE(clipped.bottom, bottomRight.y);
        const auto mouse = [&window](QEvent::Type type, Qt::MouseButton button, ulong timestamp) {
            QMouseEvent event(type, QPointF(10, 10), QPointF(10, 10), button,
                type == QEvent::MouseButtonRelease ? Qt::NoButton : button, Qt::NoModifier);
            event.setTimestamp(timestamp);
            QCoreApplication::sendEvent(&window, &event);
        };
        mouse(QEvent::MouseButtonPress, Qt::LeftButton, GetTickCount());
        QCOMPARE(outgoing.r2_state, uint8_t(255));
        mouse(QEvent::MouseButtonRelease, Qt::LeftButton, GetTickCount());
        const auto state = window.windowState();
        mouse(QEvent::MouseButtonDblClick, Qt::LeftButton, GetTickCount());
        QCOMPARE(outgoing.r2_state, uint8_t(255));
        QCOMPARE(window.windowState(), state);
        mouse(QEvent::MouseButtonRelease, Qt::LeftButton, GetTickCount());
        QCOMPARE(outgoing.r2_state, uint8_t(0));
        mouseMotionObserved = false;
        if (nativeInput) {
            INPUT input = {};
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_MOVE_NOCOALESCE;
            input.mi.dx = 8;
            input.mi.dy = -4;
            QCOMPARE(SendInput(1, &input, sizeof(input)), 1u);
        } else session.HandleRelativeMouseMove(8, -4);
        QTRY_VERIFY(mouseMotionObserved);
        QTRY_COMPARE(outgoing.right_x, int16_t(0));
        QCOMPARE(outgoing.right_y, int16_t(0));
        QQuickItem owner;
        const auto presentation = window.streamInput()->present(&owner, "menu");
        QVERIFY(!window.gameplayCaptured());
        QCOMPARE(registration().hwndTarget, previous.hwndTarget);
        QVERIFY(GetClipCursor(&clipped));
        QVERIFY(EqualRect(&before, &clipped));
        QCOMPARE(window.cursor().shape(), Qt::ArrowCursor);
        window.streamInput()->dismiss(presentation);
        window.streamInput()->settled(presentation);
        QCOMPARE(window.cursor().shape(), Qt::ArrowCursor);
        window.startGameplayCapture(GetTickCount());
        QVERIFY(window.gameplayCaptured());
        mouse(QEvent::MouseButtonPress, Qt::LeftButton, GetTickCount() - 1000);
        QCOMPARE(outgoing.r2_state, uint8_t(0));
        QWindow newestOwner;
        RAWINPUTDEVICE newer = {1, 2, 0, reinterpret_cast<HWND>(newestOwner.winId())};
        QVERIFY(RegisterRawInputDevices(&newer, 1, sizeof(newer)));
        QFocusEvent focusOut(QEvent::FocusOut);
        QCoreApplication::sendEvent(&window, &focusOut);
        QVERIFY(!window.gameplayCaptured());
        QCOMPARE(registration().hwndTarget, newer.hwndTarget);
        QVERIFY(GetClipCursor(&clipped));
        QVERIFY(EqualRect(&before, &clipped));
#endif
    }
private slots:
    void physicalModifiers_data()
    {
        QTest::addColumn<int>("key");
        QTest::addColumn<quint32>("leftScan");
        QTest::addColumn<quint32>("rightScan");
        QTest::addColumn<quint32>("button");
        QTest::newRow("shift") << int(Qt::Key_Shift) << quint32(0x2a) << quint32(0x36)
            << quint32(CHIAKI_CONTROLLER_BUTTON_L3);
        QTest::newRow("control") << int(Qt::Key_Control) << quint32(0x1d) << quint32(0xe01d)
            << quint32(CHIAKI_CONTROLLER_BUTTON_MOON);
    }
    void physicalModifiers()
    {
#ifndef Q_OS_WIN
        QSKIP("Mouse camera mode is available on Windows.");
#endif
        QFETCH(int, key);
        QFETCH(quint32, leftScan);
        QFETCH(quint32, rightScan);
        QFETCH(quint32, button);
        TestSettingsProfile profile;
        Settings settings(profile.name);
        settings.SetMouseCameraEnabled(true);
        settings.SetHardwareDecoder({});
        StreamSessionConnectInfo info(&settings, CHIAKI_TARGET_PS5_1, "127.0.0.1", "Test console",
            QByteArray(CHIAKI_SESSION_AUTH_SIZE, '\0'), QByteArray(16, '\0'), {}, {}, false, false, false, false);
        StreamSession session(info);
        const auto event = [&](quint32 scan, bool down) {
            QKeyEvent event(down ? QEvent::KeyPress : QEvent::KeyRelease, key, Qt::NoModifier,
                scan, key == Qt::Key_Shift ? 0x10 : 0x11, 0);
            session.HandleKeyboardEvent(&event);
        };
        session.SetGameplayCapture(true);
        event(leftScan, true);
        event(rightScan, true);
        event(leftScan, false);
        QCOMPARE(outgoing.buttons, button);
        event(rightScan, false);
        QCOMPARE(outgoing.buttons, 0u);
        session.SetGameplayCapture(false);
        session.SetGameplayCapture(true, {KeyboardMousePhysicalKey(key, leftScan), KeyboardMousePhysicalKey(key, rightScan)});
        event(leftScan, false);
        event(rightScan, true);
        QCOMPARE(outgoing.buttons, 0u);
        event(leftScan, true);
        QCOMPARE(outgoing.buttons, button);
        event(leftScan, false);
        QCOMPARE(outgoing.buttons, 0u);
        event(rightScan, false);
        event(rightScan, true);
        QCOMPARE(outgoing.buttons, button);
        session.SetGameplayCapture(false);
    }
    void resetKeyboardMouseDefaults()
    {
#ifndef Q_OS_WIN
        QSKIP("Mouse camera mode is available on Windows.");
#endif
        if (QGuiApplication::platformName() == "offscreen") QSKIP("Requires the native main-window renderer.");
        TestSettingsProfile profile;
        Settings settings(profile.name);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetRenderBackend(RenderBackend::OpenGL);
        settings.SetHardwareDecoder({});
        settings.SetKeyboardEnabled(false);
        settings.SetMouseTouchEnabled(false);
        settings.SetControllerButtonMapping(CHIAKI_CONTROLLER_BUTTON_CROSS, Qt::Key_K);
        settings.SetMouseCameraEnabled(true);
        MouseResponse response;
        response.horizontalSensitivity = 2;
        response.verticalSensitivity = 3;
        response.deadZoneCompensation = 0.2;
        response.invertVertical = true;
        QVERIFY(settings.SetMouseResponse(response));
        auto bindings = settings.GetKeyboardMouseBindings();
        bindings[4] = {Qt::Key_J, -int(Qt::MiddleButton)};
        QVERIFY(settings.SetKeyboardMouseBindings(bindings));
        QmlNativeServices services(&settings);
        QmlMainWindow window(services.backend(), services.engine());
        auto *backend = window.getBackend();
        backend->stopAutoConnect();
        StreamSessionConnectInfo info(&settings, CHIAKI_TARGET_PS5_1, "127.0.0.1", "Test console",
            QByteArray(CHIAKI_SESSION_AUTH_SIZE, '\0'), QByteArray(16, '\0'), {}, {}, false, false, false, false);
        StreamSession session(info);
        struct SessionBoundary {
            QmlBackend *backend;
            ~SessionBoundary() { backend->session = nullptr; backend->sessionChanged(nullptr); }
        } cleanup{backend};
        backend->session = &session;
        backend->sessionChanged(&session);
        session.SetMouseResponse(response);
        session.SetKeyboardMouseBindings(bindings);
        session.SetGameplayCapture(true);
        const auto key = [&session](int code, bool down) {
            QKeyEvent event(down ? QEvent::KeyPress : QEvent::KeyRelease, code, Qt::NoModifier);
            session.HandleKeyboardEvent(&event);
        };
        key(Qt::Key_J, true);
        key(Qt::Key_W, true);
        session.HandleRelativeMouseMove(1, 0);
        session.keyboard_mouse.advanceMouse(session.gameplay_mouse_clock.nsecsElapsed() / 1000 + 8000);
        session.SendFeedbackState();
        QCOMPARE(outgoing.buttons, uint32_t(CHIAKI_CONTROLLER_BUTTON_CROSS));
        QVERIFY(outgoing.left_y != 0);
        QVERIFY(outgoing.right_x != 0);
        QSettings persisted(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile.name);
        const auto unrelated = [&] {
            persisted.sync();
            QVariantMap values;
            for (const auto &key : persisted.allKeys())
                if (!key.startsWith("settings/keyboard_mouse/")) values[key] = persisted.value(key);
            return values;
        };
        const auto before = unrelated();
        TestSettingsProfile otherProfile;
        Settings other(otherProfile.name);
        QVERIFY(other.SetMouseResponse(response));
        other.SetMouseCameraEnabled(true);
        QmlSettings bridge(&settings);
        QVERIFY(QMetaObject::invokeMethod(&bridge, "resetKeyboardMouseDefaults"));
        QVERIFY(!settings.GetMouseCameraEnabled());
        QVERIFY(!window.gameplayCaptured());
        QCOMPARE(outgoing.buttons, 0u);
        QCOMPARE(outgoing.left_y, int16_t(0));
        QCOMPARE(outgoing.right_x, int16_t(0));
        QMetaObject::invokeMethod(session.gameplay_mouse_timer, "timeout", Qt::DirectConnection);
        QCOMPARE(outgoing.right_x, int16_t(0));
        QCOMPARE(unrelated(), before);
        Settings restarted(profile.name);
        QVERIFY(!restarted.GetMouseCameraEnabled());
        QCOMPARE(restarted.GetMouseResponse().horizontalSensitivity, 1.0);
        QCOMPARE(restarted.GetMouseResponse().verticalSensitivity, 1.0);
        QCOMPARE(restarted.GetMouseResponse().deadZoneCompensation, 0.0);
        QVERIFY(!restarted.GetMouseResponse().invertVertical);
        QCOMPARE(restarted.GetKeyboardMouseBindings(), DefaultKeyboardMouseBindings());
        Settings otherRestarted(otherProfile.name);
        QCOMPARE(otherRestarted.GetMouseResponse().horizontalSensitivity, 2.0);
        QVERIFY(otherRestarted.GetMouseCameraEnabled());
        bridge.setMouseCameraEnabled(true);
        QVERIFY(!window.gameplayCaptured());
        session.SetMouseResponse(restarted.GetMouseResponse());
        session.SetKeyboardMouseBindings(restarted.GetKeyboardMouseBindings());
        session.SetGameplayCapture(true, {Qt::Key_W});
        QCOMPARE(outgoing.buttons, 0u);
        QCOMPARE(outgoing.left_y, int16_t(0));
        QCOMPARE(outgoing.right_x, int16_t(0));
        key(Qt::Key_W, true);
        QCOMPARE(outgoing.left_y, int16_t(0));
        key(Qt::Key_W, false);
        key(Qt::Key_W, true);
        QCOMPARE(outgoing.left_y, int16_t(-32767));
        key(Qt::Key_J, true);
        QCOMPARE(outgoing.buttons, 0u);
        key(Qt::Key_Space, true);
        QCOMPARE(outgoing.buttons, uint32_t(CHIAKI_CONTROLLER_BUTTON_CROSS));
        key(Qt::Key_X, true);
        QCOMPARE(outgoing.r2_state, uint8_t(255));
        session.HandleRelativeMouseMove(1, 0);
        session.keyboard_mouse.advanceMouse(session.gameplay_mouse_clock.nsecsElapsed() / 1000 + 8000);
        session.SendFeedbackState();
        QVERIFY(outgoing.right_x > 0 && outgoing.right_x <= 2000);
        session.SetGameplayCapture(false);
    }
    void keyboardMouseSettingsRoute()
    {
#ifndef Q_OS_WIN
        QSKIP("Mouse camera mode is available on Windows.");
#endif
        if (QGuiApplication::platformName() == "offscreen") QSKIP("Requires a desktop renderer.");
        TestSettingsProfile profile;
        Settings settings(profile.name);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetMouseCameraEnabled(true);
        settings.SetRenderBackend(qEnvironmentVariable("CHIAKI_INPUT_TEST_RENDERER") == "vulkan" ? RenderBackend::Vulkan : RenderBackend::OpenGL);
        settings.SetHardwareDecoder({});
        QmlNativeServices services(&settings);
        QmlMainWindow window(services.backend(), services.engine());
        auto *backend = window.getBackend();
        backend->stopAutoConnect();
        StreamSessionConnectInfo info(&settings, CHIAKI_TARGET_PS5_1, "127.0.0.1", "Test console",
            QByteArray(CHIAKI_SESSION_AUTH_SIZE, '\0'), QByteArray(16, '\0'), {}, {}, false, false, false, false);
        StreamSession session(info);
        struct SessionBoundary {
            QmlBackend *backend;
            ~SessionBoundary() { backend->session = nullptr; backend->sessionChanged(nullptr); }
        } cleanup{backend};
        backend->session = &session;
        backend->sessionChanged(&session);
        window.resize(900, 600);
        window.show();
        window.requestActivate();
        QTRY_VERIFY(window.isActive());
        QQuickItem opener(window.quick_window->contentItem());
        opener.forceActiveFocus(Qt::TabFocusReason);
        auto *root = window.quick_item;
        QVERIFY(QMetaObject::invokeMethod(root, "openKeyboardMouseSettings"));
        QVERIFY(root->property("streamSettingsOpen").toBool());
        QCOMPARE(root->property("useSeparateStreamSettingsWindows").toBool(), window.runtimeRendererBackend() == 1);
        QVERIFY(window.streamInput()->blocked());
        QVERIFY(backend->session == &session);
        std::function<QQuickItem *(QQuickItem *, const QString &)> visualChild = [&](QQuickItem *parent, const QString &name) -> QQuickItem * {
            if (parent->objectName() == name) return parent;
            for (auto *child : parent->childItems()) if (auto *found = visualChild(child, name)) return found;
            return nullptr;
        };
        QQuickWindow *settingsWindow = window.quick_window;
        if (root->property("useSeparateStreamSettingsWindows").toBool()) {
            settingsWindow = nullptr;
            for (auto *candidate : QGuiApplication::allWindows())
                if (candidate->objectName() == "keyboardMouseSettingsWindow") settingsWindow = qobject_cast<QQuickWindow *>(candidate);
            QVERIFY(settingsWindow);
            QTRY_VERIFY(settingsWindow->isVisible());
        }
        auto *activeDialog = visualChild(settingsWindow->contentItem(), "keyboardMouseSettingsDialog");
        QVERIFY(activeDialog);
        auto *tabs = visualChild(activeDialog, "keyboardMouseTabs");
        QVERIFY(tabs);
        tabs->setProperty("currentIndex", 1);
        QTRY_VERIFY(visualChild(activeDialog, "keyboardMouseBinding4_0"));
        auto *binding = visualChild(activeDialog, "keyboardMouseBinding4_0");
        binding->forceActiveFocus(Qt::TabFocusReason);
        QVERIFY(binding->hasFocus());
        QVERIFY(QMetaObject::invokeMethod(root, "closeDialog"));
        QVERIFY(!root->property("streamSettingsOpen").toBool());
        QVERIFY(!window.streamInput()->blocked());
        QVERIFY(opener.hasFocus());
        QVERIFY(!window.gameplayCaptured());
    }
    void mouseResponseRecapture()
    {
#ifndef Q_OS_WIN
        QSKIP("Mouse camera mode is available on Windows.");
#endif
        TestSettingsProfile profile;
        Settings settings(profile.name);
        settings.SetMouseCameraEnabled(true);
        settings.SetHardwareDecoder({});
        StreamSessionConnectInfo info(&settings, CHIAKI_TARGET_PS5_1, "127.0.0.1", "Test console",
            QByteArray(CHIAKI_SESSION_AUTH_SIZE, '\0'), QByteArray(16, '\0'), {}, {}, false, false, false, false);
        StreamSession session(info);
        session.SetGameplayCapture(true);
        QCOMPARE(session.gameplay_mouse_timer->interval(), 8);
        mouseMotionObserved = false;
        session.HandleRelativeMouseMove(1, 0);
        QTRY_VERIFY_WITH_TIMEOUT(mouseMotionObserved, 200);
        QTRY_COMPARE_WITH_TIMEOUT(outgoing.right_x, int16_t(0), 200);
        session.SetGameplayCapture(false);
        session.SetMouseResponse(settings.GetMouseResponse());
        session.SetGameplayCapture(true);
        session.HandleRelativeMouseMove(1, 0);
        session.keyboard_mouse.advanceMouse(session.gameplay_mouse_clock.nsecsElapsed() / 1000 + 8000);
        session.SendFeedbackState();
        QVERIFY(outgoing.right_x > 0 && outgoing.right_x <= 2000);
        const auto before = outgoing.right_x;
        auto response = settings.GetMouseResponse();
        response.horizontalSensitivity = 2;
        QVERIFY(settings.SetMouseResponse(response));
        QCOMPARE(outgoing.right_x, before);
        session.SetGameplayCapture(false);
        QCOMPARE(outgoing.right_x, 0);
        session.SetMouseResponse(settings.GetMouseResponse());
        session.SetGameplayCapture(true);
        QCOMPARE(outgoing.right_x, 0);
        session.HandleRelativeMouseMove(1, 0);
        session.keyboard_mouse.advanceMouse(session.gameplay_mouse_clock.nsecsElapsed() / 1000 + 8000);
        session.SendFeedbackState();
        QVERIFY(outgoing.right_x > 0 && outgoing.right_x <= 4000);
        session.SetGameplayCapture(false);
    }
    void mouseDesktop() { runMouse(false); }
    void spontaneousMouse()
    {
        if (!qEnvironmentVariableIsSet("CHIAKI_INPUT_TEST_MOUSE"))
            QSKIP("Set CHIAKI_INPUT_TEST_MOUSE=1 for foreground Win32 mouse injection.");
        runMouse(true);
    }
    void bindingEditor()
    {
        if (QGuiApplication::platformName() == "offscreen") QSKIP("Requires the native main-window route.");
        TestSettingsProfile profile;
        Settings settings(profile.name);
        settings.SetDiscoveryEnabled(false);
        settings.SetRemotePlayAsk(false);
        settings.SetAddSteamShortcutAsk(false);
        settings.SetHardwareDecoder({});
        settings.SetRenderBackend(RenderBackend::OpenGL);
        settings.SetMouseCameraEnabled(true);
        QmlNativeServices services(&settings);
        InputTestWindow window(services, true);
        auto *backend = window.getBackend();
        backend->stopAutoConnect();
        StreamSessionConnectInfo info(&settings, CHIAKI_TARGET_PS5_1, "127.0.0.1", "Test console",
            QByteArray(CHIAKI_SESSION_AUTH_SIZE, '\0'), QByteArray(16, '\0'), {}, {}, false, false, false, false);
        StreamSession session(info);
        struct SessionBoundary {
            QmlBackend *backend;
            ~SessionBoundary() { backend->session = nullptr; backend->sessionChanged(nullptr); }
        } cleanup{backend};
        session.connected = true;
        backend->session = &session;
        backend->sessionChanged(&session);
        window.resize(900, 600);
        window.show();
        window.requestActivate();
        QTRY_VERIFY(window.isActive());
        QmlSettings bridge(&settings);
        QQmlComponent component(qmlEngine(window.quick_item), QUrl("qrc:/KeyboardMouseBindingsPane.qml"));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> pane(component.createWithInitialProperties({{"settings", QVariant::fromValue(&bridge)}}));
        QVERIFY(pane);
        auto *item = qobject_cast<QQuickItem *>(pane.get());
        item->setParentItem(window.quick_window->contentItem());
        item->setWidth(800);
        window.streamInput()->present(item, "settings", item);
        QVERIFY(window.streamInput()->blocked());
        std::function<QQuickItem *(QQuickItem *, const QString &)> visualChild = [&](QQuickItem *parent, const QString &name) -> QQuickItem * {
            if(parent->objectName() == name) return parent;
            for(auto *child : parent->childItems()) if(auto *found = visualChild(child, name)) return found;
            return nullptr;
        };
        const auto find = [&](const char *name) {
            auto *found = pane->findChild<QQuickItem *>(QString::fromLatin1(name));
            return found ? found : visualChild(window.quick_window->contentItem(), QString::fromLatin1(name));
        };
        auto *slot = find("keyboardMouseBinding4_0");
        QVERIFY(slot);
        const auto open = [&] {
            slot = find("keyboardMouseBinding4_0");
            QVERIFY(slot);
            QVERIFY(QMetaObject::invokeMethod(slot, "clicked"));
            QTRY_VERIFY(pane->findChild<QObject *>("keyboardMouseBindingDialog")->property("opened").toBool());
        };
        const auto key = [&](int code) {
            for (auto type : {QEvent::KeyPress, QEvent::KeyRelease})
                QWindowSystemInterface::handleKeyEvent<QWindowSystemInterface::SynchronousDelivery>(
                    &window, 100, type, code, Qt::NoModifier);
            QCoreApplication::processEvents();
        };
        open();
        const auto oldWindowState = window.windowState();
        for (int code : {Qt::Key_F8, Qt::Key_F11}) {
            key(code);
            QVERIFY(find("keyboardMouseBindingError")->property("text").toString().contains(code == Qt::Key_F8 ? "F8" : "F11"));
            QCOMPARE(window.windowState(), oldWindowState);
            QVERIFY(!window.property("gameplayCaptured").toBool());
        }
        key(Qt::Key_Escape);
        QTRY_VERIFY(find("keyboardMouseBinding4_0")->hasActiveFocus());
        QCOMPARE(settings.GetKeyboardMouseBindings()[4][0], int(Qt::Key_Space));
        open();
        key(Qt::Key_Q);
        QVERIFY(find("keyboardMouseBindingConflict")->property("visible").toBool());
        QVERIFY(find("keyboardMouseBindingConflict")->property("text").toString().contains("L1"));
        key(Qt::Key_Escape);
        QCOMPARE(settings.GetKeyboardMouseBindings()[8][0], int(Qt::Key_Q));
        QCOMPARE(settings.GetKeyboardMouseBindings()[4][0], int(Qt::Key_Space));
        open();
        key(Qt::Key_Q);
        QVERIFY(QMetaObject::invokeMethod(find("keyboardMouseMoveBinding"), "clicked"));
        QTRY_VERIFY(find("keyboardMouseBinding4_0")->hasActiveFocus());
        QCOMPARE(settings.GetKeyboardMouseBindings()[8][0], 0);
        QCOMPARE(settings.GetKeyboardMouseBindings()[4][0], int(Qt::Key_Q));
        open();
        key(Qt::Key_Tab);
        QVERIFY(find("keyboardMouseAssignTab")->hasActiveFocus());
        key(Qt::Key_Return);
        QVERIFY(find("keyboardMouseBindingConflict")->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(find("keyboardMouseMoveBinding"), "clicked"));
        QTRY_VERIFY(find("keyboardMouseBinding4_0")->hasActiveFocus());
        QCOMPARE(settings.GetKeyboardMouseBindings()[4][0], int(Qt::Key_Tab));
        QCOMPARE(settings.GetKeyboardMouseBindings()[20][0], 0);
        open();
        QVERIFY(QMetaObject::invokeMethod(find("keyboardMouseClearBinding"), "clicked"));
        QTRY_VERIFY(find("keyboardMouseBinding4_0")->hasActiveFocus());
        QCOMPARE(settings.GetKeyboardMouseBindings()[4][0], 0);
        open();
        auto *prompt = find("keyboardMouseBindingPrompt");
        const auto point = prompt->mapToScene(QPointF(prompt->width() / 2, prompt->height() / 2));
        for(auto type : {QEvent::MouseButtonPress, QEvent::MouseButtonRelease}) {
            QMouseEvent click(type, point, point, Qt::MiddleButton,
                type == QEvent::MouseButtonPress ? Qt::MiddleButton : Qt::NoButton, Qt::NoModifier);
            QCoreApplication::sendEvent(&window, &click);
        }
        QTRY_COMPARE(settings.GetKeyboardMouseBindings()[4][0], -int(Qt::MiddleButton));
        open();
        key(Qt::Key_R);
        QVERIFY(find("keyboardMouseBindingConflict")->property("visible").toBool());
        TestSettingsProfile replacementProfile;
        Settings replacementSettings(replacementProfile.name);
        QmlSettings replacementBridge(&replacementSettings);
        pane->setProperty("settings", QVariant::fromValue(&replacementBridge));
        QTRY_VERIFY(!pane->findChild<QObject *>("keyboardMouseBindingDialog")->property("opened").toBool());
        QVERIFY(QMetaObject::invokeMethod(find("keyboardMouseMoveBinding"), "clicked"));
        QCOMPARE(replacementSettings.GetKeyboardMouseBindings()[4][0], int(Qt::Key_Space));
        QCOMPARE(replacementSettings.GetKeyboardMouseBindings()[6][0], int(Qt::Key_R));
        window.hide();
        pane.reset();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
    void bindingValidation()
    {
        TestSettingsProfile profile;
        Settings settings(profile.name);
        const auto defaults = settings.GetKeyboardMouseBindings();
        QVariantList saved;
        for(const auto &action : defaults) for(int input : action) saved.append(input);
        QSettings stored(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + profile.name);
        for(const auto value : {QVariant(Qt::Key_F8), QVariant(-int(Qt::ExtraButton3)), QVariant("invalid"), QVariant(65.5), QVariant(Qt::Key_W)}) {
            auto bad = saved;
            bad[8] = value;
            stored.setValue("settings/keyboard_mouse/bindings", bad);
            QCOMPARE(settings.GetKeyboardMouseBindings(), defaults);
        }
        stored.setValue("settings/keyboard_mouse/bindings", QVariantList{Qt::Key_P});
        QCOMPARE(settings.GetKeyboardMouseBindings(), defaults);
        auto bad = defaults;
        bad[4][0] = Qt::Key_F11;
        QVERIFY(!settings.SetKeyboardMouseBindings(bad));
        QCOMPARE(settings.GetKeyboardMouseBindings(), defaults);
    }
    void editableBindings()
    {
        TestSettingsProfile profile;
        KeyboardMouseBindings saved;
        {
            Settings settings(profile.name);
            QmlSettings bridge(&settings);
            QCOMPARE(bridge.keyboardMouseBindings().size(), 22);
            auto result = bridge.changeKeyboardMouseBinding(4, 1, Qt::Key_P, false);
            QVERIFY(result.value("error").toString().isEmpty());
            QCOMPARE(result.value("conflict").toInt(), -1);
            result = bridge.changeKeyboardMouseBinding(5, 0, Qt::Key_P, false);
            QCOMPARE(result.value("conflict").toInt(), 4);
            QCOMPARE(settings.GetKeyboardMouseBindings()[4][1], int(Qt::Key_P));
            QCOMPARE(settings.GetKeyboardMouseBindings()[5][0], int(Qt::Key_Control));
            result = bridge.changeKeyboardMouseBinding(5, 0, Qt::Key_P, true);
            QCOMPARE(result.value("conflict").toInt(), -1);
            QCOMPARE(settings.GetKeyboardMouseBindings()[4][1], 0);
            QCOMPARE(settings.GetKeyboardMouseBindings()[5][0], int(Qt::Key_P));
            for (const auto key : {Qt::Key_F8, Qt::Key_F11, Qt::Key_Alt, Qt::Key_Meta})
                QVERIFY(!bridge.changeKeyboardMouseBinding(4, 0, key, true).value("error").toString().isEmpty());
            QVERIFY(bridge.changeKeyboardMouseBinding(4, 0, 0, false).value("error").toString().isEmpty());
            QCOMPARE(settings.GetKeyboardMouseBindings()[4][0], 0);
            saved = settings.GetKeyboardMouseBindings();
        }
        Settings settings(profile.name);
        QmlSettings bridge(&settings);
        QCOMPARE(settings.GetKeyboardMouseBindings(), saved);
        QVERIFY(settings.GetKeyboardEnabled());
        settings.SetMouseCameraEnabled(true);
        settings.SetHardwareDecoder({});
        StreamSessionConnectInfo info(&settings, CHIAKI_TARGET_PS5_1, "127.0.0.1", "Test console",
            QByteArray(CHIAKI_SESSION_AUTH_SIZE, '\0'), QByteArray(16, '\0'), {}, {}, false, false, false, false);
        StreamSession session(info);
        session.SetKeyboardMouseBindings(settings.GetKeyboardMouseBindings());
        session.SetGameplayCapture(true);
        QKeyEvent press(QEvent::KeyPress, Qt::Key_P, Qt::NoModifier);
        QKeyEvent release(QEvent::KeyRelease, Qt::Key_P, Qt::NoModifier);
        session.HandleKeyboardEvent(&press);
        QCOMPARE(outgoing.buttons, uint32_t(CHIAKI_CONTROLLER_BUTTON_MOON));
        bridge.changeKeyboardMouseBinding(5, 0, Qt::Key_B, true);
        session.HandleKeyboardEvent(&release);
        session.HandleKeyboardEvent(&press);
        QCOMPARE(outgoing.buttons, uint32_t(CHIAKI_CONTROLLER_BUTTON_MOON));
        session.SetGameplayCapture(false);
        session.SetKeyboardMouseBindings(settings.GetKeyboardMouseBindings());
        session.SetGameplayCapture(true);
        session.HandleKeyboardEvent(&press);
        QCOMPARE(outgoing.buttons, 0u);
        for (auto button : {Qt::LeftButton, Qt::RightButton, Qt::MiddleButton, Qt::BackButton, Qt::ForwardButton}) {
            session.SetGameplayCapture(false);
            bridge.changeKeyboardMouseBinding(4, 0, -int(button), true);
            bridge.changeKeyboardMouseBinding(4, 1, Qt::Key_Space, true);
            session.SetKeyboardMouseBindings(settings.GetKeyboardMouseBindings());
            session.SetGameplayCapture(true);
            QMouseEvent down(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(10, 10), button, button, Qt::NoModifier);
            QMouseEvent up(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10), button, Qt::NoButton, Qt::NoModifier);
            QKeyEvent spaceDown(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
            QKeyEvent spaceUp(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier);
            session.HandleMousePressEvent(&down);
            QCOMPARE(outgoing.buttons, uint32_t(CHIAKI_CONTROLLER_BUTTON_CROSS));
            session.HandleKeyboardEvent(&spaceDown);
            session.HandleMouseReleaseEvent(&up);
            QCOMPARE(outgoing.buttons, uint32_t(CHIAKI_CONTROLLER_BUTTON_CROSS));
            session.HandleKeyboardEvent(&spaceUp);
            QCOMPARE(outgoing.buttons, 0u);
        }
        session.SetGameplayCapture(false);
        bridge.changeKeyboardMouseBinding(4, 0, Qt::Key_F2, true);
        bridge.changeKeyboardMouseBinding(4, 1, 0, false);
        session.SetKeyboardMouseBindings(settings.GetKeyboardMouseBindings());
        session.SetGameplayCapture(true, {Qt::Key_F2});
        QKeyEvent f2Down(QEvent::KeyPress, Qt::Key_F2, Qt::NoModifier);
        QKeyEvent f2Up(QEvent::KeyRelease, Qt::Key_F2, Qt::NoModifier);
        session.HandleKeyboardEvent(&f2Down);
        QCOMPARE(outgoing.buttons, 0u);
        session.HandleKeyboardEvent(&f2Up);
        session.HandleKeyboardEvent(&f2Down);
        QCOMPARE(outgoing.buttons, uint32_t(CHIAKI_CONTROLLER_BUTTON_CROSS));
        session.SetGameplayCapture(false);
        bridge.changeKeyboardMouseBinding(4, 0, Qt::Key_Exclam, true);
        const int baseKey = settings.GetKeyboardMouseBindings()[4][0];
        QCOMPARE(bridge.changeKeyboardMouseBinding(5, 0, baseKey, false).value("conflict").toInt(), 4);
        session.SetKeyboardMouseBindings(settings.GetKeyboardMouseBindings());
        session.SetGameplayCapture(true);
        QKeyEvent shiftedDown(QEvent::KeyPress, Qt::Key_Exclam, Qt::ShiftModifier);
        QKeyEvent baseUp(QEvent::KeyRelease, baseKey, Qt::NoModifier);
        session.HandleKeyboardEvent(&shiftedDown);
        QCOMPARE(outgoing.buttons, uint32_t(CHIAKI_CONTROLLER_BUTTON_CROSS));
        session.HandleKeyboardEvent(&baseUp);
        QCOMPARE(outgoing.buttons, 0u);
        QKeyEvent tabDown(QEvent::KeyPress, Qt::Key_Backtab, Qt::ShiftModifier);
        QKeyEvent tabUp(QEvent::KeyRelease, Qt::Key_Tab, Qt::NoModifier);
        session.HandleKeyboardEvent(&tabDown);
        QCOMPARE(outgoing.buttons, uint32_t(CHIAKI_CONTROLLER_BUTTON_TOUCHPAD));
        session.HandleKeyboardEvent(&tabUp);
        QCOMPARE(outgoing.buttons, 0u);
        session.SetGameplayCapture(false);
    }
    void mouseMotion()
    {
        KeyboardMouseMapper mapper;
        mapper.mouseMove(8, -4, 0);
        mapper.advanceMouse(0);
        QCOMPARE(mapper.state().right_x, int16_t(0));
        QCOMPARE(mapper.state().right_y, int16_t(0));
        mapper.advanceMouse(8000);
        QCOMPARE(mapper.state().right_x, int16_t(16000));
        QCOMPARE(mapper.state().right_y, int16_t(-8000));
        mapper.advanceMouse(16000);
        QCOMPARE(mapper.state().right_x, int16_t(0));
        QCOMPARE(mapper.state().right_y, int16_t(0));
        mapper.mouseMove(1000, 1000, 20000);
        mapper.advanceMouse(24000);
        QCOMPARE(mapper.state().right_x, int16_t(23170));
        QCOMPARE(mapper.state().right_y, int16_t(23170));
        mapper.reset();
        mapper.advanceMouse(32000);
        QCOMPARE(mapper.state().right_x, int16_t(0));
        QCOMPARE(mapper.state().right_y, int16_t(0));
        const auto trace = [](int intervalUs) {
            KeyboardMouseMapper timed;
            qint64 response = 0;
            for (int now = 0; now < 64000; now += 1000) {
                if (now < 32000 && now % intervalUs == 0)
                    timed.mouseMove(intervalUs / 1000, 0, now);
                if (now % 8000 == 0) {
                    timed.advanceMouse(now);
                    response += timed.state().right_x;
                }
            }
            return response;
        };
        // Equal 32-count traces at 1000 Hz and 125 Hz have equal unsaturated response.
        QCOMPARE(trace(1000), qint64(64000));
        QCOMPARE(trace(8000), qint64(64000));
    }
    void mouseTriggers()
    {
#ifndef Q_OS_WIN
        QSKIP("Mouse camera mode is available on Windows.");
#endif
        TestSettingsProfile profile;
        Settings settings(profile.name);
        settings.SetMouseCameraEnabled(true);
        settings.SetHardwareDecoder({});
        StreamSessionConnectInfo info(&settings, CHIAKI_TARGET_PS5_1, "127.0.0.1", "Test console",
            QByteArray(CHIAKI_SESSION_AUTH_SIZE, '\0'), QByteArray(16, '\0'), {}, {}, false, false, false, false);
        StreamSession session(info);
        session.SetGameplayCapture(true);
        const auto mouse = [&session](Qt::MouseButton button, bool down) {
            QMouseEvent event(down ? QEvent::MouseButtonPress : QEvent::MouseButtonRelease,
                QPointF(10, 10), QPointF(10, 10), button, down ? button : Qt::NoButton, Qt::NoModifier);
            if (down) session.HandleMousePressEvent(&event); else session.HandleMouseReleaseEvent(&event);
        };
        const auto key = [&session](int code, bool down) {
            QKeyEvent event(down ? QEvent::KeyPress : QEvent::KeyRelease, code, Qt::NoModifier);
            session.HandleKeyboardEvent(&event);
        };
        mouse(Qt::LeftButton, true);
        QCOMPARE(outgoing.r2_state, uint8_t(255));
        key(Qt::Key_X, true);
        mouse(Qt::LeftButton, false);
        QCOMPARE(outgoing.r2_state, uint8_t(255));
        key(Qt::Key_X, false);
        QCOMPARE(outgoing.r2_state, uint8_t(0));
        mouse(Qt::RightButton, true);
        key(Qt::Key_Z, true);
        key(Qt::Key_Z, false);
        QCOMPARE(outgoing.l2_state, uint8_t(255));
        mouse(Qt::RightButton, false);
        QCOMPARE(outgoing.l2_state, uint8_t(0));
        for (const auto button : {Qt::MiddleButton, Qt::BackButton, Qt::ForwardButton}) {
            mouse(button, true);
            QCOMPARE(outgoing.buttons, 0u);
            QCOMPARE(outgoing.l2_state, uint8_t(0));
            QCOMPARE(outgoing.r2_state, uint8_t(0));
            mouse(button, false);
        }
        mouse(Qt::LeftButton, true);
        session.SetGameplayCapture(false);
        QCOMPARE(outgoing.r2_state, uint8_t(0));
        session.SetGameplayCapture(true, {-int(Qt::LeftButton)});
        mouse(Qt::LeftButton, true);
        QCOMPARE(outgoing.r2_state, uint8_t(0));
        mouse(Qt::LeftButton, false);
        mouse(Qt::LeftButton, true);
        QCOMPARE(outgoing.r2_state, uint8_t(255));
        session.SetGameplayCapture(false);
    }
    void keyboardCapture()
    {
#ifndef Q_OS_WIN
        QSKIP("Mouse camera mode is available on Windows.");
#endif
        TestSettingsProfile profile;
        Settings settings(profile.name);
        QVERIFY(!settings.GetMouseCameraEnabled());
        settings.SetMouseCameraEnabled(true);
        Settings loaded(profile.name);
        QVERIFY(loaded.GetMouseCameraEnabled());
        QVERIFY(loaded.GetKeyboardEnabled());
        TestSettingsProfile otherProfile;
        Settings other(otherProfile.name);
        QVERIFY(!other.GetMouseCameraEnabled());
        settings.SetHardwareDecoder({});
        StreamSessionConnectInfo info(&settings, CHIAKI_TARGET_PS5_1, "127.0.0.1", "Test console",
            QByteArray(CHIAKI_SESSION_AUTH_SIZE, '\0'), QByteArray(16, '\0'), {}, {}, false, false, false, false);
        StreamSession session(info);
        session.SetMouseCameraEnabled(true);
        const auto key = [&session](int code, bool down, bool repeat = false) {
            QKeyEvent event(down ? QEvent::KeyPress : QEvent::KeyRelease, code, Qt::NoModifier, {}, repeat);
            session.HandleKeyboardEvent(&event);
        };
        key(Qt::Key_Return, true);
        QCOMPARE(outgoing.buttons, 0u);
        key(Qt::Key_Return, false);
        session.SetGameplayCapture(true);
        const struct { int key; uint32_t button; } bindings[] = {
            {Qt::Key_Space, CHIAKI_CONTROLLER_BUTTON_CROSS}, {Qt::Key_Control, CHIAKI_CONTROLLER_BUTTON_MOON},
            {Qt::Key_R, CHIAKI_CONTROLLER_BUTTON_BOX}, {Qt::Key_F, CHIAKI_CONTROLLER_BUTTON_PYRAMID},
            {Qt::Key_Q, CHIAKI_CONTROLLER_BUTTON_L1}, {Qt::Key_E, CHIAKI_CONTROLLER_BUTTON_R1},
            {Qt::Key_Shift, CHIAKI_CONTROLLER_BUTTON_L3}, {Qt::Key_C, CHIAKI_CONTROLLER_BUTTON_R3},
            {Qt::Key_Up, CHIAKI_CONTROLLER_BUTTON_DPAD_UP}, {Qt::Key_Down, CHIAKI_CONTROLLER_BUTTON_DPAD_DOWN},
            {Qt::Key_Left, CHIAKI_CONTROLLER_BUTTON_DPAD_LEFT}, {Qt::Key_Right, CHIAKI_CONTROLLER_BUTTON_DPAD_RIGHT},
            {Qt::Key_Return, CHIAKI_CONTROLLER_BUTTON_OPTIONS}, {Qt::Key_Backspace, CHIAKI_CONTROLLER_BUTTON_SHARE},
            {Qt::Key_Tab, CHIAKI_CONTROLLER_BUTTON_TOUCHPAD}, {Qt::Key_F1, CHIAKI_CONTROLLER_BUTTON_PS}
        };
        for (const auto &binding : bindings) {
            key(binding.key, true);
            QCOMPARE(outgoing.buttons, binding.button);
            key(binding.key, false);
            QCOMPARE(outgoing.buttons, 0u);
        }
        key(Qt::Key_W, true);
        QCOMPARE(outgoing.left_y, int16_t(-32767));
        key(Qt::Key_S, true);
        QCOMPARE(outgoing.left_y, int16_t(0));
        key(Qt::Key_S, false);
        key(Qt::Key_D, true);
        QCOMPARE(outgoing.left_x, int16_t(23169));
        QCOMPARE(outgoing.left_y, int16_t(-23169));
        key(Qt::Key_A, true);
        QCOMPARE(outgoing.left_x, int16_t(0));
        QCOMPARE(outgoing.left_y, int16_t(-32767));
        key(Qt::Key_X, true);
        key(Qt::Key_Z, true);
        QCOMPARE(outgoing.l2_state, uint8_t(255));
        QCOMPARE(outgoing.r2_state, uint8_t(255));
        session.touch_state.buttons = CHIAKI_CONTROLLER_BUTTON_CROSS;
        session.touch_state.gyro_x = 17;
        session.dpad_touch_state.buttons = CHIAKI_CONTROLLER_BUTTON_PS;
        session.DpadSendFeedbackState();
        QCOMPARE(outgoing.buttons, 0u);
        QCOMPARE(outgoing.gyro_x, 0.0f);
        session.SetGameplayCapture(false);
        QCOMPARE(outgoing.buttons, 0u);
        QCOMPARE(outgoing.left_y, int16_t(0));
        QCOMPARE(outgoing.r2_state, uint8_t(0));
        session.SetGameplayCapture(true, {Qt::Key_W});
        key(Qt::Key_W, true, true);
        QCOMPARE(outgoing.left_y, int16_t(0));
        key(Qt::Key_W, true);
        QCOMPARE(outgoing.left_y, int16_t(0));
        key(Qt::Key_W, false);
        key(Qt::Key_W, true);
        QCOMPARE(outgoing.left_y, int16_t(-32767));
        session.BlockInput(true);
        QCOMPARE(outgoing.left_y, int16_t(0));
    }
    void sessionAndFeedback() { runNative(false); }
    void platformKeyboard()
    {
#ifndef Q_OS_WIN
        QSKIP("Mouse camera mode is available on Windows.");
#else
        runNative(false, true);
#endif
    }
    void spontaneousKeys()
    {
#ifndef Q_OS_WIN
        QSKIP("The native SendInput check requires Windows.");
#else
        if (!qEnvironmentVariableIsSet("CHIAKI_INPUT_TEST_KEYS"))
            QSKIP("Native OS keys need foreground focus. Set CHIAKI_INPUT_TEST_KEYS=1 and run spontaneousKeys separately.");
        runNative(true);
#endif
    }
};

int main(int argc, char **argv)
{
    if (chiaki_lib_init() != CHIAKI_ERR_SUCCESS) return 1;
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Chiaki input tests");
    QCoreApplication::setApplicationName("Chiaki input tests");
    StreamInputNativeTest test;
    return QTest::qExec(&test, argc, argv);
}
#include "streaminputnative.moc"
