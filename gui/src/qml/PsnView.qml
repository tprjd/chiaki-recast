import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import org.streetpea.chiaking

Item {
    id: view
    objectName: "psnConnectionView"
    property list<Item> restoreFocusItems
    property bool textVisible: true
    property bool connectionInputGrabbed: false
    property string attemptId: Chiaki.connectionAttempt.id || ""

    function grabInput(item) {
        Chiaki.window.grabInput();
        restoreFocusItems.push(Window.window.activeFocusItem);
        if (item) item.forceActiveFocus(Qt.TabFocusReason);
    }
    function releaseInput() {
        Chiaki.window.releaseInput();
        const item = restoreFocusItems.pop();
        if (item && item.visible) item.forceActiveFocus(Qt.TabFocusReason);
    }
    Component.onCompleted: {
        Chiaki.window.grabInput();
        connectionInputGrabbed = true;
    }
    Component.onDestruction: {
        if (connectionInputGrabbed) Chiaki.window.releaseInput();
        const attempt = Chiaki.connectionAttempt;
        if (attempt.id === attemptId && attempt.phase !== "video" && attempt.phase !== "ready")
            Chiaki.dismissConnection(attemptId);
    }
    ConnectionView { anchors.fill: parent; managesAttempt: false }
    Dialog {
        id: sessionPinDialog
        parent: Overlay.overlay
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)
        title: qsTr("Console Login PIN")
        modal: true
        closePolicy: Popup.NoAutoClose
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAboutToShow: {
            standardButton(Dialog.Ok).enabled = Qt.binding(function() {
                return pinField.acceptableInput;
            });
            textVisible = false
            view.grabInput(pinField);
        }
        onClosed: {
            textVisible = true
            view.releaseInput()
        }
        onAccepted: Chiaki.enterPin(pinField.text)
        onRejected: Chiaki.stopSession(false)
        Material.roundedScale: Material.MediumScale

        TextField {
            id: pinField
            echoMode: Chiaki.settings.streamerMode ? TextInput.Password : TextInput.Normal
            implicitWidth: 200
            validator: RegularExpressionValidator { regularExpression: /[0-9]{4}/ }
            Keys.onReturnPressed: {
                if(sessionPinDialog.standardButton(Dialog.Ok).enabled)
                    sessionPinDialog.standardButton(Dialog.Ok).clicked()
            }
        }
    }

    Connections {
        target: Chiaki
        function onSessionPinDialogRequested() {
            if (!sessionPinDialog.opened) sessionPinDialog.open();
        }
    }
}
