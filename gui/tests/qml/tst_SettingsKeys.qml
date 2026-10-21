import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "SettingsKeys"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; SettingsHarness {} }

    readonly property var names: ["Cross", "Moon", "Box", "Pyramid", "D-pad left", "D-pad right", "D-pad up", "D-pad down", "L1", "R1", "L3", "R3", "Options", "Share", "Touchpad", "PS", "L2", "R2", "Left stick right", "Left stick left", "Left stick up", "Left stick down", "Right stick right", "Right stick left", "Right stick up", "Right stick down"]
    function mappings(prefix) {
        return names.map((name, index) => ({buttonName: name, buttonValue: Math.pow(2, index), keyName: prefix + index}));
    }
    function createHarness(properties) {
        const harness = createTemporaryObject(harnessComponent, testCase, Object.assign({screenActive: false}, properties || {}));
        harness.settings.controllerMapping = mappings("Key ");
        harness.screenActive = true;
        harness.screen.currentCategory = 5;
        waitForRendering(harness);
        harness.actions = [];
        harness.settings.actions = [];
        harness.settings.changeCount = 0;
        return harness;
    }
    function test_captureEveryNativeMapping_data() {
        return names.map((name, index) => ({tag: name, value: Math.pow(2, index)}));
    }
    function test_captureEveryNativeMapping(data) {
        const harness = createHarness();
        const target = findChild(harness, "keyMapping" + data.value);
        verify(target);
        target.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        const capture = findChild(harness, "keyCaptureDialog");
        tryCompare(capture, "opened", true);
        keyClick(Qt.Key_Z);
        tryCompare(capture, "opened", false);
        compare(harness.settings.actions.length, 1);
        compare(harness.settings.actions[0].name, "changeControllerKey");
        compare(harness.settings.actions[0].args, [data.value, Qt.Key_Z]);
        compare(target.text, "Test key");
        tryCompare(target, "activeFocus", true);
        compare(harness.actions.length, 0);
    }

    function openCapture(harness, value) {
        const target = findChild(harness, "keyMapping" + value);
        target.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        const capture = findChild(harness, "keyCaptureDialog");
        tryCompare(capture, "opened", true);
        return capture;
    }
    function test_escapeCancelsOnlyCapture() {
        const harness = createHarness();
        const capture = openCapture(harness, 1);
        keyClick(Qt.Key_Escape);
        tryCompare(capture, "opened", false);
        compare(harness.settings.actions.length, 0);
        compare(harness.actions.length, 0);
        compare(harness.screen.currentCategory, 5);
        tryCompare(findChild(harness, "keyMapping1"), "activeFocus", true);
    }
    function test_closeCancelsWithoutWriting() {
        const harness = createHarness();
        const capture = openCapture(harness, 4);
        mouseClick(capture.standardButton(Dialog.Close));
        tryCompare(capture, "opened", false);
        compare(harness.settings.actions.length, 0);
        compare(harness.actions.length, 0);
        tryCompare(findChild(harness, "keyMapping4"), "activeFocus", true);
    }
    function test_captureEditingKeys_data() {
        return [{tag: "return", key: Qt.Key_Return}, {tag: "page-down", key: Qt.Key_PageDown},
            {tag: "up", key: Qt.Key_Up}, {tag: "tab", key: Qt.Key_Tab}, {tag: "backspace", key: Qt.Key_Backspace}];
    }
    function test_captureEditingKeys(data) {
        const harness = createHarness();
        const capture = openCapture(harness, 32);
        keyClick(data.key);
        tryCompare(capture, "opened", false);
        compare(harness.settings.actions.length, 1);
        compare(harness.settings.actions[0].args, [32, data.key]);
        compare(harness.screen.currentCategory, 5);
        compare(harness.actions.length, 0);
    }
    function test_profileAndListChangesCancelCapture_data() {
        return [{tag: "profile", profile: true}, {tag: "mapping-list", profile: false}];
    }
    function test_profileAndListChangesCancelCapture(data) {
        const harness = createHarness();
        const capture = openCapture(harness, 16);
        keyPress(Qt.Key_Z);
        if (data.profile) harness.settings.currentProfile = "Other profile";
        else harness.settings.controllerMapping = mappings("Updated ").reverse();
        tryCompare(capture, "opened", false);
        keyRelease(Qt.Key_Z);
        compare(harness.settings.actions.length, 0);
        compare(harness.actions.length, 0);
    }
    function test_modelRefreshKeepsCurrentKeyNames() {
        const harness = createHarness();
        openCapture(harness, 1);
        keyClick(Qt.Key_Z);
        tryCompare(findChild(harness, "keyMapping1"), "text", "Test key");
        harness.settings.currentProfile = "Second profile";
        harness.settings.controllerMapping = mappings("Second ");
        tryCompare(findChild(harness, "keyMapping1"), "text", "Second 0");
        compare(harness.settings.actions.length, 1);
    }
    function test_togglesResetAndNoWritesOnOpening() {
        const harness = createHarness();
        compare(harness.settings.changeCount, 0);
        compare(harness.settings.actions.length, 0);
        for (const name of ["keyboardEnabled", "mouseTouchEnabled"]) {
            const control = findChild(harness, name);
            control.forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Space);
            verify(harness.settings[name]);
        }
        compare(harness.settings.changeCount, 2);
        const reset = findChild(harness, "resetAllKeys");
        reset.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(harness.settings.actions.length, 1);
        compare(harness.settings.actions[0].name, "clearKeyMapping");
        harness.settings.controllerMapping = mappings("Default ");
        compare(findChild(harness, "keyMapping33554432").text, "Default 25");
        verify(reset.activeFocus);
    }

    function test_mouseCameraModeIsSeparateFromLegacyControls() {
        const harness = createHarness();
        const mode = findChild(harness, "mouseCameraEnabled");
        verify(mode, "The capture mode has a separate switch");
        compare(mode.checked, false);
        mode.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Space);
        compare(harness.settings.mouseCameraEnabled, true);
        compare(harness.settings.keyboardEnabled, false);
        compare(harness.settings.mouseTouchEnabled, false);
        compare(findChild(harness, "keyboardEnabled").enabled, false);
    }

    function test_completeListRemainsReachable_data() {
        return [{tag: "desktop", width: 1280, height: 720}, {tag: "minimum", width: 900, height: 600}];
    }
    function test_completeListRemainsReachable(data) {
        const harness = createHarness({width: data.width, height: data.height});
        const flick = findChild(harness, "keysFlick");
        for (let index = 0; index < 26; ++index) {
            const control = findChild(harness, "keyMapping" + Math.pow(2, index));
            verify(control, names[index] + " remains available");
            control.forceActiveFocus(Qt.TabFocusReason);
            wait(20);
            const origin = control.mapToItem(flick, 0, 0);
            verify(origin.x >= 0 && origin.x + control.width <= flick.width + 1, names[index] + " fits horizontally");
            verify(origin.y >= -1 && origin.y + control.height <= flick.height + 1, names[index] + " scrolls into view");
        }
        compare(harness.settings.actions.length, 0);
        compare(harness.settings.changeCount, 0);
    }
    function test_longLabelsAndKeyboardNavigation() {
        const harness = createHarness({width: 900, height: 600});
        const keyboard = findChild(harness, "keyboardEnabled");
        const mouse = findChild(harness, "mouseTouchEnabled");
        keyboard.text = "Use the keyboard as a controller with all of the configured key assignments";
        mouse.text = "Use mouse input for the console touchpad during the remote play session";
        wait(30);
        const flick = findChild(harness, "keysFlick");
        for (const control of [keyboard, mouse]) {
            const origin = control.mapToItem(flick, 0, 0);
            verify(origin.x >= 0 && origin.x + control.width <= flick.width + 1);
        }
        keyboard.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Tab);
        verify(mouse.activeFocus);
        keyClick(Qt.Key_Tab, Qt.ShiftModifier);
        verify(keyboard.activeFocus);
        keyClick(Qt.Key_Down);
        verify(mouse.activeFocus);
        compare(harness.settings.changeCount, 0);
    }
}
