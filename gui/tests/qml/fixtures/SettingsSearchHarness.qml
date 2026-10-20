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
    function closeDialog() { pages.pop(); }
    function showDisplaySettingsDialog(route) { pages.push(displayComponent, {searchRoute: route || null}); }
    function showPlaceboSettingsDialog(route) { pages.push(rendererComponent, {searchRoute: route || null}); }
    function showPlaceboColorMappingDialog(route) { pages.push(colorComponent, {searchRoute: route || null}); }
    function showProfileDialog(route) { pages.push(profileComponent, {searchRoute: route || null}); }
    function showControllerMappingDialog(route) { pages.push(mappingComponent, {searchRoute: route || null}); }
    function showConfirmDialog(title, text, accept, cancel) { collaborators.showConfirmDialog(title, text, accept, cancel); }
    function showRegistDialog(host, ps5) { collaborators.showRegistDialog(host, ps5); }
    SettingsHarness { id: collaborators; visible: false; screenActive: false }
    StackView {
        id: pages
        anchors.fill: parent
        initialItem: SettingsDialog { backend: collaborators.backend; navigation: harness }
    }
    Component { id: displayComponent; DisplaySettingsDialog { backend: collaborators.backend; navigation: harness } }
    Component { id: rendererComponent; PlaceboSettingsDialog { backend: collaborators.backend; navigation: harness } }
    Component { id: colorComponent; PlaceboColorMappingDialog { backend: collaborators.backend; navigation: harness } }
    Component { id: profileComponent; ProfileDialog { backend: collaborators.backend; navigation: harness } }
    Component { id: mappingComponent; ControllerMappingDialog { backend: collaborators.backend; navigation: harness } }
}
