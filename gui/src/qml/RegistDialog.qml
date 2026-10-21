import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import org.streetpea.chiaking
import "controls" as C

FocusScope {
    id: dialog
    objectName: "registrationDialog"
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
    property var registrationContext: backend.createRegistrationContext()
    property string phase: "form"
    property string diagnostics: ""
    property string resultMessage: ""
    property bool logExpanded: false
    property bool profileInvalid: false
    property var callbackScope: ({owner: dialog, attempt: 0, terminal: false})

    component SetupField: C.TextField {
        id: input
        font.pixelSize: 14
        topPadding: 12; bottomPadding: 12; leftPadding: 12; rightPadding: 12
        background: Rectangle {
            implicitHeight: 44
            color: "#11151c"; radius: 7
            border.color: input.activeFocus ? "#63b7ff" : "#3a4352"
            border.width: input.activeFocus ? 2 : 1
        }
    }
    component RegistrationButton: SetupButton {
        Layout.maximumWidth: parent.width
        textFormat: Text.PlainText
    }
    Component.onDestruction: callbackScope.owner = null

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
    function close() {
        callbackScope.owner = null;
        navigation.closeDialog();
    }
    function backToDetails() {
        phase = "form";
        hostField.forceActiveFocus(Qt.TabFocusReason);
    }
    function accountRoute(login) {
        returningFromAccount = true;
        restoreFocusItem = accountId;
        const scope = callbackScope;
        openAccount(login, function(id) {
            if (!scope.owner || scope.owner.profileInvalid) return;
            scope.owner.applyAccountId(id);
        });
    }
    function applyAccountId(id) {
        if (id) accountId.text = id;
        restoreFocusItem = accountId;
        accountId.forceActiveFocus(Qt.TabFocusReason);
    }
    function registrationEvent(msg, ok, done) {
        if (!done) diagnostics += msg + "\n";
        else {
            submitting = false;
            resultMessage = msg;
            phase = ok ? "success" : "failure";
            const scope = callbackScope;
            const attempt = scope.attempt;
            Qt.callLater(function() { if (scope.owner && scope.attempt === attempt) scope.owner.focusResult(ok); });
        }
    }
    function focusResult(ok) {
        if (phase === (ok ? "success" : "failure"))
            (ok ? doneButton : detailsButton).forceActiveFocus(Qt.TabFocusReason);
    }
    function submit() {
        if (!valid || submitting || phase !== "form" || profileInvalid) return;
        submissionError = "";
        submitting = true;
        diagnostics = "";
        resultMessage = "";
        logExpanded = false;
        const scope = callbackScope;
        const attempt = ++scope.attempt;
        scope.terminal = false;
        const address = hostField.text.trim();
        const accepted = backend.registerHost(address,
            (legacy ? onlineId.text : accountId.text).trim(), pin.text, cpin.text,
            address === "255.255.255.255", consoleChoice.currentValue,
            function(msg, ok, done) {
                if (!scope.owner || scope.attempt !== attempt || scope.terminal) return;
                if (done) scope.terminal = true;
                scope.owner.registrationEvent(msg, ok, done);
            }, registrationContext);
        if (accepted) {
            if (!scope.terminal) {
                phase = "progress";
                closeProgress.forceActiveFocus(Qt.TabFocusReason);
            }
        }
        else {
            scope.terminal = true;
            submitting = false;
            if (!submissionError) submissionError = qsTr("The registration request did not start. Check your details and try again.");
            (legacy ? onlineId : accountId).forceActiveFocus(Qt.TabFocusReason);
        }
    }
    function ensureFocusVisible() {
        const item = logArea.activeFocus ? logView : Window.window ? Window.window.activeFocusItem : null;
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
            if (dialog.submitting && dialog.phase === "form") dialog.submissionError = title + ": " + text;
        }
    }
    Connections {
        target: dialog.backend.settings
        function onCurrentProfileChanged() {
            dialog.profileInvalid = true;
            dialog.callbackScope.attempt++;
            dialog.submissionError = qsTr("The profile changed. Close this screen and open registration again.");
            if (dialog.phase === "progress") {
                dialog.submitting = false;
                dialog.resultMessage = dialog.submissionError;
                dialog.phase = "failure";
                closeResult.forceActiveFocus(Qt.TabFocusReason);
            }
        }
    }
    Keys.onEscapePressed: close()
    Keys.onMenuPressed: submit()

    Rectangle { anchors.fill: parent; color: "#11151c" }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16
        RowLayout {
            Layout.fillWidth: true
            spacing: 20
            RegistrationButton {
                objectName: "registrationBack"
                text: qsTr("‹ Back")
                Accessible.name: qsTr("Back from console registration")
                flat: true
                onClicked: dialog.close()
            }
            Label { text: qsTr("Register console"); font.pixelSize: 30; font.bold: true; Layout.fillWidth: true; wrapMode: Text.Wrap }
        }
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
            onContentHeightChanged: Qt.callLater(dialog.ensureFocusVisible)
            onHeightChanged: Qt.callLater(dialog.ensureFocusVisible)
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
                        ColumnLayout {
                            visible: dialog.phase === "form"
                            Layout.fillWidth: true
                            spacing: 9
                            Label { Layout.fillWidth: true; wrapMode: Text.Wrap; text: qsTr("CONSOLE DETAILS"); color: "#9ca8b9"; font.pixelSize: 11; font.letterSpacing: 1.5 }
                            Label { Layout.fillWidth: true; wrapMode: Text.Wrap; text: qsTr("Set up Remote Play"); font.pixelSize: 22; font.bold: true; Layout.bottomMargin: 12 }
                            Label { font.pixelSize: 14; text: qsTr("Console"); Layout.fillWidth: true; wrapMode: Text.Wrap }
                            C.ComboBox {
                                id: consoleChoice
                                font.pixelSize: 14
                                implicitHeight: Math.max(44, contentItem.implicitHeight + topPadding + bottomPadding)
                                background: Rectangle {
                                    implicitHeight: 44
                                    color: "#11151c"; radius: 7
                                    border.color: consoleChoice.activeFocus ? "#63b7ff" : "#3a4352"
                                    border.width: consoleChoice.activeFocus ? 2 : 1
                                }
                                contentItem: Text {
                                    text: consoleChoice.displayText
                                    font: consoleChoice.font
                                    color: "#f2f4f8"
                                    textFormat: Text.PlainText
                                    wrapMode: Text.Wrap
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: 12
                                }
                                delegate: ItemDelegate {
                                    required property var modelData
                                    required property int index
                                    width: consoleChoice.width
                                    highlighted: consoleChoice.highlightedIndex === index
                                    contentItem: Text {
                                        text: modelData.label
                                        font: consoleChoice.font
                                        color: "#f2f4f8"
                                        textFormat: Text.PlainText
                                        wrapMode: Text.Wrap
                                    }
                                }
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
                            Label { font.pixelSize: 14; text: qsTr("Console address"); Layout.fillWidth: true; wrapMode: Text.Wrap }
                            SetupField {
                                id: hostField
                                objectName: "registrationHost"
                                Layout.fillWidth: true
                                Accessible.name: qsTr("Console address")
                                echoMode: dialog.streamerMode ? TextInput.Password : TextInput.Normal
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
                            Label { font.pixelSize: 14; text: dialog.legacy ? qsTr("PSN Online ID") : qsTr("PSN Account ID"); Layout.fillWidth: true; wrapMode: Text.Wrap }
                            SetupField {
                                id: onlineId
                                objectName: "registrationOnlineId"
                                visible: dialog.legacy
                                Layout.fillWidth: true
                                Accessible.name: qsTr("PSN Online ID")
                                echoMode: dialog.streamerMode ? TextInput.Password : TextInput.Normal
                            }
                            SetupField {
                                id: accountId
                                objectName: "registrationAccountId"
                                visible: !dialog.legacy
                                Layout.fillWidth: true
                                Accessible.name: qsTr("PSN Account ID in Base64")
                                echoMode: dialog.streamerMode ? TextInput.Password : TextInput.Normal
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
                                RegistrationButton {
                                    objectName: "registrationLogin"
                                    text: qsTr("PSN sign-in")
                                    Accessible.name: text
                                    flat: true
                                    onClicked: dialog.accountRoute(true)
                                }
                                RegistrationButton {
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
                                    Label { font.pixelSize: 14; text: qsTr("Remote Play PIN"); Layout.fillWidth: true; wrapMode: Text.Wrap }
                                    SetupField {
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
                                    Label { font.pixelSize: 14; text: qsTr("Console PIN, optional"); Layout.fillWidth: true; wrapMode: Text.Wrap }
                                    SetupField {
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
                                textFormat: Text.PlainText
                                color: "#ffb0af"
                                wrapMode: Text.Wrap
                                Accessible.role: Accessible.AlertMessage
                            }
                            Rectangle { Layout.fillWidth: true; height: 1; color: "#2b323e"; Layout.topMargin: 12 }
                            GridLayout {
                                Layout.fillWidth: false
                                Layout.alignment: Qt.AlignRight
                                Layout.preferredWidth: Math.min(formPanel.availableWidth, cancelButton.implicitWidth + submitButton.implicitWidth + columnSpacing)
                                Layout.maximumWidth: formPanel.availableWidth
                                columns: cancelButton.implicitWidth + submitButton.implicitWidth + columnSpacing > width ? 1 : 2
                                RegistrationButton {
                                    id: cancelButton
                                    objectName: "registrationCancel"
                                    text: qsTr("Cancel")
                                    Accessible.name: text
                                    onClicked: dialog.close()
                                }
                                RegistrationButton {
                                    id: submitButton
                                    objectName: "registrationSubmit"
                                    text: qsTr("Register console")
                                    Accessible.name: text
                                    enabled: dialog.valid && !dialog.submitting && !dialog.profileInvalid
                                    primary: true
                                    onClicked: dialog.submit()
                                }
                            }
                        }
                        ColumnLayout {
                            visible: dialog.phase !== "form"
                            Layout.fillWidth: true
                            spacing: 16
                            BusyIndicator {
                                objectName: "registrationProgressIndicator"
                                visible: dialog.phase === "progress"
                                running: visible
                                Layout.alignment: Qt.AlignHCenter
                                Accessible.name: qsTr("Registration in progress")
                            }
                            Label {
                                objectName: "registrationProgress"
                                visible: dialog.phase === "progress"
                                Layout.fillWidth: true
                                text: qsTr("Registering console")
                                font.pixelSize: 24; font.bold: true; wrapMode: Text.Wrap
                                Accessible.role: Accessible.AlertMessage
                            }
                            Label {
                                objectName: "registrationSuccess"
                                visible: dialog.phase === "success"
                                Layout.fillWidth: true
                                text: qsTr("Console registered")
                                font.pixelSize: 24; font.bold: true; wrapMode: Text.Wrap
                                Accessible.role: Accessible.AlertMessage
                            }
                            Label {
                                objectName: "registrationFailure"
                                visible: dialog.phase === "failure"
                                Layout.fillWidth: true
                                text: qsTr("Registration failed")
                                font.pixelSize: 24; font.bold: true; wrapMode: Text.Wrap
                                Accessible.role: Accessible.AlertMessage
                            }
                            Label {
                                Layout.fillWidth: true
                                text: dialog.phase === "progress" ? qsTr("Waiting for the console. Keep it on and connected to your network.")
                                    : dialog.phase === "success" ? qsTr("You can now select this console from your console library.")
                                    : qsTr("The console could not be registered. Check your details and try again. Open diagnostics for available information.")
                                wrapMode: Text.Wrap; color: "#9ca8b9"; font.pixelSize: 14
                            }
                            Label {
                                objectName: "registrationResultDetails"
                                Layout.fillWidth: true
                                textFormat: Text.PlainText
                                text: consoleChoice.currentText + "\n" + (dialog.streamerMode ? qsTr("Address hidden")
                                    : dialog.registrationContext.manualAddress || hostField.text.trim())
                                wrapMode: Text.WrapAnywhere; font.pixelSize: 14
                            }
                            Label {
                                objectName: "registrationResultMessage"
                                Layout.fillWidth: true
                                visible: text.length > 0
                                text: dialog.streamerMode ? "" : dialog.resultMessage
                                textFormat: Text.PlainText
                                wrapMode: Text.Wrap; color: "#ffb0af"; font.pixelSize: 14
                            }
                            Label {
                                objectName: "registrationDismissHelp"
                                Layout.fillWidth: true
                                visible: dialog.phase === "progress"
                                text: qsTr("Closing this screen does not cancel registration. The operation can continue in the background.")
                                wrapMode: Text.Wrap; color: "#9ca8b9"; font.pixelSize: 12
                            }
                            RegistrationButton {
                                objectName: "registrationDiagnostics"
                                visible: !dialog.streamerMode
                                text: dialog.logExpanded ? qsTr("Hide diagnostics") : qsTr("Show diagnostics")
                                Accessible.name: text
                                onClicked: dialog.logExpanded = !dialog.logExpanded
                            }
                            Label {
                                Layout.fillWidth: true
                                visible: dialog.streamerMode
                                text: qsTr("Diagnostics are hidden in streamer mode.")
                                wrapMode: Text.Wrap; color: "#9ca8b9"; font.pixelSize: 12
                            }
                            ScrollView {
                                id: logView
                                objectName: "registrationLog"
                                visible: dialog.logExpanded && !dialog.streamerMode
                                Layout.fillWidth: true
                                Layout.preferredHeight: 180
                                clip: true
                                contentWidth: availableWidth
                                TextArea {
                                    id: logArea
                                    objectName: "registrationLogText"
                                    text: dialog.streamerMode ? "" : dialog.diagnostics
                                    textFormat: TextEdit.PlainText
                                    readOnly: true
                                    selectByMouse: true
                                    wrapMode: TextEdit.Wrap
                                    font.pixelSize: 12
                                    Accessible.name: qsTr("Registration diagnostics")
                                    Keys.onPressed: event => {
                                        if (event.key === Qt.Key_Up || event.key === Qt.Key_Down) {
                                            logView.contentItem.contentY = Math.max(0, Math.min(logView.contentItem.contentHeight - logView.height,
                                                logView.contentItem.contentY + (event.key === Qt.Key_Up ? -40 : 40)));
                                            event.accepted = true;
                                        }
                                    }
                                }
                            }
                            Flow {
                                Layout.fillWidth: true
                                spacing: 8
                                RegistrationButton {
                                    id: closeProgress
                                    objectName: "registrationCloseProgress"
                                    width: Math.min(implicitWidth, parent.width)
                                    visible: dialog.phase === "progress"
                                    text: qsTr("Close progress")
                                    Accessible.name: text
                                    onClicked: dialog.close()
                                }
                                RegistrationButton {
                                    id: detailsButton
                                    objectName: "registrationDetails"
                                    width: Math.min(implicitWidth, parent.width)
                                    visible: dialog.phase === "failure" && !dialog.profileInvalid
                                    text: qsTr("Back to details")
                                    Accessible.name: text
                                    primary: true
                                    onClicked: dialog.backToDetails()
                                }
                                RegistrationButton {
                                    id: closeResult
                                    objectName: "registrationCloseResult"
                                    width: Math.min(implicitWidth, parent.width)
                                    visible: dialog.phase === "failure"
                                    text: qsTr("Close")
                                    Accessible.name: text
                                    onClicked: dialog.close()
                                }
                                RegistrationButton {
                                    id: doneButton
                                    objectName: "registrationDone"
                                    width: Math.min(implicitWidth, parent.width)
                                    visible: dialog.phase === "success"
                                    text: qsTr("Back to consoles")
                                    Accessible.name: text
                                    primary: true
                                    onClicked: {
                                        dialog.callbackScope.owner = null;
                                        dialog.navigation.showMainView();
                                    }
                                }
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
    Component { id: psnLoginDialogComponent; PSNLoginDialog {} }
}
