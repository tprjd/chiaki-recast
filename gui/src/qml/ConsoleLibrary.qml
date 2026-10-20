pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import "controls" as Controls

FocusScope {
    id: library

    property var hosts: []
    property var sourceHosts: hosts
    property var sourceIndices: []
    property bool streamerMode: false
    property string profileKey: ""
    property int targetEpoch: 0
    onProfileKeyChanged: targetEpoch++
    readonly property var displayedHosts: hosts.filter(host => host && host.display !== false)
    signal connectRequested(int sourceIndex, string nickname)

    implicitHeight: cards.implicitHeight

    function identityOf(host) {
        if (!host)
            return "";
        if (host.discovered)
            return host.mac ? JSON.stringify([targetEpoch, "discovered", host.mac.toLowerCase()]) : "";
        if (host.duid)
            return JSON.stringify([targetEpoch, "psn", host.duid]);
        return host.manual && host.address
            ? JSON.stringify([targetEpoch, "manual", host.address, host.mac ? host.mac.toLowerCase() : ""]) : "";
    }

    function resolveTarget(identity) {
        if (!identity || (sourceIndices.length && sourceIndices.length !== sourceHosts.length))
            return null;
        let target = null;
        for (let index = 0; index < sourceHosts.length; index++) {
            const host = sourceHosts[index];
            if (!host || host.display === false || identityOf(host) !== identity)
                continue;
            if (target)
                return null;
            const sourceIndex = sourceIndices.length ? sourceIndices[index] : index;
            if (!Number.isInteger(sourceIndex) || sourceIndex < 0)
                return null;
            target = { host, sourceIndex, nickname: host.discovered ? host.name || "" : "" };
        }
        return target;
    }

    function connectToTarget(identity) {
        const target = resolveTarget(identity);
        if (!target)
            return false;
        connectRequested(target.sourceIndex, target.nickname);
        return true;
    }

    GridLayout {
        id: cards
        width: parent.width
        columns: Math.max(1, Math.floor(width / 300))
        columnSpacing: 18
        rowSpacing: 18

        Repeater {
            model: library.displayedHosts

            delegate: Rectangle {
                id: card
                required property var modelData
                readonly property var host: modelData
                readonly property string consoleName: {
                    const name = host.name || host.address || qsTr("Console");
                    return library.streamerMode && name === host.address
                        ? qsTr("Manual console") : name;
                }

                Layout.fillWidth: true
                Layout.preferredHeight: 400
                color: "#171d26"
                radius: 15
                border.color: connectButton.visualFocus ? "#70baff" : "#2b3442"
                border.width: connectButton.visualFocus ? 2 : 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 8

                    Image {
                        objectName: "consoleArtwork"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100
                        fillMode: Image.PreserveAspectFit
                        source: typeof card.host.ps5 === "boolean"
                            ? "image://svg/console-ps" + (card.host.ps5 ? "5" : "4")
                              + (card.host.discovered && card.host.state === "ready" ? "#light_on"
                                 : card.host.discovered && card.host.state === "standby" ? "#light_standby" : "#console")
                            : ""
                        sourceSize: Qt.size(width, height)
                        Accessible.ignored: true

                        Label {
                            objectName: "consoleType"
                            anchors.centerIn: parent
                            visible: typeof card.host.ps5 !== "boolean"
                            text: qsTr("Console")
                            font.pixelSize: 20
                            color: "#aab6c7"
                        }
                    }

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
                        font.pixelSize: 14
                        text: {
                            if (!card.host.registered)
                                return qsTr("Not registered");
                            if (!card.host.discovered && card.host.duid)
                                return qsTr("PSN");
                            if (card.host.discovered && card.host.state === "ready")
                                return qsTr("Online");
                            if (card.host.discovered && card.host.state === "standby")
                                return qsTr("In standby");
                            return qsTr("Status unknown");
                        }
                        color: "#aab6c7"
                    }

                    Label {
                        objectName: "consoleRoute"
                        Layout.fillWidth: true
                        font.pixelSize: 14
                        text: card.host.discovered ? qsTr("Local network")
                            : card.host.duid ? qsTr("Remote connection via PSN") : qsTr("Manual address")
                        color: "#aab6c7"
                    }

                    Label {
                        objectName: "automaticRegistration"
                        Layout.fillWidth: true
                        font.pixelSize: 14
                        visible: !!card.host.discovered && !card.host.registered && !!card.host.duid
                        text: qsTr("Automatic registration available")
                        wrapMode: Text.Wrap
                    }

                    Item { Layout.fillHeight: true }

                    Controls.Button {
                        id: connectButton
                        objectName: "connectButton"
                        Layout.fillWidth: true
                        font.pixelSize: 16
                        text: !card.host.registered ? qsTr("Register")
                            : card.host.discovered && card.host.state === "standby"
                                ? qsTr("Wake and connect") : qsTr("Connect")
                        Accessible.name: !card.host.registered ? qsTr("Register %1").arg(card.consoleName)
                            : card.host.discovered && card.host.state === "standby"
                                ? qsTr("Wake and connect to %1").arg(card.consoleName)
                                : qsTr("Connect to %1").arg(card.consoleName)
                        focusPolicy: Qt.StrongFocus
                        enabled: library.resolveTarget(library.identityOf(card.host)) !== null
                        Material.background: Material.accent
                        onClicked: library.connectToTarget(library.identityOf(card.host))
                    }
                }
            }
        }
    }
}
