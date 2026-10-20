import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"
import "../../src/qml"

TestCase {
    id: testCase
    name: "SettingsProfiles"
    width: 1280; height: 720
    visible: true
    when: windowShown
    Component {
        id: harnessComponent
        Item {
            id: root
            width: 1280; height: 720
            property alias settings: fakeSettings
            property alias screen: loader.item
            property int closeCount: 0
            function closeDialog() { closeCount++; }
            FakeSettings {
                id: fakeSettings
                currentProfile: "Primary"
                profiles: ["default", "Primary", "Guest", "create new profile"]
                function deleteProfile(name) { record("deleteProfile", [name]); }
            }
            QtObject { id: fakeBackend; property var settings: fakeSettings }
            Loader { id: loader; anchors.fill: parent; sourceComponent: ProfileDialog { backend: fakeBackend; navigation: root } }
        }
    }
    function choose(combo, index) {
        combo.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", true);
        keyClick(Qt.Key_Home);
        for (let i = 0; i < index; i++) keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", false);
    }
    function test_switchIsExplicitAndDefaultUsesEmptyKey() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        verify(harness);
        const combo = findChild(harness, "profileSelection");
        verify(combo);
        compare(combo.currentIndex, 1);
        verify(!harness.screen.buttonEnabled);
        harness.settings.changeCount = 0;
        choose(combo, 0);
        compare(harness.settings.currentProfile, "Primary");
        compare(harness.settings.changeCount, 0);
        verify(!findChild(harness, "deleteProfile").visible);
        keyClick(Qt.Key_Menu);
        compare(harness.settings.currentProfile, "");
        compare(harness.settings.changeCount, 1);
        compare(harness.closeCount, 1);
    }
    function test_protectedAndDeleteCases() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        const combo = findChild(harness, "profileSelection");
        const deletion = findChild(harness, "deleteProfile");
        verify(!deletion.visible);
        choose(combo, 0);
        verify(!deletion.visible);
        choose(combo, 3);
        verify(!deletion.visible);
        choose(combo, 2);
        verify(deletion.visible);
        deletion.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Space);
        compare(harness.settings.actions.length, 0);
        keyClick(Qt.Key_Menu);
        compare(harness.settings.actions.length, 1);
        compare(harness.settings.actions[0].name, "deleteProfile");
        compare(harness.settings.actions[0].args[0], "Guest");
        compare(harness.settings.currentProfile, "Primary");
        compare(harness.closeCount, 1);
    }
    function test_creationIsTrimmedAndExplicit() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        const combo = findChild(harness, "profileSelection");
        choose(combo, 3);
        const name = findChild(harness, "newProfileName");
        verify(name.visible);
        verify(!harness.screen.buttonEnabled);
        name.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        keyClick(Qt.Key_Space);
        verify(!harness.screen.buttonEnabled);
        keyClick(Qt.Key_A);
        keyClick(Qt.Key_Space);
        compare(harness.settings.currentProfile, "Primary");
        verify(harness.screen.buttonEnabled);
        keyClick(Qt.Key_Return);
        compare(harness.settings.currentProfile, "Primary");
        keyClick(Qt.Key_Menu);
        compare(harness.settings.currentProfile, "a");
        compare(harness.closeCount, 1);
    }
    function test_staleSelectionIsCleared_data() {
        return [{tag: "profile", profile: true}, {tag: "list", profile: false}];
    }
    function test_staleSelectionIsCleared(data) {
        const harness = createTemporaryObject(harnessComponent, testCase);
        const combo = findChild(harness, "profileSelection");
        const deletion = findChild(harness, "deleteProfile");
        choose(combo, 2);
        deletion.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Space);
        if (data.profile) harness.settings.currentProfile = "Guest";
        else harness.settings.profiles = ["default", "Primary", "Replacement", "create new profile"];
        verify(!deletion.checked);
        verify(!harness.screen.buttonEnabled);
        combo.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Menu);
        compare(harness.settings.actions.length, 0);
        compare(harness.closeCount, 0);
    }
    function test_popupRefreshAndEscapeDoNotWrite() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        const combo = findChild(harness, "profileSelection");
        combo.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", true);
        harness.settings.profiles = ["default", "Primary", "Other", "create new profile"];
        tryCompare(combo.popup, "visible", false);
        harness.settings.changeCount = 0;
        keyClick(Qt.Key_Escape);
        compare(harness.settings.changeCount, 0);
        compare(harness.settings.actions.length, 0);
        compare(harness.closeCount, 1);
    }
    function test_privacyAndBounds_data() {
        return [{tag: "desktop", width: 1280, height: 720}, {tag: "minimum", width: 900, height: 600}];
    }
    function test_privacyAndBounds(data) {
        const harness = createTemporaryObject(harnessComponent, testCase, {width: data.width, height: data.height});
        harness.settings.streamerMode = true;
        const combo = findChild(harness, "profileSelection");
        verify(!combo.displayText.includes("Primary"));
        verify(!combo.model.join(" ").includes("Guest"));
        verify(!findChild(harness, "activeProfile").text.includes("Primary"));
        choose(combo, 3);
        const field = findChild(harness, "newProfileName");
        compare(field.echoMode, TextInput.Password);
        waitForRendering(harness);
        for (const control of [combo, field]) {
            const point = control.mapToItem(harness, 0, 0);
            verify(point.x >= 0 && point.x + control.width <= harness.width);
            verify(point.y >= 0 && point.y + control.height <= harness.height);
        }
    }
    function test_cancelCreationAndDeletion() {
        for (const creating of [true, false]) {
            const harness = createTemporaryObject(harnessComponent, testCase);
            const combo = findChild(harness, "profileSelection");
            choose(combo, creating ? 3 : 2);
            if (creating) {
                const name = findChild(harness, "newProfileName");
                name.forceActiveFocus(Qt.TabFocusReason);
                keyClick(Qt.Key_Return);
                keyClick(Qt.Key_A);
                keyClick(Qt.Key_Escape);
                compare(harness.closeCount, 0);
            } else {
                findChild(harness, "deleteProfile").forceActiveFocus(Qt.TabFocusReason);
                keyClick(Qt.Key_Space);
            }
            harness.settings.changeCount = 0;
            keyClick(Qt.Key_Escape);
            compare(harness.closeCount, 1);
            compare(harness.settings.actions.length, 0);
            compare(harness.settings.changeCount, 0);
            harness.destroy();
        }
    }
    function test_profileChangeClearsDraftName() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        const combo = findChild(harness, "profileSelection");
        choose(combo, 3);
        const name = findChild(harness, "newProfileName");
        name.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        keyClick(Qt.Key_A);
        harness.settings.currentProfile = "Guest";
        compare(name.text, "");
        verify(!name.visible);
        verify(!harness.screen.buttonEnabled);
        combo.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Menu);
        compare(harness.settings.currentProfile, "Guest");
        compare(harness.closeCount, 0);
    }
}
