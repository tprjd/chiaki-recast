import QtQuick
import "../../../src/qml"

Item {
    id: harness
    width: 1280
    height: 720
    property alias settings: fakeSettings
    property alias backend: fakeBackend
    property alias screen: screen
    property var actions: []
    property var registeredHosts: []
    function closeDialog() { actions = actions.concat([{name: "close"}]); }
    QtObject {
        id: fakeSettings
        property var registeredHosts: harness.registeredHosts
        property string currentProfile: "Test profile"
        property bool streamerMode: false
    }
    QtObject {
        id: fakeBackend
        property var settings: fakeSettings
        property bool rejectSave: false
        function addManualHost(index, address) {
            harness.actions = harness.actions.concat([{name: "add", index: index, address: address}]);
            if (rejectSave) throw new Error("Synthetic save rejection");
        }
    }
    ManualHostDialog {
        id: screen
        anchors.fill: parent
        backend: fakeBackend
        navigation: harness
    }
}
