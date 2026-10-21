import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"
import "../../src/qml"

TestCase {
    id: testCase
    name: "KeyboardMouseReset"
    width: 900; height: 600
    visible: true
    when: windowShown
    Component {
        id: harnessComponent
        Item {
            width: 900; height: 600
            property alias settings: settings
            property alias page: page
            FakeSettings { id: settings }
            KeyboardMousePage { id: page; width: parent.width; settings: settings }
        }
    }
    Component { id: settingsComponent; FakeSettings {} }
    Component { id: settingsHarnessComponent; SettingsHarness {} }
    function openReset(h, tab) {
        h.settings.mouseCameraEnabled = true;
        h.settings.mouseHorizontalSensitivity = 2;
        h.page.currentTab = tab;
        const reset = findChild(h, "keyboardMouseReset");
        verify(reset, "Reset defaults is available on either tab");
        reset.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        const dialog = findChild(h, "keyboardMouseResetDialog");
        tryCompare(dialog, "opened", true);
        return dialog;
    }
    function test_reset_data() {
        return [{tag: "response-cancel", tab: 0, accept: false},
            {tag: "bindings-cancel", tab: 1, accept: false},
            {tag: "response-confirm", tab: 0, accept: true},
            {tag: "bindings-confirm", tab: 1, accept: true}];
    }
    function test_reset(data) {
        const h = createTemporaryObject(harnessComponent, testCase);
        const dialog = openReset(h, data.tab);
        tryCompare(findChild(dialog, "confirmationCancel"), "activeFocus", true);
        compare(h.settings.mouseHorizontalSensitivity, 2);
        if (data.accept) {
            findChild(dialog, "confirmationAction").forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Return);
        } else keyClick(Qt.Key_Escape);
        tryCompare(dialog, "opened", false);
        tryCompare(h.settings, "mouseCameraEnabled", !data.accept);
        compare(h.settings.mouseHorizontalSensitivity, data.accept ? 1 : 2);
        tryCompare(findChild(h, "keyboardMouseReset"), "activeFocus", true);
        compare(h.page.currentTab, data.tab);
    }
    function test_replacementCancels_data() {
        return [{tag: "profile", profile: true}, {tag: "settings", profile: false}];
    }
    function test_replacementCancels(data) {
        const h = createTemporaryObject(harnessComponent, testCase);
        const dialog = openReset(h, 0);
        const original = h.settings;
        if (data.profile) original.currentProfile = "Another profile";
        else h.page.settings = createTemporaryObject(settingsComponent, testCase);
        tryCompare(dialog, "opened", false);
        dialog.choose("primary");
        compare(original.mouseCameraEnabled, true);
        compare(original.mouseHorizontalSensitivity, 2);
    }
    function test_searchFindsResetWithoutOpeningConfirmation() {
        const h = createTemporaryObject(settingsHarnessComponent, testCase);
        h.screen.currentCategory = 5;
        findChild(h, "keyboardMouseTabs").currentIndex = 1;
        const search = findChild(h, "settingsSearch");
        search.text = "Restore mouse response";
        search.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        const reset = findChild(h, "keyboardMouseReset");
        tryCompare(reset, "activeFocus", true);
        verify(reset.enabled);
        verify(!findChild(h, "keyboardMouseResetDialog").opened);
        h.settings.mouseCameraSupported = false;
        search.text = "Restore mouse response";
        compare(h.screen.searchResults.length, 0);
    }
}
