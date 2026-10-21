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
    function showControllerMappingDialog() { record("mapping"); }
    function showConfirmDialog(title, text, actionText, callback, cancel) { record("confirm", [title, text, actionText, callback, cancel]); }
    FakeSettings { id: fakeSettings }
    QtObject {
        id: fakeBackend
        property var settings: fakeSettings
        property var hiddenHosts: []
        property bool controllerMappingInProgress: false
        property bool controllerMappingDefaultMapping: true
        signal controllerMappingSteamControllerSelected()
        signal controllerMappingButtonSelected(string pressedButton)
        property string currentControllerType: "Synthetic controller"
        property var currentControllerMapping: []
        property bool controllerMappingAltered: false
        property bool enableAnalogStickMapping: false
        property QtObject window: QtObject {
            property int runtimeRendererBackend: 0
            property int videoPreset: 1
        }
        function beginControllerMapping(reset) { harness.record("beginMapping", [reset]); }
        function controllerMappingQuit() { harness.record("quitMapping"); controllerMappingInProgress = false; }
        function controllerMappingReset() { harness.record("resetMapping"); controllerMappingInProgress = false; }
        function controllerMappingSelectButton() { harness.record("selectButton"); }
        function controllerMappingButtonQuit() { harness.record("quitButton"); }
        function controllerMappingApply() { harness.record("applyMapping"); }
        function updateButton(value, button, slot) {
            harness.record("updateButton", [value, button, slot]);
            controllerMappingAltered = true;
            currentControllerMapping = currentControllerMapping.map(entry => {
                if (entry.buttonValue !== value) return entry;
                const buttons = entry.physicalButton.slice();
                buttons[slot] = button;
                return {buttonValue: entry.buttonValue, buttonName: entry.buttonName, physicalButton: buttons};
            });
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
