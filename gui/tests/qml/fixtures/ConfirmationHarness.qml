import QtQuick
import QtQuick.Controls
import "../../../src/qml"

Item {
    id: harness
    property bool settingsMode: false
    property alias backend: backend
    property alias settings: settings
    property alias dialog: dialog
    property alias screen: screen.item
    property var actions: []
    function record(name, args) { actions = actions.concat([{name: name, args: args || []}]); }
    function restoreMainFocus() { if (!settingsMode) screen.item.restoreFocus(); }
    function showConfirmDialog(title, text, actionText, accept, cancel) {
        if (!settingsMode) screen.item.rememberFocus();
        dialog.title = title;
        dialog.text = text;
        dialog.actionText = actionText;
        dialog.callback = accept;
        dialog.rejectCallback = cancel;
        dialog.restoreFocusItem = Window.window.activeFocusItem;
        dialog.open();
    }
    FakeSettings { id: settings }
    QtObject {
        id: backend
        property alias settings: settings
        property var hosts: []
        property var hiddenHosts: []
        property var controllers: []
        property bool discoveryEnabled: true
        property bool controllerMappingInProgress: false
        signal controllerMappingSteamControllerSelected()
        function deleteHost(index) { harness.record("delete", [index]); }
        function hideHost(mac, name) { harness.record("hide", [mac, name]); }
        function unhideHost(mac) { harness.record("unhide", [mac]); }
        property QtObject window: QtObject { property int runtimeRendererBackend: 0; property int videoPreset: 1 }
    }
    Loader {
        id: screen
        anchors.fill: parent
        sourceComponent: harness.settingsMode ? settingsScreen : libraryScreen
    }
    Component { id: libraryScreen; MainView { backend: harness.backend; navigation: harness } }
    Component { id: settingsScreen; SettingsDialog { backend: harness.backend; navigation: harness; currentCategory: 4 } }
    ConfirmDialog { id: dialog; navigation: harness }
}
