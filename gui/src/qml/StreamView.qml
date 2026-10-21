import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Window

import org.streetpea.chiaking

import "controls" as C

Item {
    id: view
    property var backend: Chiaki
    property var settings: backend.settings
    property var streamWindow: backend.window
    property var navigation: root

    readonly property var hostWindow: view.Window.window
    readonly property bool settingsOpen: !!view.navigation.streamSettingsOpen
    readonly property bool managedConnection: !!view.backend.connectionAttempt.presented
    property bool legacySessionError: false
    property bool legacySessionLoading: true
    readonly property bool sessionError: managedConnection ? view.backend.connectionAttempt.phase === "failure" : legacySessionError
    readonly property bool sessionLoading: managedConnection ? view.backend.connectionAttempt.phase !== "ready" && !sessionError : legacySessionLoading
    property string connectionPresentation: ""
    property string legacyErrorPresentation: ""
    readonly property bool useSeparateMenuWindow: view.streamWindow.runtimeRendererBackend === 1
    readonly property int streamMenuHeight: useSeparateMenuWindow ? separateMenuWindow.menuHeight : inlineMenuContent.item ? inlineMenuContent.item.implicitHeight : 80
    readonly property real streamMenuWidth: Math.min(820, Math.max(0, view.width - 28))
    readonly property bool streamStatsVisible: !!view.backend.session && view.settings.showStreamStats && !(menuState.open || menuState.closing) && !sessionLoading && !sessionError && !(view.settings.audioVideoDisabled & 0x02)
    property int separateMenuX: 0
    property int separateMenuY: 0
    property int separateMenuWidth: 0
    property int separateStatsX: 0
    property int separateStatsY: 0
    property int separateStatsWidth: 0
    property int separateStatsHeight: 0

    function updateSeparateMenuGeometry() {
        if (!hostWindow)
            return;
        const topLeft = view.mapToGlobal(0, 0);
        if (useSeparateMenuWindow) {
            separateMenuX = Math.round(topLeft.x + (view.width - streamMenuWidth) / 2);
            separateMenuY = Math.round(topLeft.y + view.height - streamMenuHeight - 18);
            separateMenuWidth = Math.round(streamMenuWidth);
            separateStatsX = Math.round(topLeft.x);
            separateStatsY = Math.round(topLeft.y);
            separateStatsWidth = Math.round(view.width);
            separateStatsHeight = Math.round(view.height);
        }
    }

    StackView.onActivating: { view.streamWindow.keepVideo = true; connectionView.retiring = false; }
    StackView.onDeactivating: connectionView.retiring = true
    StackView.onDeactivated: view.streamWindow.keepVideo = false

    Component.onCompleted: {
        updateSeparateMenuGeometry();
    }
    onWidthChanged: updateSeparateMenuGeometry()
    onStreamMenuHeightChanged: updateSeparateMenuGeometry()
    onHeightChanged: updateSeparateMenuGeometry()
    onUseSeparateMenuWindowChanged: {
        updateSeparateMenuGeometry();
        if (actionNoticeVisible) view.focusNotice();
    }

    Connections {
        target: view.hostWindow
        function onXChanged() { view.updateSeparateMenuGeometry() }
        function onYChanged() { view.updateSeparateMenuGeometry() }
        function onWidthChanged() { view.updateSeparateMenuGeometry() }
        function onHeightChanged() { view.updateSeparateMenuGeometry() }
        function onVisibilityChanged() { view.updateSeparateMenuGeometry() }
    }

    Connections {
        target: view.streamWindow
        function onXChanged() { view.updateSeparateMenuGeometry() }
        function onYChanged() { view.updateSeparateMenuGeometry() }
        function onWidthChanged() { view.updateSeparateMenuGeometry() }
        function onHeightChanged() { view.updateSeparateMenuGeometry() }
        function onVisibilityChanged() { view.updateSeparateMenuGeometry() }
        function onScreenChanged() { view.updateSeparateMenuGeometry() }
    }

    QtObject {
        id: menuState
        property bool closing: false
        property bool open: false
        property string presentation: ""
        property string closingPresentation: ""
        property var context: view
        readonly property bool separate: view.useSeparateMenuWindow
        property string flyout: ""
        property bool disconnectPending: false
        property string settingsTarget: ""
        onClosingChanged: {
            const closeId = closingPresentation;
            if (!closing && disconnectPending) {
                disconnectPending = false;
                // Let the native menu hide before the modal dialog takes focus.
                Qt.callLater(() => {
                    if (closeId !== closingPresentation || closeId !== presentation || open) return;
                    if (!view.backend.sessionPinRequest.id && view.backend.session && !view.sessionLoading && !view.sessionError) sessionStopDialog.open();
                });
            }
            if (!closing && settingsTarget) {
                const target = settingsTarget;
                Qt.callLater(() => {
                    if (closeId !== closingPresentation || closeId !== presentation || open) return;
                    if (settingsTarget === target && !disconnectPending && !sessionStopDialog.opened
                            && view.backend.session && !view.backend.sessionPinRequest.id && !view.sessionLoading && !view.sessionError) {
                        if (target === "display") view.navigation.openDisplaySettings();
                        else if (target === "keyboard-mouse") view.navigation.openKeyboardMouseSettings();
                        else view.navigation.openPlaceboSettings();
                    }
                    settingsTarget = "";
                });
            }
        }

        property real lastToggleTime: 0
        function toggle() {
            if (closing || disconnectPending || settingsTarget || view.backend.sessionPinRequest.id || view.managedConnection && view.backend.connectionAttempt.phase !== "ready")
                return;
            var now = Date.now();
            if (now - lastToggleTime < 200)
                return;
            lastToggleTime = now;
            if (open)
                close();
            else {
                if (useSeparateMenuWindow) {
                    view.updateSeparateMenuGeometry();
                }
                presentation = view.streamWindow.streamInput.present(menuState, "menu", null);
                open = true;
            }
        }

        function close() {
            if (!open || closing)
                return;
            flyout = "";
            closingPresentation = presentation;
            const id = presentation;
            const complete = function() {
                if (inlineMenuAnimation.running) return;
                inlineMenuAnimation.runningChanged.disconnect(complete);
                if (!view.useSeparateMenuWindow) menuState.finishClose(id);
            };
            inlineMenuAnimation.runningChanged.connect(complete);
            closing = true;
            open = false;
            view.streamWindow.streamInput.dismiss(closingPresentation);
        }
        function openSettings(target) {
            settingsTarget = target;
            close();
        }
        function dismiss() {
            if (flyout) flyout = "";
            else close();
        }
        function finishClose(id) {
            if (!id || id !== closingPresentation || id !== presentation) return;
            closing = false;
            view.streamWindow.streamInput.settled(id);
        }
    }

    Rectangle {
        id: loadingView
        anchors.fill: parent
        color: "black"
        opacity: !view.managedConnection && (sessionError || sessionLoading) || (!sessionLoading && !sessionError && (view.settings.audioVideoDisabled & 0x02)) ? 1.0 : 0.0
        visible: opacity

        Behavior on opacity { NumberAnimation { duration: 250 } }

        Item {
            anchors {
                top: parent.verticalCenter
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            BusyIndicator {
                id: spinner
                anchors.centerIn: parent
                width: 70
                height: width
                visible: sessionLoading
            }

            Label {
                anchors {
                    top: spinner.bottom
                    horizontalCenter: spinner.horizontalCenter
                    topMargin: 30
                }
                text: {
                    if(view.settings.dpadTouchEnabled)
                    {
                        if(view.settings.audioVideoDisabled == 0x01)
                            qsTr("Audio Disabled in settings\n") + qsTr("Press %1 to open stream menu").arg(view.backend.controllers.length ? view.settings.stringForStreamMenuShortcut() : "Ctrl+O") + "\n" + qsTr("Press %1 to toggle between regular dpad and dpad touch").arg(view.settings.stringForDpadShortcut())
                        else
                            qsTr("Press %1 to open stream menu").arg(view.backend.controllers.length ? view.settings.stringForStreamMenuShortcut() : "Ctrl+O") + "\n" + qsTr("Press %1 to toggle between regular dpad and dpad touch").arg(view.settings.stringForDpadShortcut())
                    }
                    else
                    {
                        if(view.settings.audioVideoDisabled == 0x01)
                            qsTr("Audio Disabled in settings\n") + qsTr("Press %1 to open stream menu").arg(view.backend.controllers.length ? view.settings.stringForStreamMenuShortcut() : "Ctrl+O")
                        else
                            qsTr("Press %1 to open stream menu").arg(view.backend.controllers.length ? view.settings.stringForStreamMenuShortcut() : "Ctrl+O")
                    }
                }
                visible: sessionLoading
            }

            Label {
                id: errorTitleLabel
                anchors {
                    bottom: spinner.top
                    horizontalCenter: spinner.horizontalCenter
                }
                font.pixelSize: 24
                visible: text
                onVisibleChanged: if (visible) view.legacyErrorPresentation = view.streamWindow.streamInput.present(errorTitleLabel, "legacy-error", errorTitleLabel)
                Keys.onReturnPressed: view.navigation.showMainView()
                Keys.onEscapePressed: view.navigation.showMainView()
            }

            Label {
                id: errorTextLabel
                anchors {
                    top: errorTitleLabel.bottom
                    horizontalCenter: errorTitleLabel.horizontalCenter
                    topMargin: 10
                }
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 20
                visible: text
            }
        }
    }

    ConnectionView {
        id: connectionView
        backend: view.backend
        navigation: view.navigation
        anchors.fill: parent
        managesAttempt: view.managedConnection && !view.backend.connectionAttempt.waitingForWake
        visible: view.managedConnection && view.backend.connectionAttempt.phase !== "ready"
        enabled: visible
        onVisibleChanged: {
            if (visible) view.connectionPresentation = view.streamWindow.streamInput.present(connectionView, "connection", null);
            else {
                const id = view.connectionPresentation;
                view.streamWindow.streamInput.dismiss(id);
                view.streamWindow.streamInput.settled(id);
                view.connectionPresentation = "";
            }
        }
    }

    readonly property bool noticesReady: !!view.backend.session && view.backend.session.connected
        && !sessionLoading && !sessionError && !view.backend.sessionPinRequest.id && !settingsOpen
        && !sessionStopDialog.opened && !menuState.disconnectPending && !menuState.settingsTarget
    readonly property bool audioNoticeVisible: noticesReady && view.settings.audioVideoDisabled === 1
    property real networkNoticeOpacity: networkIndicatorTimer.running ? 1 : 0
    property var networkNoticeSession: null
    Behavior on networkNoticeOpacity { NumberAnimation { duration: 400 } }
    readonly property bool passiveNoticesVisible: noticesReady && !(menuState.open || menuState.closing)
        && (networkNoticeSession === view.backend.session && networkNoticeOpacity > 0 || audioNoticeVisible)
    Timer {
        id: networkIndicatorTimer
        running: view.noticesReady && Number.isFinite(view.backend.session.averagePacketLoss)
            && view.backend.session.averagePacketLoss > (view.settings.wifiDroppedNotif * 0.01)
        interval: 400
        onRunningChanged: if (running) view.networkNoticeSession = view.backend.session
    }

    Component {
        id: passiveNoticesContent
        StreamPassiveNotices {
            networkOpacity: view.networkNoticeSession === view.backend.session ? view.networkNoticeOpacity : 0
            audioDisabled: view.audioNoticeVisible
            packetLoss: view.backend.session && Number.isFinite(view.backend.session.averagePacketLoss) ? view.backend.session.averagePacketLoss : 0
        }
    }
    readonly property bool gameplayNoticeVisible: view.settings.mouseCameraEnabled
        && (view.streamWindow.gameplayCaptured || view.noticesReady && view.streamWindow.gameplayCaptureAvailable)
        && !(menuState.open || menuState.closing)
    Component {
        id: gameplayNoticeContent
        StreamNotice {
            objectName: "gameplayCaptureNotice"
            title: view.streamWindow.gameplayCaptured
                ? qsTr("Mouse captured · F8 to release.") : qsTr("Keyboard and mouse ready · F8 to capture.")
            message: ""
        }
    }
    Loader {
        objectName: "gameplayNoticeInline"
        anchors { bottom: parent.bottom; left: parent.left; margins: 18 }
        width: Math.min(340, view.width - 36)
        height: item ? item.implicitHeight : 0
        visible: !view.useSeparateMenuWindow && view.gameplayNoticeVisible
        sourceComponent: gameplayNoticeContent
    }
    Window {
        objectName: "gameplayNoticeWindow"
        transientParent: view.streamWindow
        flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowTransparentForInput | Qt.WindowDoesNotAcceptFocus
        modality: Qt.NonModal
        color: "transparent"
        visible: view.useSeparateMenuWindow && view.gameplayNoticeVisible
        x: view.streamWindow.x + 18
        y: view.streamWindow.y + view.streamWindow.height - height - 18
        width: Math.min(340, view.width - 36)
        height: gameplayWindowContent.item ? gameplayWindowContent.item.implicitHeight : 0
        Loader { id: gameplayWindowContent; width: parent.width; sourceComponent: gameplayNoticeContent }
    }
    Loader {
        id: passiveNoticesInline
        objectName: "streamPassiveNoticesInline"
        anchors { top: parent.top; right: parent.right; margins: 18 }
        width: Math.min(320, view.width - 36)
        height: item ? item.implicitHeight : 0
        visible: !view.useSeparateMenuWindow && view.passiveNoticesVisible
        sourceComponent: passiveNoticesContent
    }
    Window {
        id: passiveNoticesWindow
        objectName: "streamPassiveNoticesWindow"
        transientParent: view.streamWindow
        flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowTransparentForInput | Qt.WindowDoesNotAcceptFocus
        modality: Qt.NonModal
        color: "transparent"
        visible: view.useSeparateMenuWindow && view.passiveNoticesVisible
        x: view.streamWindow.x + view.streamWindow.width - width - 18
        y: view.streamWindow.y + 18
        width: Math.min(320, view.width - 36)
        height: passiveWindowContent.item ? passiveWindowContent.item.implicitHeight : 0
        Loader {
            id: passiveWindowContent
            width: parent.width
            sourceComponent: passiveNoticesContent
        }
    }

    readonly property bool videoDisabled: !!(view.settings.audioVideoDisabled & 0x02)
    readonly property bool blockedContent: !videoDisabled && view.streamWindow.hasVideo
        && !!view.backend.session && view.backend.session.cantDisplay
    readonly property bool actionNoticeVisible: noticesReady && !(menuState.open || menuState.closing)
        && (videoDisabled || blockedContent)
    property string noticePresentation: ""

    function focusNotice() {
        if (!actionNoticeVisible) return;
        const item = useSeparateMenuWindow ? actionWindowContent.item : actionNoticeInline.item;
        noticePresentation = view.streamWindow.streamInput.present(view, "notice", null);
        noticeFocusTimer.presentation = noticePresentation;
        noticeFocusTimer.restart();
    }
    Timer {
        id: noticeFocusTimer
        property string presentation: ""
        interval: 0
        onTriggered: {
            if (presentation !== view.noticePresentation || !view.actionNoticeVisible) return;
            const item = view.useSeparateMenuWindow ? actionWindowContent.item : actionNoticeInline.item;
            if (view.useSeparateMenuWindow) actionNoticeWindow.requestActivate();
            else if (view.hostWindow && view.hostWindow.visible) view.hostWindow.requestActivate();
            else view.streamWindow.requestActivate();
            if (item) item.initialFocusItem.forceActiveFocus(Qt.TabFocusReason);
        }
    }
    onActionNoticeVisibleChanged: {
        if (actionNoticeVisible) focusNotice();
        else {
            const id = noticePresentation;
            view.streamWindow.streamInput.dismiss(id);
            view.streamWindow.streamInput.settled(id);
            noticePresentation = "";
        }
    }
    Component {
        id: actionNoticeContent
        StreamActionNotice {
            blocked: view.blockedContent
            audioDisabled: !!(view.settings.audioVideoDisabled & 0x01)
            enabled: view.actionNoticeVisible
            onTriggered: {
                if (!view.actionNoticeVisible || !view.backend.session || !view.backend.session.connected) return;
                if (view.blockedContent) view.backend.sessionGoHome();
                else menuState.toggle();
            }
        }
    }
    Loader {
        id: actionNoticeInline
        objectName: "streamActionNoticeInline"
        anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 72 }
        width: Math.min(360, view.width - 36)
        height: item ? item.implicitHeight : 0
        sourceComponent: actionNoticeContent
        visible: !view.useSeparateMenuWindow && view.actionNoticeVisible
    }
    Window {
        id: actionNoticeWindow
        objectName: "streamActionNoticeWindow"
        transientParent: view.streamWindow
        flags: Qt.Tool | Qt.FramelessWindowHint
        modality: Qt.NonModal
        color: "transparent"
        visible: view.useSeparateMenuWindow && view.actionNoticeVisible
        x: view.streamWindow.x + Math.round((view.streamWindow.width - width) / 2)
        y: view.streamWindow.y + view.streamWindow.height - height - 72
        width: Math.min(360, view.width - 36)
        height: actionWindowContent.item ? actionWindowContent.item.implicitHeight : 0
        onVisibleChanged: if (visible) { requestActivate(); view.focusNotice(); }
        onActiveChanged: if (active && visible) view.focusNotice()
        onClosing: event => { event.accepted = false; view.streamWindow.close(); }
        Shortcut { sequence: "Ctrl+O"; enabled: actionNoticeWindow.visible; onActivated: menuState.toggle() }
        Loader {
            id: actionWindowContent
            anchors.fill: parent
            sourceComponent: actionNoticeContent
        }
    }

    Component {
        id: streamStatsContent
        StreamStatsContent { backend: view.backend; streamWindow: view.streamWindow }
    }

    Item {
        id: streamStatsInline
        objectName: "streamStatsInline"
        anchors.fill: parent
        visible: streamStatsVisible && !useSeparateMenuWindow
        Loader {
            anchors.fill: parent
            sourceComponent: streamStatsContent
            active: streamStatsInline.visible
        }
    }

    Window {
        id: streamStatsWindow
        objectName: "streamStatsWindow"
        flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowTransparentForInput | Qt.WindowDoesNotAcceptFocus
        color: "transparent"
        modality: Qt.NonModal
        transientParent: view.streamWindow
        visible: streamStatsVisible && useSeparateMenuWindow
        x: separateStatsX
        y: separateStatsY
        width: separateStatsWidth
        height: separateStatsHeight
        Loader {
            anchors.fill: parent
            sourceComponent: streamStatsContent
            active: streamStatsWindow.visible
        }
    }

    Component {
        id: menuContentComponent
        StreamMenuContent {
            backend: view.backend
            settings: view.settings
            streamWindow: view.streamWindow
            navigation: view.navigation
            menuController: menuState
            separate: view.useSeparateMenuWindow
        }
    }

    Item {
        id: menuView
        objectName: "streamMenuInline"
        width: streamMenuWidth
        x: (parent.width - width) / 2
        height: streamMenuHeight
        y: menuState.open ? parent.height - height - 18 : parent.height
        visible: !useSeparateMenuWindow && (menuState.open || menuState.closing)
        enabled: !useSeparateMenuWindow && menuState.open
        onVisibleChanged: {
            if (visible)
                if (inlineMenuContent.item) inlineMenuContent.item.initialFocusItem.forceActiveFocus(Qt.TabFocusReason);
        }

        Behavior on y {
            NumberAnimation {
                id: inlineMenuAnimation
                duration: 250
            }
        }

        Loader {
            id: inlineMenuContent
            anchors.fill: parent
            active: !useSeparateMenuWindow
            sourceComponent: menuContentComponent
        }
    }

    StreamMenuWindow {
        id: separateMenuWindow
        objectName: "streamMenuWindow"
        menuContent: menuContentComponent
        transientParent: view.streamWindow
        x: separateMenuX
        y: separateMenuY
        width: separateMenuWidth > 0 ? separateMenuWidth : view.width
        height: streamMenuHeight
        open: useSeparateMenuWindow && menuState.open
        presentation: menuState.presentation
        flyoutOpen: !!menuState.flyout
        closing: useSeparateMenuWindow && menuState.closing
        onCloseRequested: menuState.dismiss()
        onMenuCloseRequested: menuState.close()
        onCloseAnimationFinished: id => menuState.finishClose(id)
    }

    StreamDisconnectDialog {
        id: sessionStopDialog
        objectName: "streamDisconnectDialog"
        backend: view.backend
        hostWindow: view.streamWindow

    }

    Timer {
        id: closeTimer
        interval: 2000
        onTriggered: view.navigation.showMainView()
    }

    Connections {
        target: view.backend

        function onSessionChanged() {
            view.networkNoticeSession = null;
            if (view.noticesReady && Number.isFinite(view.backend.session.averagePacketLoss)
                    && view.backend.session.averagePacketLoss > (view.settings.wifiDroppedNotif * 0.01))
                networkIndicatorTimer.restart();
            if (!view.backend.session) {
                menuState.disconnectPending = false;
                menuState.close();
            }
            if (view.managedConnection) {
                if (!view.backend.session && view.backend.connectionAttempt.phase === "ready") view.navigation.showMainView();
                return;
            }
            if (!view.backend.session) {
                if (errorTitleLabel.text)
                    closeTimer.start();
                else
                    view.navigation.showMainView();
            }
        }

        function onSessionError(title, text) {
            if (view.managedConnection) return;
            legacySessionError = true;
            legacySessionLoading = false;
            errorTitleLabel.text = title;
            errorTextLabel.text = text;
            closeTimer.start();
        }

        function onSessionPinRequestChanged() {
            if (view.backend.sessionPinRequest.id) menuState.close();
        }

        function onConnectionAttemptChanged() {
            if (view.managedConnection && view.backend.connectionAttempt.phase !== "ready") menuState.close();
        }

        function onSessionStopDialogRequested() {
            if (view.backend.sessionPinRequest.id || sessionStopDialog.opened)
                return;
            menuState.settingsTarget = "";
            if (menuState.open || menuState.closing) {
                menuState.disconnectPending = true;
                menuState.close();
            } else sessionStopDialog.open();
        }
    }

    Connections {
        target: view.streamWindow

        function onHasVideoChanged() {
            if (view.streamWindow.hasVideo && !view.managedConnection)
                legacySessionLoading = false;
        }

        function onMenuRequested() {
            if (view.backend.sessionPinRequest.id || sessionStopDialog.opened)
                return;
            menuState.toggle();
        }
    }
    Connections {
        target: view.backend.session

        function onConnectedChanged() {
            if (view.backend.session && view.backend.session.connected && !view.managedConnection && (view.settings.audioVideoDisabled & 0x02))
                legacySessionLoading = false;
        }
    }
}
