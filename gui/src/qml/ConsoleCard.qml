pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import "controls" as Controls

Rectangle {
    id: card
    property var host: ({})
    property bool streamerMode: false
    property bool selected: false
    property bool targetEnabled: false
    signal activated()
    signal actionsRequested(Item opener)
    signal selectionRequested()
    readonly property string consoleName: {
        const name = host.name || host.address || qsTr("Console");
        return streamerMode && name === host.address ? qsTr("Manual console") : name;
    }
    readonly property string statusText: {
        if (!host.registered) return qsTr("Not registered");
        if (!host.discovered && host.duid) return qsTr("PSN");
        if (host.discovered && host.state === "ready") return qsTr("Online");
        if (host.discovered && host.state === "standby") return qsTr("In standby");
        return qsTr("Status unknown");
    }
    objectName: "consoleCard"
    implicitHeight: 400
    color: "#181d25"
    radius: 15
    border.color: selected ? "#70baff" : "#2b323d"
    border.width: selected ? 2 : 1
    Accessible.name: consoleName

    function focusPrimary() { connectButton.forceActiveFocus(Qt.TabFocusReason); }

    Rectangle {
        id: art
        anchors { top: parent.top; left: parent.left; right: parent.right; margins: 1 }
        height: 180
        radius: 14
        color: card.host.discovered && card.host.state === "standby" ? "#25252a" : "#1c2633"
        Rectangle {
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
            height: 15
            color: parent.color
        }
        Label {
            objectName: "consoleType"
            anchors { left: parent.left; top: parent.top; margins: 20 }
            text: typeof card.host.ps5 === "boolean"
                ? (card.host.ps5 ? qsTr("PLAYSTATION 5") : qsTr("PLAYSTATION 4")) : qsTr("Console")
            color: "#b8c4d4"
            font.pixelSize: 11
            font.letterSpacing: 1
        }
        Image {
            objectName: "consoleArtwork"
            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 20; verticalCenterOffset: 12 }
            height: 90
            fillMode: Image.PreserveAspectFit
            source: typeof card.host.ps5 === "boolean"
                ? "image://svg/console-ps" + (card.host.ps5 ? "5" : "4")
                  + (card.host.discovered && card.host.state === "ready" ? "#light_on"
                     : card.host.discovered && card.host.state === "standby" ? "#light_standby" : "#console")
                : ""
            sourceSize: Qt.size(width, height)
            Accessible.ignored: true
        }
    }

    ColumnLayout {
        anchors { top: art.bottom; left: parent.left; right: parent.right; bottom: parent.bottom; margins: 20 }
        spacing: 8
        Label {
            objectName: "consoleName"
            Layout.fillWidth: true
            text: card.consoleName
            font.pixelSize: 20
            font.bold: true
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
            Accessible.name: card.consoleName
            ToolTip.visible: nameHover.hovered
            ToolTip.text: card.consoleName
            HoverHandler { id: nameHover }
        }
        Label {
            objectName: "consoleStatus"
            Layout.fillWidth: true
            text: card.statusText
            font.pixelSize: 13
            color: card.host.discovered && card.host.state === "ready" ? "#86d6b5"
                : card.host.discovered && card.host.state === "standby" ? "#dfbd78" : "#aab6c7"
        }
        Label {
            objectName: "consoleRoute"
            Layout.fillWidth: true
            font.pixelSize: 13
            text: card.host.discovered ? qsTr("Local network")
                : card.host.duid ? qsTr("Remote connection via PSN") : qsTr("Manual address")
            color: "#aab6c7"
            elide: Text.ElideRight
        }
        Label {
            objectName: "automaticRegistration"
            Layout.fillWidth: true
            font.pixelSize: 12
            visible: !!card.host.discovered && !card.host.registered && !!card.host.duid
            text: qsTr("Automatic registration available")
            wrapMode: Text.Wrap
        }
        Item { Layout.fillHeight: true }
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Controls.Button {
                id: connectButton
                objectName: "connectButton"
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                topInset: 0
                bottomInset: 0
                font.pixelSize: 14
                text: !card.host.registered ? qsTr("Register")
                    : card.host.discovered && card.host.state === "standby" ? qsTr("Wake and connect") : qsTr("Connect")
                Accessible.name: !card.host.registered ? qsTr("Register %1").arg(card.consoleName)
                    : card.host.discovered && card.host.state === "standby"
                        ? qsTr("Wake and connect to %1").arg(card.consoleName) : qsTr("Connect to %1").arg(card.consoleName)
                focusPolicy: Qt.StrongFocus
                firstInFocusChain: true
                lastInFocusChain: true
                sendOutput: true
                enabled: card.targetEnabled
                Material.background: "#69b8ff"
                Material.foreground: "#101c29"
                Material.roundedScale: Material.SmallScale
                background: Rectangle {
                    radius: 8
                    color: connectButton.enabled ? "#69b8ff" : "#485361"
                    border.color: connectButton.visualFocus ? "#e4f3ff" : "transparent"
                    border.width: 2
                }
                onActiveFocusChanged: if (activeFocus) card.selectionRequested()
                onClicked: card.activated()
            }
            Controls.Button {
                id: actionsButton
                objectName: "consoleActionsButton"
                Layout.preferredWidth: 36
                Layout.preferredHeight: 40
                topInset: 0
                bottomInset: 0
                padding: 0
                flat: true
                text: "\u2026"
                font.pixelSize: 20
                Accessible.name: qsTr("Details for %1").arg(card.consoleName)
                ToolTip.visible: hovered
                ToolTip.text: Accessible.name
                firstInFocusChain: true
                lastInFocusChain: true
                sendOutput: true
                enabled: card.targetEnabled
                onActiveFocusChanged: if (activeFocus) card.selectionRequested()
                onClicked: card.actionsRequested(actionsButton)
            }
        }
    }
}
