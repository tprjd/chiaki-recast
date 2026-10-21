import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.streetpea.chiaking
import "controls" as C

FocusScope {
    id: dialog
    property var backend: Chiaki
    property var navigation: root
    property var callback: null
    property bool login: false
    property var lookup: null
    property bool submitting: false
    property bool leaving: false
    property bool retiring: false
    property bool invalidContext: false
    property string errorText: ""
    property var callbackScope: ({owner: dialog, attempt: 0})

    function requestLookup(username, done) {
        const request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState === XMLHttpRequest.DONE) done(request.status, request.responseText);
        };
        request.open("GET", "https://psn.flipscreen.games/search.php?username=" + encodeURIComponent(username));
        request.send();
    }
    function submit() {
        if (submitting || !callbackScope.owner) return;
        if (!username.text.trim()) { errorText = qsTr("Enter your PSN username."); return; }
        errorText = "";
        submitting = true;
        timeout.start();
        Qt.callLater(focusAction);
        const scope = callbackScope;
        const attempt = ++scope.attempt;
        const done = function(status, text) {
            if (!scope.owner || scope.attempt !== attempt || !scope.owner.submitting) return;
            scope.owner.lookupResult(status, text);
        };
        try { (lookup || requestLookup)(username.text.trim(), done); }
        catch (error) { done(0, ""); }
    }
    function lookupResult(status, text) {
        timeout.stop();
        submitting = false;
        Qt.callLater(focusAction);
        if (status === 408) { errorText = qsTr("The lookup timed out. Check your connection and try again."); return; }
        if (status !== 200) { errorText = qsTr("The lookup service could not be reached. Check your connection and try again."); return; }
        let response;
        try { response = JSON.parse(text); }
        catch (error) { errorText = qsTr("The lookup service returned an unreadable response. Try again."); return; }
        const id = response && response.encoded_id;
        if (typeof id !== "string" || !/^[A-Za-z0-9+/]{10}[AEIMQUYcgkosw048]=$/.test(id)) {
            errorText = qsTr("No valid Account ID was found. Check your username and allow anyone to find you in PSN search, then try again.");
            return;
        }
        complete(id);
    }
    function complete(id) {
        if (!callbackScope.owner || !id) return;
        if (!callback || callback(id) !== false) close();
    }
    function focusAction() {
        if (login || leaving || !visible || !callbackScope.owner) return;
        (submitting ? cancelButton : errorText ? submitButton : username).forceActiveFocus(Qt.TabFocusReason);
    }
    function ensureFocusVisible() {
        const item = Window.window ? Window.window.activeFocusItem : null;
        if (!item || login) return;
        let ancestor = item;
        while (ancestor && ancestor !== workspace) ancestor = ancestor.parent;
        if (!ancestor) return;
        const y = item.mapToItem(workspace, 0, 0).y;
        if (y < scroll.contentY) scroll.contentY = y;
        else if (y + item.height > scroll.contentY + scroll.height) scroll.contentY = y + item.height - scroll.height;
    }
    function close() { if (leaving) return; leaving = true; callbackScope.owner = null; timeout.stop(); navigation.closeDialog(); }
    Component.onDestruction: callbackScope.owner = null
    StackView.onDeactivating: { retiring = true; callbackScope.owner = null; timeout.stop(); }
    StackView.onActivated: if (!login) { username.readOnly = false; username.forceActiveFocus(Qt.TabFocusReason); }
    Keys.onEscapePressed: close()
    Keys.onMenuPressed: submit()
    Timer {
        id: timeout
        interval: 30000
        onTriggered: {
            if (!dialog.callbackScope.owner || !dialog.submitting) return;
            dialog.callbackScope.attempt++;
            dialog.lookupResult(408, "");
        }
    }
    Connections {
        target: dialog.Window.window
        function onActiveFocusItemChanged() { Qt.callLater(dialog.ensureFocusVisible); }
    }
    Connections {
        target: dialog.backend.settings
        function onCurrentProfileChanged() {
            dialog.invalidContext = true;
            dialog.callbackScope.owner = null;
            timeout.stop();
            dialog.submitting = false;
            dialog.errorText = qsTr("The profile changed. Close this screen and open registration again.");
            if (!dialog.login) cancelButton.forceActiveFocus(Qt.TabFocusReason);
        }
    }
    Rectangle { anchors.fill: parent; color: "#11151c" }
    Flickable {
        id: scroll
        objectName: "lookupScroll"
        visible: !dialog.login
        anchors.fill: parent; anchors.margins: dialog.width < 1000 ? 24 : 32
        contentWidth: width; contentHeight: workspace.implicitHeight
        clip: true; boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {}
        onHeightChanged: Qt.callLater(dialog.ensureFocusVisible)
        onContentHeightChanged: Qt.callLater(dialog.ensureFocusVisible)
        ColumnLayout {
            id: workspace
            width: scroll.width
            spacing: 16
            Label { text: qsTr("Console setup / Account ID"); color: "#adb8c8"; font.pixelSize: 12 }
            Label { objectName: "lookupHeading"; text: qsTr("Find a public Account ID"); color: "#ffffff"; font.pixelSize: 30; font.bold: true; Layout.fillWidth: true; wrapMode: Text.Wrap }
            Label {
                objectName: "lookupExplanation"
                text: qsTr("This sends your PSN username to the third-party service psn.flipscreen.games. Your PSN privacy settings must allow anyone to find you in search. Public lookup retrieves your Account ID. It does not sign you in or save authentication credentials.")
                color: "#adb8c8"; font.pixelSize: 14; Layout.fillWidth: true; wrapMode: Text.Wrap; textFormat: Text.PlainText
            }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: panel.implicitHeight + 40
                color: "#171b23"; radius: 12; border.color: "#2b323e"
                ColumnLayout {
                    id: panel
                    anchors { left: parent.left; right: parent.right; top: parent.top; margins: 20 }
                    spacing: 16
                    Label { text: qsTr("PSN username"); color: "#ffffff" }
                    C.TextField {
                        id: username
                        objectName: "lookupUsername"
                        Layout.fillWidth: true
                        Accessible.name: qsTr("PSN username")
                        enabled: !dialog.submitting
                        echoMode: backend.settings.streamerMode ? TextInput.Password : TextInput.Normal
                        selectByMouse: true
                        inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                        onAccepted: dialog.submit()
                        background: Rectangle { implicitHeight: 46; color: "#11151c"; radius: 7; border.color: username.activeFocus ? "#63b7ff" : "#3a4352"; border.width: username.activeFocus ? 2 : 1 }
                    }
                    Label { visible: dialog.submitting; text: qsTr("Retrieving your Account ID"); color: "#adb8c8"; wrapMode: Text.Wrap; Layout.fillWidth: true }
                    BusyIndicator { visible: dialog.submitting; running: visible; Layout.preferredWidth: 40; Layout.preferredHeight: 40 }
                    Label { objectName: "lookupError"; visible: !!dialog.errorText; text: dialog.errorText; color: "#ffb4ab"; wrapMode: Text.Wrap; textFormat: Text.PlainText; Layout.fillWidth: true }
                    Flow {
                        Layout.fillWidth: true; spacing: 10
                        SetupButton { id: submitButton; objectName: "lookupSubmit"; text: dialog.errorText ? qsTr("Retry lookup") : qsTr("Find Account ID"); primary: true; enabled: !dialog.submitting && !dialog.invalidContext; width: Math.min(implicitWidth, parent.width); onClicked: dialog.submit() }
                        SetupButton { id: cancelButton; objectName: "lookupCancel"; text: qsTr("Cancel"); width: Math.min(implicitWidth, parent.width); onClicked: dialog.close() }
                    }
                }
            }
        }
    }
    Loader {
        anchors.fill: parent
        active: dialog.login
        sourceComponent: PSNTokenDialog {
            backend: dialog.backend
            navigation: dialog
            retiring: dialog.retiring || dialog.leaving
            registration: true
            onCompleted: accountId => dialog.complete(accountId)
        }
    }
    function closeDialog() { close(); }
}
