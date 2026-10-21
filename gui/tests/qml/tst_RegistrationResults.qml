import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "RegistrationResults"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; RegistrationHarness {} }
    function field(harness, name) { return findChild(harness.screen, "registration" + name); }
    function activate(control) {
        control.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
    }
    function checkVisible(harness, control) {
        verify(control && control.visible && control.enabled);
        control.forceActiveFocus(Qt.TabFocusReason);
        tryVerify(() => {
            const point = control.mapToItem(harness, 0, 0);
            const scroll = field(harness, "Scroll");
            const viewportPoint = control.mapToItem(scroll, 0, 0);
            return point.x >= 0 && point.y >= 0 && point.x + control.width <= harness.width + 1
                && point.y + control.height <= harness.height + 1 && viewportPoint.x >= 0 && viewportPoint.y >= 0
                && viewportPoint.x + control.width <= scroll.width + 1 && viewportPoint.y + control.height <= scroll.height + 1;
        }, 1000, control.objectName + " is inside its viewport");
        verify(control.activeFocus);
    }
    function start(harness) {
        field(harness, "AccountId").text = "EjRWeJCrze8=";
        field(harness, "Pin").text = "00123456";
        activate(field(harness, "Submit"));
    }
    function test_acceptedRequestShowsProgressThenSuccess() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        start(harness);
        tryCompare(field(harness, "Progress"), "visible", true);
        verify(field(harness, "ProgressIndicator").running);
        keyClick(Qt.Key_Menu);
        compare(harness.backend.requests.length, 1);
        harness.backend.callback("controlled diagnostic", true, false);
        harness.backend.callback("", true, true);
        tryCompare(field(harness, "Success"), "visible", true);
        tryVerify(() => field(harness, "Done").activeFocus);
        verify(field(harness, "ResultDetails").text.indexOf("console.local") >= 0);
        activate(field(harness, "Done"));
        compare(harness.actions[0].name, "consoles");
        compare(harness.backend.requests.length, 1);
    }
    function test_failureRetainsDetailsAndRetryOwnsItsResult() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        field(harness, "ConsolePin").text = "0042";
        start(harness);
        const oldCallback = harness.backend.callback;
        oldCallback("first diagnostic", true, false);
        oldCallback("", false, true);
        tryCompare(field(harness, "Failure"), "visible", true);
        verify(field(harness, "Failure").text.indexOf("PIN") < 0);
        oldCallback("", true, true);
        verify(field(harness, "Failure").visible);
        activate(field(harness, "Diagnostics"));
        verify(field(harness, "Log").visible);
        compare(field(harness, "LogText").text, "first diagnostic\n");
        activate(field(harness, "Details"));
        tryVerify(() => field(harness, "Host").activeFocus);
        compare(field(harness, "Host").text, "console.local");
        compare(field(harness, "Pin").text, "00123456");
        compare(field(harness, "ConsolePin").text, "0042");
        activate(field(harness, "Submit"));
        compare(harness.backend.requests.length, 2);
        oldCallback("late diagnostic", true, false);
        oldCallback("", true, true);
        verify(field(harness, "Progress").visible);
        harness.backend.callback("second diagnostic", true, false);
        harness.backend.callback("", true, true);
        activate(field(harness, "Diagnostics"));
        compare(field(harness, "LogText").text, "second diagnostic\n");
    }
    function test_formKeepsItsOriginalConsoleContext() {
        const harness = createTemporaryObject(harnessComponent, testCase, {initialHost: "edited.local",
            initialContext: {profileGeneration: 1, manualId: 7, manualAddress: "original.local", discovered: false}});
        start(harness);
        harness.backend.callback("", false, true);
        activate(field(harness, "Details"));
        activate(field(harness, "Submit"));
        compare(harness.backend.requests.length, 2);
        for (const request of harness.backend.requests) {
            compare(request.context.manualId, 7);
            compare(request.context.manualAddress, "original.local");
        }
        harness.backend.callback("", true, true);
        verify(field(harness, "ResultDetails").text.indexOf("original.local") >= 0);
        verify(field(harness, "ResultDetails").text.indexOf("edited.local") < 0);
    }
    function test_dismissalAndDestructionIgnoreLateCallbacks_data() {
        return [{tag: "close", destroy: false}, {tag: "destroy", destroy: true}];
    }
    function test_keyboardCanInspectAndLeaveDiagnostics() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        start(harness);
        activate(field(harness, "Diagnostics"));
        keyClick(Qt.Key_Tab);
        verify(field(harness, "LogText").activeFocus);
        keyClick(Qt.Key_Tab);
        verify(field(harness, "CloseProgress").activeFocus);
        keyClick(Qt.Key_Tab, Qt.ShiftModifier);
        verify(field(harness, "LogText").activeFocus);
        keyClick(Qt.Key_Escape);
        compare(harness.actions[0].name, "close");
        compare(harness.backend.requests.length, 1);
    }
    function test_longFormActionsStayInsidePanel() {
        const harness = createTemporaryObject(harnessComponent, testCase, {width: 900, height: 600});
        field(harness, "AccountId").text = "EjRWeJCrze8=";
        field(harness, "Pin").text = "00123456";
        field(harness, "Cancel").text = "Cancel this console registration ".repeat(3);
        field(harness, "Submit").text = "Register this console in the console library ".repeat(3);
        waitForRendering(harness);
        checkVisible(harness, field(harness, "Cancel"));
        checkVisible(harness, field(harness, "Submit"));
        compare(harness.backend.requests.length, 0);
    }
    function test_dismissalAndDestructionIgnoreLateCallbacks(data) {
        const harness = createTemporaryObject(harnessComponent, testCase);
        start(harness);
        const oldCallback = harness.backend.callback;
        if (!data.destroy) {
            verify(field(harness, "DismissHelp").text.indexOf("does not cancel") >= 0);
            activate(field(harness, "CloseProgress"));
            oldCallback("late", true, true);
            verify(!field(harness, "Success").visible);
            compare(harness.actions.length, 1);
        }
        harness.screenActive = false;
        oldCallback("destroyed diagnostic", true, false);
        oldCallback("", true, true);
        harness.screenActive = true;
        verify(field(harness, "Submit").visible);
        oldCallback("", true, true);
        verify(field(harness, "Submit").visible);
        start(harness);
        oldCallback("", false, true);
        verify(field(harness, "Progress").visible);
    }
    function test_profileChangeInvalidatesFormAndInFlightResult_data() {
        return [{tag: "form", active: false}, {tag: "progress", active: true}];
    }
    function test_profileChangeInvalidatesFormAndInFlightResult(data) {
        const harness = createTemporaryObject(harnessComponent, testCase);
        if (data.active) start(harness);
        harness.settings.currentProfile = "profile-b";
        harness.settings.currentProfile = "profile-a";
        if (data.active) {
            harness.backend.callback("", true, true);
            verify(field(harness, "Failure").visible);
            verify(!field(harness, "Details").visible);
            verify(field(harness, "ResultMessage").text.indexOf("profile changed") >= 0);
        } else {
            field(harness, "AccountId").text = "EjRWeJCrze8=";
            field(harness, "Pin").text = "00123456";
            verify(!field(harness, "Submit").enabled);
            keyClick(Qt.Key_Menu);
            compare(harness.backend.requests.length, 0);
        }
    }
    function test_resultsAndDiagnosticsStayReachable_data() {
        return [{tag: "minimum", width: 900, height: 600}, {tag: "normal", width: 1280, height: 720}];
    }
    function test_resultsAndDiagnosticsStayReachable(data) {
        const harness = createTemporaryObject(harnessComponent, testCase, {width: data.width, height: data.height});
        start(harness);
        const longLine = "diagnostic ".repeat(100);
        for (let i = 0; i < 12; i++) harness.backend.callback(longLine, true, false);
        activate(field(harness, "Diagnostics"));
        const log = field(harness, "LogText");
        log.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Down);
        for (const name of ["Diagnostics", "LogText", "CloseProgress"]) {
            const control = field(harness, name);
            control.forceActiveFocus(Qt.TabFocusReason);
            wait(30);
            if (name !== "LogText") checkVisible(harness, control);
            else {
                const viewport = field(harness, "Log");
                tryVerify(() => {
                    const point = viewport.mapToItem(field(harness, "Scroll"), 0, 0);
                    return point.y >= 0 && point.y + viewport.height <= field(harness, "Scroll").height + 1;
                });
            }
        }
        harness.settings.streamerMode = true;
        verify(!field(harness, "Log").visible);
        verify(!field(harness, "Diagnostics").visible);
        compare(log.text, "");
        verify(field(harness, "ResultDetails").text.indexOf("console.local") < 0);
        harness.backend.callback("private address", false, true);
        verify(!field(harness, "ResultMessage").visible);
        verify(field(harness, "Failure").visible);
    }
    function test_longActionsAndRenderedPanels_data() {
        const rows = [];
        for (const width of [900, 1280])
            for (const phase of ["progress", "success", "failure"])
                rows.push({tag: width + "-" + phase, width: width, height: width === 900 ? 600 : 720, phase: phase});
        return rows;
    }
    function test_longActionsAndRenderedPanels(data) {
        const harness = createTemporaryObject(harnessComponent, testCase, {width: data.width, height: data.height});
        start(harness);
        harness.backend.callback("Long diagnostic ".repeat(100), true, false);
        if (data.phase !== "progress") harness.backend.callback("", data.phase === "success", true);
        const action = field(harness, data.phase === "progress" ? "CloseProgress" : data.phase === "success" ? "Done" : "Details");
        action.text = "Return from console registration to the previous screen ".repeat(3);
        waitForRendering(harness);
        checkVisible(harness, action);
        verify(action.contentItem.contentWidth <= action.contentItem.width + 1, "Long action text wraps");
        field(harness, "Diagnostics").forceActiveFocus(Qt.TabFocusReason);
        waitForRendering(harness);
        const image = grabImage(harness);
        compare(image.width, data.width);
        compare(image.height, data.height);
        verify(image.blue(1, 1) > image.red(1, 1), "Dark blue registration workspace");
    }
}
