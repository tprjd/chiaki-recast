pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

FocusScope {
    id: library
    property var hosts: []
    property bool streamerMode: false
    property string profileKey: ""
    property int targetEpoch: 0
    property bool discoveryEnabled: true
    property bool psnAvailable: false
    property bool steamAvailable: false
    property string applicationVersion: ""
    property bool playStationController: false
    property string selectedIdentity: ""
    property var pendingManagement: null
    property var savePin: (sourceIndex, pin) => { pinRequested(sourceIndex, pin); return true; }
    property Item returnFocusItem: null
    property string returnFocusIdentity: ""
    property bool returnToDetails: false
    property alias searchText: consoleSearch.text
    readonly property var visibleHosts: hosts.filter(host => host && host.display !== false)
    readonly property var displayedHosts: visibleHosts.filter(host => matchesSearch(host))
    readonly property bool collapsedSidebar: width < 1000
    readonly property var selectedTarget: resolveTarget(selectedIdentity)
    signal connectRequested(int sourceIndex, string nickname)
    signal settingsRequested()
    signal addConsoleRequested()
    signal refreshPsnRequested()
    signal steamShortcutRequested()
    signal discoveryToggled(bool enabled)
    signal quitRequested()
    signal managementRequested(string action, var accept, var cancel, var consoleName)
    signal wakeRequested(int sourceIndex)
    signal deleteRequested(int sourceIndex)
    signal hideRequested(string mac, string nickname)
    signal pinRequested(int sourceIndex, string pin)
    implicitWidth: 1280
    implicitHeight: 720
    onDisplayedHostsChanged: updateSelection()
    onHostsChanged: {
        if (!pendingManagement) return;
        const target = resolveTarget(pendingManagement.identity);
        if (!target || !eligible(pendingManagement.action, target.host)) pendingManagement = null;
    }
    onProfileKeyChanged: {
        targetEpoch++;
        selectedIdentity = "";
        updateSelection();
    }
    Component.onCompleted: updateSelection()

    function matchesSearch(host) {
        const name = host.name || host.address || qsTr("Console");
        const consoleName = streamerMode && name === host.address ? qsTr("Manual console") : name;
        const type = typeof host.ps5 === "boolean"
            ? (host.ps5 ? "PS5 " + qsTr("PlayStation 5") : "PS4 " + qsTr("PlayStation 4")) : "";
        const text = (consoleName + " " + type).toLowerCase();
        return searchText.trim().toLowerCase().split(/\s+/).every(term => text.includes(term));
    }
    function identityOf(host) {
        if (!host) return "";
        if (host.discovered)
            return host.mac ? JSON.stringify([targetEpoch, "discovered", host.mac.toLowerCase()]) : "";
        if (host.duid) return JSON.stringify([targetEpoch, "psn", host.duid]);
        return host.manual && host.address
            ? JSON.stringify([targetEpoch, "manual", host.address, host.mac ? host.mac.toLowerCase() : ""]) : "";
    }
    function resolveTarget(identity) {
        if (!identity) return null;
        let target = null;
        for (let index = 0; index < hosts.length; index++) {
            const host = hosts[index];
            if (!host || host.display === false || identityOf(host) !== identity) continue;
            if (target) return null;
            target = { host, sourceIndex: index, nickname: host.discovered ? host.name || "" : "" };
        }
        return target;
    }
    function connectToTarget(identity) {
        const target = resolveTarget(identity);
        if (!target) return false;
        connectRequested(target.sourceIndex, target.nickname);
        return true;
    }
    function updateSelection() {
        if (!resolveTarget(selectedIdentity) || !displayedHosts.some(host => identityOf(host) === selectedIdentity)) {
            const first = displayedHosts.find(host => resolveTarget(identityOf(host)));
            selectedIdentity = first ? identityOf(first) : "";
        }
    }
    function moveSelection(offset) {
        const candidates = displayedHosts.filter(host => resolveTarget(identityOf(host)));
        if (!candidates.length) return;
        const current = candidates.findIndex(host => identityOf(host) === selectedIdentity);
        const next = Math.max(0, Math.min(candidates.length - 1, current + offset));
        selectedIdentity = identityOf(candidates[next]);
        const index = displayedHosts.findIndex(host => identityOf(host) === selectedIdentity);
        const card = cardRepeater.itemAt(index) as ConsoleCard;
        if (card) card.focusPrimary();
    }
    function revealControl(control) {
        const top = control.mapToItem(libraryScroll.contentItem, 0, 0).y;
        const bottom = top + control.height;
        if (top < libraryScroll.contentY) libraryScroll.contentY = top;
        else if (bottom > libraryScroll.contentY + libraryScroll.height)
            libraryScroll.contentY = bottom - libraryScroll.height;
    }
    function moveFromGrid(offset) {
        if (cardGrid.activeFocus || Window.window.activeFocusItem === library)
            moveSelection(offset);
    }
    function rememberFocus() {
        returnFocusItem = Window.window.activeFocusItem;
        returnFocusIdentity = cardGrid.activeFocus ? selectedIdentity : "";
        returnToDetails = returnFocusItem && returnFocusItem.objectName === "consoleActionsButton";
    }
    function restoreFocus() {
        if (returnFocusItem && returnFocusItem.visible && returnFocusItem.enabled)
            returnFocusItem.forceActiveFocus(Qt.TabFocusReason);
        else restoreCardFocus(returnFocusIdentity || selectedIdentity, returnToDetails);
    }
    function restoreCardFocus(identity, details = false) {
        let index = displayedHosts.findIndex(host => identityOf(host) === identity && resolveTarget(identity));
        if (index < 0) {
            index = displayedHosts.findIndex(host => resolveTarget(identityOf(host)));
            details = false;
        }
        const card = cardRepeater.itemAt(index) as ConsoleCard;
        if (card) {
            if (details) card.focusDetails();
            else card.focusPrimary();
        }
        else addConsoleButton.forceActiveFocus(Qt.TabFocusReason);
    }
    function eligible(action, host) {
        if (!host) return false;
        if (action === "pin") return !!host.registered;
        if (action === "wake") return !!host.registered && !host.discovered && !host.duid;
        if (action === "delete") return !!host.manual;
        return action === "hide" && !!host.discovered && !host.registered && !host.manual;
    }
    function requestManagement(action, identity) {
        const target = resolveTarget(identity);
        if (!target || !eligible(action, target.host)) return false;
        if (action === "wake") wakeRequested(target.sourceIndex);
        else {
            rememberFocus();
            const pending = { action, identity };
            pendingManagement = pending;
            managementRequested(action, value => finishManagement(pending, true, value),
                () => finishManagement(pending, false), () => {
                    const current = resolveTarget(identity);
                    return current ? current.host.name || current.host.address || qsTr("Console") : qsTr("Console unavailable");
                });
        }
        return true;
    }
    function finishManagement(pending, accepted, value) {
        if (pendingManagement !== pending) return false;
        if (accepted && pending.action === "pin"
            && (typeof value !== "string" || value.length !== 4 || !/^[0-9]{4}$/.test(value))) return false;
        pendingManagement = null;
        const target = resolveTarget(pending.identity);
        if (accepted && target && eligible(pending.action, target.host)) {
            if (pending.action === "delete") deleteRequested(target.sourceIndex);
            else if (pending.action === "hide") hideRequested(target.host.mac, target.host.name || "");
            else if (pending.action === "pin") {
                const saved = savePin(target.sourceIndex, value) === true;
                restoreFocus();
                return saved;
            }
            restoreFocus();
            return true;
        }
        restoreFocus();
        return false;
    }

    Keys.onMenuPressed: settingsRequested()
    Keys.onLeftPressed: moveFromGrid(-1)
    Keys.onRightPressed: moveFromGrid(1)
    Keys.onUpPressed: moveFromGrid(-cards.columns)
    Keys.onDownPressed: moveFromGrid(cards.columns)
    Keys.onReturnPressed: connectToTarget(selectedIdentity)
    Keys.onYesPressed: requestManagement("wake", selectedIdentity)
    Keys.onNoPressed: {
        if (selectedTarget) requestManagement(selectedTarget.host.manual ? "delete" : "hide", selectedIdentity);
    }
    Keys.onEscapePressed: quitRequested()
    Keys.onPressed: event => {
        if (event.modifiers) return;
        switch (event.key) {
        case Qt.Key_PageUp: requestManagement("pin", selectedIdentity); break;
        case Qt.Key_PageDown: if (psnAvailable) refreshPsnRequested(); break;
        case Qt.Key_F1: if (steamAvailable) steamShortcutRequested(); break;
        case Qt.Key_F2: addConsoleRequested(); break;
        default: return;
        }
        event.accepted = true;
    }

    Rectangle { anchors.fill: parent; color: "#11151b" }
    Rectangle {
        id: sidebar
        width: library.collapsedSidebar ? 64 : 205
        anchors { top: parent.top; bottom: parent.bottom; left: parent.left }
        color: "#11151b"
        Rectangle {
            anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
            width: 1
            color: "#2b323d"
        }
        ColumnLayout {
            anchors { fill: parent; leftMargin: library.collapsedSidebar ? 10 : 22; rightMargin: library.collapsedSidebar ? 10 : 22; topMargin: 34; bottomMargin: 22 }
            spacing: 8
            RowLayout {
                Layout.fillWidth: true
                Layout.bottomMargin: 34
                spacing: 10
                Rectangle {
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                    color: "#69b8ff"
                    radius: 8
                    Image {
                        anchors { fill: parent; margins: 5 }
                        source: "qrc:/icons/chiaking-logo-white.svg"
                        fillMode: Image.PreserveAspectFit
                        Accessible.ignored: true
                    }
                }
                Label { visible: !library.collapsedSidebar; text: "Chiaki Recast"; Layout.fillWidth: true; wrapMode: Text.WordWrap; font.pixelSize: 18; font.bold: true }
            }
            ConsoleButton {
                id: consolesButton
                objectName: "consolesButton"
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                text: qsTr("Consoles")
                icon.source: "qrc:/icons/discover-24px.svg"
                display: library.collapsedSidebar ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                font.pixelSize: 14
                Accessible.name: qsTr("Consoles")
                Material.foreground: "#69b8ff"
                highlighted: true
                onClicked: library.restoreCardFocus(library.selectedIdentity)
                ToolTip.visible: hovered && library.collapsedSidebar
                ToolTip.text: text
            }
            ConsoleButton {
                objectName: "settingsButton"
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                text: qsTr("Settings")
                icon.source: "qrc:/icons/options.svg"
                display: library.collapsedSidebar ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                font.pixelSize: 14
                Accessible.name: qsTr("Settings")
                onClicked: library.settingsRequested()
                ToolTip.visible: hovered && library.collapsedSidebar
                ToolTip.text: text
            }
            Item { Layout.fillHeight: true }
            ConsoleButton {
                objectName: "steamShortcutButton"
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                visible: library.steamAvailable
                text: qsTr("Steam shortcut")
                icon.source: "qrc:/icons/l3.svg"
                display: library.collapsedSidebar ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                font.pixelSize: 13
                Accessible.name: qsTr("Create Steam shortcut")
                onClicked: library.steamShortcutRequested()
                ToolTip.visible: hovered
                ToolTip.text: Accessible.name
            }
            ConsoleButton {
                objectName: "quitButton"
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                text: library.collapsedSidebar ? "\u00d7" : qsTr("Quit")
                icon.source: "image://svg/button-" + (library.playStationController ? "ps" : "deck") + "#moon"
                display: library.collapsedSidebar ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                font.pixelSize: 14
                Accessible.name: qsTr("Quit")
                onClicked: library.quitRequested()
            }
            Label {
                Layout.fillWidth: true
                text: library.applicationVersion
                font.pixelSize: 12
                color: "#94a0b0"
                horizontalAlignment: Text.AlignHCenter
                Accessible.name: qsTr("Version %1").arg(library.applicationVersion)
            }
        }
    }

    ColumnLayout {
        id: contentColumn
        anchors { top: parent.top; bottom: parent.bottom; left: sidebar.right; right: parent.right; topMargin: 36; bottomMargin: 22; leftMargin: library.collapsedSidebar ? 22 : 38; rightMargin: library.collapsedSidebar ? 22 : 38 }
        spacing: 20
        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("Consoles"); font.pixelSize: library.width < 630 ? 28 : 35; font.bold: true }
            Item { Layout.fillWidth: true }
            ConsoleButton {
                id: addConsoleButton
                objectName: "addConsoleButton"
                text: qsTr("Add console")
                icon.source: "qrc:/icons/r3.svg"
                font.pixelSize: 14
                Layout.preferredHeight: 42
                onClicked: library.addConsoleRequested()
            }
        }
        GridLayout {
            id: libraryToolbar
            Layout.fillWidth: true
            Layout.minimumHeight: 42
            columns: library.width < 630 ? 1 : 3
            TextField {
                id: consoleSearch
                objectName: "consoleSearch"
                Layout.fillWidth: true
                Layout.maximumWidth: library.width < 630 ? Infinity : 235
                Layout.preferredHeight: 42
                placeholderText: qsTr("Search consoles")
                Accessible.name: qsTr("Search consoles")
                font.pixelSize: 14
                leftPadding: 42
                background: Rectangle {
                    radius: 8
                    color: "#181d25"
                    border.color: consoleSearch.visualFocus ? "#69b8ff" : "#3b4655"
                    border.width: consoleSearch.visualFocus ? 2 : 1
                }
                Image {
                    objectName: "consoleSearchIcon"
                    anchors { left: parent.left; leftMargin: 12; verticalCenter: parent.verticalCenter }
                    width: 20
                    height: 20
                    sourceSize: Qt.size(width, height)
                    source: "qrc:/icons/search-24px.svg"
                    Accessible.ignored: true
                }
                Keys.priority: Keys.AfterItem
                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Tab || event.key === Qt.Key_Backtab) {
                        event.accepted = false;
                        return;
                    }
                    if (event.key === Qt.Key_Escape) {
                        if (text.length) {
                            clear();
                            event.accepted = true;
                        }
                        return;
                    }
                    event.accepted = true;
                }
            }
            Item { visible: library.width >= 630; Layout.fillWidth: true }
            ConsoleButton {
                objectName: "refreshPsnButton"
                visible: library.psnAvailable
                text: qsTr("Refresh PSN consoles")
                icon.source: "qrc:/icons/r1.svg"
                font.pixelSize: 13
                Layout.preferredHeight: 40
                onClicked: library.refreshPsnRequested()
            }
        }
        FocusScope {
            id: cardGrid
            Layout.fillWidth: true
            Layout.fillHeight: true
            Flickable {
                id: libraryScroll
                objectName: "libraryScroll"
                anchors.fill: parent
                clip: true
                contentWidth: width
                contentHeight: libraryFooter.y + libraryFooter.height + 8
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar { }
                GridLayout {
                    id: cards
                    readonly property real cellWidth: (libraryScroll.width - columnSpacing * (columns - 1)) / columns
                    width: Math.max(0, Math.min(columns, library.displayedHosts.length) * (cellWidth + columnSpacing) - columnSpacing)
                    columns: Math.max(1, Math.floor((libraryScroll.width + 18) / 288))
                    columnSpacing: 18
                    rowSpacing: 18
                    Repeater {
                        id: cardRepeater
                        model: library.displayedHosts
                        delegate: ConsoleCard {
                            id: card
                            required property var modelData
                            Layout.fillWidth: true
                            Layout.preferredHeight: implicitHeight
                            host: modelData
                            streamerMode: library.streamerMode
                            playStationController: library.playStationController
                            selected: library.selectedIdentity === library.identityOf(host)
                            targetEnabled: library.resolveTarget(library.identityOf(host)) !== null
                            onSelectionRequested: control => {
                                library.selectedIdentity = library.identityOf(host);
                                library.revealControl(control);
                            }
                            onActivated: library.connectToTarget(library.identityOf(host))
                            onActionsRequested: opener => {
                                consoleDetails.identity = library.identityOf(host);
                                consoleDetails.opener = opener;
                                consoleDetails.open();
                            }
                        }
                    }
                }
                ColumnLayout {
                    id: emptyState
                    readonly property bool noMatches: library.visibleHosts.length > 0 && !!library.searchText.trim()
                    visible: library.displayedHosts.length === 0
                    width: Math.min(parent.width, 440)
                    x: (parent.width - width) / 2
                    y: 32
                    spacing: 16
                    Label {
                        objectName: "emptyStateTitle"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                        font.pixelSize: 22
                        font.bold: true
                        text: emptyState.noMatches ? qsTr("No matching consoles")
                            : library.discoveryEnabled ? qsTr("No consoles found") : qsTr("Discovery is off")
                    }
                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                        font.pixelSize: 14
                        color: "#94a0b0"
                        text: emptyState.noMatches ? qsTr("Try a different name or console type.")
                            : library.discoveryEnabled ? qsTr("Add a console or wait for local discovery.")
                            : qsTr("Enable discovery to find consoles on your local network.")
                    }
                    ConsoleButton {
                        objectName: "emptyStateAction"
                        Layout.alignment: Qt.AlignHCenter
                        font.pixelSize: 14
                        text: emptyState.noMatches ? qsTr("Clear search")
                            : library.discoveryEnabled ? qsTr("Add console") : qsTr("Enable discovery")
                        onClicked: {
                            if (emptyState.noMatches) {
                                consoleSearch.clear();
                                consoleSearch.forceActiveFocus(Qt.TabFocusReason);
                            } else if (library.discoveryEnabled) library.addConsoleRequested();
                            else library.discoveryToggled(true);
                        }
                    }
                }
                RowLayout {
                    id: libraryFooter
                    y: (emptyState.visible ? emptyState.y + emptyState.height : cards.height) + 16
                    width: parent.width
                    ConsoleButton {
                        id: discoveryButton
                        objectName: "discoveryButton"
                        icon.source: "qrc:/icons/discover-" + (library.discoveryEnabled ? "" : "off-") + "24px.svg"
                        text: library.discoveryEnabled ? qsTr("Discovery on") : qsTr("Discovery off")
                        font.pixelSize: 15
                        icon.width: 24
                        icon.height: 24
                        Layout.preferredHeight: 42
                        onActiveFocusChanged: if (activeFocus) library.revealControl(discoveryButton)
                        onClicked: library.discoveryToggled(!library.discoveryEnabled)
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        objectName: "consoleCount"
                        font.pixelSize: 13
                        color: "#94a0b0"
                        text: library.searchText.trim()
                            ? (library.visibleHosts.length === 1 ? qsTr("%1 of 1 console").arg(library.displayedHosts.length)
                                : qsTr("%1 of %2 consoles").arg(library.displayedHosts.length).arg(library.visibleHosts.length))
                            : library.visibleHosts.length === 1 ? qsTr("1 console")
                            : qsTr("%1 consoles").arg(library.visibleHosts.length)
                    }
                }
            }
        }
        Flow {
            objectName: "controllerHints"
            Layout.fillWidth: true
            visible: library.selectedTarget !== null
            spacing: 18
            Repeater {
                model: ["pin", "wake", "delete", "hide"].filter(action =>
                    library.selectedTarget && library.eligible(action, library.selectedTarget.host))
                ConsoleButton {
                    required property string modelData
                    objectName: modelData + "ShortcutButton"
                    icon.width: 24
                    icon.height: 24
                    icon.source: modelData === "pin" ? "qrc:/icons/l1.svg"
                            : "image://svg/button-" + (library.playStationController ? "ps" : "deck")
                              + (modelData === "wake" ? "#pyramid" : "#box")
                    text: modelData === "pin" ? qsTr("PIN") : modelData === "wake" ? qsTr("Wake")
                        : modelData === "delete" ? qsTr("Delete") : qsTr("Hide")
                    onClicked: library.requestManagement(modelData, library.selectedIdentity)
                }
            }
        }
    }

    // The native render-control window has a zero-size Controls overlay.
    Rectangle {
        objectName: "detailsBackdrop"
        anchors.fill: parent
        visible: consoleDetails.visible
        color: "#b8080c14"
        MouseArea {
            anchors.fill: parent
            onClicked: consoleDetails.close()
        }
    }

    ConsoleDetails {
        id: consoleDetails
        property string identity: ""
        property Item opener: null
        property string requestedAction: ""
        property string requestedIdentity: ""
        readonly property var target: library.resolveTarget(identity)
        onTargetChanged: if (!target) close()
        host: target ? target.host : ({})
        streamerMode: library.streamerMode
        availableActions: target ? ["pin", "wake", "delete", "hide"].filter(action => library.eligible(action, target.host)) : []
        playStationController: library.playStationController
        onManagementRequested: action => {
            requestedAction = action;
            requestedIdentity = identity;
            close();
        }
        onConnectRequested: {
            requestedAction = "connect";
            requestedIdentity = identity;
            close();
        }
        onClosed: Qt.callLater(() => {
            if (opener && opener.visible) {
                opener.focus = false;
                opener.forceActiveFocus(Qt.TabFocusReason);
            } else library.restoreCardFocus(identity);
            if (requestedAction) {
                const action = requestedAction;
                requestedAction = "";
                if (action === "connect") library.connectToTarget(requestedIdentity);
                else library.requestManagement(action, requestedIdentity);
            }
        })
    }
}
