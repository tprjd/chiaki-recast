pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import "controls" as Controls

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
    readonly property var displayedHosts: hosts.filter(host => host && host.display !== false)
    readonly property bool collapsedSidebar: width < 1000
    readonly property var selectedTarget: resolveTarget(selectedIdentity)
    signal connectRequested(int sourceIndex, string nickname)
    signal settingsRequested()
    signal addConsoleRequested()
    signal refreshPsnRequested()
    signal steamShortcutRequested()
    signal discoveryToggled(bool enabled)
    signal quitRequested()
    signal managementRequested(string action, string identity)
    implicitWidth: 1280
    implicitHeight: 720
    onDisplayedHostsChanged: updateSelection()
    onProfileKeyChanged: {
        targetEpoch++;
        actionMenu.close();
        selectedIdentity = "";
        updateSelection();
    }
    Component.onCompleted: updateSelection()

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
        if (!resolveTarget(selectedIdentity))
            selectedIdentity = displayedHosts.length ? identityOf(displayedHosts[0]) : "";
        if (actionMenu.opened && !resolveTarget(actionMenu.identity)) actionMenu.close();
    }
    function moveSelection(offset) {
        if (!displayedHosts.length) return;
        const current = displayedHosts.findIndex(host => identityOf(host) === selectedIdentity);
        const next = Math.max(0, Math.min(displayedHosts.length - 1, current + offset));
        selectedIdentity = identityOf(displayedHosts[next]);
        const card = cardRepeater.itemAt(next) as ConsoleCard;
        if (card) {
            card.focusPrimary();
            libraryScroll.contentY = Math.max(0, Math.min(card.y, cards.height - libraryScroll.height));
        }
    }
    function restoreCardFocus(identity) {
        const index = displayedHosts.findIndex(host => identityOf(host) === identity);
        const card = cardRepeater.itemAt(index >= 0 ? index : 0) as ConsoleCard;
        if (card) card.focusPrimary();
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
        if (target && eligible(action, target.host)) managementRequested(action, identity);
    }

    Keys.onMenuPressed: settingsRequested()
    Keys.onLeftPressed: moveSelection(-1)
    Keys.onRightPressed: moveSelection(1)
    Keys.onUpPressed: moveSelection(-cards.columns)
    Keys.onDownPressed: moveSelection(cards.columns)
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
                Label { visible: !library.collapsedSidebar; text: "chiaki-ng"; font.pixelSize: 20; font.bold: true }
            }
            Controls.Button {
                id: consolesButton
                objectName: "consolesButton"
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                flat: true
                text: qsTr("Consoles")
                icon.source: "qrc:/icons/discover-24px.svg"
                display: library.collapsedSidebar ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                font.pixelSize: 14
                Accessible.name: qsTr("Consoles")
                Material.foreground: "#69b8ff"
                background: Rectangle { radius: 8; color: "#1d354c"; border.color: consolesButton.visualFocus ? "#69b8ff" : "transparent" }
                onClicked: library.restoreCardFocus(library.selectedIdentity)
                ToolTip.visible: hovered && library.collapsedSidebar
                ToolTip.text: text
            }
            Controls.Button {
                objectName: "settingsButton"
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                flat: true
                text: qsTr("Settings")
                icon.source: "qrc:/icons/settings-20px.svg"
                display: library.collapsedSidebar ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                font.pixelSize: 14
                Accessible.name: qsTr("Settings")
                onClicked: library.settingsRequested()
                ToolTip.visible: hovered && library.collapsedSidebar
                ToolTip.text: text
            }
            Item { Layout.fillHeight: true }
            Controls.Button {
                objectName: "steamShortcutButton"
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                flat: true
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
            Controls.Button {
                objectName: "quitButton"
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                flat: true
                text: library.collapsedSidebar ? "\u00d7" : qsTr("Quit")
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
            Label { text: qsTr("Consoles"); font.pixelSize: 35; font.bold: true }
            Item { Layout.fillWidth: true }
            Controls.Button {
                id: addConsoleButton
                objectName: "addConsoleButton"
                text: qsTr("Add console")
                icon.source: "qrc:/icons/add-24px.svg"
                font.pixelSize: 14
                Layout.preferredHeight: 42
                Material.roundedScale: Material.SmallScale
                background: Rectangle {
                    radius: 8
                    color: "transparent"
                    border.color: addConsoleButton.visualFocus ? "#69b8ff" : "#2b323d"
                }
                onClicked: library.addConsoleRequested()
            }
        }
        RowLayout {
            id: libraryToolbar
            Layout.fillWidth: true
            Layout.minimumHeight: 42
            Item { Layout.fillWidth: true }
            Controls.Button {
                objectName: "refreshPsnButton"
                visible: library.psnAvailable
                flat: true
                text: qsTr("Refresh PSN consoles")
                font.pixelSize: 13
                Layout.preferredHeight: 40
                onClicked: library.refreshPsnRequested()
            }
        }
        FocusScope {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Flickable {
                id: libraryScroll
                objectName: "libraryScroll"
                anchors.fill: parent
                clip: true
                contentWidth: width
                contentHeight: cards.height + discoveryButton.height + 24
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar { }
                GridLayout {
                    id: cards
                    width: parent.width
                    columns: Math.max(1, Math.floor((width + 18) / 288))
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
                            selected: library.selectedIdentity === library.identityOf(host)
                            targetEnabled: library.resolveTarget(library.identityOf(host)) !== null
                            onSelectionRequested: library.selectedIdentity = library.identityOf(host)
                            onActivated: library.connectToTarget(library.identityOf(host))
                            onActionsRequested: opener => {
                                actionMenu.identity = library.identityOf(host);
                                actionMenu.opener = opener;
                                actionMenu.popup(card, card.width - actionMenu.width, card.height - 40);
                            }
                        }
                    }
                }
                Controls.Button {
                    id: discoveryButton
                    objectName: "discoveryButton"
                    y: cards.height + 16
                    flat: true
                    icon.source: "qrc:/icons/discover-" + (library.discoveryEnabled ? "" : "off-") + "24px.svg"
                    text: library.discoveryEnabled ? qsTr("Discovery on") : qsTr("Discovery off")
                    font.pixelSize: 13
                    height: 36
                    onClicked: library.discoveryToggled(!library.discoveryEnabled)
                }
            }
        }
    }

    Menu {
        id: actionMenu
        objectName: "consoleActionsMenu"
        property string identity: ""
        property Item opener: null
        readonly property var target: library.resolveTarget(identity)
        onClosed: Qt.callLater(() => {
            if (opener && opener.visible)
                opener.forceActiveFocus(Qt.TabFocusReason);
            else
                library.restoreCardFocus(identity);
        })
        MenuItem {
            objectName: "pinAction"
            text: qsTr("Update Console Pin")
            icon.source: "qrc:/icons/l1.svg"
            visible: actionMenu.target && library.eligible("pin", actionMenu.target.host)
            height: visible ? implicitHeight : 0
            onTriggered: {
                const identity = actionMenu.identity;
                Qt.callLater(() => library.requestManagement("pin", identity));
            }
        }
        MenuItem {
            objectName: "wakeAction"
            text: qsTr("Wake Up")
            icon.source: "image://svg/button-" + (library.playStationController ? "ps" : "deck") + "#pyramid"
            visible: actionMenu.target && library.eligible("wake", actionMenu.target.host)
            height: visible ? implicitHeight : 0
            onTriggered: {
                const identity = actionMenu.identity;
                Qt.callLater(() => library.requestManagement("wake", identity));
            }
        }
        MenuItem {
            objectName: "removeAction"
            text: actionMenu.target && actionMenu.target.host.manual ? qsTr("Delete") : qsTr("Hide")
            icon.source: "image://svg/button-" + (library.playStationController ? "ps" : "deck") + "#box"
            visible: actionMenu.target && (library.eligible("delete", actionMenu.target.host) || library.eligible("hide", actionMenu.target.host))
            height: visible ? implicitHeight : 0
            onTriggered: {
                if (!actionMenu.target) return;
                const action = actionMenu.target.host.manual ? "delete" : "hide";
                const identity = actionMenu.identity;
                Qt.callLater(() => library.requestManagement(action, identity));
            }
        }
    }
}
