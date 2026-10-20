import QtQuick
import QtQuick.Controls.Material
import "controls" as Controls

Controls.Button {
    id: button
    property bool primary: false
    implicitHeight: 42
    topInset: 0
    bottomInset: 0
    leftPadding: 14
    rightPadding: 14
    font.pixelSize: 14
    icon.width: 20
    icon.height: 20
    Material.foreground: primary ? "#101c29" : highlighted ? "#69b8ff" : "#e7ebf2"
    background: Rectangle {
        radius: 8
        color: button.primary ? (button.enabled ? "#69b8ff" : "#485361")
            : button.down || button.hovered || button.visualFocus || button.highlighted ? "#1d354c" : "transparent"
        border.color: button.visualFocus ? (button.primary ? "#e4f3ff" : "#69b8ff") : "#3b4655"
        border.width: button.visualFocus ? 2 : 1
    }
}
