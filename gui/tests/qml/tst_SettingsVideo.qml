import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "SettingsVideo"
    width: 1280
    height: 720
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
    function openVideo(properties) {
        const harness = createTemporaryObject(harnessComponent, testCase, properties || {});
        if (harness.width < 760) choose(findChild(harness, "settingsCategorySelector"), 1);
        else mouseClick(findChild(harness, "settingsCategory1"));
        return harness;
    }
    function test_videoChoices() {
        const harness = openVideo();
        const decoder = findChild(harness, "decoder");
        verify(decoder, "Hardware decoder is reachable");
        choose(decoder, 1);
        compare(harness.settings.decoder, "d3d11va");
        choose(decoder, 0);
        compare(harness.settings.decoder, "");
        const windowType = findChild(harness, "windowType");
        compare(windowType.count, 6);
        for (let i = 0; i < 6; i++) {
            choose(windowType, i);
            compare(harness.settings.windowType, i);
        }
    }

    function test_videoActionsAndValues() {
        const harness = openVideo();
        harness.settings.changeCount = 0;
        harness.settings.actions = [];
        for (const name of ["useZeroCopy", "fullscreenDoubleClick", "hideCursor"]) {
            const control = findChild(harness, name);
            control.forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Space);
            compare(harness.settings[name], true);
        }
        const sync = findChild(harness, "vSyncEnabled");
        sync.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Space);
        compare(harness.settings.vSyncEnabled, false);
        compare(harness.settings.actions.length, 0);
        harness.backend.window.runtimeRendererBackend = 1;
        keyClick(Qt.Key_Space);
        compare(harness.settings.actions[0].name, "restartApplication");
        harness.settings.actions = [];
        const renderer = findChild(harness, "rendererBackend");
        choose(renderer, 0);
        compare(harness.settings.actions.length, 0);
        choose(renderer, 1);
        compare(harness.settings.rendererBackend, 1);
        compare(harness.settings.actions.length, 1);
        compare(harness.settings.actions[0].name, "restartApplication");
        const preset = findChild(harness, "videoPreset");
        compare(preset.count, 6);
        for (let i = 0; i < 6; i++) {
            choose(preset, i);
            compare(harness.settings.videoPreset, i);
            compare(harness.backend.window.videoPreset, i);
        }
        const queue = findChild(harness, "queueDepthLimit");
        queue.forceActiveFocus(Qt.TabFocusReason);
        for (let i = 0; i < 3; i++) keyClick(Qt.Key_Right);
        compare(harness.settings.queueDepthLimit, 5);
        for (let i = 0; i < 3; i++) keyClick(Qt.Key_Left);
        compare(harness.settings.queueDepthLimit, 2);
        findChild(harness, "customRendererSettings").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        findChild(harness, "displaySettings").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(harness.actions[0].name, "renderer");
        compare(harness.actions[1].name, "display");
    }
    function test_dimensionsAndProfileChange() {
        const harness = openVideo();
        choose(findChild(harness, "windowType"), 1);
        for (const name of ["customResolutionWidth", "customResolutionHeight"]) {
            const input = findChild(harness, name);
            input.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
            keyClick(Qt.Key_A, Qt.ControlModifier);
            for (let i = 0; i < 4; i++) keyClick(Qt.Key_9);
            keyClick(Qt.Key_Tab);
            compare(harness.settings[name], 9999);
            input.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
            keyClick(Qt.Key_A, Qt.ControlModifier);
            keyClick(Qt.Key_X);
            keyClick(Qt.Key_Tab);
            compare(harness.settings[name], 0);
            compare(input.text, "");
            input.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return);
            keyClick(Qt.Key_A, Qt.ControlModifier);
            keyClick(Qt.Key_1); keyClick(Qt.Key_2); keyClick(Qt.Key_3); keyClick(Qt.Key_4);
            harness.settings.currentProfile = "Other profile " + name;
            harness.settings[name] = 640;
            harness.settings.changeCount = 0;
            keyClick(Qt.Key_Tab);
            compare(harness.settings[name], 640);
            compare(input.text, "640");
            compare(harness.settings.changeCount, 0);
        }
    }
    function test_responsiveVideo_data() {
        return [{tag: "desktop", width: 1280, height: 720}, {tag: "narrow", width: 800, height: 600}, {tag: "minimum", width: 390, height: 600}];
    }
    function test_responsiveVideo(data) {
        const harness = openVideo({width: data.width, height: data.height});
        choose(findChild(harness, "windowType"), 1);
        harness.settings.availableDecoders = ["Auto", "Long decoder name with details for a translated interface"];
        for (const name of ["decoder", "windowType", "customResolutionWidth", "customResolutionHeight", "vSyncEnabled", "videoPreset", "queueDepthLimit", "rendererBackend", "displaySettings"]) {
            const control = findChild(harness, name);
            control.forceActiveFocus(Qt.TabFocusReason);
            wait(20);
            const point = control.mapToItem(harness, 0, 0);
            verify(point.x >= 0 && point.x + control.width <= harness.width, name + " fits width");
            verify(point.y >= 0 && point.y + control.height <= harness.height, name + " stays visible on focus");
        }
    }
    function test_navigationAndDeviceRefreshDoNotWrite() {
        const harness = createTemporaryObject(harnessComponent, testCase, {screenActive: false});
        harness.settings.decoder = "Unavailable decoder";
        harness.settings.customResolutionWidth = 1920;
        harness.settings.customResolutionHeight = 1080;
        harness.settings.changeCount = 0;
        harness.settings.actions = [];
        harness.screenActive = true;
        mouseClick(findChild(harness, "settingsCategory1"));
        compare(harness.settings.changeCount, 0);
        compare(harness.settings.actions.filter(action => action.name !== "refreshAudioDevices").length, 0);
        harness.settings.availableDecoders = ["Auto"];
        harness.settings.changeCount = 0;
        wait(20);
        compare(findChild(harness, "decoder").currentIndex, 0);
        compare(harness.settings.decoder, "Unavailable decoder");
        compare(harness.settings.changeCount, 0);
    }
}
