import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "SettingsControllers"
    width: 1280; height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; SettingsHarness {} }
    function choose(combo, index) {
        combo.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", true);
        keyClick(Qt.Key_Home);
        for (let i = 0; i < index; i++) keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", false);
    }
    function openControllers(properties) {
        const harness = createTemporaryObject(harnessComponent, testCase, properties || {});
        mouseClick(findChild(harness, "settingsCategory6"));
        return harness;
    }
    function test_controllerValues() {
        const harness = openControllers();
        const background = findChild(harness, "allowJoystickBackgroundEvents");
        verify(background, "Background input setting is reachable");
        background.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Space);
        verify(harness.settings.allowJoystickBackgroundEvents);
    }

    function test_allControllerSettings() {
        const harness = openControllers();
        harness.settings.dpadTouchIncrement = 30;
        harness.settings.hapticOverride = 1;
        const position = findChild(harness, "buttonsByPosition");
        position.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Space);
        verify(harness.settings.buttonsByPosition);
        verify(!findChild(harness, "dpadTouchIncrement").visible);
        const dpad = findChild(harness, "dpadTouchEnabled");
        dpad.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Space);
        verify(harness.settings.dpadTouchEnabled);
        for (let i = 1; i <= 4; i++) {
            const combo = findChild(harness, "dpadTouchShortcut" + i);
            compare(combo.count, 17);
            choose(combo, i * 3);
            compare(harness.settings["dpadTouchShortcut" + i], i * 3);
        }
        const increment = findChild(harness, "dpadTouchIncrement");
        compare(findChild(harness, "dpadTouchIncrementValue").text, "0.3 mm");
        increment.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Right);
        compare(harness.settings.dpadTouchIncrement, 31);
        mouseClick(increment, increment.width - increment.rightPadding, increment.height / 2);
        compare(harness.settings.dpadTouchIncrement, 1079);
        mouseClick(increment, increment.leftPadding, increment.height / 2);
        compare(harness.settings.dpadTouchIncrement, 1);
        const rumble = findChild(harness, "rumbleHapticsIntensity");
        compare(rumble.count, 6);
        for (let i = 0; i < 6; i++) { choose(rumble, i); compare(harness.settings.rumbleHapticsIntensity, i); }
        const haptics = findChild(harness, "hapticOverride");
        haptics.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Right);
        fuzzyCompare(harness.settings.hapticOverride, 1.1, 0.001);
        compare(findChild(harness, "hapticOverrideValue").text, "110% of console setting");
        mouseClick(haptics, haptics.width - haptics.rightPadding, haptics.height / 2);
        compare(harness.settings.hapticOverride, 2);
        mouseClick(haptics, haptics.leftPadding, haptics.height / 2);
        compare(harness.settings.hapticOverride, 0);
        dpad.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Space);
        verify(!increment.visible);
        verify(!findChild(harness, "dpadTouchShortcut4").visible);
    }
    function test_cancelSelectionStopsDelayedOpen() {
        const harness = openControllers();
        const opener = findChild(harness, "controllerMappingChange");
        opener.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
        const capture = findChild(harness, "controllerCapture");
        verify(capture);
        tryCompare(capture, "opened", true);
        compare(harness.actions[0].name, "beginMapping");
        harness.backend.controllerMappingInProgress = true;
        keyClick(Qt.Key_Escape);
        tryCompare(capture, "visible", false);
        wait(150);
        compare(harness.actions.filter(action => action.name === "mapping").length, 0);
        compare(harness.actions.filter(action => action.name === "close").length, 0);
        compare(harness.actions.filter(action => action.name === "quitMapping").length, 1);
        tryCompare(opener, "activeFocus", true);
    }
    function test_selectAndResetController() {
        const harness = openControllers();
        findChild(harness, "controllerMappingChange").forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
        const capture = findChild(harness, "controllerCapture");
        tryCompare(capture, "opened", true);
        harness.backend.controllerMappingInProgress = true;
        tryVerify(() => harness.actions.some(action => action.name === "mapping"));
        verify(harness.backend.controllerMappingInProgress);
        compare(harness.actions.filter(action => action.name === "quitMapping").length, 0);
        harness.backend.controllerMappingInProgress = false;
        harness.actions = [];
        findChild(harness, "controllerMappingReset").forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
        tryCompare(capture, "opened", true);
        compare(harness.actions[0].args[0], true);
        harness.backend.controllerMappingDefaultMapping = false;
        harness.backend.controllerMappingInProgress = true;
        tryVerify(() => harness.actions.some(action => action.name === "resetMapping"));
        compare(harness.actions.filter(action => action.name === "mapping").length, 0);
    }
    function test_steamControllerAndProfileCancellation() {
        const harness = openControllers();
        findChild(harness, "controllerMappingChange").forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
        const capture = findChild(harness, "controllerCapture");
        tryCompare(capture, "opened", true);
        harness.backend.controllerMappingSteamControllerSelected();
        const steam = findChild(harness, "steamControllerNotice");
        tryCompare(steam, "opened", true);
        keyClick(Qt.Key_Escape);
        tryCompare(steam, "visible", false);
        compare(harness.actions.filter(action => action.name === "mapping").length, 0);
        findChild(harness, "controllerMappingChange").forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
        tryCompare(capture, "opened", true);
        harness.settings.currentProfile = "Other";
        tryCompare(capture, "visible", false);
        verify(!harness.backend.controllerMappingInProgress);
    }
    function test_layoutAndNoWrites_data() { return [{tag: "desktop", width: 1280, height: 720}, {tag: "minimum", width: 900, height: 600}]; }
    function test_layoutAndNoWrites(data) {
        const harness = openControllers({width: data.width, height: data.height});
        harness.settings.dpadTouchEnabled = true;
        harness.settings.changeCount = 0;
        for (const name of ["controllerMappingChange", "controllerMappingReset", "allowJoystickBackgroundEvents", "buttonsByPosition", "dpadTouchEnabled", "dpadTouchIncrement", "dpadTouchShortcut4", "rumbleHapticsIntensity", "hapticOverride"]) {
            const control = findChild(harness, name);
            control.forceActiveFocus(Qt.TabFocusReason); wait(10);
            const point = control.mapToItem(harness, 0, 0);
            verify(point.x >= 0 && point.x + control.width <= harness.width, name + " fits width");
            verify(point.y >= 0 && point.y + control.height <= harness.height, name + " stays visible on focus");
        }
        compare(harness.settings.changeCount, 0);
        compare(harness.actions.length, 0);
    }
}
