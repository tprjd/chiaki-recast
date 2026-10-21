import QtQuick
import QtQuick.Controls
import "../../../src/qml"

Item {
    id: harness
    width: 1280
    height: 720
    property alias backend: fakeBackend
    property alias prompt: prompt
    property bool screenActive: true
    property var actions: []
    function showMainView() { actions = actions.concat(["consoles"]); }
    Button { id: opener; objectName: "livePinOpener"; text: "Connect"; enabled: !prompt.blocking }
    QtObject { id: settings; property bool streamerMode: false }
    QtObject {
        id: window
        property int grabs: 0
        property int releases: 0
        property int runtimeRendererBackend: 0
        function grabInput() { grabs++; }
        function releaseInput() { releases++; }
    }
    QtObject {
        id: fakeBackend
        property alias settings: settings
        property alias window: window
        property var sessionPinRequest: ({})
        property var connectionAttempt: ({id: "1", route: "local", phase: "connecting"})
        property var commands: []
        property string received: ""
        property string saved: "unchanged synthetic saved PIN"
        function request(incorrect = false) {
            sessionPinRequest = {id: "request1", attemptId: "1", phase: "form", incorrect: incorrect, name: "Living room"};
        }
        function change(values) { sessionPinRequest = Object.assign({}, sessionPinRequest, values); }
        function enterPin(id, value) {
            if (id !== sessionPinRequest.id || sessionPinRequest.phase !== "form") return false;
            commands = commands.concat([{name: "submit", id: id}]);
            received = value;
            change({phase: "submitting"});
            return true;
        }
        function cancelSessionPin(id) {
            if (id !== sessionPinRequest.id || ["form", "submitting"].indexOf(sessionPinRequest.phase) < 0) return false;
            commands = commands.concat([{name: "cancel", id: id}]);
            change({phase: "cancelling"});
            return true;
        }
        function dismissSessionPin(id) {
            if (id !== sessionPinRequest.id || sessionPinRequest.phase !== "cancelled") return false;
            commands = commands.concat([{name: "dismiss", id: id}]);
            sessionPinRequest = ({});
            return true;
        }
    }
    LiveConsolePinHost {
        id: prompt
        anchors.fill: parent
        backend: fakeBackend
        navigation: harness
        hostWindow: harness.Window.window
        enabled: harness.screenActive
    }
    Component.onCompleted: opener.forceActiveFocus()
}
