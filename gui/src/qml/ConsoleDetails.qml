pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: details
    property var host: ({})
    property bool streamerMode: false
    property var availableActions: []
    property bool playStationController: false
    signal managementRequested(string action)
    signal connectRequested()
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
    readonly property color statusColor: host.discovered && host.state === "ready" ? "#86d6b5"
        : host.discovered && host.state === "standby" ? "#dfbd78" : "#aab6c7"
    readonly property var fields: {
        const rows = [];
        if (typeof host.ps5 === "boolean") rows.push({ label: qsTr("Console"), value: host.ps5 ? "PS5" : "PS4" });
        if (typeof host.registered === "boolean")
            rows.push({ label: qsTr("Registration"), value: host.registered ? qsTr("Registered") : qsTr("Not registered") });
        if (host.address) rows.push({ label: qsTr("Address"), value: streamerMode ? qsTr("hidden") : host.address });
        const route = host.discovered ? qsTr("Local network")
            : host.duid ? qsTr("Remote connection via PSN") : host.manual ? qsTr("Manual address") : "";
        if (route) rows.push({ label: qsTr("Connection"), value: route });
        if (host.mac || host.duid)
            rows.push({ label: qsTr("ID"), value: streamerMode ? qsTr("hidden") : host.mac || host.duid, wide: true });
        if (host.app) rows.push({ label: qsTr("App"), value: host.app });
        if (host.titleId) rows.push({ label: qsTr("Title ID"), value: host.titleId });
        return rows;
    }
    objectName: "consoleDetails"
    width: Math.min(480, parent.width - 32)
    height: Math.min(implicitHeight, parent.height - 32)
    anchors.centerIn: parent
    padding: 28
    modal: true
    dim: false
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    background: Rectangle { color: "#181d25"; radius: 18; border.color: "#3b4655" }
    onOpened: {
        closeButton.focus = false;
        closeButton.forceActiveFocus(Qt.TabFocusReason);
    }

    contentItem: ColumnLayout {
        spacing: 20
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
        RowLayout {
            Layout.fillWidth: true
            spacing: 16
            Label {
                objectName: "detailsConsoleName"
                Layout.fillWidth: true
                text: details.consoleName
                textFormat: Text.PlainText
                font.pixelSize: 24
                font.bold: true
                wrapMode: Text.Wrap
                maximumLineCount: 2
                elide: Text.ElideRight
                Accessible.name: text
            }
            ConsoleButton {
                id: closeButton
                objectName: "closeDetailsButton"
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 36
                Layout.preferredHeight: 36
                leftPadding: 0
                rightPadding: 0
                text: qsTr("Close")
                display: AbstractButton.IconOnly
                icon.source: "qrc:/icons/close-24px.svg"
                onClicked: details.close()
            }
        }
        RowLayout {
            spacing: 8
            Rectangle { width: 6; height: 6; radius: 3; color: details.statusColor }
            Label {
                objectName: "detailsStatus"
                text: details.statusText
                font.pixelSize: 15
                color: details.statusColor
            }
        }
        ScrollView {
            objectName: "detailsScroll"
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 0
            implicitHeight: detailsInfo.implicitHeight
            contentWidth: availableWidth
            clip: true
            GridLayout {
                id: detailsInfo
                width: parent.width
                columns: details.width < 440 ? 1 : 2
                columnSpacing: 24
                rowSpacing: 20
                Repeater {
                    model: details.fields
                    ColumnLayout {
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                        Layout.columnSpan: modelData.wide ? detailsInfo.columns : 1
                        spacing: 8
                        Label {
                            objectName: "detailsFieldLabel"
                            Layout.fillWidth: true
                            text: parent.modelData.label
                            textFormat: Text.PlainText
                            font.pixelSize: 13
                            color: "#94a0b0"
                        }
                        Label {
                            objectName: "detailsFieldValue"
                            Layout.fillWidth: true
                            text: parent.modelData.value
                            textFormat: Text.PlainText
                            font.pixelSize: 15
                            wrapMode: Text.Wrap
                            Accessible.name: text
                        }
                    }
                }
            }
        }
        GridLayout {
            Layout.alignment: Qt.AlignRight
            columns: details.availableWidth < 360 ? 1 : 2
            columnSpacing: 10
            rowSpacing: 10
            Repeater {
                model: details.availableActions
                ConsoleButton {
                    required property string modelData
                    objectName: modelData + "Action"
                    text: modelData === "pin" ? qsTr("Console PIN") : modelData === "wake" ? qsTr("Wake up")
                        : modelData === "delete" ? qsTr("Delete") : qsTr("Hide")
                    icon.source: modelData === "pin" ? "qrc:/icons/l1.svg"
                        : "image://svg/button-" + (details.playStationController ? "ps" : "deck")
                          + (modelData === "wake" ? "#pyramid" : "#box")
                    onClicked: details.managementRequested(modelData)
                }
            }
            ConsoleButton {
                objectName: "detailsConnectButton"
                primary: true
                text: !details.host.registered ? qsTr("Register")
                    : details.host.discovered && details.host.state === "standby" ? qsTr("Wake and connect") : qsTr("Connect")
                icon.source: "qrc:/icons/arrow-right-24px.svg"
                onClicked: details.connectRequested()
            }
        }
    }
}
