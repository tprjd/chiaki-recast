import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "controls" as C

ColumnLayout {
    id: page
    property var settings
    property alias currentTab: tabs.currentIndex
    property alias firstFocusItem: mode
    signal saved()
    spacing: 14

    C.CheckBox {
        id: mode
        objectName: "mouseCameraEnabled"
        text: qsTr("Enable mouse camera mode")
        Accessible.name: text
        Accessible.description: qsTr("During a stream, press F8 to capture or release your controls.")
        checked: page.settings.mouseCameraEnabled
        onToggled: { page.settings.mouseCameraEnabled = checked; page.saved(); }
    }
    Label {
        Layout.fillWidth: true
        text: qsTr("During a stream, press F8 to capture or release your controls.")
        wrapMode: Text.Wrap
        font.pixelSize: 13
    }
    TabBar {
        id: tabs
        objectName: "keyboardMouseTabs"
        Layout.fillWidth: true
        TabButton { text: qsTr("Mouse response"); Accessible.name: text }
        TabButton { text: qsTr("Bindings"); Accessible.name: text }
    }
    MouseResponsePane {
        Layout.fillWidth: true
        visible: tabs.currentIndex === 0
        enabled: page.settings.mouseCameraEnabled
        settings: page.settings
        onSaved: page.saved()
    }
    Loader {
        id: bindings
        Layout.fillWidth: true
        active: tabs.currentIndex === 1 && page.settings.keyboardMouseBindings !== undefined
        visible: tabs.currentIndex === 1
        enabled: page.settings.mouseCameraEnabled
        source: active ? "KeyboardMouseBindingsPane.qml" : ""
        onLoaded: { item.settings = Qt.binding(() => page.settings); item.saved.connect(page.saved); }
    }
    Label {
        objectName: "keyboardMouseSaved"
        text: qsTr("Saved · Applies the next time you press F8.")
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        font.pixelSize: 13
    }
}
