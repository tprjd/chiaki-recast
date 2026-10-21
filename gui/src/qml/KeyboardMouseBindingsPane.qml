import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "controls" as C

ColumnLayout {
    id: pane
    objectName: "keyboardMouseBindingsPane"
    property var settings
    signal saved()
    onSettingsChanged: if (capture) capture.close()
    spacing: 10
    onEnabledChanged: if (!enabled) capture.close()
    onVisibleChanged: if (!visible) capture.close()

    function findSlot(item, name) {
        if (item.objectName === name) return item;
        for (const child of item.children) {
            const found = findSlot(child, name);
            if (found) return found;
        }
        return null;
    }

    Label {
        Layout.fillWidth: true
        text: qsTr("Choose a slot to assign or clear one key or mouse button. Mouse movement controls the right stick.")
        wrapMode: Text.WordWrap
    }
    RowLayout {
        Layout.fillWidth: true
        Label { Layout.fillWidth: true; Layout.preferredWidth: 180; text: qsTr("Action"); font.bold: true }
        Label { Layout.fillWidth: true; Layout.preferredWidth: 140; text: qsTr("Primary"); font.bold: true }
        Label { Layout.fillWidth: true; Layout.preferredWidth: 140; text: qsTr("Alternative"); font.bold: true }
    }
    Repeater {
        model: pane.settings ? pane.settings.keyboardMouseBindings : []
        ColumnLayout {
            id: actionRow
            required property var modelData
            Layout.fillWidth: true
            Label {
                Layout.fillWidth: true
                Layout.topMargin: 10
                visible: [0, 4, 8, 14, 18].indexOf(actionRow.modelData.action) >= 0
                text: [qsTr("Movement"), qsTr("Buttons"), qsTr("Shoulders and sticks"), qsTr("D-pad"), qsTr("System")][actionRow.modelData.group]
                font.bold: true
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.fillWidth: true
                Label {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    Layout.preferredWidth: 180
                    text: actionRow.modelData.name
                    wrapMode: Text.WordWrap
                }
                Repeater {
                    model: 2
                    C.Button {
                        required property int index
                        objectName: "keyboardMouseBinding" + actionRow.modelData.action + "_" + index
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredWidth: 140
                        text: index === 0 ? actionRow.modelData.primaryName : actionRow.modelData.alternativeName
                        Accessible.name: qsTr("%1, %2, %3").arg(actionRow.modelData.name)
                            .arg(index === 0 ? qsTr("Primary") : qsTr("Alternative")).arg(text)
                        onClicked: capture.show(actionRow.modelData.action, index)
                    }
                }
            }
        }
    }

    Dialog {
        id: capture
        objectName: "keyboardMouseBindingDialog"
        parent: Overlay.overlay
        width: Math.min(520, parent ? parent.width - 32 : 520)
        x: parent ? Math.round((parent.width - width) / 2) : 0
        y: parent ? Math.round((parent.height - height) / 2) : 0
        modal: true
        focus: false
        closePolicy: Popup.CloseOnEscape
        title: qsTr("Assign binding")
        property int action: -1
        property int slot: 0
        property int pressedKey: 0
        property int pendingInput: 0
        property string conflictName: ""
        property string error: ""
        onOpened: prompt.forceActiveFocus(Qt.TabFocusReason)
        onAboutToHide: pressedKey = 0
        onClosed: Qt.callLater(() => {
            if (capture.opened || !pane.visible || !pane.enabled) return;
            const target = pane.findSlot(pane, "keyboardMouseBinding" + action + "_" + slot);
            if (target) target.forceActiveFocus(Qt.TabFocusReason);
        })
        function show(action, slot) {
            this.action = action;
            this.slot = slot;
            pressedKey = 0;
            conflictName = "";
            error = "";
            open();
        }
        function offer(input, move) {
            if (!opened || !pane.settings) return;
            const result = pane.settings.changeKeyboardMouseBinding(action, slot, input, !!move);
            error = result.error;
            if (error.length) return;
            if (result.conflict >= 0) {
                pendingInput = input;
                conflictName = result.name;
                moveButton.forceActiveFocus(Qt.TabFocusReason);
                return;
            }
            pane.saved();
            close();
        }
        Connections {
            target: pane.settings
            ignoreUnknownSignals: true
            function onCurrentProfileChanged() { capture.close(); }
        }
        contentItem: FocusScope {
            property bool keyboardMouseBindingCapture: capture.opened
            implicitHeight: captureLayout.implicitHeight
            Keys.onShortcutOverride: event => event.accepted = true
            ColumnLayout {
                id: captureLayout
                width: parent.width
                spacing: 12
                Label {
                    id: prompt
                    objectName: "keyboardMouseBindingPrompt"
                    visible: !capture.conflictName.length
                    Layout.fillWidth: true
                    padding: 16
                    text: qsTr("Press a key, or click this box with a mouse button. Escape cancels. Tab moves focus. Use Assign Tab to bind Tab.")
                    wrapMode: Text.WordWrap
                    Accessible.name: text
                    background: Rectangle { color: "#1a202b"; radius: 6; border.color: "#64748b" }
                    Keys.onPressed: event => {
                        if (event.key === Qt.Key_Tab || event.key === Qt.Key_Backtab) return;
                        event.accepted = true;
                        if (!event.isAutoRepeat) capture.pressedKey = pane.settings.normalizeKeyboardMouseKey(event.key);
                    }
                    Keys.onReleased: event => {
                        if (event.key === Qt.Key_Tab || event.key === Qt.Key_Backtab) return;
                        event.accepted = true;
                        if (event.isAutoRepeat || capture.pressedKey !== pane.settings.normalizeKeyboardMouseKey(event.key)) return;
                        capture.pressedKey = 0;
                        if (event.key === Qt.Key_Escape) capture.close();
                        else capture.offer(event.key, false);
                    }
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton | Qt.BackButton | Qt.ForwardButton
                        onReleased: mouse => capture.offer(-mouse.button, false)
                    }
                }
                Label {
                    objectName: "keyboardMouseBindingConflict"
                    Layout.fillWidth: true
                    visible: capture.conflictName.length > 0
                    text: qsTr("This input is assigned to %1. Move binding or cancel.").arg(capture.conflictName)
                    wrapMode: Text.WordWrap
                    Accessible.name: text
                }
                Label {
                    objectName: "keyboardMouseBindingError"
                    Layout.fillWidth: true
                    visible: text.length > 0
                    text: capture.error
                    wrapMode: Text.WordWrap
                    Accessible.name: text
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: 8
                    C.Button {
                        objectName: "keyboardMouseAssignTab"
                        visible: !capture.conflictName.length
                        text: qsTr("Assign Tab")
                        onClicked: capture.offer(Qt.Key_Tab, false)
                    }
                    C.Button {
                        objectName: "keyboardMouseClearBinding"
                        visible: !capture.conflictName.length
                        text: qsTr("Clear binding")
                        onClicked: capture.offer(0, false)
                    }
                    C.Button {
                        id: moveButton
                        objectName: "keyboardMouseMoveBinding"
                        visible: capture.conflictName.length > 0
                        text: qsTr("Move binding")
                        onClicked: capture.offer(capture.pendingInput, true)
                    }
                    C.Button { text: qsTr("Cancel"); onClicked: capture.close() }
                }
            }
        }
    }
}
