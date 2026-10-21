import QtQuick
import QtQuick.Controls
import Chiaki.Test 1.0
import "../../../src/qml"

Item {
    id: harness
    property alias backend: fakeBackend
    property alias settings: settings
    property alias window: streamWindow
    readonly property var screen: streamLoader.item
    property bool screenActive: true
    property alias opener: opener
    property var actions: []
    function showMainView() { actions = actions.concat(["consoles"]); }
    function openDisplaySettings() { actions = actions.concat(["display"]); }
    function openPlaceboSettings() { actions = actions.concat(["renderer"]); }
    QtObject {
        id: liveSession
        property bool connected: true
        property bool muted: true
        property bool cantDisplay: false
        property string host: "Living room"
        property real measuredBitrate: 15.2
        property real averagePacketLoss: 0.01
        property int framesLost: 4
    }
    QtObject {
        id: settings
        property int audioVolume: 64
        property real sZoomFactor: 0
        property int videoPreset: 1
        property bool showStreamStats: true
        property bool streamerMode: false
        property int audioVideoDisabled: 0
        property bool dpadTouchEnabled: false
        property int wifiDroppedNotif: 3
        property int disconnectAction: 2
        function stringForStreamMenuShortcut() { return "Menu"; }
        function stringForDpadShortcut() { return "Dpad"; }
    }
    StreamTestWindow {
        id: streamWindow
        width: harness.width
        height: harness.height
        property int runtimeRendererBackend: 0
        property int videoMode: 0
        property int videoPreset: 1
        property bool hasVideo: true
        property bool keepVideo: false
        property int droppedFrames: 3
        property real queueDepthAverage: 1.2
        property real pendingFrameAge: 0.016
        property int grabs: 0
        property int releases: 0
        signal menuRequested()
        function grabInput() { grabs++; }
        function releaseInput() { releases++; }
        function close() {
            if (settings.disconnectAction === 2) fakeBackend.sessionStopDialogRequested();
            else fakeBackend.stopSession(settings.disconnectAction === 1);
        }
    }
    QtObject {
        id: fakeBackend
        property alias settings: settings
        property alias window: streamWindow
        property var session: liveSession
        property var controllers: []
        property var connectionAttempt: ({presented: true, id: "1", phase: "ready", route: "local", targetValid: true, cleanupPending: false})
        property var sessionPinRequest: ({id: ""})
        property var commands: []
        signal sessionError(string title, string text)
        signal sessionStopDialogRequested()
        function cancelConnection(id) { commands = commands.concat([{name: "cancel", id: id}]); return true; }
        function retryConnection(id) { commands = commands.concat([{name: "retry", id: id}]); return true; }
        function dismissConnection(id) { commands = commands.concat([{name: "dismiss", id: id}]); return true; }
        function stopSession(sleep) { commands = commands.concat([{name: "stop", sleep: sleep}]); }
        function sessionGoHome() { commands = commands.concat([{name: "home"}]); }
    }
    Button { id: opener; text: "Game" }
    Loader {
        id: streamLoader
        anchors.fill: parent
        active: harness.screenActive
        sourceComponent: Component {
            StreamView {
                backend: fakeBackend
                settings: harness.settings
                streamWindow: harness.window
                navigation: harness
            }
        }
    }
}
