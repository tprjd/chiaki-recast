import QtQuick
import "../../../src/qml"

Item {
    id: harness
    width: 1280
    height: 720
    property alias backend: fakeBackend
    property bool screenActive: true
    property bool wakeScreen: false
    readonly property var screen: screenLoader.item
    property var actions: []
    function showMainView() { actions = actions.concat(["consoles"]); }
    QtObject {
        id: settings
        property bool streamerMode: false
    }
    QtObject {
        id: fakeBackend
        property alias settings: settings
        property var connectionAttempt: ({id: "1", phase: "connecting", route: "local", name: "Living room",
            address: "192.0.2.42", ps5: true, cleanupPending: true, targetValid: true, errorTitle: "", errorText: ""})
        property var commands: []
        function cancelConnection(id) { commands = commands.concat([{name: "cancel", id: id}]); return true; }
        function retryConnection(id) { commands = commands.concat([{name: "retry", id: id}]); return true; }
        function dismissConnection(id) { commands = commands.concat([{name: "dismiss", id: id}]); return true; }
        function change(values) { connectionAttempt = Object.assign({}, connectionAttempt, values); }
    }
    Loader {
        id: screenLoader
        anchors.fill: parent
        active: harness.screenActive
        sourceComponent: harness.wakeScreen ? wakeComponent : localComponent
    }
    Component { id: localComponent; ConnectionView { backend: fakeBackend; navigation: harness } }
    Component { id: wakeComponent; AutoConnectView { backend: fakeBackend; navigation: harness } }
}
