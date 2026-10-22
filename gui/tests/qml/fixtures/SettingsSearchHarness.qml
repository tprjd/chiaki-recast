import QtQuick
import QtQuick.Controls
import "../../../src/qml"

Item {
    id: harness
    width: 1280; height: 720
    property alias settings: collaborators.settings
    property alias backend: collaborators.backend
    property alias actions: collaborators.actions
    property alias pages: pages
    property alias screen: pages.currentItem
    property bool startAtLibrary: false
    function closeDialog() { pages.pop(); }
    function showSettingsDialog() { pages.push(settingsComponent); }
    function showDisplaySettingsDialog(route) { pages.push(displayComponent, {searchRoute: route || null}); }
    function showPlaceboSettingsDialog(route) { pages.push(rendererComponent, {searchRoute: route || null}); }
    function showPlaceboColorMappingDialog(route) { pages.push(colorComponent, {searchRoute: route || null}); }
    function showProfileDialog(route) { pages.push(profileComponent, {searchRoute: route || null}); }
    function showConfirmDialog(title, text, actionText, accept, cancel) { collaborators.showConfirmDialog(title, text, actionText, accept, cancel); }
    function showRegistDialog(host, ps5) { collaborators.showRegistDialog(host, ps5); }
    SettingsHarness { id: collaborators; visible: false; screenActive: false }
    StackView {
        id: pages
        anchors.fill: parent
        initialItem: harness.startAtLibrary ? libraryComponent : settingsComponent
    }
    Component { id: libraryComponent; Item { objectName: "library" } }
    Component { id: settingsComponent; SettingsDialog { backend: collaborators.backend; navigation: harness } }
    Component { id: displayComponent; DisplaySettingsDialog { backend: collaborators.backend; navigation: harness } }
    Component { id: rendererComponent; PlaceboSettingsDialog { backend: collaborators.backend; navigation: harness } }
    Component { id: colorComponent; PlaceboColorMappingDialog { backend: collaborators.backend; navigation: harness } }
    Component { id: profileComponent; ProfileDialog { backend: collaborators.backend; navigation: harness } }
}
