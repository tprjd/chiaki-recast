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
    function showRegistDialog() { record("register"); }
    function showDisplaySettingsDialog() { record("display"); }
    function showPlaceboSettingsDialog() { record("renderer"); }
    function showProfileDialog() { record("profiles"); }
    function showPSNTokenDialog(remote) { record("login", [remote]); }
    function showControllerMappingDialog() { record("mapping"); }
    function showConfirmDialog(title, text, callback) { record("confirm", [title, text, callback]); }
    FakeSettings { id: fakeSettings }
    QtObject {
        id: fakeBackend
        property var settings: fakeSettings
        property var hiddenHosts: []
        property bool controllerMappingInProgress: false
        property bool controllerMappingDefaultMapping: true
        signal controllerMappingSteamControllerSelected()
        property QtObject window: QtObject {
            property int runtimeRendererBackend: 0
            property int videoPreset: 1
        }
        function beginControllerMapping(reset) { harness.record("beginMapping", [reset]); }
        function controllerMappingQuit() { harness.record("quitMapping"); }
        function controllerMappingReset() { harness.record("resetMapping"); }
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
