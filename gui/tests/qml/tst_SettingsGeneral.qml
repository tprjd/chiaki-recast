import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "SettingsGeneral"
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

    function test_generalChoices_data() {
        return [
            { tag: "disconnect", name: "disconnectAction", value: 1, count: 3 },
            { tag: "suspend", name: "suspendAction", value: 1, count: 2 },
            { tag: "media", name: "audioVideoDisabled", value: 3, count: 4 },
            { tag: "shortcut1", name: "streamMenuShortcut1", value: 2, count: 17 },
            { tag: "shortcut2", name: "streamMenuShortcut2", value: 4, count: 17 },
            { tag: "shortcut3", name: "streamMenuShortcut3", value: 6, count: 17 },
            { tag: "shortcut4", name: "streamMenuShortcut4", value: 16, count: 17 }
        ];
    }
    function test_generalChoices(data) {
        const harness = createTemporaryObject(harnessComponent, testCase);
        const control = findChild(harness, data.name);
        verify(control);
        compare(control.count, data.count);
        harness.settings.changeCount = 0;
        choose(control, data.value);
        compare(harness.settings[data.name], data.value);
        compare(harness.settings.changeCount, 1);
        compare(harness.actions.length, 0);
    }

    function test_generalConditionsAndPrivacy() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        const haptics = findChild(harness, "steamDeckHaptics");
        const vertical = findChild(harness, "verticalDeck");
        verify(!haptics.visible);
        verify(!vertical.visible);
        harness.settings.steamDeckHaptics = false;
        harness.settings.verticalDeck = false;
        verify(haptics.visible && vertical.visible);
        haptics.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Space);
        verify(harness.settings.steamDeckHaptics);
        vertical.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Space);
        verify(harness.settings.verticalDeck);
        const streamer = findChild(harness, "streamerMode");
        streamer.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Space);
        verify(harness.settings.streamerMode);
        verify(!findChild(harness, "logDirectory").text.includes("synthetic"));
        verify(!findChild(harness, "settingsProfile").text.includes("Test profile"));
        const menu = findChild(harness, "streamMenuEnabled");
        menu.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Space);
        verify(!harness.settings.streamMenuEnabled);
        for (let index = 1; index <= 4; index++)
            verify(!findChild(harness, "streamMenuShortcut" + index).visible);
    }

    function test_generalActions() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        const open = findChild(harness, "openLogDirectory");
        open.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(harness.actions.length, 1);
        compare(harness.actions[0].name, "openExternal");
        compare(harness.actions[0].args[0], "file://C:/synthetic/logs");
        mouseClick(findChild(harness, "settingsBack"));
        compare(harness.actions[1].name, "close");
    }

    function test_responsiveGeneral_data() {
        return [{tag: "desktop", width: 1280, height: 720},
            {tag: "narrow", width: 800, height: 600},
            {tag: "minimum", width: 390, height: 600}];
    }
    function test_responsiveGeneral(data) {
        const harness = createTemporaryObject(harnessComponent, testCase, {width: data.width, height: data.height});
        waitForRendering(harness);
        for (const name of ["disconnectAction", "suspendAction", "audioVideoDisabled", "streamerMode", "openLogDirectory", "streamMenuEnabled", "streamMenuShortcut4"]) {
            const control = findChild(harness, name);
            control.forceActiveFocus(Qt.TabFocusReason);
            wait(10);
            const point = control.mapToItem(harness, 0, 0);
            verify(point.x >= 0 && point.x + control.width <= harness.width, name + " fits width");
            verify(point.y >= 0 && point.y + control.height <= harness.height, name + " remains visible on focus");
        }
        if (data.width < 760) {
            const selector = findChild(harness, "settingsCategorySelector");
            choose(selector, 8);
            compare(harness.screen.currentCategory, 8);
            choose(selector, 0);
            compare(harness.screen.currentCategory, 0);
        }
    }

    function test_constructionDoesNotChangeSettings() {
        const harness = createTemporaryObject(harnessComponent, testCase, {screenActive: false});
        harness.settings.changeCount = 0;
        harness.settings.actions = [];
        harness.screenActive = true;
        waitForRendering(harness);
        compare(harness.settings.changeCount, 0);
        compare(harness.settings.actions.filter(action => action.name !== "refreshAudioDevices").length, 0);
        compare(harness.actions.length, 0);
    }

    function test_popupEscapeDoesNotCloseSettings() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        const combo = findChild(harness, "disconnectAction");
        combo.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", true);
        keyClick(Qt.Key_Escape);
        tryCompare(combo.popup, "visible", false);
        compare(harness.actions.length, 0);
        keyClick(Qt.Key_Escape);
        compare(harness.actions.length, 1);
        compare(harness.actions[0].name, "close");
    }

    function test_sidebarAndGeneralOpenWithoutWrites() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        verify(harness);
        waitForRendering(harness);
        harness.settings.changeCount = 0;
        for (let category = 0; category < 9; category++) {
            const button = findChild(harness, "settingsCategory" + category);
            verify(button && button.visible, "Category " + category + " is reachable");
            mouseClick(button);
            compare(harness.screen.currentCategory, category);
            verify(button.checked, "Selected category has a persistent selected state");
        }
        compare(harness.settings.changeCount, 0);
        compare(harness.actions.length, 0);
    }
}
