import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: tests
    name: "LiveConsolePin"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; LiveConsolePinHarness {} }
    function test_currentSessionCanSubmitOnceWithLeadingZeros() {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.backend.request();
        tryVerify(() => !!findChild(harness.prompt, "consoleLoginPin"));
        const field = findChild(harness.prompt, "consoleLoginPin");
        verify(field);
        tryCompare(field, "activeFocus", true);
        compare(field.echoMode, TextInput.Password);
        verify(!findChild(harness.prompt, "pinSubmit").enabled);
        field.text = "0123";
        keyClick(Qt.Key_Return);
        keyClick(Qt.Key_Return);
        compare(harness.backend.commands.length, 1);
        verify(harness.backend.received === "0123", "Leading zero text reaches only the live submission");
        compare(harness.backend.saved, "unchanged synthetic saved PIN");
        verify(field.text === "", "Submission clears transient PIN text");
        compare(harness.backend.window.grabs, 1);
    }
    function test_cancelWaitsForNativeCleanup_data() {
        return [{tag: "button", escape: false}, {tag: "escape", escape: true}, {tag: "submitting", submitting: true},
            {tag: "escape-while-submitting", submitting: true, escape: true}, {tag: "ctrl-q", ctrlQ: true}, {tag: "right-click", rightClick: true}];
    }
    function test_cancelWaitsForNativeCleanup(data) {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.backend.request();
        tryVerify(() => !!findChild(harness.prompt, "consoleLoginPin"));
        const field = findChild(harness.prompt, "consoleLoginPin");
        field.text = "0123";
        if (data.submitting) {
            keyClick(Qt.Key_Return);
            compare(harness.backend.commands.length, 1);
        }
        if (data.ctrlQ) keyClick(Qt.Key_Q, Qt.ControlModifier);
        else if (data.rightClick) mouseClick(harness.prompt, 2, 2, Qt.RightButton);
        else if (data.escape) keyClick(Qt.Key_Escape);
        else mouseClick(findChild(harness.prompt, "pinCancel"));
        keyClick(Qt.Key_Escape);
        compare(harness.backend.commands.filter(command => command.name === "cancel").length, 1);
        compare(findChild(harness.prompt, "pinHeading").text, "Cancelling connection");
        verify(field.text === "", "Cancellation clears transient input");
        wait(30);
        compare(harness.actions.length, 0);
        harness.backend.change({phase: "cancelled"});
        const back = findChild(harness.prompt, "pinBack");
        tryVerify(() => back.activeFocus);
        keyClick(Qt.Key_Return);
        compare(harness.actions.length, 1);
        compare(harness.actions[0], "consoles");
        compare(harness.backend.window.grabs, 1);
        compare(harness.backend.window.releases, 1);
    }
    function test_invalidInputCannotSubmit_data() {
        return ["", "123", "abcd", "١٢٣٤", "12\n3"].map((value, index) => ({tag: String(index), value}));
    }
    function test_invalidInputCannotSubmit(data) {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.backend.request();
        tryVerify(() => !!findChild(harness.prompt, "consoleLoginPin"));
        findChild(harness.prompt, "consoleLoginPin").text = data.value;
        verify(!findChild(harness.prompt, "pinSubmit").enabled);
        keyClick(Qt.Key_Return);
        compare(harness.backend.commands.length, 0);
    }
    function test_incorrectSavedPinAndRejectionShowFreshMaskedEntry() {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.backend.request(true);
        tryVerify(() => !!findChild(harness.prompt, "consoleLoginPin"));
        compare(findChild(harness.prompt, "pinHeading").text, "Incorrect console login PIN");
        findChild(harness.prompt, "consoleLoginPin").text = "0123";
        mouseClick(findChild(harness.prompt, "pinReveal"));
        findChild(harness.prompt, "consoleLoginPin").forceActiveFocus();
        keyClick(Qt.Key_Return);
        compare(harness.backend.commands.length, 1);
        compare(findChild(harness.prompt, "pinHeading").text, "Signing in");
        verify(!findChild(harness.prompt, "pinSubmit").enabled);
        harness.backend.change({id: "request2", phase: "form", incorrect: true});
        const field = findChild(harness.prompt, "consoleLoginPin");
        tryCompare(field, "activeFocus", true);
        verify(field.text === "", "A rejected PIN is cleared");
        compare(field.echoMode, TextInput.Password);
        field.text = "0456";
        keyClick(Qt.Key_Return);
        compare(harness.backend.commands.length, 2);
        compare(harness.backend.commands[1].id, "request2");
        compare(harness.backend.window.grabs, 1);
    }
    function test_privacyHidesRevealedPinAndConsoleIdentity() {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.backend.request();
        tryVerify(() => !!findChild(harness.prompt, "consoleLoginPin"));
        const field = findChild(harness.prompt, "consoleLoginPin");
        field.text = "0123";
        mouseClick(findChild(harness.prompt, "pinReveal"));
        compare(field.echoMode, TextInput.Normal);
        harness.backend.settings.streamerMode = true;
        compare(field.echoMode, TextInput.Password);
        verify(!findChild(harness.prompt, "pinReveal").enabled);
        compare(findChild(harness.prompt, "pinConsoleName").text, "Console details hidden");
        harness.backend.settings.streamerMode = false;
        compare(field.echoMode, TextInput.Password);
    }
    function test_routeHandoffRetainsOnePromptAndReleasesAfterNativeAcceptance() {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.backend.connectionAttempt = {id: "1", route: "psn", phase: "data-connection"};
        harness.backend.request();
        tryVerify(() => !!findChild(harness.prompt, "consoleLoginPin"));
        const field = findChild(harness.prompt, "consoleLoginPin");
        field.text = "0123";
        harness.backend.connectionAttempt = {id: "1", route: "psn", phase: "video"};
        harness.backend.change({name: "Living room"});
        verify(findChild(harness.prompt, "consoleLoginPin") === field);
        verify(field.text === "0123", "A repeated current request keeps entry across route handoff");
        compare(harness.backend.window.grabs, 1);
        keyClick(Qt.Key_Return);
        harness.backend.sessionPinRequest = ({});
        verify(!harness.prompt.blocking);
        compare(harness.backend.window.releases, 1);
        compare(harness.actions.length, 0);
        tryVerify(() => findChild(harness, "livePinOpener").activeFocus);
    }
    function test_replacementAndRemovalClearInputWithoutSendingToNewRequest() {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.backend.request();
        tryVerify(() => !!findChild(harness.prompt, "consoleLoginPin"));
        findChild(harness.prompt, "consoleLoginPin").text = "0123";
        const oldId = harness.backend.sessionPinRequest.id;
        harness.backend.change({id: "request2", attemptId: "2"});
        verify(findChild(harness.prompt, "consoleLoginPin").text === "", "Replacement clears entry");
        verify(!harness.backend.enterPin(oldId, "0123"));
        verify(!harness.backend.cancelSessionPin(oldId));
        harness.backend.sessionPinRequest = ({});
        compare(harness.backend.commands.length, 0);
        compare(harness.backend.window.grabs, 1);
        compare(harness.backend.window.releases, 1);
    }
    function test_compactWindowCloseUsesTheSameCancelAndWaits() {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.backend.window.runtimeRendererBackend = 1;
        harness.backend.request();
        const window = findChild(harness.prompt, "liveConsolePinWindow");
        tryVerify(() => window.visible);
        tryVerify(() => !!findChild(window.contentItem, "consoleLoginPin"));
        const field = findChild(window.contentItem, "consoleLoginPin");
        tryCompare(field, "activeFocus", true);
        compare(field.echoMode, TextInput.Password);
        field.text = "0123";
        window.close();
        window.close();
        compare(harness.backend.commands.length, 1);
        compare(harness.backend.commands[0].name, "cancel");
        verify(window.visible);
        compare(findChild(window.contentItem, "pinHeading").text, "Cancelling connection");
        harness.backend.change({phase: "cancelled"});
        const back = findChild(window.contentItem, "pinBack");
        tryVerify(() => back.activeFocus);
        keyClick(Qt.Key_Return);
        tryVerify(() => !window.visible);
        compare(harness.backend.window.grabs, 1);
        compare(harness.backend.window.releases, 1);
        compare(harness.actions.length, 1);
    }
    function test_contentAndActionsRemainReachable_data() {
        return [{tag: "900", width: 900, height: 600}, {tag: "1280", width: 1280, height: 720},
            {tag: "compact", width: 620, height: 480, separate: true}];
    }
    function test_contentAndActionsRemainReachable(data) {
        const harness = createTemporaryObject(harnessComponent, tests, {width: data.width, height: data.height});
        harness.backend.window.runtimeRendererBackend = data.separate ? 1 : 0;
        harness.backend.request(true);
        const root = data.separate ? findChild(harness.prompt, "liveConsolePinWindow").contentItem : harness.prompt;
        tryVerify(() => !!findChild(root, "consoleLoginPin"));
        harness.backend.change({name: "A long synthetic console name ".repeat(10)});
        findChild(root, "pinHeading").text = "Please sign in to this console user ".repeat(5);
        findChild(root, "consoleLoginPin").text = "0123";
        wait(30);
        for (const name of ["consoleLoginPin", "pinReveal", "pinCancel", "pinSubmit"]) {
            const control = findChild(root, name);
            control.forceActiveFocus();
            const scroll = findChild(root, "pinScroll");
            tryCompare(control, "activeFocus", true);
            tryVerify(() => {
                const point = control.mapToItem(scroll, 0, 0);
                return point.x >= -1 && point.y >= -1 && point.x + control.width <= scroll.width + 1 && point.y + control.height <= scroll.height + 1;
            }, 5000, name + " is reachable");
        }
        compare(harness.backend.commands.length, 0);
    }
}
