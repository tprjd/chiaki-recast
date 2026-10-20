import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import org.streetpea.chiaking
import "controls" as C

DialogView {
    id: dialog
    objectName: "controllerMappingDialog"
    property var backend: Chiaki
    property var navigation: root
    property bool finalClosing: false
    title: qsTr("Controller mapping")
    buttonVisible: false
    buttonEnabled: backend.controllerMappingInProgress && backend.controllerMappingAltered && !buttonDialog.visible
    Material.theme: Material.Dark
    Material.primary: "#11151c"
    Material.background: "#11151c"
    function close() {
        if (finalClosing) return;
        finalClosing = true;
        buttonDialog.close();
        if (backend.controllerMappingInProgress) backend.controllerMappingQuit();
        navigation.closeDialog();
    }
    onAccepted: {
        if (!buttonEnabled || finalClosing) return;
        backend.controllerMappingApply();
        backend.controllerMappingQuit();
    }
    Component.onDestruction: {
        if (!finalClosing && backend.controllerMappingInProgress) {
            finalClosing = true;
            backend.controllerMappingButtonQuit();
            backend.controllerMappingQuit();
        }
    }
    function focusMapping(value, slot) {
        for (let i = 0; i < chiakiButtons.count; i++) {
            const row = chiakiButtons.itemAt(i);
            if (row && row.modelData.buttonValue === value) {
                (slot === 0 ? row.primaryButton : row.alternateButton).forceActiveFocus(Qt.TabFocusReason);
                return;
            }
        }
        analogStickMapping.forceActiveFocus(Qt.TabFocusReason);
    }
    function navigateMapping(index, slot, event) {
        let target;
        if (event.key === Qt.Key_Up) {
            const row = chiakiButtons.itemAt(index - 1);
            target = row ? (slot === 0 ? row.primaryButton : row.alternateButton) : analogStickMapping;
        } else if (event.key === Qt.Key_Down) {
            const row = chiakiButtons.itemAt(index + 1);
            target = row ? (slot === 0 ? row.primaryButton : row.alternateButton) : (applyMapping.enabled ? applyMapping : cancelMapping);
        } else if (event.key === Qt.Key_Left || event.key === Qt.Key_Right) {
            const row = chiakiButtons.itemAt(index);
            target = row ? (event.key === Qt.Key_Left ? row.primaryButton : row.alternateButton) : analogStickMapping;
        } else if (event.key === Qt.Key_Return) {
            const row = chiakiButtons.itemAt(index);
            if (row) buttonDialog.show(row.modelData.buttonValue, slot);
        } else return;
        if (target) target.forceActiveFocus(Qt.TabFocusReason);
        event.accepted = true;
    }
    function ensureFocusVisible() {
        const item = Window.window ? Window.window.activeFocusItem : null;
        let ancestor = item;
        while (ancestor && ancestor !== mappingFlick.contentItem) ancestor = ancestor.parent;
        if (!ancestor) return;
        const point = item.mapToItem(mappingFlick.contentItem, 0, 0);
        if (point.y < mappingFlick.contentY) mappingFlick.contentY = point.y;
        else if (point.y + item.height > mappingFlick.contentY + mappingFlick.height)
            mappingFlick.contentY = point.y + item.height - mappingFlick.height;
    }
    Item {
        Rectangle { parent: dialog; anchors.fill: parent; color: "#11151c"; z: -1 }
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12
            Label {
                objectName: "mappingControllerName"
                Layout.fillWidth: true
                text: dialog.backend.currentControllerType
                font.pixelSize: 14
                wrapMode: Text.Wrap
            }
            C.CheckBox {
                id: analogStickMapping
                objectName: "enableAnalogStickMapping"
                firstInFocusChain: true
                text: qsTr("Allow mapping analog sticks and L2/R2")
                font.pixelSize: 14
                checked: dialog.backend.enableAnalogStickMapping
                onToggled: dialog.backend.enableAnalogStickMapping = checked
            }
            Label { text: qsTr("Select a primary or alternate input to capture a controller button."); font.pixelSize: 14 }
            Flickable {
                id: mappingFlick
                objectName: "controllerMappingFlick"
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: width
                contentHeight: mappingRows.height
                flickableDirection: Flickable.VerticalFlick
                ScrollBar.vertical: ScrollBar {}
                ColumnLayout {
                    id: mappingRows
                    width: Math.max(0, mappingFlick.width - 16)
                    spacing: 8
                    Repeater {
                        id: chiakiButtons
                        model: dialog.backend.currentControllerMapping
                        RowLayout {
                            id: mappingRow
                            required property var modelData
                            required property int index
                            property alias primaryButton: primary
                            property alias alternateButton: alternate
                            Layout.fillWidth: true
                            spacing: 16
                            Label {
                                Layout.preferredWidth: 180
                                Layout.minimumWidth: 0
                                font.pixelSize: 14
                                text: mappingRow.modelData.buttonName
                                wrapMode: Text.Wrap
                            }
                            C.Button {
                                id: primary
                                background: Rectangle { color: "#1a202b"; radius: 8; border.color: primary.visualFocus ? "#00a7ff" : "#303a48"; border.width: primary.visualFocus ? 2 : 1 }
                                objectName: "controllerMap" + mappingRow.modelData.buttonValue + "Slot0"
                                Accessible.name: qsTr("%1 primary input").arg(mappingRow.modelData.buttonName)
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                font.pixelSize: 14
                                text: mappingRow.modelData.physicalButton.length ? mappingRow.modelData.physicalButton[0] : qsTr("Not mapped")
                                Keys.onPressed: event => dialog.navigateMapping(mappingRow.index, 0, event)
                                onClicked: buttonDialog.show(mappingRow.modelData.buttonValue, 0)
                            }
                            C.Button {
                                id: alternate
                                background: Rectangle { color: "#1a202b"; radius: 8; border.color: alternate.visualFocus ? "#00a7ff" : "#303a48"; border.width: alternate.visualFocus ? 2 : 1 }
                                objectName: "controllerMap" + mappingRow.modelData.buttonValue + "Slot1"
                                Accessible.name: qsTr("%1 alternate input").arg(mappingRow.modelData.buttonName)
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                font.pixelSize: 14
                                text: mappingRow.modelData.physicalButton.length > 1 ? mappingRow.modelData.physicalButton[1] : qsTr("Not mapped")
                                Keys.onPressed: event => dialog.navigateMapping(mappingRow.index, 1, event)
                                onClicked: buttonDialog.show(mappingRow.modelData.buttonValue, 1)
                            }
                        }
                    }
                }
            }
            RowLayout {
                Layout.alignment: Qt.AlignRight
                C.Button {
                    id: cancelMapping
                    objectName: "controllerMappingCancel"
                    text: qsTr("Cancel")
                    font.pixelSize: 14
                    onClicked: dialog.close()
                }
                C.Button {
                    id: applyMapping
                    objectName: "controllerMappingApply"
                    text: qsTr("Update controller mapping")
                    font.pixelSize: 14
                    enabled: dialog.buttonEnabled
                    lastInFocusChain: true
                    onClicked: dialog.accepted()
                }
            }
        }
        Dialog {
            id: buttonDialog
            objectName: "controllerButtonCapture"
            property int buttonValue
            property int buttonIndex
            property bool acceptingInput: false
            parent: Overlay.overlay
            width: 600
            x: Math.round((dialog.navigation.width - width) / 2)
            y: Math.round((dialog.navigation.height - height) / 2)
            title: qsTr("Button Capture")
            modal: true
            standardButtons: Dialog.Close
            closePolicy: Popup.CloseOnPressOutside
            onOpened: {
                acceptingInput = true;
                dialog.backend.controllerMappingSelectButton();
                buttonLabel.forceActiveFocus(Qt.TabFocusReason);
            }
            onAboutToHide: acceptingInput = false
            onClosed: {
                dialog.backend.controllerMappingButtonQuit();
                if (!dialog.finalClosing) dialog.focusMapping(buttonValue, buttonIndex);
            }
            function show(value, slot) {
                if (!dialog.backend.controllerMappingInProgress || dialog.finalClosing) return;
                buttonValue = value;
                buttonIndex = slot;
                open();
            }
            contentItem: Label {
                id: buttonLabel
                wrapMode: Text.Wrap
                text: qsTr("Press a button on %1 to map it, or close to cancel.").arg(dialog.backend.currentControllerType)
                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Escape) buttonDialog.close();
                    event.accepted = true;
                }
                Keys.onReleased: event => event.accepted = true
            }
        }
        Connections {
            target: dialog.Window.window
            function onActiveFocusItemChanged() { dialog.ensureFocusVisible(); }
        }
        Connections {
            target: dialog.backend.settings
            function onCurrentProfileChanged() { dialog.close(); }
        }
        Connections {
            target: dialog.backend
            function onControllerMappingInProgressChanged() {
                if (!dialog.backend.controllerMappingInProgress && !dialog.finalClosing) dialog.close();
            }
            function onControllerMappingButtonSelected(pressedButton) {
                if (!buttonDialog.acceptingInput || !dialog.backend.controllerMappingInProgress || dialog.finalClosing) return;
                if (!dialog.backend.currentControllerMapping.some(row => row.buttonValue === buttonDialog.buttonValue)) return;
                buttonDialog.acceptingInput = false;
                dialog.backend.updateButton(buttonDialog.buttonValue, pressedButton, buttonDialog.buttonIndex);
                buttonDialog.close();
            }
        }
    }
}
