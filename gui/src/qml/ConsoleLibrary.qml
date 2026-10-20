pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import "controls" as Controls

FocusScope {
    id: library

    property var hosts: []
    property var sourceIndices: []
    property bool streamerMode: false
    signal connectRequested(int sourceIndex, string nickname)

    implicitHeight: cards.implicitHeight

    GridLayout {
        id: cards
        width: parent.width
        columns: Math.max(1, Math.floor(width / 300))
        columnSpacing: 18
        rowSpacing: 18

        Repeater {
            model: library.hosts

            delegate: Rectangle {
                id: card
                required property var modelData
                required property int index
                readonly property string consoleName: {
                    const name = modelData.name || modelData.address || qsTr("Console");
                    return library.streamerMode && name === modelData.address
                        ? qsTr("Manual console") : name;
                }

                Layout.fillWidth: true
                Layout.preferredHeight: 300
                color: "#171d26"
                radius: 15
                border.color: connectButton.visualFocus ? "#70baff" : "#2b3442"
                border.width: connectButton.visualFocus ? 2 : 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 12

                    Image {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100
                        fillMode: Image.PreserveAspectFit
                        source: typeof card.modelData.ps5 === "boolean"
                            ? "image://svg/console-ps" + (card.modelData.ps5 ? "5" : "4")
                              + (card.modelData.state === "standby" ? "#light_standby" : "#light_on")
                            : ""
                        sourceSize: Qt.size(width, height)
                        Accessible.ignored: true
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
                        text: card.modelData.discovered && card.modelData.state === "ready"
                            ? qsTr("Online") : qsTr("Status unknown")
                        color: "#aab6c7"
                    }

                    Item { Layout.fillHeight: true }

                    Controls.Button {
                        id: connectButton
                        objectName: "connectButton"
                        Layout.fillWidth: true
                        text: qsTr("Connect")
                        Accessible.name: qsTr("Connect to %1").arg(card.consoleName)
                        focusPolicy: Qt.StrongFocus
                        Material.background: Material.accent
                        onClicked: library.connectRequested(
                            library.sourceIndices.length === library.hosts.length
                                ? library.sourceIndices[card.index] : card.index,
                            card.modelData.discovered ? card.modelData.name || "" : "")
                    }
                }
            }
        }
    }
}
