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
    property string presentation: ""

    function open() {
        if (opened) return;
        presentation = backend.window.streamInput.present(host, "disconnect", null);
        dialog.callback = () => backend.stopSession(true);
        dialog.secondaryCallback = () => backend.stopSession(false);
        if (separate) stopWindow.show();
        dialog.open();
    }
    ConfirmDialog {
        id: dialog
        parent: host.separate ? stopWindow.contentItem : Overlay.overlay
        title: qsTr("Disconnect session")
        text: qsTr("Choose whether to put the console into sleep mode when you disconnect. Cancel keeps the session running.")
        actionText: qsTr("Disconnect and sleep")
        secondaryActionText: qsTr("Disconnect")
        onAboutToHide: {
            const id = host.presentation;
            const complete = function() {
                dialog.closed.disconnect(complete);
                host.finishClose(id);
            };
            dialog.closed.connect(complete);
        }
    }
    function finishClose(id) {
        if (!id || id !== presentation || dialog.visible) return;
        if (stopWindow.visible) {
            stopWindow.hide();
            if (hostWindow) hostWindow.requestActivate();
        }
        backend.window.streamInput.dismiss(id);
        backend.window.streamInput.settled(id);
        presentation = "";
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
