pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "controls" as Controls

Popup {
    id: details
    property var host: ({})
    property bool streamerMode: false
    property bool managementAvailable: false
    signal managementRequested(Item opener)
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
                        font.pixelSize: 14
                        wrapMode: Text.Wrap
                        Accessible.name: text
                    }
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Controls.Button {
                id: manageButton
                objectName: "consoleManageButton"
                visible: details.managementAvailable
                text: qsTr("Manage")
                onClicked: details.managementRequested(manageButton)
            }
            Item { Layout.fillWidth: true }
            Controls.Button {
                id: closeButton
                objectName: "closeDetailsButton"
                text: qsTr("Close")
                onClicked: details.close()
            }
        }
    }
}
