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
    readonly property bool managedConnection: !!view.backend.connectionAttempt.presented
    property bool legacySessionError: false
    property bool legacySessionLoading: true
    readonly property bool sessionError: managedConnection ? view.backend.connectionAttempt.phase === "failure" : legacySessionError
    readonly property bool sessionLoading: managedConnection ? view.backend.connectionAttempt.phase !== "ready" && !sessionError : legacySessionLoading
    property bool connectionInputGrabbed: false
    property list<Item> restoreFocusItems
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

    function grabInput(item) {
        view.streamWindow.grabInput();
        restoreFocusItems.push(hostWindow ? hostWindow.activeFocusItem : null);
        if (item)
            item.forceActiveFocus(Qt.TabFocusReason);
    }

    function releaseInput() {
        if (!restoreFocusItems.length) return;
        view.streamWindow.releaseInput();
        let item = restoreFocusItems.pop();
        if (item && item.visible)
            item.forceActiveFocus(Qt.TabFocusReason);
    }

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
    Component.onDestruction: {
        menuState.releaseInput();
        if (connectionInputGrabbed) view.streamWindow.releaseInput();
    }
    onWidthChanged: updateSeparateMenuGeometry()
    onStreamMenuHeightChanged: updateSeparateMenuGeometry()
    onHeightChanged: updateSeparateMenuGeometry()
    onUseSeparateMenuWindowChanged: updateSeparateMenuGeometry()

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
        property bool inputGrabbed: false
        property string flyout: ""
        property var returnFocusItem: null
        property bool disconnectPending: false
        onClosingChanged: {
            if (!closing && disconnectPending) {
                disconnectPending = false;
                // Let the native menu hide before the modal dialog takes focus.
                Qt.callLater(() => {
                    if (!view.backend.sessionPinRequest.id && view.backend.session && !view.sessionLoading && !view.sessionError) sessionStopDialog.open();
                });
            }
        }

        property real lastToggleTime: 0
        function toggle() {
            if (closing || disconnectPending || view.backend.sessionPinRequest.id || view.managedConnection && view.backend.connectionAttempt.phase !== "ready")
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
                returnFocusItem = view.hostWindow ? view.hostWindow.activeFocusItem : null;
                view.grabInput(null);
                inputGrabbed = true;
                open = true;
            }
        }

        function close() {
            if (!open || closing)
                return;
            flyout = "";
            closing = true;
            open = false;
            releaseInput();
        }
        function dismiss() {
            if (flyout) flyout = "";
            else close();
        }
        function releaseInput() {
            if (!inputGrabbed) return;
            inputGrabbed = false;
            view.releaseInput();
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
                id: audioVideoDisabledTitleLabel
                anchors {
                    bottom: spinner.top
                    horizontalCenter: spinner.horizontalCenter
                }
                text: (view.settings.audioVideoDisabled & 0x01) ? qsTr("Audio and Video Disabled") : qsTr("Video Disabled")
                font.pixelSize: 24
                visible: !sessionLoading && !sessionError && (view.settings.audioVideoDisabled & 0x02)
            }

            Label {
                id: audioVideoDisabledTextLabel
                anchors {
                    top: audioVideoDisabledTitleLabel.bottom
                    horizontalCenter: audioVideoDisabledTitleLabel.horizontalCenter
                    topMargin: 10
                }
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 20
                text: (view.settings.audioVideoDisabled & 0x01) ? qsTr("You have disabled audio and video in your settings.\nTo re-enable change Audio/Video to Audio and Video Enabled in the General tab of the settings.") : qsTr("You have disabled video in your settings.\nTo re-enable change Audio/Video to Audio and Video Enabled in the General tab of the settings.")
                visible: !sessionLoading && !sessionError && (view.settings.audioVideoDisabled & 0x02)
            }

            Label {
                id: errorTitleLabel
                anchors {
                    bottom: spinner.top
                    horizontalCenter: spinner.horizontalCenter
                }
                font.pixelSize: 24
                visible: text
                onVisibleChanged: if (visible) view.grabInput(errorTitleLabel)
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
            if (visible && !view.connectionInputGrabbed) {
                view.streamWindow.grabInput();
                view.connectionInputGrabbed = true;
            } else if (!visible && view.connectionInputGrabbed) {
                view.streamWindow.releaseInput();
                view.connectionInputGrabbed = false;
            }
        }
    }

    ColumnLayout {
        id: cantDisplayMessage
        anchors.centerIn: parent
        opacity: view.streamWindow.hasVideo && view.backend.session && view.backend.session.cantDisplay ? 1.0 : 0.0
        visible: opacity
        spacing: 30

        Behavior on opacity { NumberAnimation { duration: 250 } }

        onVisibleChanged: {
            if (visible) {
                menuState.close();
                view.grabInput(goToHomeButton);
            } else {
                view.releaseInput();
            }
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("The screen contains content that can't be displayed using Remote Play.")
        }

        Button {
            id: goToHomeButton
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: 60
            text: qsTr("Go to Home Screen")
            Material.background: activeFocus ? parent.Material.accent : undefined
            Material.roundedScale: Material.SmallScale
            onClicked: view.backend.sessionGoHome()
            Keys.onReturnPressed: clicked()
            Keys.onEscapePressed: clicked()
        }
    }

    RoundButton {
        anchors {
            right: parent.right
            top: parent.top
            margins: 40
        }
        icon.source: "qrc:/icons/discover-off-24px.svg"
        icon.width: 50
        icon.height: 50
        padding: 20
        checked: true
        opacity: networkIndicatorTimer.running ? 0.7 : 0.0
        visible: opacity
        Material.background: Material.accent

        Behavior on opacity { NumberAnimation { duration: 400 } }

        Timer {
            id: networkIndicatorTimer
            running: view.backend.session?.averagePacketLoss > (view.settings.wifiDroppedNotif * 0.01)
            interval: 400
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
                onRunningChanged: {
                    if (!running && !menuState.open)
                        menuState.closing = false;
                }
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
        flyoutOpen: !!menuState.flyout
        closing: useSeparateMenuWindow && menuState.closing
        onCloseRequested: menuState.dismiss()
        onCloseAnimationFinished: {
            menuState.closing = false;
            if (!sessionStopDialog.opened && !view.backend.sessionPinRequest.id && !view.sessionLoading && !menuState.open) {
                if (view.streamWindow) view.streamWindow.requestActivate();
                if (menuState.returnFocusItem && menuState.returnFocusItem.visible)
                    menuState.returnFocusItem.forceActiveFocus(Qt.TabFocusReason);
            }
        }
    }

    StreamDisconnectDialog {
        id: sessionStopDialog
        objectName: "streamDisconnectDialog"
        backend: view.backend
        hostWindow: view.streamWindow
        onInputRequested: view.grabInput(null)
        onInputReleased: view.releaseInput()
    }

    Timer {
        id: closeTimer
        interval: 2000
        onTriggered: view.navigation.showMainView()
    }

    Connections {
        target: view.backend

        function onSessionChanged() {
            if (!view.backend.session) {
                menuState.disconnectPending = false;
                menuState.close();
            }
            if (view.managedConnection) {
                if (!view.backend.sessionPinRequest.id
                        && (!view.backend.session || view.backend.connectionAttempt.route !== "psn")) view.connectionInputGrabbed = false;
                if (connectionView.visible && !view.connectionInputGrabbed) {
                    view.streamWindow.grabInput();
                    view.connectionInputGrabbed = true;
                }
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
