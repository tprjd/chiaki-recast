import QtQuick
import QtQuick.Controls
import QtQuick.Window
import org.streetpea.chiaking

Item {
    id: host
    property var backend: Chiaki
    property var hostWindow: host.Window.window
    readonly property bool separate: backend.window.runtimeRendererBackend === 1
    readonly property bool opened: dialog.visible
    property bool inputGrabbed: false
    signal inputRequested()
    signal inputReleased()

    function open() {
        if (opened) return;
        inputGrabbed = true;
        inputRequested();
        dialog.callback = () => backend.stopSession(true);
        dialog.secondaryCallback = () => backend.stopSession(false);
        if (separate) stopWindow.show();
        dialog.open();
    }
    function releaseInput() {
        if (!inputGrabbed) return;
        inputGrabbed = false;
        inputReleased();
    }
    Component.onDestruction: releaseInput()

    ConfirmDialog {
        id: dialog
        parent: host.separate ? stopWindow.contentItem : Overlay.overlay
        title: qsTr("Disconnect session")
        text: qsTr("Choose whether to put the console into sleep mode when you disconnect. Cancel keeps the session running.")
        actionText: qsTr("Disconnect and sleep")
        secondaryActionText: qsTr("Disconnect")
    }
    Connections {
        target: dialog
        function onClosed() {
            if (stopWindow.visible) {
                stopWindow.hide();
                if (host.hostWindow) host.hostWindow.requestActivate();
            }
            host.releaseInput();
        }
    }
    Window {
        id: stopWindow
        objectName: "streamDisconnectWindow"
        transientParent: host.hostWindow
        flags: Qt.Dialog | Qt.FramelessWindowHint
        modality: Qt.ApplicationModal
        color: "transparent"
        title: dialog.title
        width: Math.min(552, host.hostWindow ? host.hostWindow.width : 552)
        height: Math.min(dialog.implicitHeight + 32, host.hostWindow ? host.hostWindow.height : 600)
        x: host.hostWindow ? Math.round(host.hostWindow.x + (host.hostWindow.width - width) / 2) : 0
        y: host.hostWindow ? Math.round(host.hostWindow.y + (host.hostWindow.height - height) / 2) : 0
        onVisibleChanged: if (visible) requestActivate()
        onClosing: close => { close.accepted = false; dialog.choose("cancel"); }
    }
}
