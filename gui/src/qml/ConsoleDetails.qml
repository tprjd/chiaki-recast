pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "controls" as Controls

Popup {
    id: details
    property var host: ({})
    property bool streamerMode: false
    property var availableActions: []
    property bool playStationController: false
    signal managementRequested(string action)
    readonly property string consoleName: {
        const name = host.name || host.address || qsTr("Console");
        return streamerMode && name === host.address ? qsTr("Manual console") : name;
    }
    readonly property var fields: {
        const rows = [];
        if (host.address) rows.push(qsTr("Address: %1").arg(streamerMode ? qsTr("hidden") : host.address));
        if (host.mac || host.duid) rows.push(qsTr("ID: %1").arg(streamerMode ? qsTr("hidden") : host.mac || host.duid));
        if (typeof host.registered === "boolean")
            rows.push(qsTr("Registration: %1").arg(host.registered ? qsTr("Registered") : qsTr("Not registered")));
        const route = host.discovered ? qsTr("Local network")
            : host.duid ? qsTr("Remote connection via PSN") : host.manual ? qsTr("Manual address") : "";
        if (route) rows.push(qsTr("Route: %1").arg(route));
        if (host.app) rows.push(qsTr("App: %1").arg(host.app));
        if (host.titleId) rows.push(qsTr("Title ID: %1").arg(host.titleId));
        return rows;
    }
    objectName: "consoleDetails"
    width: Math.min(420, parent.width - 32)
    height: Math.min(implicitHeight, parent.height - 32)
    anchors.centerIn: parent
    padding: 24
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    Overlay.modal: Rectangle { color: "#99000000" }
    background: Rectangle { color: "#181d25"; radius: 12; border.color: "#445164" }
    onOpened: {
        closeButton.focus = false;
        closeButton.forceActiveFocus(Qt.TabFocusReason);
    }

    contentItem: ColumnLayout {
        spacing: 16
        Keys.onPressed: event => {
            let action = "";
            if (event.key === Qt.Key_PageUp) action = "pin";
            else if (event.key === Qt.Key_Yes) action = "wake";
            else if (event.key === Qt.Key_No)
                action = details.availableActions.includes("delete") ? "delete" : "hide";
            else if (event.key === Qt.Key_Escape) details.close();
            if (!event.modifiers && details.availableActions.includes(action))
                details.managementRequested(action);
            event.accepted = true;
        }
        ScrollView {
            objectName: "detailsScroll"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 0
            implicitHeight: detailsInfo.implicitHeight
            contentWidth: availableWidth
            clip: true
            ColumnLayout {
                id: detailsInfo
                width: parent.width
                spacing: 16
                Label {
                    Layout.fillWidth: true
                    text: details.consoleName
                    textFormat: Text.PlainText
                    font.pixelSize: 20
                    font.bold: true
                    wrapMode: Text.Wrap
                    Accessible.name: text
                }
                Repeater {
                    model: details.fields
                    Label {
                        required property string modelData
                        Layout.fillWidth: true
                        text: modelData
                        textFormat: Text.PlainText
                        font.pixelSize: 14
                        wrapMode: Text.Wrap
                        Accessible.name: text
                    }
                }
            }
        }
        Flow {
            Layout.fillWidth: true
            visible: details.availableActions.length > 0
            spacing: 8
            Repeater {
                model: details.availableActions
                Controls.Button {
                    id: managementButton
                    required property string modelData
                    objectName: modelData + "Action"
                    text: modelData === "pin" ? qsTr("Update Console Pin") : modelData === "wake" ? qsTr("Wake Up")
                        : modelData === "delete" ? qsTr("Delete") : qsTr("Hide")
                    icon.source: modelData === "pin" ? "qrc:/icons/l1.svg"
                        : "image://svg/button-" + (details.playStationController ? "ps" : "deck")
                          + (modelData === "wake" ? "#pyramid" : "#box")
                    background: Rectangle {
                        radius: 8
                        color: managementButton.visualFocus ? "#1d354c" : "transparent"
                        border.color: managementButton.visualFocus ? "#69b8ff" : "transparent"
                        border.width: 2
                    }
                    onClicked: details.managementRequested(modelData)
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Controls.Button {
                id: closeButton
                objectName: "closeDetailsButton"
                text: qsTr("Close")
                background: Rectangle {
                    radius: 8
                    color: closeButton.visualFocus ? "#1d354c" : "transparent"
                    border.color: closeButton.visualFocus ? "#69b8ff" : "transparent"
                    border.width: 2
                }
                onClicked: details.close()
            }
        }
    }
}
