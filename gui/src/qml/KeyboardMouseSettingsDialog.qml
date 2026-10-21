import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import org.streetpea.chiaking
import "SettingsSearch.js" as Search

DialogView {
    id: dialog
    objectName: "keyboardMouseSettingsDialog"
    property var backend: Chiaki
    property var navigation: root
    property alias gridHeight: page.implicitHeight
    title: qsTr("Keyboard and mouse")
    header: ""
    buttonVisible: false
    Material.theme: Material.Dark
    function close() { navigation.closeDialog(); }
    function ensureFocusVisible() { Search.ensureFocusVisible(flick, Window.window.activeFocusItem); }
    Flickable {
        id: flick
        clip: true
        contentWidth: width
        contentHeight: page.implicitHeight + 32
        flickableDirection: Flickable.VerticalFlick
        ScrollBar.vertical: ScrollBar {}
        KeyboardMousePage {
            id: page
            x: 16; y: 16
            width: Math.max(0, flick.width - 48)
            settings: dialog.backend.settings
        }
        Connections {
            target: dialog.Window.window
            function onActiveFocusItemChanged() { dialog.ensureFocusVisible(); }
        }
    }
}
