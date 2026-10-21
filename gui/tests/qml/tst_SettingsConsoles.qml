import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "SettingsConsoles"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; SettingsHarness {} }
    function hosts() {
        return [{mac: "001122334455", name: "Living room", ps5: true},
            {mac: "aabbccddeeff", name: "Bedroom", ps5: false}];
    }
    function openConsoles(properties) {
        const harness = createTemporaryObject(harnessComponent, testCase, Object.assign({screenActive: false}, properties || {}));
        harness.settings.registeredHosts = hosts();
        harness.backend.hiddenHosts = [{mac: "112233445566", name: "Hidden room"}];
        harness.screenActive = true;
        harness.screen.currentCategory = 4;
        waitForRendering(harness);
        harness.actions = [];
        harness.settings.actions = [];
        harness.settings.changeCount = 0;
        return harness;
    }
    function withText(item, text, result) {
        result = result || [];
        if (item.text === text && item.clicked !== undefined) result.push(item);
        for (const child of item.children || []) withText(child, text, result);
        return result;
    }
    function press(control) {
        verify(control);
        control.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
    }
    function test_confirmationInvalidation_data() {
        const cases = [];
        for (const change of ["removed", "duplicate", "profile", "cancel", "superseded"])
            for (const action of ["delete", "unhide"])
                cases.push({tag: action + "-" + change, action: action, change: change});
        return cases;
    }
    function test_confirmationInvalidation(data) {
        const harness = openConsoles();
        const button = findChild(harness, data.action === "delete" ? "deleteRegisteredConsole0" : "unhideConsole0");
        press(button);
        const accept = harness.actions[0].args[3];
        const cancel = harness.actions[0].args[4];
        if (data.change === "removed") {
            if (data.action === "delete") harness.settings.registeredHosts = [hosts()[1]];
            else harness.backend.hiddenHosts = [];
        } else if (data.change === "duplicate") {
            if (data.action === "delete") harness.settings.registeredHosts = [hosts()[0], hosts()[0]];
            else harness.backend.hiddenHosts = [harness.backend.hiddenHosts[0], harness.backend.hiddenHosts[0]];
        } else if (data.change === "profile") {
            harness.settings.currentProfile = "Another profile";
            harness.settings.currentProfile = "Test profile";
        } else if (data.change === "cancel") {
            cancel();
            wait(1);
            verify(button.activeFocus, "Cancel restores its opener");
        } else {
            press(button);
        }
        accept();
        accept();
        compare(harness.settings.actions.length, 0);
        compare(harness.actions.filter(action => action.name === "unhide").length, 0);
    }
    function test_unhideUsesOriginalIdentityOnceAndRestoresFallbackFocus() {
        const harness = openConsoles();
        press(findChild(harness, "unhideConsole0"));
        const accept = harness.actions[0].args[3];
        harness.backend.hiddenHosts = [{mac: "another", name: "Other"}, {mac: "112233445566", name: "Renamed"}];
        wait(1);
        accept();
        accept();
        const actions = harness.actions.filter(action => action.name === "unhide");
        compare(actions.length, 1);
        compare(actions[0].args[0], "112233445566");
        tryCompare(findChild(harness, "registerConsole"), "activeFocus", true);
    }
    function test_cancelAfterProfileChangeRestoresValidFocus() {
        const harness = openConsoles();
        press(findChild(harness, "deleteRegisteredConsole0"));
        const cancel = harness.actions[0].args[4];
        harness.settings.currentProfile = "Another profile";
        harness.settings.registeredHosts = [];
        harness.backend.hiddenHosts = [];
        wait(1);
        cancel();
        tryCompare(findChild(harness, "registerConsole"), "activeFocus", true);
        compare(harness.settings.actions.length, 0);
    }
    function test_autoConnectIsExclusiveAndFollowsIdentity() {
        const harness = openConsoles();
        const first = findChild(harness, "autoConnectConsole0");
        const second = findChild(harness, "autoConnectConsole1");
        press(first);
        compare(harness.settings.autoConnectMac, "001122334455");
        verify(first.checked && !second.checked);
        press(second);
        compare(harness.settings.autoConnectMac, "aabbccddeeff");
        verify(!first.checked && second.checked);
        harness.settings.registeredHosts = [hosts()[1], hosts()[0]];
        wait(1);
        verify(findChild(harness, "autoConnectConsole0").checked);
        verify(!findChild(harness, "autoConnectConsole1").checked);
        press(findChild(harness, "autoConnectConsole0"));
        compare(harness.settings.autoConnectMac, "");
        harness.settings.currentProfile = "Another profile";
        harness.settings.autoConnectMac = "001122334455";
        verify(findChild(harness, "autoConnectConsole1").checked);
    }
    function test_registrationAndEmptyListsDoNotWriteSettings() {
        const harness = openConsoles();
        harness.settings.registeredHosts = [];
        harness.backend.hiddenHosts = [];
        harness.settings.changeCount = 0;
        verify(findChild(harness, "registeredConsolesEmpty").visible);
        verify(findChild(harness, "hiddenConsolesEmpty").visible);
        press(findChild(harness, "registerConsole"));
        compare(harness.actions[0].name, "register");
        compare(harness.actions[0].args[0], "255.255.255.255");
        compare(harness.actions[0].args[1], true);
        compare(harness.settings.changeCount, 0);
        compare(harness.settings.actions.length, 0);
    }
    function test_openingAndListUpdatesDoNotWriteSettings() {
        const harness = openConsoles();
        compare(harness.settings.changeCount, 0);
        harness.settings.registeredHosts = [hosts()[1], hosts()[0]];
        harness.backend.hiddenHosts = [];
        harness.settings.changeCount = 0;
        harness.screen.currentCategory = 0;
        harness.screen.currentCategory = 4;
        wait(1);
        compare(harness.settings.changeCount, 0);
        compare(harness.settings.actions.length, 0);
        compare(harness.actions.length, 0);
    }
    function test_directionalNavigationAndLongActionLabels() {
        const harness = openConsoles({width: 900, height: 600});
        const autoConnect = findChild(harness, "autoConnectConsole0");
        const deleteButton = findChild(harness, "deleteRegisteredConsole0");
        autoConnect.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Right);
        verify(deleteButton.activeFocus);
        keyClick(Qt.Key_Left);
        verify(autoConnect.activeFocus);
        autoConnect.text = "Automatically connect to this selected console when the application starts";
        deleteButton.text = "Delete this console registration";
        waitForRendering(harness);
        for (const control of [autoConnect, deleteButton]) {
            const point = control.mapToItem(harness, 0, 0);
            verify(point.x >= 0 && point.x + control.width <= harness.width);
        }
        compare(harness.settings.changeCount, 0);
    }
    function test_narrowControlsAndPrivacy() {
        const harness = openConsoles({width: 900, height: 600});
        harness.settings.registeredHosts = [{mac: "001122334455", name: "A long private console name repeated ".repeat(7), ps5: true}];
        harness.settings.streamerMode = true;
        waitForRendering(harness);
        const info = findChild(harness, "registeredConsoleInfo0");
        verify(info);
        verify(!info.text.includes("private"));
        verify(!info.text.includes("001122334455"));
        verify(!findChild(harness, "hiddenConsoleInfo0").text.includes("Hidden room"));
        verify(!findChild(harness, "deleteRegisteredConsole0").Accessible.name.includes("private"));
        verify(!findChild(harness, "autoConnectConsole0").Accessible.name.includes("private"));
        verify(!findChild(harness, "unhideConsole0").Accessible.name.includes("Hidden room"));
        for (const name of ["registerConsole", "autoConnectConsole0", "deleteRegisteredConsole0", "unhideConsole0"]) {
            const control = findChild(harness, name);
            verify(control, name);
            control.forceActiveFocus(Qt.TabFocusReason);
            wait(30);
            const point = control.mapToItem(harness, 0, 0);
            verify(point.x >= 0 && point.x + control.width <= harness.width, name + " horizontal bounds");
            verify(point.y >= 0 && point.y + control.height <= harness.height, name + " vertical bounds");
        }
    }
    function test_deleteResolvesOriginalIdentityAfterReorder() {
        const harness = openConsoles();
        press(withText(harness, "Delete")[0]);
        compare(harness.actions[0].name, "confirm");
        const accept = harness.actions[0].args[3];
        harness.settings.registeredHosts = [hosts()[1], hosts()[0]];
        wait(1);
        accept();
        compare(harness.settings.actions.length, 1);
        compare(harness.settings.actions[0].name, "deleteRegisteredHost");
        compare(harness.settings.actions[0].args[0], 1);
    }
}
