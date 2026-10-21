import QtQuick
import QtQuick.Controls
import Chiaki.Test 1.0
import "../../../src/qml"

Item {
    id: harness
    property alias prompt: prompt
    property alias backend: fakeBackend
    property alias opener: opener
    readonly property int grabs: fakeBackend.window.streamInput.grabs
    readonly property int releases: fakeBackend.window.streamInput.releases
    QtObject {
        id: fakeBackend
        property var commands: []
        property QtObject settings: QtObject { property int disconnectAction: 0 }
        property QtObject window: QtObject { property int runtimeRendererBackend: 0; property StreamInputFixture streamInput: StreamInputFixture {} }
        function stopSession(sleep) { commands = commands.concat([sleep]); }
    }
    Button { id: opener; text: "Stream" }
    Component.onCompleted: opener.forceActiveFocus(Qt.TabFocusReason)
    StreamDisconnectDialog {
        id: prompt
        backend: fakeBackend
        hostWindow: harness.Window.window
    }
}
