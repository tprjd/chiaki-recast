import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import org.streetpea.chiaking
import "controls" as C

FocusScope {
    id: dialog
    property var backend: Chiaki
    property var navigation: root
    property var openAccount: function(login, callback) {
        stack.push(psnLoginDialogComponent, {login: login, callback: callback});
    }
    property bool ps5: true
    property alias host: hostField.text
    property Item restoreFocusItem
    property bool returningFromAccount: false
    readonly property bool legacy: consoleChoice.currentValue === 800
    readonly property bool streamerMode: backend.settings.streamerMode
    readonly property bool valid: hostField.text.trim().length > 0
        && (legacy ? onlineId.text.trim().length > 0 : accountId.text.trim().length > 0)
        && /^[0-9]{8}$/.test(pin.text) && (/^$|^[0-9]{4}$/.test(cpin.text))
    property bool submitting: false
    property string submissionError: ""

    Component.onCompleted: {
        consoleChoice.currentIndex = ps5 ? 0 : 3;
        accountId.text = backend.settings.psnAccountId || "";
    }
    StackView.onDeactivating: restoreFocusItem = Window.window.activeFocusItem
    StackView.onActivated: {
        const item = returningFromAccount ? accountId : restoreFocusItem && restoreFocusItem.visible && restoreFocusItem.enabled
            ? restoreFocusItem : hostField;
        item.forceActiveFocus(Qt.TabFocusReason);
        restoreFocusItem = null;
        returningFromAccount = false;
    }
    function close() { navigation.closeDialog(); }
    function accountRoute(login) {
        returningFromAccount = true;
        restoreFocusItem = accountId;
        openAccount(login, function(id) {
            if (id) accountId.text = id;
            restoreFocusItem = accountId;
            accountId.forceActiveFocus(Qt.TabFocusReason);
        });
    }
    function submit() {
        if (!valid || submitting) return;
        submissionError = "";
        submitting = true;
        logArea.text = "";
        logDialog.standardButtons = Dialog.Cancel;
        const address = hostField.text.trim();
        const accepted = backend.registerHost(address,
            (legacy ? onlineId.text : accountId.text).trim(), pin.text, cpin.text,
            address === "255.255.255.255", consoleChoice.currentValue,
            function(msg, ok, done) {
                if (!done) logArea.text += msg + "\n";
                else logDialog.standardButtons = Dialog.Close;
            });
        if (accepted) logDialog.open();
        else {
            submitting = false;
            if (!submissionError) submissionError = qsTr("The registration request did not start. Check your details and try again.");
            accountId.forceActiveFocus(Qt.TabFocusReason);
        }
    }
    function ensureFocusVisible() {
        const item = Window.window ? Window.window.activeFocusItem : null;
        if (!item) return;
        let ancestor = item;
        while (ancestor && ancestor !== workspace) ancestor = ancestor.parent;
        if (!ancestor) return;
        const point = item.mapToItem(workspace, 0, 0);
        if (point.y < scroll.contentY + 12) scroll.contentY = Math.max(0, point.y - 12);
        else if (point.y + item.height > scroll.contentY + scroll.height - 12)
            scroll.contentY = Math.min(Math.max(0, scroll.contentHeight - scroll.height), point.y + item.height - scroll.height + 12);
    }
    Connections {
        target: dialog.Window.window
        function onActiveFocusItemChanged() { Qt.callLater(dialog.ensureFocusVisible); }
    }
    Connections {
        target: dialog.backend
        function onError(title, text) {
            if (dialog.submitting && !logDialog.visible) dialog.submissionError = title + ": " + text;
        }
    }
    Keys.onEscapePressed: close()
    Keys.onMenuPressed: submit()

    Rectangle { anchors.fill: parent; color: "#11151c" }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 14
        ConsoleButton {
            objectName: "registrationBack"
            text: qsTr("‹ Back")
            Accessible.name: qsTr("Back from console registration")
            flat: true
            onClicked: dialog.close()
        }
        Label { text: qsTr("Register console"); font.pixelSize: 30; font.bold: true }
        Label {
            Layout.fillWidth: true
            text: qsTr("Pair your console with your PlayStation Network account.")
            color: "#9ca8b9"
            font.pixelSize: 14
            wrapMode: Text.Wrap
        }
        Flickable {
            id: scroll
            objectName: "registrationScroll"
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: width
            contentHeight: workspace.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.VerticalFlick
            ScrollBar.vertical: ScrollBar {}
            RowLayout {
                id: workspace
                width: scroll.width
                spacing: 24
                Pane {
                    id: formPanel
                    objectName: "registrationFormPanel"
                    Layout.fillWidth: true
                    Layout.preferredWidth: workspace.width * 0.63
                    Layout.alignment: Qt.AlignTop
                    padding: 24
                    font.pixelSize: 14
                    background: Rectangle { color: "#171b23"; radius: 12; border.color: "#2b323e" }
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 9
                        Label { Layout.fillWidth: true; wrapMode: Text.Wrap; text: qsTr("CONSOLE DETAILS"); color: "#9ca8b9"; font.pixelSize: 11; font.letterSpacing: 1.5 }
                        Label { Layout.fillWidth: true; wrapMode: Text.Wrap; text: qsTr("Set up Remote Play"); font.pixelSize: 22; font.bold: true; Layout.bottomMargin: 12 }
                        Label { text: qsTr("Console"); Layout.fillWidth: true; wrapMode: Text.Wrap }
                        C.ComboBox {
                            id: consoleChoice
                            objectName: "registrationConsole"
                            Layout.fillWidth: true
                            textRole: "label"
                            valueRole: "target"
                            onActivated: index => currentIndex = index
                            Accessible.name: qsTr("Console and firmware")
                            model: [
                                {label: qsTr("PS5"), target: 1000100},
                                {label: qsTr("PS4 firmware before 7.0"), target: 800},
                                {label: qsTr("PS4 firmware 7.0 to before 8.0"), target: 900},
                                {label: qsTr("PS4 firmware 8.0 or later"), target: 1000}
                            ]
                        }
                        Label { text: qsTr("Console address"); Layout.fillWidth: true; wrapMode: Text.Wrap }
                        C.TextField {
                            id: hostField
                            objectName: "registrationHost"
                            Layout.fillWidth: true
                            Accessible.name: qsTr("Console address")
                            echoMode: dialog.streamerMode ? TextInput.Password : TextInput.Normal
                            placeholderText: qsTr("IP address or hostname")
                        }
                        Label {
                            objectName: "registrationHostHelp"
                            Layout.fillWidth: true
                            text: hostField.text.length && !hostField.text.trim().length
                                ? qsTr("Enter a console address.") : qsTr("Use an IP address, hostname, or 255.255.255.255 for local broadcast.")
                            wrapMode: Text.Wrap
                            color: hostField.text.length && !hostField.text.trim().length ? "#ffb0af" : "#9ca8b9"
                            font.pixelSize: 12
                        }
                        Rectangle { Layout.fillWidth: true; height: 1; color: "#2b323e"; Layout.topMargin: 12; Layout.bottomMargin: 12 }
                        Label { Layout.fillWidth: true; wrapMode: Text.Wrap; text: qsTr("YOUR ACCOUNT"); color: "#9ca8b9"; font.pixelSize: 11; font.letterSpacing: 1.5 }
                        Label { text: dialog.legacy ? qsTr("PSN Online ID") : qsTr("PSN Account ID"); Layout.fillWidth: true; wrapMode: Text.Wrap }
                        C.TextField {
                            id: onlineId
                            objectName: "registrationOnlineId"
                            visible: dialog.legacy
                            Layout.fillWidth: true
                            Accessible.name: qsTr("PSN Online ID")
                            echoMode: dialog.streamerMode ? TextInput.Password : TextInput.Normal
                        }
                        C.TextField {
                            id: accountId
                            objectName: "registrationAccountId"
                            visible: !dialog.legacy
                            Layout.fillWidth: true
                            Accessible.name: qsTr("PSN Account ID in Base64")
                            echoMode: dialog.streamerMode ? TextInput.Password : TextInput.Normal
                            placeholderText: qsTr("Base64 Account ID")
                        }
                        Label {
                            objectName: "registrationAccountIdHelp"
                            Layout.fillWidth: true
                            text: dialog.legacy ? (onlineId.text.length && !onlineId.text.trim().length
                                ? qsTr("Enter your PSN Online ID.") : qsTr("Use the PSN username for your console user. The Online ID is case-sensitive."))
                                : accountId.text.length && !accountId.text.trim().length ? qsTr("Enter your PSN Account ID.")
                                : qsTr("Use your Account ID encoded as Base64. This is different from your PSN username.")
                            color: "#9ca8b9"
                            font.pixelSize: 12
                            wrapMode: Text.Wrap
                        }
                        Flow {
                            Layout.fillWidth: true
                            visible: !dialog.legacy && !dialog.backend.settings.psnAccountId
                            spacing: 8
                            ConsoleButton {
                                objectName: "registrationLogin"
                                text: qsTr("PSN sign-in")
                                Accessible.name: text
                                flat: true
                                onClicked: dialog.accountRoute(true)
                            }
                            ConsoleButton {
                                objectName: "registrationLookup"
                                text: qsTr("Public lookup")
                                Accessible.name: text
                                flat: true
                                onClicked: dialog.accountRoute(false)
                            }
                        }
                        Rectangle { Layout.fillWidth: true; height: 1; color: "#2b323e"; Layout.topMargin: 12; Layout.bottomMargin: 12 }
                        Label { Layout.fillWidth: true; wrapMode: Text.Wrap; text: qsTr("PAIRING"); color: "#9ca8b9"; font.pixelSize: 11; font.letterSpacing: 1.5 }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 18
                            ColumnLayout {
                                Layout.fillWidth: true
                                Layout.preferredWidth: 1
                                Layout.alignment: Qt.AlignTop
                                spacing: 9
                                Label { text: qsTr("Remote Play PIN"); Layout.fillWidth: true; wrapMode: Text.Wrap }
                                C.TextField {
                                    id: pin
                                    objectName: "registrationPin"
                                    Layout.fillWidth: true
                                    Accessible.name: qsTr("Eight-digit Remote Play PIN")
                                    echoMode: dialog.streamerMode ? TextInput.Password : TextInput.Normal
                                    validator: RegularExpressionValidator { regularExpression: /[0-9]{8}/ }
                                    inputMethodHints: Qt.ImhDigitsOnly
                                }
                                Label {
                                    objectName: "registrationPinHelp"
                                    Layout.fillWidth: true
                                    text: pin.text.length && !pin.acceptableInput ? qsTr("Enter exactly eight digits.")
                                        : qsTr("Enter the temporary eight-digit code shown in your console's Remote Play settings.")
                                    color: pin.text.length && !pin.acceptableInput ? "#ffb0af" : "#9ca8b9"
                                    font.pixelSize: 12
                                    wrapMode: Text.Wrap
                                }
                            }
                            ColumnLayout {
                                Layout.fillWidth: true
                                Layout.preferredWidth: 1
                                Layout.alignment: Qt.AlignTop
                                spacing: 9
                                Label { text: qsTr("Console PIN, optional"); Layout.fillWidth: true; wrapMode: Text.Wrap }
                                C.TextField {
                                    id: cpin
                                    objectName: "registrationConsolePin"
                                    Layout.fillWidth: true
                                    Accessible.name: qsTr("Optional four-digit console login PIN")
                                    echoMode: dialog.streamerMode ? TextInput.Password : TextInput.Normal
                                    validator: RegularExpressionValidator { regularExpression: /^$|[0-9]{4}/ }
                                    inputMethodHints: Qt.ImhDigitsOnly
                                }
                                Label {
                                    objectName: "registrationConsolePinHelp"
                                    Layout.fillWidth: true
                                    text: cpin.text.length && !cpin.acceptableInput ? qsTr("Enter exactly four digits, or leave this empty.")
                                        : qsTr("This four-digit PIN signs in to your console user. Leave it empty if your user has no login PIN.")
                                    color: cpin.text.length && !cpin.acceptableInput ? "#ffb0af" : "#9ca8b9"
                                    font.pixelSize: 12
                                    wrapMode: Text.Wrap
                                }
                            }
                        }
                        Label {
                            objectName: "registrationError"
                            Layout.fillWidth: true
                            visible: text.length > 0
                            text: dialog.submissionError
                            color: "#ffb0af"
                            wrapMode: Text.Wrap
                            Accessible.role: Accessible.AlertMessage
                        }
                        Rectangle { Layout.fillWidth: true; height: 1; color: "#2b323e"; Layout.topMargin: 12 }
                        RowLayout {
                            Layout.fillWidth: true
                            ConsoleButton {
                                objectName: "registrationCancel"
                                text: qsTr("Cancel")
                                Accessible.name: text
                                onClicked: dialog.close()
                            }
                            Item { Layout.fillWidth: true }
                            ConsoleButton {
                                objectName: "registrationSubmit"
                                text: qsTr("Register console")
                                Accessible.name: text
                                enabled: dialog.valid && !dialog.submitting
                                primary: true
                                onClicked: dialog.submit()
                            }
                        }
                    }
                }
                ColumnLayout {
                    objectName: "registrationInstructions"
                    Layout.fillWidth: true
                    Layout.preferredWidth: workspace.width * 0.37
                    Layout.alignment: Qt.AlignTop
                    Layout.topMargin: 24
                    Layout.rightMargin: 12
                    spacing: 18
                    Label { Layout.fillWidth: true; wrapMode: Text.Wrap; text: qsTr("ON YOUR CONSOLE"); color: "#9ca8b9"; font.pixelSize: 11; font.letterSpacing: 1.5 }
                    Label { Layout.fillWidth: true; text: qsTr("Get your pairing code"); font.pixelSize: 24; font.bold: true; wrapMode: Text.Wrap }
                    Label { Layout.fillWidth: true; text: qsTr("Use the same PlayStation Network account as the user signed in on your console."); wrapMode: Text.Wrap; color: "#9ca8b9"; font.pixelSize: 12 }
                    Label {
                        objectName: "registrationConsoleHelp"
                        Layout.fillWidth: true
                        text: consoleChoice.currentValue === 1000100
                            ? qsTr("1. Open Settings, then System, then Remote Play.\n\n2. Enable Remote Play. Keep your console on and connected to your network.\n\n3. Select Link Device. Enter the eight-digit code before it expires.")
                            : qsTr("1. Open Settings, then Remote Play Connection Settings.\n\n2. Enable Remote Play. Keep your console on and connected to your network.\n\n3. Select Add Device. Enter the eight-digit code before it expires.")
                        wrapMode: Text.Wrap
                        color: "#9ca8b9"
                        font.pixelSize: 12
                    }
                    Rectangle { Layout.fillWidth: true; height: 1; color: "#2b323e"; Layout.topMargin: 8 }
                    Label { Layout.fillWidth: true; text: qsTr("Two PINs, two purposes"); font.pixelSize: 14; font.bold: true; wrapMode: Text.Wrap }
                    Label { Layout.fillWidth: true; text: qsTr("The Remote Play PIN pairs the console. The optional console PIN signs in to your console user."); wrapMode: Text.Wrap; color: "#9ca8b9"; font.pixelSize: 12 }
                }
            }
        }
    }
    Dialog {
        id: logDialog
        objectName: "registrationLog"
        parent: Overlay.overlay
        x: Math.round((dialog.width - width) / 2)
        y: Math.round((dialog.height - height) / 2)
        width: Math.min(640, dialog.width - 48)
        height: Math.min(500, dialog.height - 48)
        title: qsTr("Register Console")
        modal: true
        closePolicy: Popup.NoAutoClose
        standardButtons: Dialog.Cancel
        Material.roundedScale: Material.MediumScale
        onOpened: logArea.forceActiveFocus(Qt.TabFocusReason)
        onClosed: dialog.navigation.closeDialog()
        Flickable {
            id: logFlick
            anchors.fill: parent
            contentWidth: width
            contentHeight: logArea.contentHeight
            clip: true
            ScrollBar.vertical: ScrollBar { id: logScrollbar }
            Label {
                id: logArea
                objectName: "registrationLogText"
                width: logFlick.width
                wrapMode: Text.Wrap
                Keys.onReturnPressed: if (logDialog.standardButtons === Dialog.Close) logDialog.close()
                Keys.onEscapePressed: logDialog.close()
                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Up) { logFlick.flick(0, 500); event.accepted = true; }
                    else if (event.key === Qt.Key_Down) { logFlick.flick(0, -500); event.accepted = true; }
                }
            }
        }
    }
    Component { id: psnLoginDialogComponent; PSNLoginDialog {} }
}
