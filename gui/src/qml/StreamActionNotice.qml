import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import "controls" as C

Rectangle {
    id: notice
    objectName: "streamActionNotice"
    property bool blocked: false
    property bool audioDisabled: false
    readonly property string title: blocked ? qsTr("Content blocked by the console")
        : audioDisabled ? qsTr("Audio and video disabled") : qsTr("Video disabled")
    readonly property string message: blocked ? qsTr("This screen cannot be shown through Remote Play.")
        : qsTr("Still connected. Enable audio and video in Settings → General → Audio/Video.")
    property alias initialFocusItem: action
    signal triggered()
    implicitHeight: content.implicitHeight + 20
    radius: 7
    color: Qt.rgba(9 / 255, 14 / 255, 21 / 255, 0.65)
    border.color: "#24ffffff"
    Accessible.role: Accessible.Grouping
    Accessible.name: title + ". " + message
    ColumnLayout {
        id: content
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 13; topMargin: 10 }
        spacing: 3
        Label {
            objectName: "streamNoticeTitle"
            Layout.fillWidth: true
            text: notice.title
            color: "white"
            font.pixelSize: 13
            font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
        }
        Label {
            objectName: "streamNoticeMessage"
            Layout.fillWidth: true
            text: notice.message
            color: "#e0e5ed"
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
        C.Button {
            id: action
            objectName: "streamNoticeAction"
            Layout.topMargin: 5
            Layout.maximumWidth: content.width
            implicitHeight: Math.max(34, contentItem.implicitHeight + topPadding + bottomPadding)
            text: notice.blocked ? qsTr("Go to console home") : qsTr("Open stream menu")
            font.pixelSize: 13
            background: Rectangle {
                radius: 4
                color: action.activeFocus ? action.Material.accent : "#12ffffff"
                border.color: action.activeFocus ? "white" : "#24ffffff"
                border.width: action.activeFocus ? 2 : 1
            }
            contentItem: Label {
                text: action.text
                font: action.font
                color: "white"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            firstInFocusChain: true
            lastInFocusChain: true
            onClicked: notice.triggered()
            Keys.onReturnPressed: clicked()
            Keys.onEscapePressed: if (notice.blocked) clicked()
        }
    }
}
