#include <QtTest>
#include <QApplication>
#include <memory>
#include <algorithm>
#include <QTemporaryDir>
#include <QQuickItem>
#include "qmlmainwindow.h"
#include "qmlbackend.h"
#include "streamsession.h"
#ifdef Q_OS_WIN
#include <windows.h>
#endif

static ChiakiControllerState outgoing;
static QList<uint32_t> feedbackButtons;
extern "C" ChiakiErrorCode __wrap_chiaki_session_set_controller_state(ChiakiSession *, ChiakiControllerState *state)
{
    outgoing = *state;
    feedbackButtons.append(state->buttons);
    return CHIAKI_ERR_SUCCESS;
}

class StreamInputNativeTest : public QObject
{
    Q_OBJECT
private:
    void runNative(bool nativeKeys)
    {
        if (QGuiApplication::platformName() == "offscreen") QSKIP("Requires desktop activation and an OpenGL or Vulkan device.");
        QTemporaryDir directory;
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, directory.path());
        Settings settings(QString{});
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
        QQuickItem owner(window.quick_window->contentItem());
        owner.setVisible(true);
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
#ifdef Q_OS_WIN
        if (nativeKeys) {
            window.requestActivate();
            QTRY_COMPARE(QGuiApplication::focusWindow(), &window);
            INPUT keys[2] = {};
            keys[0].type = keys[1].type = INPUT_KEYBOARD;
            keys[0].ki.wVk = keys[1].ki.wVk = VK_RETURN;
            keys[1].ki.dwFlags = KEYEVENTF_KEYUP;
            const auto previous = feedbackButtons.size();
            QCOMPARE(SendInput(2, keys, sizeof(INPUT)), 2u);
            QTRY_VERIFY(std::any_of(feedbackButtons.cbegin() + previous, feedbackButtons.cend(),
                [](uint32_t buttons) { return buttons & CHIAKI_CONTROLLER_BUTTON_CROSS; }));
            QTRY_COMPARE(outgoing.buttons, 0u);
        }
#endif
        changeSession(nullptr);
        QVERIFY(!input->blocked());
        window.hide();
    }
private slots:
    void sessionAndFeedback() { runNative(false); }
    void spontaneousKeys()
    {
#ifndef Q_OS_WIN
        QSKIP("The native SendInput check requires Windows.");
#else
        if (!qEnvironmentVariableIsSet("CHIAKI_INPUT_TEST_KEYS"))
            QSKIP("Native OS key delivery needs an interactive desktop. Set CHIAKI_INPUT_TEST_KEYS=1 and run spontaneousKeys separately. The initial SendInput attempt did not prove this route.");
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
