import QtQuick
import QtQuick.Controls
import "../../../src/qml"

Item {
    id: harness
    property alias library: library
    property alias dialog: loader.item
    property bool streamerMode: false
    property int saveCount: 0
    property int savedIndex: -1
    property string savedPin: ""
    property bool saveOutcome: true
    property int connectionCount: 0
    property var acceptPin
    property var cancelPin
    property var consoleName
    ConsoleLibrary {
        id: library
        anchors.fill: parent
        visible: !loader.active
        streamerMode: harness.streamerMode
        savePin: (index, pin) => {
            harness.saveCount++;
            harness.savedIndex = index;
            harness.savedPin = pin;
            return harness.saveOutcome;
        }
        onConnectRequested: harness.connectionCount++
        onManagementRequested: (action, accept, cancel, name) => {
            if (loader.active) return;
            harness.acceptPin = accept;
            harness.cancelPin = cancel;
            harness.consoleName = name;
            loader.active = true;
        }
    }
    Loader {
        id: loader
        anchors.fill: parent
        active: false
        sourceComponent: ConsolePinDialog {
            submitPin: harness.acceptPin
            cancelPin: harness.cancelPin
            consoleName: harness.consoleName()
            privateMode: harness.streamerMode
            navigation: ({closeDialog: () => { loader.active = false; library.restoreFocus(); }})
        }
    }
}
