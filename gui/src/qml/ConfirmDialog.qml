import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Dialog {
    id: dialog
    property alias text: message.text
    property string actionText: ""
    property string secondaryActionText: ""
    property var callback: null
    property var secondaryCallback: null
    property var rejectCallback: null
    property var navigation: null
    property Item restoreFocusItem
    property bool handled: false
    property var completion: null
    objectName: "confirmationDialog"
    parent: Overlay.overlay
    width: Math.min(520, parent.width - 32)
    height: Math.min(implicitHeight, parent.height - 32)
    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    padding: 28
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape
    background: Rectangle { color: "#181d25"; radius: 18; border.color: "#3b4655" }
    header: Label {
        text: dialog.title
        textFormat: Text.PlainText
        wrapMode: Text.Wrap
        font.pixelSize: 24
        font.bold: true
        color: "#e7ebf2"
        padding: 28
        bottomPadding: 0
    }
    function choose(action) {
        if (handled || !visible) return;
        handled = true;
        completion = action === "primary" ? callback : action === "secondary" ? secondaryCallback : rejectCallback;
        close();
    }
    onAboutToShow: {
        handled = false;
        completion = null;
        cancelButton.focus = true;
    }
    onOpened: {
        cancelButton.focus = false;
        cancelButton.forceActiveFocus(Qt.TabFocusReason);
    }
    onAccepted: choose("primary")
    onRejected: choose("cancel")
    onAboutToHide: {
        if (!handled) { handled = true; completion = rejectCallback; }
    }
    onClosed: {
        const done = completion;
        completion = null;
        callback = null; secondaryCallback = null; rejectCallback = null;
        if (restoreFocusItem && restoreFocusItem.visible && restoreFocusItem.enabled)
            restoreFocusItem.forceActiveFocus(Qt.TabFocusReason);
        else if (navigation) navigation.restoreMainFocus();
        restoreFocusItem = null;
        if (done) done();
    }
    contentItem: ScrollView {
        id: scroll
        clip: true
        contentWidth: availableWidth
        implicitHeight: body.implicitHeight
        ColumnLayout {
            id: body
            width: scroll.availableWidth
            spacing: 20
            Keys.onEscapePressed: dialog.choose("cancel")
            Label {
                id: message
                objectName: "confirmationMessage"
                Layout.fillWidth: true
                color: "#aab6c7"
                font.pixelSize: 14
                wrapMode: Text.Wrap
                textFormat: Text.PlainText
            }
        }
    }
    footer: Item {
        implicitHeight: actions.implicitHeight + 28
        ColumnLayout {
            id: actions
            spacing: 10
            // Keep actions outside the scroll area so long instructions cannot hide Cancel.
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 28
            anchors.rightMargin: 28
            SetupButton {
                id: primaryButton
                objectName: "confirmationAction"
                Layout.fillWidth: true
                text: dialog.actionText
                textFormat: Text.PlainText
                primary: true
                onClicked: dialog.choose("primary")
                KeyNavigation.up: cancelButton
                KeyNavigation.down: secondaryButton.visible ? secondaryButton : cancelButton
            }
            SetupButton {
                id: secondaryButton
                objectName: "confirmationSecondaryAction"
                Layout.fillWidth: true
                visible: dialog.secondaryActionText.length > 0
                text: dialog.secondaryActionText
                textFormat: Text.PlainText
                onClicked: dialog.choose("secondary")
                KeyNavigation.up: primaryButton
                KeyNavigation.down: cancelButton
            }
            SetupButton {
                id: cancelButton
                objectName: "confirmationCancel"
                Layout.fillWidth: true
                text: qsTr("Cancel")
                onClicked: dialog.choose("cancel")
                KeyNavigation.up: secondaryButton.visible ? secondaryButton : primaryButton
                KeyNavigation.down: primaryButton
            }
        }
    }
}
