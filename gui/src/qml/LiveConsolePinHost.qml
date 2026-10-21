import QtQuick
import QtQuick.Controls
import QtQuick.Window
import org.streetpea.chiaking

FocusScope {
    id: host
    objectName: "liveConsolePinHost"
    property var backend: Chiaki
    property var navigation: root
    property var hostWindow: backend.window
    readonly property var request: backend.sessionPinRequest || ({})
    readonly property bool blocking: !!request.id
    readonly property bool separate: backend.window.runtimeRendererBackend === 1
    property string currentId: ""
    property bool inputGrabbed: false
    property var restoreFocusItem: null
    visible: blocking
    z: 1000

    function updateContent() {
        if (currentId !== (request.id || "")) {
            inlineLoader.active = false;
            separateLoader.active = false;
            currentId = request.id || "";
        }
        inlineLoader.active = !!request.id && !separate;
        separateLoader.active = !!request.id && separate;
    }
    function updateInput() {
        if (request.id && !inputGrabbed) {
            restoreFocusTimer.stop();
            restoreFocusItem = host.Window.window ? host.Window.window.activeFocusItem : null;
            backend.window.grabInput();
            inputGrabbed = true;
        } else if (!request.id && inputGrabbed) {
            backend.window.releaseInput();
            inputGrabbed = false;
            restoreFocusTimer.restart();
        }
    }
    Timer {
        id: restoreFocusTimer
        interval: 0
        onTriggered: {
            if (host.restoreFocusItem && host.restoreFocusItem.visible && host.restoreFocusItem.enabled)
                host.restoreFocusItem.forceActiveFocus(Qt.TabFocusReason);
            else if (host.navigation.restoreMainFocus) host.navigation.restoreMainFocus();
            host.restoreFocusItem = null;
        }
    }
    function cancel() {
        const content = separate ? separateLoader.item : inlineLoader.item;
        if (content) content.cancel();
    }
    onRequestChanged: { updateInput(); updateContent(); }
    onSeparateChanged: updateContent()
    Component.onCompleted: { updateInput(); updateContent(); }
    Component.onDestruction: {
        if (inputGrabbed) backend.window.releaseInput();
        if (request.id && (request.phase === "form" || request.phase === "submitting"))
            backend.cancelSessionPin(request.id);
    }
    Keys.onEscapePressed: cancel()
    Shortcut { sequence: "Ctrl+Q"; enabled: host.blocking; onActivated: host.cancel() }
    MouseArea { anchors.fill: parent; acceptedButtons: Qt.AllButtons; onClicked: mouse => { if (mouse.button === Qt.RightButton) host.cancel(); } }
    Loader {
        id: inlineLoader
        anchors.fill: parent
        active: false
        sourceComponent: LiveConsolePinContent {
            backend: host.backend
            requestId: ""
            navigation: host.navigation
            showSidebar: true
        }
        onLoaded: { item.requestId = host.currentId; item.focusPin(); }
    }
    Window {
        id: pinWindow
        objectName: "liveConsolePinWindow"
        visible: host.blocking && host.separate
        transientParent: host.hostWindow
        flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint
        title: qsTr("Console login PIN")
        modality: Qt.ApplicationModal
        color: "#11151c"
        width: Math.min(720, host.hostWindow ? host.hostWindow.width : 720)
        height: Math.min(560, host.hostWindow ? host.hostWindow.height : 560)
        x: host.hostWindow ? Math.round(host.hostWindow.x + (host.hostWindow.width - width) / 2) : 0
        y: host.hostWindow ? Math.round(host.hostWindow.y + (host.hostWindow.height - height) / 2) : 0
        onClosing: close => { close.accepted = false; host.cancel(); }
        onVisibleChanged: if (visible) { requestActivate(); if (separateLoader.item) separateLoader.item.focusPin(); }
        Loader {
            id: separateLoader
            anchors.fill: parent
            active: false
            sourceComponent: LiveConsolePinContent {
                backend: host.backend
                requestId: ""
                navigation: host.navigation
            }
            onLoaded: { item.requestId = host.currentId; pinWindow.requestActivate(); item.focusPin(); }
        }
    }
}
