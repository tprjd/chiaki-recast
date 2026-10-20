import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import org.streetpea.chiaking
import "controls" as C

FocusScope {
    id: dialog
    objectName: "manualHostDialog"
    property var backend: Chiaki
    property var navigation: root
    property string phase: "form"
    property bool dispatching: false
    property string savedAddress: ""
    property string selectionKind: ""
    property string selectedMac: ""
    property var savedConsole: null
    property bool savedDeferred: false
    property string failureText: ""
    property string profileKey: backend.settings.currentProfile
    readonly property bool privateMode: backend.settings.streamerMode
    readonly property var registeredHosts: backend.settings.registeredHosts
    onRegisteredHostsChanged: {
        if (phase !== "saved" && selectionKind === "registered" && registrationIndex() < 0)
            invalidateSelection();
    }
    onProfileKeyChanged: {
        if (phase !== "saved" && selectionKind) invalidateSelection();
    }
    readonly property var consoleChoices: {
        const choices = [{name: qsTr("Select a console"), kind: ""}];
        for (const host of registeredHosts)
            choices.push({name: registrationLabel(host), kind: "registered", mac: host.mac});
        choices.push({name: qsTr("Register on first connection"), kind: "deferred"});
        return choices;
    }
    function registrationLabel(host) {
        const name = privateMode && (host.name === host.address || host.name === host.mac
            || host.name === hostField.text.trim() || host.name === savedAddress)
            ? qsTr("Console") : host.name || qsTr("Console");
        return qsTr("%1 · %2 · %3").arg(name).arg(host.ps5 ? "PS5" : "PS4")
            .arg(privateMode ? qsTr("ID hidden") : host.mac);
    }
    function registrationIndex() {
        let found = -1;
        for (let index = 0; index < registeredHosts.length; index++) {
            if (!selectedMac || registeredHosts[index].mac !== selectedMac) continue;
            if (found !== -1) return -1;
            found = index;
        }
        return found;
    }
    function invalidateSelection() {
        selectionKind = "";
        selectedMac = "";
        showFailure(qsTr("The selected registration is no longer available. Choose a console again."));
    }
    function showFailure(message) {
        failureText = message;
        phase = "failure";
        backToDetails.forceActiveFocus(Qt.TabFocusReason);
    }

    function close() { navigation.closeDialog(); }
    function save() {
        if (phase !== "form" || dispatching || !hostField.text.trim() || !selectionKind) return;
        const index = selectionKind === "deferred" ? registeredHosts.length : registrationIndex();
        if (index < 0) { invalidateSelection(); return; }
        dispatching = true;
        savedAddress = hostField.text.trim();
        savedDeferred = selectionKind === "deferred";
        savedConsole = savedDeferred ? null : registeredHosts[index];
        try {
            backend.addManualHost(index, savedAddress);
            phase = "saved";
            backToConsoles.forceActiveFocus(Qt.TabFocusReason);
        } catch (error) {
            showFailure(qsTr("The address could not be added. Check the details and try again."));
        } finally {
            dispatching = false;
        }
    }
    function revealFocus() {
        const item = Window.window ? Window.window.activeFocusItem : null;
        if (!item || !item.visible) return;
        let ancestor = item;
        while (ancestor && ancestor !== scroll.contentItem) ancestor = ancestor.parent;
        if (!ancestor) return;
        const top = item.mapToItem(scroll.contentItem, 0, 0).y;
        if (top < scroll.contentY) scroll.contentY = top;
        else if (top + item.height > scroll.contentY + scroll.height)
            scroll.contentY = top + item.height - scroll.height;
    }
    Connections {
        target: dialog.Window.window
        function onActiveFocusItemChanged() { Qt.callLater(dialog.revealFocus); }
    }
    Keys.onEscapePressed: close()
    Keys.onMenuPressed: if (addButton.enabled && phase === "form") save()
    StackView.onActivated: hostField.forceActiveFocus(Qt.TabFocusReason)
    Component.onCompleted: {
        if (!registeredHosts.length) selectionKind = "deferred";
        hostField.forceActiveFocus(Qt.TabFocusReason);
    }

    Rectangle { anchors.fill: parent; color: "#11151c" }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16
        RowLayout {
            Layout.fillWidth: true
            spacing: 20
            ConsoleButton {
                objectName: "manualBack"
                text: qsTr("Consoles")
                Accessible.name: qsTr("Back to consoles")
                onClicked: dialog.close()
            }
            Label { text: qsTr("Add console"); font.pixelSize: 30; font.bold: true; Layout.fillWidth: true }
        }
        Label { font.pixelSize: 14;
            Layout.fillWidth: true
            text: qsTr("Save a console address and choose when to register it.")
            color: "#9ca8b9"
            wrapMode: Text.Wrap
        }
        Flickable {
            id: scroll
            objectName: "manualScroll"
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: width
            contentHeight: workspace.height
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}
            RowLayout {
                id: workspace
                width: scroll.width
                spacing: 24
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 620
                    Layout.alignment: Qt.AlignTop
                    implicitHeight: panel.implicitHeight + 48
                    radius: 12
                    color: "#171b23"
                    border.color: "#2b323e"
                    ColumnLayout {
                        id: panel
                        anchors { top: parent.top; left: parent.left; right: parent.right; margins: 24 }
                        spacing: 12
                        ColumnLayout {
                            visible: dialog.phase === "form"
                            Layout.fillWidth: true
                            spacing: 10
                            Label { text: qsTr("MANUAL ADDRESS"); font.pixelSize: 11; color: "#9ca8b9" }
                            Label { text: qsTr("Find your console"); font.pixelSize: 22; font.bold: true; Layout.fillWidth: true; wrapMode: Text.Wrap }
                            Label { font.pixelSize: 14; text: qsTr("Console address"); topPadding: 12; Layout.fillWidth: true; wrapMode: Text.Wrap }
                            C.TextField {
                                id: hostField
                                objectName: "manualAddress"
                                Layout.fillWidth: true
                                Accessible.name: qsTr("Console address")
                                font.pixelSize: 14
                                topPadding: 12
                                bottomPadding: 12
                                leftPadding: 12
                                rightPadding: 12
                                background: Rectangle {
                                    implicitHeight: 44
                                    color: "#11151c"
                                    radius: 7
                                    border.color: hostField.activeFocus ? "#63b7ff" : "#3a4352"
                                    border.width: hostField.activeFocus ? 2 : 1
                                }
                                echoMode: dialog.privateMode ? TextInput.Password : TextInput.Normal
                            }
                            Label {
                                Layout.fillWidth: true
                                text: qsTr("An IP address or hostname, such as 192.0.2.42 or ps5.local.")
                                color: "#9ca8b9"; font.pixelSize: 12; wrapMode: Text.Wrap
                            }
                            Label { font.pixelSize: 14; text: qsTr("Registered console"); topPadding: 12; Layout.fillWidth: true; wrapMode: Text.Wrap }
                            C.ComboBox {
                                id: consoleCombo
                                objectName: "manualRegistration"
                                Layout.fillWidth: true
                                font.pixelSize: 14
                                background: Rectangle {
                                    implicitHeight: 44
                                    color: "#11151c"
                                    radius: 7
                                    border.color: consoleCombo.activeFocus ? "#63b7ff" : "#3a4352"
                                    border.width: consoleCombo.activeFocus ? 2 : 1
                                }
                                model: dialog.consoleChoices
                                textRole: "name"
                                contentItem: Text {
                                    text: consoleCombo.displayText
                                    font: consoleCombo.font
                                    color: "#f2f4f8"
                                    textFormat: Text.PlainText
                                    wrapMode: Text.WrapAnywhere
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 12
                                }
                                delegate: ItemDelegate {
                                    required property var modelData
                                    required property int index
                                    width: consoleCombo.width
                                    highlighted: consoleCombo.highlightedIndex === index
                                    contentItem: Text {
                                        text: modelData.name
                                        font: consoleCombo.font
                                        color: "#f2f4f8"
                                        textFormat: Text.PlainText
                                        wrapMode: Text.WrapAnywhere
                                    }
                                }
                                currentIndex: dialog.selectionKind === "deferred" ? count - 1
                                    : dialog.selectionKind === "registered" && dialog.registrationIndex() >= 0
                                        ? dialog.registrationIndex() + 1 : 0
                                onActivated: index => {
                                    const choice = model[index];
                                    dialog.selectionKind = choice.kind;
                                    dialog.selectedMac = choice.mac || "";
                                }
                                Accessible.name: qsTr("Registered console")
                            }
                            Label {
                                Layout.fillWidth: true
                                text: qsTr("Use an existing registration, or register when you first connect.")
                                color: "#9ca8b9"; font.pixelSize: 12; wrapMode: Text.Wrap
                            }
                            Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: "#2b323e"; Layout.topMargin: 12 }
                            Label {
                                Layout.fillWidth: true
                                text: qsTr("Adding an address does not start a connection.")
                                color: "#9ca8b9"; font.pixelSize: 12; wrapMode: Text.Wrap
                            }
                            RowLayout {
                                Layout.alignment: Qt.AlignRight
                                ConsoleButton { objectName: "manualCancel"; text: qsTr("Cancel"); onClicked: dialog.close() }
                                ConsoleButton {
                                    id: addButton
                                    objectName: "manualAdd"
                                    text: qsTr("Add console")
                                    enabled: !!hostField.text.trim() && !!dialog.selectionKind && !dialog.dispatching && dialog.phase === "form"
                                    primary: true
                                    onClicked: dialog.save()
                                }
                            }
                        }
                        ColumnLayout {
                            visible: dialog.phase === "saved"
                            Layout.fillWidth: true
                            spacing: 16
                            Label { text: "✓"; color: "#80dcb0"; font.pixelSize: 36 }
                            Label {
                                objectName: "manualSavedTitle"
                                text: qsTr("Manual console added")
                                font.pixelSize: 22; font.bold: true; Layout.fillWidth: true; wrapMode: Text.Wrap
                            }
                            Label { font.pixelSize: 14;
                                text: qsTr("The address is now available in your console library.")
                                Layout.fillWidth: true; wrapMode: Text.Wrap; color: "#9ca8b9"
                            }
                            Label { font.pixelSize: 14;
                                objectName: "manualSavedAddress"
                                text: dialog.privateMode ? qsTr("Address hidden") : dialog.savedAddress
                                textFormat: Text.PlainText
                                Layout.fillWidth: true; wrapMode: Text.WrapAnywhere
                            }
                            Label { font.pixelSize: 14;
                                objectName: "manualSavedRegistration"
                                text: dialog.savedDeferred ? qsTr("Registration opens on the first connection.")
                                    : qsTr("Uses the registration for %1.").arg(dialog.savedConsole ? dialog.registrationLabel(dialog.savedConsole) : "")
                                textFormat: Text.PlainText
                                Layout.fillWidth: true; wrapMode: Text.Wrap; color: "#9ca8b9"
                            }
                            ConsoleButton {
                                id: backToConsoles
                                objectName: "manualBackToConsoles"
                                text: qsTr("Back to consoles")
                                primary: true
                                Layout.alignment: Qt.AlignRight
                                onClicked: dialog.close()
                            }
                        }
                        ColumnLayout {
                            visible: dialog.phase === "failure"
                            Layout.fillWidth: true
                            spacing: 16
                            Label { text: "!"; color: "#ffb0af"; font.pixelSize: 36 }
                            Label {
                                objectName: "manualFailureTitle"
                                text: qsTr("Console was not added")
                                Layout.fillWidth: true; wrapMode: Text.Wrap; font.pixelSize: 22; font.bold: true
                            }
                            Label { font.pixelSize: 14;
                                objectName: "manualFailureMessage"
                                text: dialog.failureText
                                Layout.fillWidth: true; wrapMode: Text.Wrap; color: "#9ca8b9"
                            }
                            ConsoleButton {
                                id: backToDetails
                                objectName: "manualBackToDetails"
                                text: qsTr("Back to details")
                                Layout.alignment: Qt.AlignRight
                                onClicked: {
                                    dialog.phase = "form";
                                    consoleCombo.forceActiveFocus(Qt.TabFocusReason);
                                }
                            }
                        }
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 340
                    Layout.maximumWidth: 360
                    Layout.alignment: Qt.AlignTop
                    spacing: 16
                    Label { text: qsTr("CONNECT BY ADDRESS"); color: "#9ca8b9"; font.pixelSize: 11; topPadding: 24 }
                    Label { text: qsTr("Same console.\nAnother way to find it."); font.pixelSize: 22; font.bold: true; Layout.fillWidth: true; wrapMode: Text.Wrap }
                    Label { font.pixelSize: 14; text: qsTr("Use a manual address when automatic discovery does not show your console."); color: "#9ca8b9"; Layout.fillWidth: true; wrapMode: Text.Wrap }
                    Label { font.pixelSize: 14; text: qsTr("1. Enter its address"); font.bold: true; Layout.fillWidth: true; wrapMode: Text.Wrap }
                    Label { font.pixelSize: 14; text: qsTr("Find the console's IP address in its network settings."); color: "#9ca8b9"; Layout.fillWidth: true; wrapMode: Text.Wrap }
                    Label { font.pixelSize: 14; text: qsTr("2. Choose its registration"); font.bold: true; Layout.fillWidth: true; wrapMode: Text.Wrap }
                    Label { font.pixelSize: 14; text: qsTr("Select the console that belongs to this address."); color: "#9ca8b9"; Layout.fillWidth: true; wrapMode: Text.Wrap }
                    Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: "#2b323e" }
                    Label { font.pixelSize: 14; text: qsTr("Not registered yet?"); font.bold: true; Layout.fillWidth: true; wrapMode: Text.Wrap }
                    Label { font.pixelSize: 14; text: qsTr("Choose Register on first connection. You can pair it when you connect from the library."); color: "#9ca8b9"; Layout.fillWidth: true; wrapMode: Text.Wrap }
                }
            }
        }
    }
}
