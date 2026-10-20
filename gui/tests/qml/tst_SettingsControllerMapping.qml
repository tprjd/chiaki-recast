import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"
import "../../src/qml"

TestCase {
    id: testCase
    name: "SettingsControllerMapping"
    width: 1280; height: 720
    visible: true
    when: windowShown
    Component {
        id: mappingHarness
        Item {
            id: root
            width: 1280; height: 720
            property alias backend: collaborators.backend
            property alias settings: collaborators.settings
            property alias actions: collaborators.actions
            property bool screenActive: false
            property int closeCount: 0
            function closeDialog() { closeCount++; screenActive = false; }
            SettingsHarness { id: collaborators; screenActive: false; visible: false }
            Loader {
                anchors.fill: parent
                active: root.screenActive
                sourceComponent: ControllerMappingDialog { backend: root.backend; navigation: root }
            }
        }
    }
    Component {
        id: navigationHarness
        Item {
            id: root
            width: 900; height: 600
            property alias backend: collaborators.backend
            function closeDialog() { pages.pop(); }
            function showControllerMappingDialog() { pages.push(mappingComponent); }
            SettingsHarness { id: collaborators; visible: false; screenActive: false }
            StackView {
                id: pages
                objectName: "controllerSettingsStack"
                anchors.fill: parent
                initialItem: SettingsDialog { backend: collaborators.backend; navigation: root }
            }
            Component { id: mappingComponent; ControllerMappingDialog { backend: collaborators.backend; navigation: root } }
        }
    }
    function openMapping(properties) {
        const harness = createTemporaryObject(mappingHarness, testCase, properties || {});
        const mappings = [];
        for (let i = 0; i < 23; i++) mappings.push({buttonValue: i + 1, buttonName: "Target " + (i + 1), physicalButton: ["b" + i, "alt" + i]});
        harness.backend.currentControllerMapping = mappings;
        harness.backend.controllerMappingInProgress = true;
        harness.screenActive = true;
        return harness;
    }
    function test_allMappingTargetsAreReachable() {
        const harness = openMapping();
        for (let target = 1; target <= 23; target++) {
            for (let slot = 0; slot < 2; slot++) verify(findChild(harness, "controllerMap" + target + "Slot" + slot));
        }
    }

    function test_dispatchEveryPrimaryAndAlternateInput() {
        const harness = openMapping();
        for (let target = 1; target <= 23; target++) {
            for (let slot = 0; slot < 2; slot++) {
                const name = "controllerMap" + target + "Slot" + slot;
                findChild(harness, name).forceActiveFocus(Qt.TabFocusReason);
                keyClick(Qt.Key_Return);
                const capture = findChild(harness, "controllerButtonCapture");
                tryCompare(capture, "opened", true);
                harness.actions = [];
                harness.backend.controllerMappingButtonSelected("input" + target + "-" + slot);
                tryCompare(capture, "visible", false);
                const updates = harness.actions.filter(action => action.name === "updateButton");
                compare(updates.length, 1);
                compare(updates[0].args[0], target);
                compare(updates[0].args[1], "input" + target + "-" + slot);
                compare(updates[0].args[2], slot);
                tryCompare(findChild(harness, name), "activeFocus", true);
                compare(findChild(harness, name).text, "input" + target + "-" + slot);
            }
        }
        verify(harness.backend.controllerMappingAltered);
    }
    function test_captureCancelAndLateInput() {
        const harness = openMapping();
        const opener = findChild(harness, "controllerMap7Slot1");
        opener.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
        const capture = findChild(harness, "controllerButtonCapture");
        tryCompare(capture, "opened", true);
        harness.actions = [];
        for (const key of [Qt.Key_Up, Qt.Key_Down, Qt.Key_Return, Qt.Key_Menu]) keyClick(key);
        verify(capture.visible);
        compare(harness.actions.length, 0);
        keyClick(Qt.Key_Escape);
        tryCompare(capture, "visible", false);
        tryCompare(opener, "activeFocus", true);
        compare(harness.closeCount, 0);
        harness.backend.controllerMappingButtonSelected("late");
        compare(harness.actions.filter(action => action.name === "updateButton").length, 0);
        compare(harness.actions.filter(action => action.name === "quitButton").length, 1);
        verify(!harness.backend.controllerMappingAltered);
    }
    function test_analogApplyAndCancel() {
        const harness = openMapping();
        const analog = findChild(harness, "enableAnalogStickMapping");
        analog.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Space);
        verify(harness.backend.enableAnalogStickMapping);
        const apply = findChild(harness, "controllerMappingApply");
        verify(!apply.enabled);
        findChild(harness, "controllerMap1Slot0").forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
        tryCompare(findChild(harness, "controllerButtonCapture"), "opened", true);
        harness.backend.controllerMappingButtonSelected("a0");
        tryCompare(findChild(harness, "controllerButtonCapture"), "visible", false);
        tryCompare(apply, "enabled", true);
        harness.actions = [];
        apply.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
        compare(harness.actions.filter(action => action.name === "applyMapping").length, 1);
        compare(harness.actions.filter(action => action.name === "quitMapping").length, 1);
        compare(harness.closeCount, 1);
        const cancelled = openMapping();
        cancelled.backend.controllerMappingAltered = true;
        findChild(cancelled, "controllerMappingCancel").forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
        compare(cancelled.actions.filter(action => action.name === "applyMapping").length, 0);
        compare(cancelled.actions.filter(action => action.name === "quitMapping").length, 1);
        compare(cancelled.closeCount, 1);
    }
    function test_removedControllerAndProfileCloseCapture() {
        for (const profileChange of [false, true]) {
            const harness = openMapping();
            findChild(harness, "controllerMap23Slot1").forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
            tryCompare(findChild(harness, "controllerButtonCapture"), "opened", true);
            harness.actions = [];
            if (profileChange) harness.settings.currentProfile = "Other";
            else harness.backend.controllerMappingInProgress = false;
            compare(harness.closeCount, 1);
            harness.backend.controllerMappingButtonSelected("late");
            compare(harness.actions.filter(action => action.name === "updateButton").length, 0);
            compare(harness.actions.filter(action => action.name === "applyMapping").length, 0);
        }
    }
    function test_layoutAndOpeningDoNotWrite_data() { return [{tag: "desktop", width: 1280, height: 720}, {tag: "minimum", width: 900, height: 600}]; }
    function test_layoutAndOpeningDoNotWrite(data) {
        const harness = openMapping({width: data.width, height: data.height});
        harness.settings.changeCount = 0;
        harness.actions = [];
        for (const name of ["enableAnalogStickMapping", "controllerMap1Slot0", "controllerMap23Slot1", "controllerMappingCancel"]) {
            const control = findChild(harness, name);
            control.forceActiveFocus(Qt.TabFocusReason); wait(10);
            const point = control.mapToItem(harness, 0, 0);
            verify(point.x >= 0 && point.x + control.width <= harness.width, name + " fits width");
            verify(point.y >= 0 && point.y + control.height <= harness.height, name + " stays visible on focus");
        }
        compare(harness.settings.changeCount, 0);
        compare(harness.actions.length, 0);
    }
    function test_directionalNavigationKeepsInputSlot() {
        const harness = openMapping();
        findChild(harness, "controllerMap1Slot0").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Down);
        verify(findChild(harness, "controllerMap2Slot0").activeFocus);
        keyClick(Qt.Key_Right);
        verify(findChild(harness, "controllerMap2Slot1").activeFocus);
        keyClick(Qt.Key_Up);
        verify(findChild(harness, "controllerMap1Slot1").activeFocus);
        keyClick(Qt.Key_Left);
        verify(findChild(harness, "controllerMap1Slot0").activeFocus);
    }
    function test_returnRestoresMappingOpener() {
        const harness = createTemporaryObject(navigationHarness, testCase);
        const pages = findChild(harness, "controllerSettingsStack");
        mouseClick(findChild(harness, "settingsCategory6"));
        const opener = findChild(harness, "controllerMappingChange");
        opener.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
        tryCompare(findChild(harness, "controllerCapture"), "opened", true);
        harness.backend.currentControllerMapping = [{buttonValue: 1, buttonName: "Cross", physicalButton: ["b0"]}];
        harness.backend.controllerMappingInProgress = true;
        tryCompare(pages, "depth", 2);
        tryCompare(pages, "busy", false);
        findChild(harness, "controllerMappingCancel").forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
        tryCompare(pages, "depth", 1);
        tryCompare(pages, "busy", false);
        tryCompare(opener, "activeFocus", true);
    }
}
