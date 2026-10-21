import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import org.streetpea.chiaking

FocusScope {
    id: view
    objectName: "connectionView"
    property var backend: Chiaki
    property var navigation: root
    property bool managesAttempt: true
    readonly property var attempt: backend.connectionAttempt || ({})
    readonly property string phase: attempt.phase || "connecting"
    readonly property bool psn: attempt.route === "psn"
    readonly property bool terminal: phase === "failure" || phase === "cancelled" || phase === "registered"
    readonly property bool busy: !terminal && phase !== "ready"
    readonly property bool streamerMode: backend.settings.streamerMode
    readonly property bool wakeFlow: attempt.waitingForWake === true
    readonly property string heading: wakeFlow ? (attempt.context === "startup" ? qsTr("Auto-connect") : qsTr("Wake console")) : psn ? qsTr("Connect over PSN") : qsTr("Connect to console")
    property string commandAttempt: ""
    property bool commandPending: false
    property bool detailsExpanded: false
    property bool leaving: false
    readonly property bool canRetry: phase === "failure" && !attempt.cleanupPending && attempt.targetValid && !commandPending
    readonly property bool canCancel: busy && phase !== "cancelling" && attempt.cancelable !== false && !commandPending
    readonly property string consoleName: streamerMode ? qsTr("Your console") : attempt.name || qsTr("Console")

    component ConnectionLabel: Label {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        textFormat: Text.PlainText
        font.pixelSize: 14
    }
    component ConnectionButton: SetupButton {
        Layout.maximumWidth: parent.width
        textFormat: Text.PlainText
    }

    function cancel() {
        if (!canCancel) return;
        commandPending = true;
        if (!backend.cancelConnection(attempt.id)) commandPending = false;
    }
    function retry() {
        if (!canRetry) return;
        commandPending = true;
        if (!backend.retryConnection(attempt.id)) commandPending = false;
    }
    function back() {
        if (!terminal || attempt.cleanupPending || leaving || commandPending) return;
        leaving = true;
        backend.dismissConnection(attempt.id);
        navigation.showMainView();
    }
    function focusAction() {
        if (!visible || leaving || commandPending) return;
        if (canRetry) retryButton.forceActiveFocus(Qt.TabFocusReason);
        else if (terminal && !attempt.cleanupPending) backButton.forceActiveFocus(Qt.TabFocusReason);
        else if (canCancel) cancelButton.forceActiveFocus(Qt.TabFocusReason);
        else view.forceActiveFocus(Qt.TabFocusReason);
    }
    function ensureFocusVisible() {
        const item = view.Window.window ? view.Window.window.activeFocusItem : null;
        if (!item) return;
        let ancestor = item;
        while (ancestor && ancestor !== workspace) ancestor = ancestor.parent;
        if (!ancestor) return;
        const point = item.mapToItem(workspace, 0, 0);
        if (point.y < scroll.contentY + 12) scroll.contentY = Math.max(0, point.y - 12);
        else if (point.y + item.height > scroll.contentY + scroll.height - 12)
            scroll.contentY = Math.min(Math.max(0, scroll.contentHeight - scroll.height), point.y + item.height - scroll.height + 12);
    }
    onAttemptChanged: {
        if (attempt.id !== commandAttempt) {
            commandAttempt = attempt.id || "";
            commandPending = false;
            detailsExpanded = false;
        }
        if (terminal || phase === "cancelling") commandPending = false;
        Qt.callLater(focusAction);
    }
    onVisibleChanged: if (visible) Qt.callLater(focusAction)
    Component.onCompleted: Qt.callLater(focusAction)
    Component.onDestruction: if (managesAttempt && !leaving && attempt.id && phase !== "ready") backend.dismissConnection(attempt.id)
    Keys.onEscapePressed: terminal ? back() : cancel()
    Shortcut { sequence: "Ctrl+Q"; enabled: view.visible && view.enabled; onActivated: view.terminal ? view.back() : view.cancel() }
    MouseArea { anchors.fill: parent; acceptedButtons: Qt.RightButton; onClicked: view.terminal ? view.back() : view.cancel() }
    Connections { target: view.Window.window; function onActiveFocusItemChanged() { Qt.callLater(view.ensureFocusVisible); } }

    Rectangle { anchors.fill: parent; color: "#11151c" }
    Rectangle {
        id: sidebar
        width: view.width < 1000 ? 76 : 176
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
        color: "#141821"
        ColumnLayout {
            anchors.fill: parent; anchors.margins: 16; spacing: 28
            RowLayout {
                Layout.fillWidth: true
                Rectangle {
                    Layout.preferredWidth: 32; Layout.preferredHeight: 32
                    color: "#69b8ff"; radius: 8
                    Image {
                        anchors.fill: parent; anchors.margins: 5
                        source: "qrc:/icons/chiaking-logo-white.svg"
                        fillMode: Image.PreserveAspectFit
                        Accessible.ignored: true
                    }
                }
                Label { visible: view.width >= 1000; text: "chiaki-ng"; font.pixelSize: 18; font.bold: true }
            }
            ConsoleButton {
                Layout.fillWidth: true
                text: qsTr("Consoles")
                icon.source: "qrc:/icons/discover-24px.svg"
                display: view.width < 1000 ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                highlighted: true
                enabled: view.terminal && !view.attempt.cleanupPending
                Accessible.name: qsTr("Back to consoles")
                onClicked: view.back()
            }
            Item { Layout.fillHeight: true }
        }
    }
    ColumnLayout {
        anchors { left: sidebar.right; right: parent.right; top: parent.top; bottom: parent.bottom; margins: 24 }
        spacing: 16
        ConnectionLabel { text: qsTr("Consoles / %1").arg(view.heading); color: "#9ca8b9"; font.pixelSize: 12 }
        RowLayout {
            Layout.fillWidth: true
            ConnectionLabel { objectName: "connectionHeading"; text: view.heading; font.pixelSize: 30; font.bold: true }
            Label { objectName: "connectionRoute"; text: view.wakeFlow ? qsTr("Local discovery") : view.psn ? qsTr("Remote connection") : qsTr("Local network"); color: "#91caff"; font.pixelSize: 13; textFormat: Text.PlainText; wrapMode: Text.Wrap; Layout.maximumWidth: 180 }
        }
        ConnectionLabel { text: qsTr("Remote Play with your selected console."); color: "#9ca8b9" }
        Flickable {
            id: scroll
            objectName: "connectionScroll"
            Layout.fillWidth: true; Layout.fillHeight: true
            contentWidth: width; contentHeight: workspace.implicitHeight
            clip: true; boundsBehavior: Flickable.StopAtBounds; flickableDirection: Flickable.VerticalFlick
            ScrollBar.vertical: ScrollBar {}
            onContentHeightChanged: Qt.callLater(view.ensureFocusVisible)
            onHeightChanged: Qt.callLater(view.ensureFocusVisible)
            RowLayout {
                id: workspace
                width: scroll.width; spacing: 24
                Pane {
                    Layout.fillWidth: true; Layout.preferredWidth: workspace.width * 0.64; Layout.alignment: Qt.AlignTop
                    padding: 24
                    background: Rectangle { color: "#171b23"; radius: 12; border.color: "#2b323e" }
                    ColumnLayout {
                        anchors.fill: parent; spacing: 20
                        RowLayout {
                            Layout.fillWidth: true; spacing: 16
                            Image {
                                Layout.preferredWidth: 56; Layout.preferredHeight: 48
                                source: "image://svg/console-ps" + (view.attempt.ps5 === false ? "4" : "5") + "#console"
                                fillMode: Image.PreserveAspectFit
                                sourceSize: Qt.size(56, 48)
                                Accessible.ignored: true
                            }
                            ColumnLayout {
                                Layout.fillWidth: true
                                ConnectionLabel { objectName: "connectionConsole"; text: view.consoleName; font.bold: true; font.pixelSize: 17 }
                                ConnectionLabel {
                                    objectName: "connectionAddress"
                                    text: view.psn ? qsTr("PlayStation Network") : view.streamerMode ? qsTr("Address hidden") : view.attempt.address || qsTr("Console address unavailable")
                                    color: "#9ca8b9"; font.pixelSize: 12
                                }
                            }
                        }
                        Rectangle { Layout.fillWidth: true; height: 1; color: "#2b323e" }
                        RowLayout {
                            Layout.fillWidth: true; spacing: 14
                            BusyIndicator { Layout.preferredWidth: 34; Layout.preferredHeight: 34; running: view.busy; visible: view.busy }
                            Label { visible: !view.busy; text: view.phase === "failure" ? "!" : "✓"; color: view.phase === "failure" ? "#ffb4ab" : "#8cd5b0"; font.pixelSize: 28 }
                            ConnectionLabel {
                                objectName: "connectionTitle"; font.pixelSize: 22; font.bold: true
                                text: view.phase === "waiting" ? qsTr("Waiting for console")
                                    : view.phase === "waking" ? qsTr("Waiting for the console to wake")
                                    : view.phase === "found" ? qsTr("Console found")
                                    : view.phase === "video" ? qsTr("Waiting for video")
                                    : view.phase === "internet" ? qsTr("Waiting for internet")
                                    : view.phase === "initiating" ? qsTr("Contacting PlayStation Network")
                                    : view.phase === "linking" ? qsTr("Linking your console")
                                    : view.phase === "registering" ? qsTr("Registering your console")
                                    : view.phase === "registered" ? qsTr("Console registered")
                                    : view.phase === "data" ? qsTr("Opening the data connection")
                                    : view.phase === "ready" ? qsTr("Your stream is ready")
                                    : view.phase === "failure" ? (view.streamerMode ? qsTr("Could not start Remote Play") : view.attempt.errorTitle || qsTr("Could not start Remote Play"))
                                    : view.phase === "cancelling" ? (view.psn ? qsTr("Cancelling remote connection") : qsTr("Cancelling connection"))
                                    : view.phase === "cancelled" ? qsTr("Connection cancelled") : qsTr("Connecting to console")
                            }
                        }
                        ConnectionLabel {
                            objectName: "connectionDescription"; color: "#adb8c8"
                            text: view.phase === "waiting" ? qsTr("Looking for the selected console on your network.")
                                : view.phase === "waking" ? qsTr("A wake request was sent. Waiting for the console to become available.")
                                : view.phase === "found" ? qsTr("Continuing to the Remote Play connection.")
                                : view.phase === "video" ? (view.psn ? qsTr("The remote connection is ready. Waiting for the first video frame.") : qsTr("The session is connected. Waiting for the first video frame."))
                                : view.phase === "internet" ? qsTr("Waiting for an internet connection to reach PlayStation Network.")
                                : view.phase === "initiating" ? qsTr("Preparing a remote connection to your console.")
                                : view.phase === "linking" ? qsTr("This step must finish before cancellation is available.")
                                : view.phase === "registering" ? qsTr("Automatic registration is in progress.")
                                : view.phase === "registered" ? qsTr("Registration is complete. Return to the library to connect.")
                                : view.phase === "data" ? qsTr("Your console is linked. Establishing the Remote Play connection.")
                                : view.phase === "ready" ? qsTr("Opening your Remote Play stream.")
                                : view.phase === "failure" ? (view.wakeFlow ? qsTr("Check the console power and network settings, then try again.") : view.psn ? qsTr("Check that your console is online and available for Remote Play.") : qsTr("Check that the console is on and Remote Play is enabled."))
                                : view.phase === "cancelling" ? qsTr("Waiting for the current connection attempt to stop.")
                                : view.phase === "cancelled" ? qsTr("Your console is still available in the library.")
                                : qsTr("Opening a Remote Play session on your local network.")
                        }
                        ConnectionLabel {
                            objectName: "connectionCompleted"
                            visible: view.psn && text.length > 0
                            color: "#8cd5b0"
                            text: (view.attempt.completedStages || []).map(stage => {
                                switch (stage) {
                                case "psn": return qsTr("✓ Reached PlayStation Network");
                                case "linking": return qsTr("✓ Console linked");
                                case "registration": return qsTr("✓ Registration completed");
                                case "data": return qsTr("✓ Data connection opened");
                                default: return "";
                                }
                            }).filter(text => text.length > 0).join("\n")
                        }
                        ConnectionLabel {
                            objectName: "connectionFallback"; color: "#91caff"
                            visible: view.attempt.fallback === true && view.attempt.route === "psn" && !view.wakeFlow
                            text: qsTr("The console was not found locally. Continuing through PSN.")
                        }
                        ConnectionLabel {
                            objectName: "connectionInvalidTarget"; color: "#ffb4ab"
                            visible: view.phase === "failure" && !view.attempt.targetValid
                            text: qsTr("This console or profile is no longer available. Select the console again from the library.")
                        }
                        ConnectionButton {
                            objectName: "connectionDetailsToggle"
                            visible: view.phase === "failure" && !!view.attempt.errorText
                            text: view.detailsExpanded ? qsTr("Hide connection details") : qsTr("Connection details")
                            flat: true; onClicked: view.detailsExpanded = !view.detailsExpanded
                        }
                        ConnectionLabel {
                            objectName: "connectionDetails"; visible: view.phase === "failure" && view.detailsExpanded
                            color: "#adb8c8"
                            text: view.streamerMode ? qsTr("Connection details are hidden in streamer mode.") : view.attempt.errorText || ""
                        }
                        ConnectionLabel {
                            objectName: "connectionCancelReason"; color: "#9ca8b9"
                            visible: view.busy || (view.terminal && view.attempt.cleanupPending)
                            text: view.phase === "cancelling" || view.attempt.cleanupPending && view.terminal
                                ? qsTr("Please wait for the connection to stop.")
                                : view.attempt.cancelable === false ? qsTr("Please wait for this connection step to finish before cancelling.") : qsTr("Esc to cancel")
                        }
                        Flow {
                            Layout.fillWidth: true
                            spacing: 12
                            ConnectionButton {
                                id: backButton; objectName: "connectionBack"
                                width: Math.min(implicitWidth, parent.width)
                                visible: view.terminal; enabled: !view.attempt.cleanupPending && !view.leaving && !view.commandPending
                                text: qsTr("Back to consoles"); onClicked: view.back()
                            }
                            ConnectionButton {
                                id: retryButton; objectName: "connectionRetry"
                                width: Math.min(implicitWidth, parent.width)
                                visible: view.phase === "failure"; enabled: view.canRetry; primary: true
                                text: qsTr("Try again"); onClicked: view.retry()
                            }
                            ConnectionButton {
                                id: cancelButton; objectName: "connectionCancel"
                                width: Math.min(implicitWidth, parent.width)
                                visible: view.busy; enabled: view.canCancel
                                text: view.phase === "cancelling" ? qsTr("Cancelling…") : qsTr("Cancel connection")
                                onClicked: view.cancel()
                            }
                        }
                    }
                }
                ColumnLayout {
                    objectName: "connectionHelp"
                    Layout.fillWidth: true; Layout.preferredWidth: workspace.width * 0.36; Layout.alignment: Qt.AlignTop
                    spacing: 16
                    ConnectionLabel { text: qsTr("GOOD TO KNOW"); color: "#9ca8b9"; font.pixelSize: 11; font.letterSpacing: 1.5 }
                    ConnectionLabel { text: view.wakeFlow ? qsTr("Your console may be waking") : view.psn ? qsTr("Connect away from home") : qsTr("Ready for Remote Play"); font.pixelSize: 21; font.bold: true }
                    ConnectionLabel { text: view.wakeFlow ? qsTr("In rest mode, the console must allow network access and wake-up for Remote Play.") : view.psn ? qsTr("Keep your console connected to the internet. A remote connection can take longer to start.") : qsTr("Keep your console on and connected to your local network."); color: "#adb8c8" }
                    Rectangle { Layout.fillWidth: true; height: 1; color: "#2b323e" }
                    ConnectionLabel { text: view.wakeFlow ? qsTr("Choose another console") : view.psn ? qsTr("A brief wait") : qsTr("Connection, then video"); font.bold: true }
                    ConnectionLabel { text: view.wakeFlow ? qsTr("Cancel this attempt to return to your console library. Your saved auto-connect choice is kept.") : view.psn ? qsTr("Some connection steps must finish before cancellation is available.") : qsTr("The session can connect before the first video frame arrives."); color: "#adb8c8" }
                }
            }
        }
    }
}
