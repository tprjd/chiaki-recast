#include <QtTest>
#include <QApplication>
#include <memory>
#include <algorithm>
#include <QUuid>
#include <QQuickItem>
#include <qpa/qwindowsysteminterface.h>
#include "qmlmainwindow.h"
#include "qmlbackend.h"
#include "streamsession.h"
#ifdef Q_OS_WIN
#include <windows.h>
#endif

static ChiakiControllerState outgoing;
extern "C" ChiakiErrorCode __wrap_chiaki_session_set_controller_state(ChiakiSession *, ChiakiControllerState *state)
{
    outgoing = *state;
    return CHIAKI_ERR_SUCCESS;
}

struct TestSettingsProfile {
    const QString name = QUuid::createUuid().toString(QUuid::WithoutBraces);
    ~TestSettingsProfile() {
        QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-" + name).clear();
    }
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
        QmlMainWindow window(&settings);
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
private slots:
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
