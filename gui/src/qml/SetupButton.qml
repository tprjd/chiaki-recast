import QtQuick
import QtQuick.Controls.Material

ConsoleButton {
    id: actionButton
    property alias textFormat: label.textFormat
    topPadding: 10
    bottomPadding: 10
    implicitHeight: Math.max(42, contentItem.implicitHeight + topPadding + bottomPadding)
    contentItem: Text {
        id: label
        text: actionButton.text
        font: actionButton.font
        color: actionButton.Material.foreground
        wrapMode: Text.Wrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
