import QtQuick
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "ConnectionPsn"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; ConnectionHarness {} }
    function control(harness, name) { return findChild(harness.screen, "connection" + name); }
    function test_remoteProgressUsesActualStages() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({route: "psn", phase: "internet", address: ""});
        compare(control(harness, "Route").text, "Remote connection");
        compare(control(harness, "Title").text, "Waiting for internet");
        compare(control(harness, "Address").text, "PlayStation Network");
        harness.backend.change({phase: "initiating"});
        compare(control(harness, "Title").text, "Contacting PlayStation Network");
        harness.backend.change({phase: "linking", cancelable: false});
        compare(control(harness, "Title").text, "Linking your console");
        verify(!control(harness, "Cancel").enabled);
        keyClick(Qt.Key_Escape);
        keyClick(Qt.Key_Q, Qt.ControlModifier);
        mouseClick(harness.screen, 20, 20, Qt.RightButton);
        compare(harness.backend.commands.length, 0);
        harness.backend.change({phase: "data", cancelable: true});
        compare(control(harness, "Title").text, "Opening the data connection");
        harness.backend.change({phase: "video", cancelable: false});
        compare(control(harness, "Title").text, "Waiting for video");
        verify(control(harness, "Description").text.indexOf("first video frame") >= 0);
        compare(harness.actions.length, 0);
    }
    function test_onlyObservedStagesAreCompletedAndRegistrationIsConditional() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({route: "psn", phase: "linking", completedStages: ["psn"]});
        compare(control(harness, "Completed").text, "✓ Reached PlayStation Network");
        harness.backend.change({phase: "data", completedStages: ["psn", "linking"]});
        compare(control(harness, "Completed").text, "✓ Reached PlayStation Network\n✓ Console linked");
        harness.backend.change({phase: "registering", registrationOnly: true, cancelable: false});
        compare(control(harness, "Title").text, "Registering your console");
        harness.backend.change({phase: "registered", completedStages: ["psn", "registration"], cleanupPending: true});
        compare(control(harness, "Title").text, "Console registered");
        verify(!control(harness, "Back").enabled);
        verify(!control(harness, "Retry").visible);
        wait(2150);
        compare(control(harness, "Title").text, "Console registered");
        harness.backend.change({cleanupPending: false});
        tryVerify(() => control(harness, "Back").activeFocus);
        keyClick(Qt.Key_Return);
        compare(harness.actions[0], "consoles");
    }
    function test_failureRecovery_data() {
        return [
            {tag: "start", title: "PSN connection did not start"},
            {tag: "general", title: "Could not connect over PSN"},
            {tag: "unreachable", title: "Console is unreachable"},
            {tag: "registration", title: "Registration failed", registrationOnly: true}
        ];
    }
    function test_failureRecovery(data) {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({route: "psn", phase: "failure", errorTitle: data.title,
            errorText: "Synthetic diagnostic <b>plain text</b>", registrationOnly: !!data.registrationOnly});
        compare(control(harness, "Title").text, data.title);
        verify(!control(harness, "Retry").enabled);
        control(harness, "DetailsToggle").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(control(harness, "Details").text, "Synthetic diagnostic <b>plain text</b>");
        compare(control(harness, "Details").textFormat, Text.PlainText);
        wait(2150);
        compare(control(harness, "Title").text, data.title);
        harness.backend.change({cleanupPending: false});
        tryVerify(() => control(harness, "Retry").activeFocus);
        keyClick(Qt.Key_Return);
        keyClick(Qt.Key_Return);
        compare(harness.backend.commands.length, 1);
        compare(harness.backend.commands[0].name, "retry");
        compare(harness.backend.commands[0].id, "1");
        harness.backend.change({id: "2", phase: "internet", cleanupPending: false});
        compare(control(harness, "Title").text, "Waiting for internet");
        verify(control(harness, "Cancel").enabled);
    }
    function test_cancellationWaitsForCleanupAndInvalidTargetCannotRetry() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.backend.change({route: "psn", phase: "data", cancelable: true});
        control(harness, "Cancel").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        keyClick(Qt.Key_Escape);
        compare(harness.backend.commands.length, 1);
        compare(harness.backend.commands[0].name, "cancel");
        harness.backend.change({phase: "cancelling"});
        compare(control(harness, "Title").text, "Cancelling remote connection");
        verify(!control(harness, "Cancel").enabled);
        wait(2150);
        compare(harness.actions.length, 0);
        harness.backend.change({phase: "cancelled", cleanupPending: false});
        verify(control(harness, "Back").enabled);
        harness.backend.change({phase: "failure", targetValid: false});
        verify(!control(harness, "Retry").enabled);
        verify(control(harness, "InvalidTarget").visible);
    }
    function test_privacyAndLongContent_data() {
        return [{tag: "900x600", width: 900, height: 600}, {tag: "1280x720", width: 1280, height: 720}];
    }
    function test_privacyAndLongContent(data) {
        const harness = createTemporaryObject(harnessComponent, testCase, {width: data.width, height: data.height});
        harness.backend.change({route: "psn", phase: "failure", cleanupPending: false,
            name: "Private synthetic console ".repeat(12), errorTitle: "Private console failure ".repeat(6),
            errorText: "Private diagnostic detail ".repeat(30)});
        control(harness, "DetailsToggle").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        for (const name of ["Back", "Retry"]) {
            const item = control(harness, name);
            item.forceActiveFocus(Qt.TabFocusReason);
            const scroll = control(harness, "Scroll");
            tryVerify(() => {
                const p = item.mapToItem(scroll, 0, 0);
                return p.y >= -1 && p.y + item.height <= scroll.height + 1;
            });
        }
        harness.backend.settings.streamerMode = true;
        compare(control(harness, "Console").text, "Your console");
        compare(control(harness, "Title").text, "Could not start Remote Play");
        compare(control(harness, "Details").text, "Connection details are hidden in streamer mode.");
        compare(control(harness, "Address").text, "PlayStation Network");
    }
}
