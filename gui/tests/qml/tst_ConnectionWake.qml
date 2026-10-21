import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"
import "../../src/qml"

TestCase {
    id: testCase
    name: "ConnectionWake"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; ConnectionHarness { wakeScreen: true } }
    Component {
        id: stackComponent
        StackView {
            id: stack
            width: 1280; height: 720
            property var backend
            property var navigation
            initialItem: Component { AutoConnectView { backend: stack.backend; navigation: stack.navigation } }
            replaceExit: Transition { NumberAnimation { property: "opacity"; to: 0; duration: 200 } }
        }
    }
    Component { id: blankComponent; Item {} }
    function control(harness, name) { return findChild(harness.screen, "connection" + name); }
    function activate(item) { item.forceActiveFocus(Qt.TabFocusReason); keyClick(Qt.Key_Return); }
    function test_startupWaitIdentifiesSelectedConsole() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({phase: "waiting", context: "startup", waitingForWake: true, cleanupPending: false});
        compare(control(harness, "Heading").text, "Auto-connect");
        compare(control(harness, "Title").text, "Waiting for console");
        compare(control(harness, "Console").text, "Living room");
        compare(control(harness, "Route").text, "Local discovery");
        verify(control(harness, "Cancel").enabled);
    }
    function test_manualWakeUsesItsOwnContextAndWaitsForActualResult() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({phase: "waking", context: "wake", waitingForWake: true});
        compare(control(harness, "Heading").text, "Wake console");
        compare(control(harness, "Title").text, "Waiting for the console to wake");
        verify(control(harness, "Description").text.indexOf("wake request was sent") >= 0);
        wait(1600);
        compare(control(harness, "Title").text, "Waiting for the console to wake");
        compare(harness.backend.commands.length, 0);
        harness.backend.change({phase: "found"});
        compare(control(harness, "Title").text, "Console found");
        verify(control(harness, "Description").text.indexOf("Remote Play connection") >= 0);
    }
    function test_timeoutPersistsAndRetryWaitsForCleanup() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({phase: "failure", context: "wake", waitingForWake: true,
            errorTitle: "Console did not respond", errorText: "Timed out waiting for the console to wake."});
        compare(control(harness, "Title").text, "Console did not respond");
        verify(!control(harness, "Retry").enabled);
        wait(2150);
        compare(harness.actions.length, 0);
        compare(control(harness, "Title").text, "Console did not respond");
        harness.backend.change({cleanupPending: false});
        activate(control(harness, "Retry"));
        keyClick(Qt.Key_Return);
        compare(harness.backend.commands.length, 1);
        compare(harness.backend.commands[0].name, "retry");
        harness.backend.change({id: "2", phase: "waking", cleanupPending: true, errorTitle: "", errorText: ""});
        verify(control(harness, "Cancel").enabled);
    }
    function test_cancelAndTimeoutCannotSendCompetingCommands() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({phase: "waking", context: "wake", waitingForWake: true});
        activate(control(harness, "Cancel"));
        keyClick(Qt.Key_Escape);
        keyClick(Qt.Key_Q, Qt.ControlModifier);
        mouseClick(harness.screen, 10, 10, Qt.RightButton);
        compare(harness.backend.commands.length, 1);
        harness.backend.change({phase: "cancelling"});
        verify(!control(harness, "Cancel").enabled);
        compare(harness.actions.length, 0);
        harness.backend.change({phase: "cancelled", cleanupPending: false});
        tryVerify(() => control(harness, "Back").activeFocus);
        activate(control(harness, "Back"));
        compare(harness.actions.length, 1);
        compare(harness.backend.commands.length, 2);
    }
    function test_handoffKeepsAttemptOwnership() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({phase: "found", context: "startup", waitingForWake: true});
        harness.backend.change({phase: "connecting", waitingForWake: false});
        harness.screenActive = false;
        compare(harness.backend.commands.length, 0);
        compare(harness.actions.length, 0);
    }
    function test_oldExitCannotDismissANewWait() {
        const harness = createTemporaryObject(harnessComponent, testCase, {screenActive: false});
        harness.backend.change({phase: "waking", context: "wake", waitingForWake: true});
        const stack = createTemporaryObject(stackComponent, testCase, {backend: harness.backend, navigation: harness});
        harness.backend.change({phase: "connecting", waitingForWake: false});
        stack.replace(blankComponent);
        harness.backend.change({id: "2", phase: "waiting", context: "startup", waitingForWake: true});
        wait(300);
        compare(harness.backend.commands.length, 0);
    }
    function test_fallbackAppearsOnlyWhenThePsnRouteStarts() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({phase: "waiting", context: "startup", waitingForWake: true});
        verify(!control(harness, "Fallback").visible);
        harness.backend.change({route: "psn", phase: "initiating", waitingForWake: false, fallback: true});
        verify(control(harness, "Fallback").visible);
        verify(control(harness, "Fallback").text.indexOf("Continuing through PSN") >= 0);
        harness.screenActive = false;
        compare(harness.backend.commands.length, 0);
    }
    function test_closedWaitDismissesAndLateResultsCannotNavigate() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({phase: "waiting", context: "startup", waitingForWake: true, cleanupPending: false});
        harness.screenActive = false;
        compare(harness.backend.commands.length, 1);
        compare(harness.backend.commands[0].name, "dismiss");
        harness.backend.change({phase: "found"});
        harness.backend.change({phase: "connecting", route: "psn"});
        compare(harness.actions.length, 0);
    }
    function test_invalidTargetHasAUsefulReturnAndNoRetry() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({phase: "failure", context: "startup", waitingForWake: true,
            cleanupPending: false, targetValid: false, name: "<b>Private console</b>"});
        verify(!control(harness, "Retry").enabled);
        verify(control(harness, "InvalidTarget").visible);
        compare(control(harness, "Console").textFormat, Text.PlainText);
        harness.backend.settings.streamerMode = true;
        compare(control(harness, "Console").text, "Your console");
        compare(control(harness, "Address").text, "Address hidden");
        activate(control(harness, "Back"));
        compare(harness.actions.length, 1);
    }
    function test_longWakeContentAndFocusRemainReachable_data() {
        return [{tag: "900x600", width: 900, height: 600}, {tag: "1280x720", width: 1280, height: 720}];
    }
    function test_longWakeContentAndFocusRemainReachable(data) {
        const harness = createTemporaryObject(harnessComponent, testCase, {width: data.width, height: data.height});
        harness.backend.change({phase: "failure", context: "wake", waitingForWake: true, cleanupPending: false,
            name: "A long synthetic console name ".repeat(12), errorTitle: "Console did not respond", errorText: "A long diagnostic message ".repeat(30)});
        activate(control(harness, "DetailsToggle"));
        control(harness, "Retry").forceActiveFocus(Qt.TabFocusReason);
        const scroll = control(harness, "Scroll");
        tryVerify(() => {
            const point = control(harness, "Retry").mapToItem(scroll, 0, 0);
            return point.y >= 0 && point.y + control(harness, "Retry").height <= scroll.height + 1;
        });
        verify(control(harness, "Retry").activeFocus);
        compare(harness.backend.commands.length, 0);
    }
}
