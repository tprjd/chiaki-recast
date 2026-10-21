import QtQuick
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "ConnectionLocal"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; ConnectionHarness {} }
    function control(harness, name) { return findChild(harness.screen, "connection" + name); }
    function activate(item) { item.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return); }
    function test_localProgressShowsConsoleRouteAndRealState() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        compare(control(harness, "Title").text, "Connecting to console");
        compare(control(harness, "Console").text, "Living room");
        compare(control(harness, "Route").text, "Local network");
        verify(control(harness, "Help").visible);
        harness.backend.change({phase: "video"});
        compare(control(harness, "Title").text, "Waiting for video");
        verify(control(harness, "Description").text.indexOf("first video frame") >= 0);
        compare(harness.backend.commands.length, 0);
    }
    function test_failureKeepsDetailsAndWaitsForCleanupBeforeRetry() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({phase: "failure", errorTitle: "Remote Play did not start", errorText: "Synthetic start failure <b>plain text</b>"});
        compare(control(harness, "Title").text, "Remote Play did not start");
        verify(!control(harness, "Retry").enabled);
        keyClick(Qt.Key_Escape);
        compare(harness.actions.length, 0);
        compare(harness.backend.commands.length, 0);
        activate(control(harness, "DetailsToggle"));
        compare(control(harness, "Details").text, "Synthetic start failure <b>plain text</b>");
        compare(control(harness, "Details").textFormat, Text.PlainText);
        wait(2150);
        compare(control(harness, "Title").text, "Remote Play did not start");
        harness.backend.change({cleanupPending: false});
        tryVerify(() => control(harness, "Retry").activeFocus);
        activate(control(harness, "Retry"));
        keyClick(Qt.Key_Return);
        compare(harness.backend.commands.length, 1);
        compare(harness.backend.commands[0].name, "retry");
        compare(harness.backend.commands[0].id, "1");
        harness.backend.change({id: "2", phase: "connecting", cleanupPending: true});
        compare(control(harness, "Title").text, "Connecting to console");
        verify(!control(harness, "Details").visible);
        verify(control(harness, "Cancel").enabled);
    }
    function test_cancelStopsOnceAndWaitsForTheNativeResult() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        activate(control(harness, "Cancel"));
        keyClick(Qt.Key_Escape);
        keyClick(Qt.Key_Q, Qt.ControlModifier);
        mouseClick(harness.screen, 20, 20, Qt.RightButton);
        compare(harness.backend.commands.length, 1);
        compare(harness.backend.commands[0].name, "cancel");
        harness.backend.change({phase: "cancelling"});
        compare(control(harness, "Title").text, "Cancelling connection");
        verify(!control(harness, "Cancel").enabled);
        wait(2150);
        compare(control(harness, "Title").text, "Cancelling connection");
        compare(harness.actions.length, 0);
        harness.backend.change({phase: "cancelled", cleanupPending: false});
        tryVerify(() => control(harness, "Back").activeFocus);
        activate(control(harness, "Back"));
        keyClick(Qt.Key_Return);
        compare(harness.actions.length, 1);
        compare(harness.actions[0], "consoles");
        compare(harness.backend.commands.length, 2);
        compare(harness.backend.commands[1].name, "dismiss");
    }
    function test_disabledCancelCannotBeBypassed() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({cancelable: false});
        verify(!control(harness, "Cancel").enabled);
        verify(control(harness, "CancelReason").text.indexOf("before cancelling") >= 0);
        keyClick(Qt.Key_Escape);
        keyClick(Qt.Key_Q, Qt.ControlModifier);
        mouseClick(harness.screen, 20, 20, Qt.RightButton);
        keyClick(Qt.Key_Return);
        compare(harness.backend.commands.length, 0);
    }
    function test_targetOrProfileInvalidationDisablesRetry() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({phase: "failure", cleanupPending: false, targetValid: false});
        verify(!control(harness, "Retry").enabled);
        verify(control(harness, "InvalidTarget").visible);
        tryVerify(() => control(harness, "Back").activeFocus);
        keyClick(Qt.Key_Return);
        compare(harness.actions[0], "consoles");
        compare(harness.backend.commands[0].name, "dismiss");
    }
    function test_streamerModeHidesNamesAddressesAndBackendDetails() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({phase: "failure", cleanupPending: false, name: "<b>Private console</b>", errorTitle: "Private console failed", errorText: "Private console at 192.0.2.42"});
        compare(control(harness, "Console").textFormat, Text.PlainText);
        activate(control(harness, "DetailsToggle"));
        harness.backend.settings.streamerMode = true;
        compare(control(harness, "Console").text, "Your console");
        compare(control(harness, "Address").text, "Address hidden");
        compare(control(harness, "Title").text, "Could not start Remote Play");
        compare(control(harness, "Details").text, "Connection details are hidden in streamer mode.");
    }
    function test_viewRemovalDismissesTheAttemptAndCannotNavigateLater() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.screenActive = false;
        compare(harness.backend.commands.length, 1);
        compare(harness.backend.commands[0].name, "dismiss");
        harness.backend.change({phase: "ready"});
        harness.backend.change({phase: "failure", errorTitle: "Late error"});
        compare(harness.actions.length, 0);
    }
    function test_exitingViewCannotDismissReplacementAttempt() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.screen.retiring = true;
        harness.backend.change({id: "2", phase: "connecting"});
        harness.screenActive = false;
        compare(harness.backend.commands.length, 0);
    }
    function test_longContentAndActionsRemainReachable_data() {
        return [{tag: "900x600", width: 900, height: 600}, {tag: "1280x720", width: 1280, height: 720}];
    }
    function test_longContentAndActionsRemainReachable(data) {
        const harness = createTemporaryObject(harnessComponent, testCase, {width: data.width, height: data.height});
        harness.backend.change({phase: "failure", cleanupPending: false, name: "A long synthetic console name ".repeat(12), errorTitle: "A long connection error title ".repeat(6), errorText: "A long plain diagnostic message ".repeat(30)});
        control(harness, "Back").text = "Return to your console library ".repeat(4);
        control(harness, "Retry").text = "Try this connection again ".repeat(4);
        activate(control(harness, "DetailsToggle"));
        for (const action of [control(harness, "Back"), control(harness, "Retry")]) {
            action.forceActiveFocus(Qt.TabFocusReason);
            const scroll = control(harness, "Scroll");
            tryVerify(() => {
                const point = action.mapToItem(scroll, 0, 0);
                return point.x >= -1 && point.y >= -1 && point.x + action.width <= scroll.width + 1 && point.y + action.height <= scroll.height + 1;
            });
            verify(action.activeFocus);
        }
        compare(harness.backend.commands.length, 0);
    }
}
