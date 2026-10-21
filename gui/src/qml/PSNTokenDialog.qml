import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.streetpea.chiaking
import "controls" as C

FocusScope {
    id: dialog
    objectName: "psnSetup"
    property var backend: Chiaki
    property var navigation: root
    property string psnurl: ""
    property bool expired: false
    property bool preferEmbedded: Qt.platform.os === "windows"
    property var createBrowser: function(parent) {
        return Qt.createQmlObject('import QtQuick; import QtWebEngine; WebEngineView { profile { offTheRecord: false; storageName: "psn-token" } settings { touchIconsEnabled: true; localContentCanAccessRemoteUrls: true } onContextMenuRequested: request => request.accepted = true }', parent, "psnBrowser");
    }
    property var browser: null
    property bool externalBrowser: false
    property bool revealed: false
    property bool detailsExpanded: false
    property bool leaving: false
    property bool retiring: false
    property string ownedAttemptId: ""
    property string completedAttemptId: ""
    readonly property var attempt: backend.psnAuthAttempt || ({})
    readonly property bool ownsAttempt: ownedAttemptId.length > 0 && attempt.id === ownedAttemptId
    readonly property string phase: ownsAttempt ? attempt.phase || "form" : "form"
    readonly property bool busy: phase === "exchanging" || phase === "account"
    readonly property bool success: phase === "success"
    readonly property bool failure: phase === "failure"
    readonly property bool privateMode: backend.settings.streamerMode
    signal completed(string accountId)

    component PsnLabel: Label {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        textFormat: Text.PlainText
        font.pixelSize: 14
        color: "#adb8c8"
    }
    component PsnButton: SetupButton { textFormat: Text.PlainText; width: Math.min(implicitWidth, parent.width) }

    function focusAction() {
        if (leaving || retiring || !visible) return;
        if (busy || success) closeButton.forceActiveFocus(Qt.TabFocusReason);
        else if (failure) freshButton.forceActiveFocus(Qt.TabFocusReason);
        else if (externalBrowser) { redirect.readOnly = false; redirect.forceActiveFocus(Qt.TabFocusReason); }
        else externalButton.forceActiveFocus(Qt.TabFocusReason);
    }
    function ensureFocusVisible() {
        const item = Window.window ? Window.window.activeFocusItem : null;
        if (!item) return;
        let ancestor = item;
        while (ancestor && ancestor !== workspace) ancestor = ancestor.parent;
        if (!ancestor) return;
        const y = item.mapToItem(workspace, 0, 0).y;
        if (y < scroll.contentY) scroll.contentY = y;
        else if (y + item.height > scroll.contentY + scroll.height) scroll.contentY = y + item.height - scroll.height;
    }
    function useExternal() {
        if (busy || leaving || retiring) return;
        if (browser) browser.stop();
        externalBrowser = true;
        psnurl = backend.openPsnLink() || String(backend.psnLoginUrl());
        Qt.callLater(focusAction);
    }
    function freshSignIn() {
        if (busy || leaving || retiring) return;
        ownedAttemptId = "";
        detailsExpanded = false;
        redirect.text = "";
        revealed = false;
        if (externalBrowser || !browser) useExternal();
        else browser.url = backend.psnLoginUrl();
    }
    function submit(value) {
        if (busy || leaving || retiring) return;
        revealed = false;
        detailsExpanded = false;
        const id = backend.startPsnAuth(value);
        if (id) ownedAttemptId = id;
        Qt.callLater(focusAction);
    }
    function close() {
        if (leaving || retiring) return;
        leaving = true;
        revealed = false;
        redirect.text = "";
        if (browser) {
            if (backend.settings.remotePlayAsk) backend.clearCookies(browser.profile);
            browser.profile.clearHttpCache();
            browser.stop();
        }
        navigation.closeDialog();
    }
    function startBrowser() {
        if (busy || leaving || retiring) return;
        if (!preferEmbedded) { useExternal(); return; }
        try {
            browser = createBrowser(browserHost);
            if (!browser) throw new Error("Browser unavailable");
            browser.anchors.fill = browserHost;
            backend.setWebEngineHints(browser.profile);
            browser.navigationRequested.connect(function(request) {
                if (backend.checkPsnRedirectURL(request.url)) { request.reject(); dialog.submit(request.url); }
                else request.accept();
            });
            browser.profile.clearHttpCacheCompleted.connect(function() {
                if (!dialog.leaving && !dialog.retiring && dialog.browser) dialog.browser.reload();
            });
            browser.url = backend.psnLoginUrl();
        } catch (error) {
            if (browser) { browser.destroy(); browser = null; }
            useExternal();
        }
    }
    function reloadBrowser() {
        if (!browser || busy || leaving || retiring) return;
        backend.clearCookies(browser.profile);
        browser.profile.clearHttpCache();
    }
    onPrivateModeChanged: if (privateMode) revealed = false
    onAttemptChanged: {
        if (!ownsAttempt || leaving || retiring) return;
        if (success && completedAttemptId !== ownedAttemptId) {
            completedAttemptId = ownedAttemptId;
            completed(attempt.accountId || "");
        }
        Qt.callLater(focusAction);
    }
    Component.onCompleted: {
        if (attempt.phase === "exchanging" || attempt.phase === "account") ownedAttemptId = attempt.id;
        else startBrowser();
        Qt.callLater(focusAction);
    }
    Component.onDestruction: {
        if (browser) {
            browser.stop();
            if (!leaving) {
                if (backend.settings.remotePlayAsk) backend.clearCookies(browser.profile);
                browser.profile.clearHttpCache();
            }
        }
    }
    StackView.onDeactivating: retiring = true
    Keys.onEscapePressed: close()
    Keys.onMenuPressed: if (externalBrowser && !busy && !success) submit(redirect.text.trim())
    Keys.onPressed: event => {
        if (event.modifiers) return;
        if (event.key === Qt.Key_PageUp) { reloadBrowser(); event.accepted = true; }
        else if (event.key === Qt.Key_PageDown) { useExternal(); event.accepted = true; }
    }
    Connections { target: dialog.Window.window; function onActiveFocusItemChanged() { Qt.callLater(dialog.ensureFocusVisible); } }

    Rectangle { anchors.fill: parent; color: "#11151c" }
    ColumnLayout {
        anchors.fill: parent; anchors.margins: dialog.width < 1000 ? 24 : 32
        spacing: 16
        PsnLabel { text: qsTr("Settings / PSN remote access"); font.pixelSize: 12 }
        PsnLabel { objectName: "psnHeading"; text: dialog.expired ? qsTr("Renew PSN credentials") : qsTr("Set up PSN remote access"); font.pixelSize: 30; font.bold: true; color: "#ffffff" }
        PsnLabel { objectName: "psnExplanation"; text: dialog.expired ? qsTr("Your saved PSN credentials expired. Sign in again to use Remote Play away from home.") : qsTr("Sign in to PlayStation Network to use Remote Play away from home.") }
        Flickable {
            id: scroll
            objectName: "psnScroll"
            Layout.fillWidth: true; Layout.fillHeight: true
            contentWidth: width; contentHeight: workspace.implicitHeight
            clip: true; boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}
            onHeightChanged: Qt.callLater(dialog.ensureFocusVisible)
            onContentHeightChanged: Qt.callLater(dialog.ensureFocusVisible)
            ColumnLayout {
                id: workspace
                width: scroll.width
                spacing: 18
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: panel.implicitHeight + 40
                    color: "#171b23"; radius: 12; border.color: "#2b323e"
                    ColumnLayout {
                        id: panel
                        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 20 }
                        spacing: 16
                        PsnLabel {
                            objectName: "psnStatus"
                            text: dialog.busy ? (dialog.phase === "account" ? qsTr("Retrieving your Account ID") : qsTr("Completing PSN sign-in")) : dialog.success ? qsTr("PSN remote access is ready") : dialog.failure ? qsTr("PSN sign-in did not finish") : qsTr("Sign in with your browser")
                            font.pixelSize: 22; font.bold: true; color: dialog.success ? "#80dcb0" : dialog.failure ? "#ffb4ab" : "#ffffff"
                        }
                        BusyIndicator { visible: dialog.busy; running: visible; Layout.preferredWidth: 40; Layout.preferredHeight: 40 }
                        PsnLabel {
                            objectName: "psnStorage"
                            text: qsTr("Sign-in saves PSN authentication credentials and your Account ID in the current profile. These credentials allow remote access to your consoles.")
                        }
                        PsnLabel {
                            objectName: "psnCloseExplanation"
                            visible: dialog.busy
                            text: qsTr("You can close this screen. Sign-in can still finish and save credentials in this profile while the app is open.")
                        }
                        PsnLabel { visible: dialog.success; text: qsTr("Your credentials and Account ID are saved. You can now close this screen.") }
                        PsnLabel { objectName: "psnFailure"; visible: dialog.failure; color: "#ffb4ab"; text: dialog.failure ? attempt.message || qsTr("Check your connection, then try again or start a fresh sign-in.") : "" }
                        ColumnLayout {
                            visible: !dialog.busy && !dialog.success
                            Layout.fillWidth: true
                            spacing: 12
                            Flow {
                                Layout.fillWidth: true; spacing: 10
                                PsnButton { id: externalButton; objectName: "psnExternal"; text: dialog.externalBrowser ? qsTr("Open sign-in browser") : qsTr("Use external browser"); onClicked: dialog.useExternal() }
                                PsnButton { objectName: "psnReload"; visible: !dialog.externalBrowser; text: qsTr("Reload and clear cookies"); onClicked: dialog.reloadBrowser() }
                                PsnButton { objectName: "psnCopy"; visible: dialog.externalBrowser; text: qsTr("Copy sign-in link"); onClicked: { linkCopy.selectAll(); linkCopy.copy(); } }
                            }
                            Item {
                                id: browserHost
                                objectName: "psnBrowserHost"
                                visible: !dialog.externalBrowser && !dialog.failure
                                Layout.fillWidth: true; Layout.preferredHeight: visible ? Math.max(260, scroll.height - 210) : 0
                            }
                            PsnLabel { visible: dialog.externalBrowser; text: qsTr("After sign-in, copy the complete final URL from your browser address bar. Paste it below. Use the full URL, including its authorization code.") }
                            PsnLabel { visible: dialog.externalBrowser; text: qsTr("Sign-in redirect URL"); color: "#ffffff" }
                            C.TextField {
                                id: redirect
                                objectName: "psnRedirect"
                                visible: dialog.externalBrowser
                                Layout.fillWidth: true
                                Accessible.name: qsTr("Sign-in redirect URL")
                                echoMode: dialog.revealed && !dialog.privateMode ? TextInput.Normal : TextInput.Password
                                inputMethodHints: Qt.ImhSensitiveData | Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
                                selectByMouse: true
                                background: Rectangle { implicitHeight: 46; color: "#11151c"; radius: 7; border.color: redirect.activeFocus ? "#63b7ff" : "#3a4352"; border.width: redirect.activeFocus ? 2 : 1 }
                            }
                            Flow {
                                visible: dialog.externalBrowser
                                Layout.fillWidth: true; spacing: 10
                                PsnButton { objectName: "psnPaste"; text: qsTr("Paste"); onClicked: { redirect.readOnly = false; redirect.paste(); } }
                                PsnButton { objectName: "psnReveal"; text: dialog.revealed ? qsTr("Hide") : qsTr("Show"); enabled: !dialog.privateMode; onClicked: dialog.revealed = !dialog.revealed }
                                PsnButton { objectName: "psnSubmit"; text: dialog.failure ? qsTr("Retry") : qsTr("Complete sign-in"); primary: true; onClicked: dialog.submit(redirect.text.trim()) }
                            }
                        }
                        PsnButton { id: freshButton; objectName: "psnFresh"; visible: dialog.failure; text: qsTr("Start fresh sign-in"); onClicked: dialog.freshSignIn() }
                        PsnButton { objectName: "psnDetailsToggle"; visible: dialog.failure && !!attempt.errorText; text: dialog.detailsExpanded ? qsTr("Hide details") : qsTr("Technical details"); flat: true; onClicked: dialog.detailsExpanded = !dialog.detailsExpanded }
                        PsnLabel { objectName: "psnDetails"; visible: dialog.failure && dialog.detailsExpanded; text: dialog.privateMode ? qsTr("Details are hidden in streamer mode.") : attempt.errorText || "" }
                        PsnButton { id: closeButton; objectName: "psnClose"; text: dialog.busy || dialog.success || dialog.failure ? qsTr("Close") : qsTr("Cancel"); onClicked: dialog.close() }
                    }
                }
                PsnLabel { text: qsTr("Use the same PlayStation account as your console. The Sony sign-in page and your external browser can show account details.") }
            }
        }
    }
    TextEdit { id: linkCopy; visible: false; text: dialog.psnurl }
}
