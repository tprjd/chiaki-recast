import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "ManualHost"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; ManualHostHarness {} }
    function openScreen(properties) {
        const harness = createTemporaryObject(harnessComponent, testCase, properties || {});
        verify(harness);
        waitForRendering(harness);
        return harness;
    }
    function press(control) {
        verify(control);
        control.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
    }
    function enterAddress(harness, value) {
        const field = findChild(harness, "manualAddress");
        verify(field);
        mouseClick(field);
        keySequence("Ctrl+A");
        for (const character of value) keyClick(character);
    }
    function hosts() {
        return [{mac: "001122334455", name: "Living room", ps5: true},
            {mac: "aabbccddeeff", name: "Living room", ps5: false}];
    }
    function choose(harness, index) {
        const combo = findChild(harness, "manualRegistration");
        combo.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        keyClick(Qt.Key_Home);
        for (let i = 0; i < index; i++) keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", false);
    }
    function test_existingRegistrationRequiresChoiceAndFollowsReorder() {
        const loaded = openScreen({registeredHosts: hosts()});
        enterAddress(loaded, "  console.example  ");
        verify(!findChild(loaded, "manualAdd").enabled);
        const combo = findChild(loaded, "manualRegistration");
        verify(combo.model[1].name !== combo.model[2].name);
        choose(loaded, 1);
        loaded.settings.registeredHosts = [hosts()[1], hosts()[0]];
        press(findChild(loaded, "manualAdd"));
        compare(loaded.actions.length, 1);
        compare(loaded.actions[0].index, 1);
        compare(loaded.actions[0].address, "console.example");
        verify(findChild(loaded, "manualSavedRegistration").text.includes("Living room"));
    }
    function test_invalidSelectionCanBeCorrectedWithoutLosingAddress_data() {
        return ["removed", "ambiguous", "profile"].map(change => ({tag: change, change: change}));
    }
    function test_invalidSelectionCanBeCorrectedWithoutLosingAddress(data) {
        const harness = openScreen({registeredHosts: hosts()});
        enterAddress(harness, "192.0.2.42");
        choose(harness, 1);
        if (data.change === "removed") harness.settings.registeredHosts = [hosts()[1]];
        else if (data.change === "ambiguous") harness.settings.registeredHosts = [hosts()[0], hosts()[0]];
        else {
            harness.settings.currentProfile = "Another profile";
            harness.settings.currentProfile = "Test profile";
        }
        const failure = findChild(harness, "manualFailureTitle");
        verify(failure && failure.visible);
        compare(harness.actions.length, 0);
        press(findChild(harness, "manualBackToDetails"));
        compare(findChild(harness, "manualAddress").text, "192.0.2.42");
        verify(!findChild(harness, "manualAdd").enabled);
        choose(harness, findChild(harness, "manualRegistration").count - 1);
        press(findChild(harness, "manualAdd"));
        compare(harness.actions.length, 1);
        compare(harness.actions[0].index, harness.settings.registeredHosts.length);
    }
    function test_backendRejectionKeepsDetailsAndSelection() {
        const harness = openScreen({registeredHosts: hosts()});
        enterAddress(harness, "192.0.2.42");
        choose(harness, 2);
        harness.backend.rejectSave = true;
        press(findChild(harness, "manualAdd"));
        verify(findChild(harness, "manualFailureTitle").visible);
        verify(findChild(harness, "manualBackToDetails").activeFocus);
        press(findChild(harness, "manualBackToDetails"));
        compare(findChild(harness, "manualAddress").text, "192.0.2.42");
        compare(findChild(harness, "manualRegistration").currentIndex, 2);
        harness.backend.rejectSave = false;
        press(findChild(harness, "manualAdd"));
        verify(findChild(harness, "manualSavedTitle").visible);
        compare(harness.actions.length, 2);
        compare(harness.actions[1].index, 1);
    }
    function test_profileChangeDuringFailureRequiresNewChoice() {
        const harness = openScreen({registeredHosts: hosts()});
        enterAddress(harness, "ps5.local");
        choose(harness, 1);
        harness.backend.rejectSave = true;
        press(findChild(harness, "manualAdd"));
        harness.settings.currentProfile = "Another profile";
        harness.settings.currentProfile = "Test profile";
        press(findChild(harness, "manualBackToDetails"));
        verify(!findChild(harness, "manualAdd").enabled);
    }
    function test_privateSelectorAndResultMaskIdentifiers() {
        const harness = openScreen({registeredHosts: [{mac: "001122334455", name: "ps5.local", ps5: true}]});
        enterAddress(harness, "ps5.local");
        choose(harness, 1);
        harness.settings.streamerMode = true;
        const combo = findChild(harness, "manualRegistration");
        verify(!combo.displayText.includes("001122334455"));
        verify(!combo.displayText.includes("ps5.local"));
        compare(findChild(harness, "manualAddress").echoMode, TextInput.Password);
        press(findChild(harness, "manualAdd"));
        for (const name of ["manualSavedAddress", "manualSavedRegistration"])
            verify(!findChild(harness, name).text.includes("ps5.local"));
        harness.settings.streamerMode = false;
        verify(findChild(harness, "manualSavedAddress").text.includes("ps5.local"));
        harness.settings.streamerMode = true;
        verify(!findChild(harness, "manualSavedRegistration").text.includes("001122334455"));
    }
    function test_whitespaceAndEditingDoNotSave() {
        const harness = openScreen({registeredHosts: hosts()});
        enterAddress(harness, "   ");
        choose(harness, 1);
        const add = findChild(harness, "manualAdd");
        verify(!add.enabled);
        keyClick(Qt.Key_Menu);
        compare(harness.actions.length, 0);
        enterAddress(harness, "ps5.local");
        verify(add.enabled);
        choose(harness, 0);
        verify(!add.enabled);
        compare(harness.actions.length, 0);
        press(findChild(harness, "manualCancel"));
        compare(harness.actions.length, 1);
        compare(harness.actions[0].name, "close");
    }
    function test_explicitDeferredChoiceWithExistingRecords() {
        const harness = openScreen({registeredHosts: hosts()});
        enterAddress(harness, "2001:db8::42");
        choose(harness, 3);
        press(findChild(harness, "manualAdd"));
        compare(harness.actions.length, 1);
        compare(harness.actions[0].index, 2);
        compare(harness.actions[0].address, "2001:db8::42");
        verify(findChild(harness, "manualSavedRegistration").text.includes("first connection"));
    }
    function test_popupEscapeAndFieldEditingKeepScreenOpen() {
        const harness = openScreen();
        const combo = findChild(harness, "manualRegistration");
        press(combo);
        verify(combo.popup.visible);
        keyClick(Qt.Key_Escape);
        tryCompare(combo.popup, "visible", false);
        compare(harness.actions.length, 0);
        enterAddress(harness, "ps5.local");
        keyClick(Qt.Key_Left);
        verify(findChild(harness, "manualAddress").activeFocus);
        keyClick(Qt.Key_Escape);
        compare(harness.actions.length, 0);
        keyClick(Qt.Key_Escape);
        compare(harness.actions.length, 1);
        compare(harness.actions[0].name, "close");
    }
    function checkVisible(harness, control) {
        verify(control && control.visible && control.enabled);
        control.forceActiveFocus(Qt.TabFocusReason);
        tryVerify(() => {
            const point = control.mapToItem(harness, 0, 0);
            return point.x >= 0 && point.y >= 0 && point.x + control.width <= harness.width + 1
                && point.y + control.height <= harness.height + 1;
        }, 1000, control.objectName + " is fully visible");
        verify(control.activeFocus, control.objectName + " has focus");
        verify(control.Accessible.name || control.text);
    }
    function test_keyboardFocusAndLongTextAtRequiredSizes_data() {
        return [{tag: "minimum", width: 900, height: 600}, {tag: "wide", width: 1280, height: 720}];
    }
    function test_keyboardFocusAndLongTextAtRequiredSizes(data) {
        const harness = openScreen({width: data.width, height: data.height,
            registeredHosts: [{mac: "001122334455", name: "Long console name ".repeat(20), ps5: true}]});
        enterAddress(harness, "ps5.local");
        choose(harness, 1);
        const address = findChild(harness, "manualAddress");
        const combo = findChild(harness, "manualRegistration");
        const cancel = findChild(harness, "manualCancel");
        const add = findChild(harness, "manualAdd");
        address.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Tab);
        verify(combo.activeFocus);
        keyClick(Qt.Key_Tab);
        verify(cancel.activeFocus);
        keyClick(Qt.Key_Tab);
        verify(add.activeFocus);
        keyClick(Qt.Key_Tab, Qt.ShiftModifier);
        verify(cancel.activeFocus);
        keyClick(Qt.Key_Up);
        verify(combo.activeFocus);
        keyClick(Qt.Key_Up);
        verify(address.activeFocus, "Focus is " + harness.Window.window.activeFocusItem.objectName);
        for (const control of [address, combo, cancel, add]) checkVisible(harness, control);
        add.text = "Add this console address to the console library";
        cancel.text = "Cancel console setup";
        waitForRendering(harness);
        checkVisible(harness, add);
        checkVisible(harness, cancel);
        verify(cancel.mapToItem(harness, 0, 0).x >= address.mapToItem(harness, 0, 0).x,
            "Long action labels stay inside the form panel");
        press(add);
        const back = findChild(harness, "manualBackToConsoles");
        tryCompare(back, "activeFocus", true);
        checkVisible(harness, back);
        harness.destroy();
        const failureHarness = openScreen({width: data.width, height: data.height, registeredHosts: hosts()});
        enterAddress(failureHarness, "ps5.local");
        choose(failureHarness, 1);
        failureHarness.settings.registeredHosts = [];
        checkVisible(failureHarness, findChild(failureHarness, "manualBackToDetails"));
    }
    function test_longSelectorTextWraps() {
        const harness = openScreen({width: 900, height: 600,
            registeredHosts: [{mac: "001122334455", name: "A very long console name ".repeat(6), ps5: true}]});
        choose(harness, 1);
        const combo = findChild(harness, "manualRegistration");
        waitForRendering(harness);
        verify(combo.contentItem.contentWidth <= combo.contentItem.width + 1, "The selected console name remains readable");
    }
    function test_mouseSelectionUsesIntendedRegistration() {
        const harness = openScreen({registeredHosts: hosts()});
        enterAddress(harness, "192.0.2.42");
        const combo = findChild(harness, "manualRegistration");
        mouseClick(combo);
        tryCompare(combo.popup, "opened", true);
        mouseClick(combo.popup.contentItem.itemAtIndex(2));
        tryCompare(combo.popup, "visible", false);
        mouseClick(findChild(harness, "manualAdd"));
        compare(harness.actions.length, 1);
        compare(harness.actions[0].index, 1);
    }
    function test_renderedWorkspace_data() {
        const rows = [];
        for (const width of [900, 1280])
            for (const phase of ["form", "saved", "failure"])
                rows.push({tag: width + "-" + phase, width: width, height: width === 900 ? 600 : 720, phase: phase});
        return rows;
    }
    function test_renderedWorkspace(data) {
        const harness = openScreen({width: data.width, height: data.height, registeredHosts: hosts()});
        enterAddress(harness, "192.0.2.42");
        choose(harness, 1);
        if (data.phase === "saved") press(findChild(harness, "manualAdd"));
        else if (data.phase === "failure") harness.settings.registeredHosts = [];
        else findChild(harness, "manualAddress").forceActiveFocus(Qt.TabFocusReason);
        waitForRendering(harness);
        const image = grabImage(harness);
        compare(image.width, data.width);
        compare(image.height, data.height);
        verify(image.blue(1, 1) > image.red(1, 1), "Dark blue page background");
        if (data.phase === "form") {
            const button = findChild(harness, "manualAdd");
            const point = button.mapToItem(harness, 6, 6);
            verify(image.blue(point.x, point.y) > 200, "The primary action has a blue background");
        }
    }
    function test_emptyListSavesTrimmedHostnameOnceAndShowsResult() {
        const harness = openScreen();
        enterAddress(harness, "  ps5.local  ");
        const add = findChild(harness, "manualAdd");
        press(add);
        keyClick(Qt.Key_Menu);
        compare(harness.actions.length, 1);
        compare(harness.actions[0].name, "add");
        compare(harness.actions[0].index, 0);
        compare(harness.actions[0].address, "ps5.local");
        verify(findChild(harness, "manualSavedTitle").visible);
        verify(findChild(harness, "manualSavedRegistration").text.includes("first connection"));
        press(findChild(harness, "manualBackToConsoles"));
        compare(harness.actions[1].name, "close");
    }
}
