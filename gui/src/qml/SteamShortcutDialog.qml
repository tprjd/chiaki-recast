import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.streetpea.chiaking
import "controls" as C

FocusScope {
    id: dialog
    objectName: "steamShortcutDialog"
    property var backend: Chiaki
    property var navigation: root
    property bool fromReminder: false
    property string phase: "form"
    property string steamBasePath: ""
    property string diagnostics: ""
    property bool detailsExpanded: false
    property bool closed: false
    property var callbackScope: ({owner: dialog, attempt: 0, terminal: false})
    readonly property bool busy: phase === "progress"
    readonly property bool privateMode: backend.settings.streamerMode
    readonly property bool validName: nameField.text.trim().length > 0

    component SteamLabel: Label {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        textFormat: Text.PlainText
        font.pixelSize: 14
    }
    component SteamField: C.TextField {
        id: field
        Layout.fillWidth: true
        font.pixelSize: 14
        echoMode: dialog.privateMode ? TextInput.Password : TextInput.Normal
        topPadding: 12; bottomPadding: 12; leftPadding: 12; rightPadding: 12
        background: Rectangle {
            implicitHeight: 44; color: "#11151c"; radius: 7
            border.color: field.activeFocus ? "#63b7ff" : "#3a4352"
            border.width: field.activeFocus ? 2 : 1
        }
    }
    component SteamButton: SetupButton {
        Layout.maximumWidth: parent.width
        textFormat: Text.PlainText
    }
    function close() {
        if (busy || closed) return;
        closed = true;
        callbackScope.owner = null;
        const navigate = navigation;
        const settings = backend.settings;
        const nextReminder = fromReminder && settings.remotePlayAsk;
        navigate.closeDialog();
        if (nextReminder) {
            if (!settings.psnRefreshToken || !settings.psnAuthToken || !settings.psnAuthTokenExpiry || !settings.psnAccountId)
                navigate.showRemindDialog(qsTr("Remote Play via PSN"), qsTr("Sign in to PSN to register automatically and play outside your home network."), true, () => navigate.showPSNTokenDialog(false));
            else settings.remotePlayAsk = false;
        }
    }
    function submit() {
        if (closed || busy || phase === "success" || !validName) return;
        diagnostics = "";
        detailsExpanded = false;
        callbackScope.attempt++;
        callbackScope.terminal = false;
        phase = "progress";
        dialog.forceActiveFocus(Qt.OtherFocusReason);
        dispatchTimer.start();
    }
    Timer {
        id: dispatchTimer
        interval: 50
        onTriggered: {
            const scope = dialog.callbackScope;
            const attempt = scope.attempt;
            try {
                dialog.backend.createSteamShortcut(nameField.text.trim(), optionsField.text.trim(), function(msg, ok, done) {
                    if (!scope.owner || scope.attempt !== attempt || scope.terminal) return;
                    scope.owner.diagnostics += msg + "\n";
                    if (!done) return;
                    scope.terminal = true;
                    scope.owner.phase = ok ? "success" : "failure";
                    if (ok) scope.owner.backend.settings.addSteamShortcutAsk = false;
                    Qt.callLater(function() { if (scope.owner && scope.attempt === attempt) scope.owner.focusResult(); });
                }, dialog.steamBasePath);
            } catch (error) {
                if (!scope.owner || scope.terminal) return;
                scope.terminal = true;
                scope.owner.phase = "failure";
                scope.owner.diagnostics += qsTr("The shortcut could not be created. Check the Steam folder and try again.");
                scope.owner.focusResult();
            }
        }
    }
    function focusResult() { (phase === "success" ? doneButton : editButton).forceActiveFocus(Qt.TabFocusReason); }
    function edit() { phase = "form"; Qt.callLater(function() { if (!dialog.closed) nameField.forceActiveFocus(Qt.TabFocusReason); }); }
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
    Keys.onMenuPressed: submit()
    StackView.onActivated: if (phase === "form") nameField.forceActiveFocus(Qt.TabFocusReason)
    Component.onCompleted: nameField.forceActiveFocus(Qt.TabFocusReason)
    Component.onDestruction: callbackScope.owner = null

    Rectangle { anchors.fill: parent; color: "#11151c" }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16
        RowLayout {
            Layout.fillWidth: true; spacing: 20
            SteamButton { objectName: "steamBack"; text: qsTr("‹ Back"); enabled: !dialog.busy; onClicked: dialog.close() }
            SteamLabel { text: qsTr("Add to Steam"); font.pixelSize: 30; font.bold: true }
        }
        SteamLabel { text: qsTr("Create or update a non-Steam game for the selected profile."); color: "#9ca8b9" }
        Flickable {
            id: scroll
            objectName: "steamScroll"
            Layout.fillWidth: true; Layout.fillHeight: true
            contentWidth: width; contentHeight: workspace.implicitHeight
            clip: true; boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}
            onHeightChanged: Qt.callLater(dialog.revealFocus)
            onContentHeightChanged: Qt.callLater(dialog.revealFocus)
            RowLayout {
                id: workspace
                width: scroll.width; spacing: 24
                Pane {
                    Layout.fillWidth: true; Layout.preferredWidth: workspace.width * 0.66; Layout.alignment: Qt.AlignTop
                    padding: 24
                    background: Rectangle { color: "#171b23"; radius: 12; border.color: "#2b323e" }
                    ColumnLayout {
                        anchors.fill: parent; spacing: 12
                        ColumnLayout {
                            visible: dialog.phase === "form"
                            Layout.fillWidth: true; spacing: 10
                            SteamLabel { text: qsTr("STEAM SHORTCUT"); color: "#9ca8b9"; font.pixelSize: 11; font.letterSpacing: 1.5 }
                            SteamLabel { text: qsTr("Launch chiaki-ng from Steam"); font.pixelSize: 22; font.bold: true }
                            SteamLabel { text: qsTr("Steam game name"); Layout.topMargin: 12 }
                            SteamField {
                                id: nameField
                                objectName: "steamName"
                                text: dialog.backend.settings.currentProfile ? qsTr("chiaki-ng %1").arg(dialog.backend.settings.currentProfile) : qsTr("chiaki-ng")
                                Accessible.name: qsTr("Steam game name")
                                firstInFocusChain: true
                            }
                            SteamLabel { objectName: "steamNameHelp"; text: dialog.validName ? qsTr("The name shown in your Steam library.") : qsTr("Enter a name for the Steam game."); color: dialog.validName ? "#9ca8b9" : "#ffb0af"; font.pixelSize: 12 }
                            SteamLabel { text: qsTr("Launch options (optional)"); Layout.topMargin: 8 }
                            SteamField {
                                id: optionsField
                                objectName: "steamOptions"
                                text: dialog.backend.settings.currentProfile ? "--profile=" + dialog.backend.settings.currentProfile : ""
                                Accessible.name: qsTr("Launch options")
                            }
                            SteamLabel { text: qsTr("Custom Steam folder (optional)"); Layout.topMargin: 8 }
                            SteamLabel { objectName: "steamPath"; text: dialog.steamBasePath ? (dialog.privateMode ? qsTr("Folder hidden") : dialog.steamBasePath) : qsTr("Use the default Steam folder"); color: "#9ca8b9" }
                            SteamButton {
                                objectName: "steamChoosePath"; text: qsTr("Choose Steam folder")
                                onClicked: { const path = dialog.backend.settings.chooseSteamBasePath(); if (path) dialog.steamBasePath = path; }
                            }
                            Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: "#2b323e"; Layout.topMargin: 10; Layout.bottomMargin: 6 }
                            Flow {
                                Layout.fillWidth: true; spacing: 10; layoutDirection: Qt.RightToLeft
                                SteamButton { objectName: "steamCreate"; text: qsTr("Create shortcut"); primary: true; enabled: dialog.validName; onClicked: dialog.submit() }
                                SteamButton { objectName: "steamCancel"; text: qsTr("Cancel"); onClicked: dialog.close() }
                            }
                        }
                        ColumnLayout {
                            visible: dialog.phase !== "form"
                            Layout.fillWidth: true; spacing: 16
                            BusyIndicator { visible: dialog.busy; running: visible; Layout.preferredHeight: 40; Layout.preferredWidth: 40 }
                            SteamLabel {
                                objectName: "steamResultHeading"
                                text: dialog.busy ? qsTr("Creating Steam shortcut…") : dialog.phase === "success" ? qsTr("Steam shortcut ready") : qsTr("Steam shortcut was not completed")
                                font.pixelSize: 22; font.bold: true
                            }
                            SteamLabel {
                                objectName: "steamResultMessage"
                                text: dialog.busy ? qsTr("Wait for Steam files to finish writing. This operation cannot be cancelled.")
                                    : dialog.phase === "success" ? qsTr("Restart Steam yourself to see the shortcut in your library.")
                                    : qsTr("Check the details and Steam folder, then try again. Some files may already have changed.")
                                color: "#aab6c7"
                            }
                            SteamButton { objectName: "steamDetailsToggle"; text: dialog.detailsExpanded ? qsTr("Hide technical details") : qsTr("Show technical details"); onClicked: dialog.detailsExpanded = !dialog.detailsExpanded }
                            SteamLabel { objectName: "steamDetails"; visible: dialog.detailsExpanded; text: dialog.privateMode ? qsTr("Technical details hidden in streamer mode.") : dialog.diagnostics; color: "#aab6c7"; font.pixelSize: 12 }
                            Flow {
                                Layout.fillWidth: true; spacing: 10
                                SteamButton { id: editButton; objectName: "steamEdit"; visible: dialog.phase === "failure"; text: qsTr("Edit details"); onClicked: dialog.edit() }
                                SteamButton { objectName: "steamRetry"; visible: dialog.phase === "failure"; text: qsTr("Retry"); primary: true; onClicked: dialog.submit() }
                                SteamButton { id: doneButton; objectName: "steamDone"; visible: dialog.phase === "success"; text: qsTr("Done"); primary: true; onClicked: dialog.close() }
                                SteamButton { objectName: "steamClose"; visible: dialog.phase === "failure"; text: qsTr("Close"); onClicked: dialog.close() }
                            }
                        }
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true; Layout.preferredWidth: 260; Layout.alignment: Qt.AlignTop; Layout.topMargin: 12
                    spacing: 16
                    SteamLabel { text: qsTr("GOOD TO KNOW"); color: "#9ca8b9"; font.pixelSize: 11; font.letterSpacing: 1.5 }
                    SteamLabel { text: qsTr("Keep your selected profile"); font.pixelSize: 20; font.bold: true }
                    SteamLabel { text: qsTr("The default launch options select the profile you use now. You can edit them before you create the shortcut."); color: "#aab6c7"; font.pixelSize: 12 }
                    Rectangle { Layout.fillWidth: true; implicitHeight: 1; color: "#2b323e" }
                    SteamLabel { text: qsTr("Existing shortcuts"); font.pixelSize: 16; font.bold: true }
                    SteamLabel { text: qsTr("A shortcut with the same executable and launch options is updated. Artwork and the Steam Deck controller configuration are also applied."); color: "#aab6c7"; font.pixelSize: 12 }
                }
            }
        }
    }
}
