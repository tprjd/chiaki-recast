import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"
import "../../src/qml"

TestCase {
    id: testCase
    name: "SettingsConfig"
    width: 1280; height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; SettingsHarness {} }
    function test_actionsAndPrivacy() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        mouseClick(findChild(harness, "settingsCategory8"));
        const profile = findChild(harness, "configProfile");
        verify(profile);
        verify(profile.text.includes("Test profile"));
        harness.settings.streamerMode = true;
        verify(!profile.text.includes("Test profile"));
        harness.settings.actions = [];
        for (const action of ["exportSettings", "importSettings"]) {
            const button = findChild(harness, action);
            button.forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Return);
        }
        compare(harness.settings.actions.length, 2);
        compare(harness.settings.actions[0].name, "exportSettings");
        compare(harness.settings.actions[1].name, "importSettings");
        findChild(harness, "manageProfiles").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(harness.actions[0].name, "profiles");
    }
    Component {
        id: navigationHarness
        Item {
            id: root
            width: 900; height: 600
            function closeDialog() { pages.pop(); }
            function showProfileDialog() { pages.push(profileComponent); }
            SettingsHarness { id: collaborators; visible: false; screenActive: false }
            StackView {
                id: pages
                objectName: "settingsStack"
                anchors.fill: parent
                initialItem: SettingsDialog { backend: collaborators.backend; navigation: root }
            }
            Component { id: profileComponent; ProfileDialog { backend: collaborators.backend; navigation: root } }
        }
    }
    function test_loggingRefreshAndAboutCancel() {
        const harness = createTemporaryObject(harnessComponent, testCase, {screenActive: false});
        harness.settings.logSanitize = true;
        harness.settings.changeCount = 0;
        harness.screenActive = true;
        mouseClick(findChild(harness, "settingsCategory8"));
        compare(harness.settings.changeCount, 0);
        for (const name of ["logSanitize", "logVerbose"]) {
            const box = findChild(harness, name);
            const before = harness.settings[name];
            box.forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Space);
            compare(harness.settings[name], !before);
        }
        compare(harness.settings.changeCount, 2);
        harness.settings.currentProfile = "Changed profile";
        harness.settings.logSanitize = true;
        harness.settings.logVerbose = false;
        harness.settings.changeCount = 0;
        waitForRendering(harness);
        verify(findChild(harness, "configProfile").text.includes("Changed profile"));
        verify(findChild(harness, "logSanitize").checked);
        verify(!findChild(harness, "logVerbose").checked);
        const about = findChild(harness, "aboutSettings");
        about.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(findChild(harness, "aboutDialog"), "visible", true);
        keyClick(Qt.Key_Escape);
        tryCompare(findChild(harness, "aboutDialog"), "visible", false);
        tryCompare(about, "activeFocus", true);
        compare(harness.settings.changeCount, 0);
        compare(harness.actions.length, 0);
    }
    function test_backRestoresConfigOpener() {
        const harness = createTemporaryObject(navigationHarness, testCase);
        const pages = findChild(harness, "settingsStack");
        mouseClick(findChild(harness, "settingsCategory8"));
        const opener = findChild(harness, "manageProfiles");
        opener.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(pages, "depth", 2);
        tryCompare(pages, "busy", false);
        findChild(harness, "profileSelection").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Escape);
        tryCompare(pages, "depth", 1);
        tryCompare(pages, "busy", false);
        tryCompare(opener, "activeFocus", true);
    }
    function test_bounds_data() {
        return [{tag: "desktop", width: 1280, height: 720}, {tag: "minimum", width: 900, height: 600}];
    }
    function test_bounds(data) {
        const harness = createTemporaryObject(harnessComponent, testCase, {width: data.width, height: data.height});
        mouseClick(findChild(harness, "settingsCategory8"));
        waitForRendering(harness);
        for (const name of ["manageProfiles", "exportSettings", "importSettings", "logSanitize", "logVerbose", "aboutSettings"]) {
            const control = findChild(harness, name);
            control.forceActiveFocus(Qt.TabFocusReason);
            wait(20);
            const point = control.mapToItem(harness, 0, 0);
            verify(point.x >= 0 && point.x + control.width <= harness.width, name + " fits width");
            verify(point.y >= 0 && point.y + control.height <= harness.height, name + " fits height");
        }
    }
}
