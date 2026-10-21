import QtQuick
import QtQuick.Controls
import "../../../src/qml"

Item {
    id: harness
    property alias prompt: prompt
    property alias backend: fakeBackend
    property alias opener: opener
    property int grabs: 0
    property int releases: 0
    QtObject {
        id: fakeBackend
        property var commands: []
        property QtObject settings: QtObject { property int disconnectAction: 0 }
        property QtObject window: QtObject { property int runtimeRendererBackend: 0 }
        function stopSession(sleep) { commands = commands.concat([sleep]); }
    }
    Button { id: opener; text: "Stream" }
    StreamDisconnectDialog {
        id: prompt
        backend: fakeBackend
        hostWindow: harness.Window.window
        onInputRequested: harness.grabs++
        onInputReleased: { harness.releases++; opener.forceActiveFocus(Qt.TabFocusReason); }
    }
}
