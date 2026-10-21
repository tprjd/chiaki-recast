import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: notice
    property string title
    property string message
    property string symbol
    property bool warning: false
    implicitWidth: content.implicitWidth + 26
    implicitHeight: content.implicitHeight + 20
    radius: 7
    color: Qt.rgba(9 / 255, 14 / 255, 21 / 255, 0.65)
    border.color: "#24ffffff"
    Accessible.role: Accessible.StaticText
    Accessible.name: title + ". " + message
    Rectangle {
        visible: notice.warning
        width: 2; height: parent.height - 10
        y: 5
        color: "#e9be79"
    }
    RowLayout {
        id: content
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 13; topMargin: 10 }
        spacing: 10
        Label {
            visible: text.length > 0
            text: notice.symbol
            color: notice.warning ? "#ffd99a" : "#e0e5ed"
            font.pixelSize: 15
        }
        ColumnLayout {
            spacing: 3
            Layout.fillWidth: true
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
                visible: text.length > 0
                Layout.fillWidth: true
                text: notice.message
                color: "#e0e5ed"
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
        }
    }
}
