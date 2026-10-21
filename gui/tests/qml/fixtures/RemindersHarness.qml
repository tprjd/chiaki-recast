import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import "../../../src/qml"

Item {
    id: harness
    Material.theme: Material.Dark
    Material.accent: "#00a7ff"
    property alias backend: backend
    property alias settings: settings
    property alias dialog: reminder
    property alias pages: pages
    property bool initialAsk: false
    property var librarySelection: null
    property bool started: false
    property bool openSteamScreen: false
    property var steamResults: []
    property var routes: []
    property var prompts: []
    function restoreMainFocus() { if (pages.currentItem && pages.depth === 1) pages.currentItem.restoreFocus(); }
    function showPsnReminder() { reminder.offerPsnReminder(); }
    function showRemindDialog(title, text, remotePlay, callback) {
        if (reminder.visible) return false;
        if (pages.currentItem && typeof pages.currentItem.rememberFocus === "function") pages.currentItem.rememberFocus();
        const accepted = reminder.showReminder(title, text, remotePlay, callback, Window.window.activeFocusItem);
        if (accepted) prompts = prompts.concat([remotePlay ? "psn" : "steam"]);
        return accepted;
    }
    function showSteamShortcutDialog(fromReminder) {
        routes = routes.concat([{name: "steam", reminder: fromReminder}]);
        if (openSteamScreen) pages.push(steam, {fromReminder: fromReminder});
    }
    function closeDialog() { pages.pop(); }
    function showPSNTokenDialog(remote) { routes = routes.concat([{name: "psn", remote: remote}]); }
    FakeSettings { id: settings }
    QtObject {
        id: backend
        property alias settings: settings
        property bool autoConnect: false
        property var hosts: []
        property var controllers: []
        property bool discoveryEnabled: true
        property var createSteamShortcut: function(name, options, callback, path) {
            harness.steamResults = harness.steamResults.concat([callback]);
        }
        property QtObject window: QtObject { property bool directStream: false }
    }
    StackView {
        id: pages
        anchors.fill: parent
    }
    Component { id: library; MainView { backend: harness.backend; navigation: harness } }
    Component { id: steam; SteamShortcutDialog { backend: harness.backend; navigation: harness } }
    onStartedChanged: if (started) pages.push(library, {}, StackView.Immediate)
    RemindDialog { id: reminder; backend: harness.backend; navigation: harness }
}
