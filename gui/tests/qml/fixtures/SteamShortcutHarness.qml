import QtQuick
import "../../../src/qml"

Item {
    id: harness
    property var backend
    property alias library: library
    property alias dialog: loader.item
    ConsoleLibrary {
        id: library
        anchors.fill: parent
        steamAvailable: true
        visible: !loader.active
        onSteamShortcutRequested: { rememberFocus(); loader.active = true; }
    }
    Loader {
        id: loader
        anchors.fill: parent
        active: false
        sourceComponent: SteamShortcutDialog {
            backend: harness.backend
            navigation: ({closeDialog: () => { loader.active = false; library.restoreFocus(); }})
        }
    }
}
