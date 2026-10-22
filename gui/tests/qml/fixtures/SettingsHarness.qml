import QtQuick
import "../../../src/qml"

Item {
    id: harness
    width: 1280
    height: 720
    property alias settings: fakeSettings
    property alias backend: fakeBackend
    property alias screen: screenLoader.item
    property bool screenActive: true
    property var actions: []
    function record(name, args) { actions = actions.concat([{name: name, args: args || []}]); }
    function closeDialog() { record("close"); }
    function showRegistDialog(host, ps5) { record("register", [host, ps5]); }
    function showDisplaySettingsDialog() { record("display"); }
    function showPlaceboSettingsDialog() { record("renderer"); }
    function showProfileDialog() { record("profiles"); }
    function showPSNTokenDialog(remote) { record("login", [remote]); }
    function showConfirmDialog(title, text, actionText, callback, cancel) { record("confirm", [title, text, actionText, callback, cancel]); }
    FakeSettings { id: fakeSettings }
    QtObject {
        id: fakeBackend
        property var settings: fakeSettings
        property var hiddenHosts: []
        property QtObject window: QtObject {
            property int runtimeRendererBackend: 0
            property int videoPreset: 1
        }
        function unhideHost(mac) { harness.record("unhide", [mac]); }
    }
    Loader {
        id: screenLoader
        anchors.fill: parent
        active: harness.screenActive
        sourceComponent: SettingsDialog {
            backend: fakeBackend
            navigation: harness
            openExternal: function(url) { harness.record("openExternal", [url]); }
        }
    }
}
