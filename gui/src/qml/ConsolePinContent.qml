import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "controls" as C

FocusScope {
    id: content
    property bool privateMode: false
    property string consoleName: ""
    property string title: qsTr("Save console login PIN")
    property string heading: qsTr("Sign in automatically")
    property string message: qsTr("Save the four-digit login PIN for this console user.")
    property string submitText: qsTr("Save PIN")
    property string guidance: qsTr("This is the four-digit PIN for your console user. It is different from the eight-digit Remote Play pairing code.")
    property string guidanceDetail: qsTr("Saving the PIN does not start a connection.")
    property string guidanceTitle: qsTr("Use your console login PIN")
    property string guidanceDetailTitle: qsTr("Saved for this console")
    property string actionHelp: qsTr("Stored for this console.")
    property bool showSidebar: false
    property string phase: "form"
    property string failureText: ""
    property bool submitting: false
    property bool revealed: false
    property alias pin: pinField.text
    readonly property bool validPin: pin.length === 4 && /^[0-9]{4}$/.test(pin)
    signal submitted(string value)
    signal cancelled()
    signal done()
    signal settingsRequested()
    onPrivateModeChanged: if (privateMode) revealed = false
    onPhaseChanged: {
        clear();
        if (phase === "saved") doneButton.forceActiveFocus(Qt.TabFocusReason);
        else if (phase === "form") focusPin();
        else backButton.forceActiveFocus(Qt.TabFocusReason);
    }
    function clear() { pin = ""; revealed = false; }
    function focusPin() { pinField.readOnly = false; pinField.forceActiveFocus(Qt.TabFocusReason); }
    function submit() { if (phase === "form" && validPin && !submitting) submitted(pin); }
    function revealFocus() {
        const item = Window.window ? Window.window.activeFocusItem : null;
        if (!item) return;
        let ancestor = item;
        while (ancestor && ancestor !== scroll.contentItem) ancestor = ancestor.parent;
        if (!ancestor) return;
        const top = item.mapToItem(scroll.contentItem, 0, 0).y;
        if (top < scroll.contentY) scroll.contentY = top;
        else if (top + item.height > scroll.contentY + scroll.height)
            scroll.contentY = top + item.height - scroll.height;
    }
    Connections {
        target: content.Window.window
        function onActiveFocusItemChanged() { Qt.callLater(content.revealFocus); }
    }
    Keys.onEscapePressed: cancelled()
    Component.onCompleted: focusPin()
    Rectangle { anchors.fill: parent; color: "#11151c" }
    Rectangle {
        id: sidebar
        visible: content.showSidebar
        width: visible ? (content.width < 1000 ? 145 : 170) : 0
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
        color: "#13161c"
        Rectangle { anchors { right: parent.right; top: parent.top; bottom: parent.bottom } width: 1; color: "#2b323e" }
        ColumnLayout {
            anchors { fill: parent; margins: 17; topMargin: 27 }
            spacing: 12
            RowLayout {
                Layout.fillWidth: true; Layout.bottomMargin: 28
                spacing: 8
                Rectangle {
                    width: 29; height: 31; radius: 8; color: "#63b7ff"
                    Label { anchors.centerIn: parent; text: "χ"; font.pixelSize: 24; font.bold: true; color: "#102030"; Accessible.ignored: true }
                }
                Label { text: "chiaki-ng"; font.pixelSize: content.width < 1000 ? 17 : 20; font.bold: true }
            }
            ConsoleButton {
                objectName: "pinConsoles"
                Layout.fillWidth: true
                text: qsTr("Consoles"); icon.source: "qrc:/icons/discover-24px.svg"; highlighted: true
                onClicked: content.cancelled()
            }
            ConsoleButton {
                Layout.fillWidth: true
                text: qsTr("Settings"); icon.source: "qrc:/icons/options.svg"
                onClicked: content.settingsRequested()
            }
            Item { Layout.fillHeight: true }
        }
    }
    ColumnLayout {
        anchors { top: parent.top; bottom: parent.bottom; left: sidebar.right; right: parent.right; margins: content.width < 1000 ? 22 : 32 }
        spacing: 16
        Label {
            visible: content.showSidebar
            text: qsTr("Consoles / %1").arg(content.title)
            font.pixelSize: 12; color: "#9ca8b9"; Layout.fillWidth: true; wrapMode: Text.Wrap
        }
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                Layout.fillWidth: true
                Label { text: content.title; font.pixelSize: content.width < 1000 ? 27 : 32; font.bold: true; Layout.fillWidth: true; wrapMode: Text.Wrap }
                Label { visible: content.showSidebar; text: qsTr("Remote Play with your selected console."); font.pixelSize: 14; color: "#9ca8b9"; Layout.fillWidth: true; wrapMode: Text.Wrap }
            }
            Label {
                visible: content.showSidebar
                text: qsTr("Console settings")
                padding: 8; font.pixelSize: 11; color: "#9ca8b9"
                background: Rectangle { color: "transparent"; radius: 16; border.color: "#2b323e" }
            }
        }
        Flickable {
            id: scroll
            objectName: "pinScroll"
            Layout.fillWidth: true; Layout.fillHeight: true
            contentWidth: width; contentHeight: workspace.height
            clip: true; boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}
            onHeightChanged: Qt.callLater(content.revealFocus)
            onContentHeightChanged: Qt.callLater(content.revealFocus)
            GridLayout {
                id: workspace
                width: scroll.width
                columns: width < 560 ? 1 : 2
                columnSpacing: 24; rowSpacing: 20
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredWidth: 600; Layout.alignment: Qt.AlignTop
                    implicitHeight: panel.implicitHeight + 48
                    color: "#171b23"; radius: 12; border.color: "#2b323e"
                    ColumnLayout {
                        id: panel
                        anchors { top: parent.top; left: parent.left; right: parent.right; margins: 24 }
                        spacing: 16
                        RowLayout {
                            Layout.fillWidth: true; spacing: 12
                            Label { text: "▣"; font.pixelSize: 30; color: "#63b7ff"; Layout.preferredWidth: 36; Accessible.ignored: true }
                            Label {
                                objectName: "pinConsoleName"
                                text: content.privateMode ? qsTr("Console details hidden") : content.consoleName || qsTr("Console")
                                textFormat: Text.PlainText; font.pixelSize: 14; font.bold: true
                                Layout.fillWidth: true; wrapMode: Text.Wrap
                            }
                        }
                        Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: "#2b323e" }
                        RowLayout {
                            Layout.fillWidth: true; Layout.topMargin: 4; spacing: 15
                            Label {
                                text: content.phase === "saved" ? "✓" : content.phase === "failure" ? "!" : "○"
                                font.pixelSize: 26; color: content.phase === "saved" ? "#80dcb0" : content.phase === "failure" ? "#ffb0af" : "#63b7ff"
                                Layout.alignment: Qt.AlignTop; Layout.preferredWidth: 30
                                Accessible.ignored: true
                            }
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 8
                                Label {
                                    objectName: "pinHeading"
                                    text: content.heading; font.pixelSize: 22; font.bold: true
                                    Layout.fillWidth: true; wrapMode: Text.Wrap
                                }
                                Label { text: content.message; font.pixelSize: 13; color: "#aab6c7"; Layout.fillWidth: true; wrapMode: Text.Wrap }
                            }
                        }
                        ColumnLayout {
                            visible: content.phase === "form"
                            Layout.fillWidth: true; spacing: 12
                            Label { text: qsTr("Console login PIN"); font.pixelSize: 14; Layout.fillWidth: true; wrapMode: Text.Wrap }
                            RowLayout {
                                Layout.fillWidth: true
                                C.TextField {
                                    id: pinField
                                    objectName: "consoleLoginPin"
                                    Layout.preferredWidth: 185
                                    Layout.maximumWidth: 185
                                    Layout.fillWidth: true
                                    Accessible.name: qsTr("Console login PIN")
                                    readOnly: false
                                    enabled: !content.submitting
                                    validator: RegularExpressionValidator { regularExpression: /[0-9]{4}/ }
                                    maximumLength: 4
                                    echoMode: content.revealed && !content.privateMode ? TextInput.Normal : TextInput.Password
                                    inputMethodHints: Qt.ImhDigitsOnly | Qt.ImhSensitiveData | Qt.ImhNoPredictiveText
                                    font.pixelSize: 22
                                    leftPadding: 12; rightPadding: 12; topPadding: 12; bottomPadding: 12
                                    background: Rectangle {
                                        implicitHeight: 48; color: "#11151c"; radius: 7
                                        border.color: pinField.activeFocus ? "#63b7ff" : "#3a4352"
                                        border.width: pinField.activeFocus ? 2 : 1
                                    }
                                    Keys.onReturnPressed: content.submit()
                                    Keys.onEnterPressed: content.submit()
                                    Keys.onEscapePressed: content.cancelled()
                                }
                                SetupButton {
                                    objectName: "pinReveal"
                                    text: content.revealed ? qsTr("Hide") : qsTr("Show")
                                    enabled: !content.privateMode && !content.submitting
                                    onClicked: content.revealed = !content.revealed
                                }
                                Item { Layout.fillWidth: true }
                            }
                            Label { text: qsTr("Four digits. Leading zeros are kept."); color: "#9ca8b9"; font.pixelSize: 12; Layout.fillWidth: true; wrapMode: Text.Wrap }
                            Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: "#2b323e"; Layout.topMargin: 8; Layout.bottomMargin: 4 }
                            RowLayout {
                                Layout.fillWidth: true; spacing: 10
                                Label { text: content.actionHelp; font.pixelSize: 11; color: "#9ca8b9"; Layout.fillWidth: true; wrapMode: Text.Wrap }
                                SetupButton { objectName: "pinCancel"; text: qsTr("Cancel"); Layout.maximumWidth: 140; onClicked: content.cancelled() }
                                SetupButton {
                                    objectName: "pinSubmit"
                                    Layout.maximumWidth: 140
                                    text: content.submitText; primary: true
                                    enabled: content.validPin && !content.submitting
                                    onClicked: content.submit()
                                }
                            }
                        }
                        Label {
                            objectName: "pinFailure"
                            visible: !!content.failureText
                            text: content.failureText; textFormat: Text.PlainText
                            color: "#efb1b1"; font.pixelSize: 14; Layout.fillWidth: true; wrapMode: Text.Wrap
                        }
                        Rectangle { visible: content.phase !== "form"; Layout.fillWidth: true; implicitHeight: 1; color: "#2b323e" }
                        SetupButton {
                            id: doneButton
                            objectName: "pinDone"
                            visible: content.phase === "saved"
                            Layout.alignment: Qt.AlignRight
                            text: qsTr("Done"); primary: true; onClicked: content.done()
                        }
                        SetupButton {
                            id: backButton
                            objectName: "pinBack"
                            visible: content.phase === "failure"
                            Layout.alignment: Qt.AlignRight
                            text: qsTr("Back to consoles"); onClicked: content.cancelled()
                        }
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true; Layout.preferredWidth: 280; Layout.alignment: Qt.AlignTop
                    spacing: 16
                    Layout.topMargin: 12
                    Label { text: qsTr("GOOD TO KNOW"); font.pixelSize: 10; font.letterSpacing: 1.8; color: "#9ca8b9"; Layout.fillWidth: true; wrapMode: Text.Wrap }
                    Label { text: content.guidanceTitle; font.pixelSize: 20; font.bold: true; Layout.fillWidth: true; wrapMode: Text.Wrap }
                    Label { text: content.guidance; font.pixelSize: 12; color: "#aab6c7"; Layout.fillWidth: true; wrapMode: Text.Wrap }
                    Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: "#2b323e"; Layout.topMargin: 8 }
                    Label { text: content.guidanceDetailTitle; font.pixelSize: 16; font.bold: true; Layout.fillWidth: true; wrapMode: Text.Wrap }
                    Label { text: content.guidanceDetail; font.pixelSize: 12; color: "#aab6c7"; Layout.fillWidth: true; wrapMode: Text.Wrap }
                }
            }
        }
    }
}
