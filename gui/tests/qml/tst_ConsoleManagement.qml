import QtQuick
import QtQuick.Controls
import QtTest
import "../../src/qml"

TestCase {
    id: testCase
    name: "ConsoleManagement"
    width: 1280
    height: 720
    visible: true
    when: windowShown

    Component { id: screenComponent; ConsoleLibrary { width: 1280; height: 720 } }
    SignalSpy { id: actionSpy }
    SignalSpy { id: dialogSpy; signalName: "managementRequested" }
    SignalSpy { id: connectSpy; signalName: "connectRequested" }

    function host(changes) {
        return Object.assign({ display: true, discovered: false, manual: true,
            registered: true, name: "Living room", mac: "00:11:22:33:44:55",
            address: "192.0.2.7", ps5: true }, changes || {});
    }

    function cleanup() {
        for (const spy of [actionSpy, dialogSpy, connectSpy]) {
            spy.target = null;
            spy.clear();
        }
    }

    function openDetails(screen) {
        mouseClick(findChild(screen, "consoleActionsButton"));
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        return details;
    }

    function test_explicitWakeDispatchesOnceWithoutConnecting() {
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host()] });
        actionSpy.signalName = "wakeRequested";
        actionSpy.target = screen;
        connectSpy.target = screen;
        const details = openDetails(screen);
        const wake = findChild(details.contentItem, "wakeAction");
        verify(wake && wake.visible && wake.enabled);
        mouseClick(wake);
        tryCompare(actionSpy, "count", 1);
        compare(actionSpy.signalArguments[0][0], 0);
        compare(connectSpy.count, 0);
        tryCompare(details, "opened", false);
    }

    function test_deleteConfirmationUsesCurrentTargetOnce() {
        const target = host();
        const other = host({ name: "Bedroom", address: "192.0.2.8", mac: "00:11:22:33:44:66" });
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [target, other] });
        dialogSpy.target = screen;
        actionSpy.signalName = "deleteRequested";
        actionSpy.target = screen;
        connectSpy.target = screen;
        const details = openDetails(screen);
        mouseClick(findChild(details.contentItem, "deleteAction"));
        tryCompare(dialogSpy, "count", 1);
        compare(dialogSpy.signalArguments[0][0], "delete");
        const accept = dialogSpy.signalArguments[0][1];
        verify(typeof accept === "function");
        compare(actionSpy.count, 0);
        screen.hosts = [other, target];
        verify(accept());
        compare(actionSpy.count, 1);
        compare(actionSpy.signalArguments[0][0], 1);
        verify(!accept());
        compare(actionSpy.count, 1);
        compare(connectSpy.count, 0);
    }

    function test_hideConfirmationUsesCurrentFieldsOnce() {
        const target = host({ discovered: true, manual: false, registered: false });
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [target] });
        dialogSpy.target = screen;
        actionSpy.signalName = "hideRequested";
        actionSpy.target = screen;
        const details = openDetails(screen);
        mouseClick(findChild(details.contentItem, "hideAction"));
        tryCompare(dialogSpy, "count", 1);
        compare(dialogSpy.signalArguments[0][0], "hide");
        const accept = dialogSpy.signalArguments[0][1];
        compare(actionSpy.count, 0);
        screen.hosts = [Object.assign({}, target, { name: "Studio" })];
        verify(accept());
        compare(actionSpy.count, 1);
        compare(actionSpy.signalArguments[0][0], "00:11:22:33:44:55");
        compare(actionSpy.signalArguments[0][1], "Studio");
        verify(!accept());
        compare(actionSpy.count, 1);
    }

    function test_pinSubmissionValidatesAndUsesCurrentTargetOnce() {
        const target = host();
        const other = host({ address: "192.0.2.8", mac: "00:11:22:33:44:66" });
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [target, other] });
        dialogSpy.target = screen;
        actionSpy.signalName = "pinRequested";
        actionSpy.target = screen;
        connectSpy.target = screen;
        const details = openDetails(screen);
        const pin = findChild(details.contentItem, "pinAction");
        pin.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(dialogSpy, "count", 1);
        compare(dialogSpy.signalArguments[0][0], "pin");
        const submit = dialogSpy.signalArguments[0][1];
        for (const invalid of ["", "123", "12345", "abcd", " 1234 ", "0123\n", "١٢٣٤", 1234]) {
            verify(!submit(invalid));
            compare(actionSpy.count, 0);
        }
        screen.hosts = [other, target];
        verify(submit("0123"));
        compare(actionSpy.count, 1);
        compare(actionSpy.signalArguments[0][0], 1);
        verify(actionSpy.signalArguments[0][1] === "0123", "Save preserves the complete PIN text");
        verify(!submit("4567"));
        compare(actionSpy.count, 1);
        compare(connectSpy.count, 0);
    }

    function hostForAction(action) {
        return host(action === "hide" ? { discovered: true, manual: false, registered: false }
            : action === "delete" ? { discovered: true } : {});
    }

    function test_pinSaveOutcomeReachesTheForm() {
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [host()], savePin: () => false
        });
        dialogSpy.target = screen;
        verify(screen.requestManagement("pin", screen.selectedIdentity));
        verify(!dialogSpy.signalArguments[0][1]("0123"));
        verify(!dialogSpy.signalArguments[0][1]("0123"));
    }

    function test_cancelConsumesPendingAction_data() {
        return ["pin", "delete", "hide"].map(action => ({ tag: action, action }));
    }

    function test_cancelConsumesPendingAction(data) {
        const target = hostForAction(data.action);
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [target] });
        dialogSpy.target = screen;
        actionSpy.signalName = data.action + "Requested";
        actionSpy.target = screen;
        connectSpy.target = screen;
        const details = openDetails(screen);
        mouseClick(findChild(details.contentItem, data.action + "Action"));
        tryCompare(dialogSpy, "count", 1);
        const accept = dialogSpy.signalArguments[0][1];
        const cancel = dialogSpy.signalArguments[0][2];
        verify(typeof cancel === "function");
        cancel();
        verify(!accept("0123"));
        cancel();
        compare(actionSpy.count, 0);
        compare(connectSpy.count, 0);
        compare(screen.hosts.length, 1);
        compare(screen.hosts[0].name, "Living room");
        verify(findChild(screen, "consoleActionsButton").activeFocus);
    }

    function test_staleAcceptanceNeverDispatches_data() {
        const rows = [];
        for (const action of ["pin", "delete", "hide"])
            for (const change of ["removed", "replaced", "hidden", "ambiguous", "profile", "eligibility"])
                rows.push({ tag: action + "-" + change, action, change });
        return rows;
    }

    function test_staleAcceptanceNeverDispatches(data) {
        const target = hostForAction(data.action);
        const other = host({ address: "192.0.2.8", mac: "00:11:22:33:44:66" });
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [target, other] });
        dialogSpy.target = screen;
        actionSpy.signalName = data.action + "Requested";
        actionSpy.target = screen;
        connectSpy.target = screen;
        const details = openDetails(screen);
        mouseClick(findChild(details.contentItem, data.action + "Action"));
        tryCompare(dialogSpy, "count", 1);
        const accept = dialogSpy.signalArguments[0][1];
        if (data.change === "removed") screen.hosts = [other];
        if (data.change === "replaced") screen.hosts = [other, host({ address: "192.0.2.9", mac: "00:11:22:33:44:77" })];
        if (data.change === "hidden") screen.hosts = [Object.assign({}, target, { display: false }), other];
        if (data.change === "ambiguous") screen.hosts = [target, Object.assign({}, target), other];
        if (data.change === "profile") {
            screen.profileKey = "Other profile";
            screen.profileKey = "";
        }
        if (data.change === "eligibility") screen.hosts = [Object.assign({}, target,
            data.action === "pin" ? { registered: false }
            : data.action === "hide" ? { registered: true } : { manual: false }), other];
        verify(!accept("0123"));
        verify(!accept("4567"));
        compare(actionSpy.count, 0);
        compare(connectSpy.count, 0);
    }

    function test_actionsMatchHostEligibility_data() {
        return [
            { tag: "registered-manual", changes: {}, actions: ["pin", "wake", "delete"] },
            { tag: "unregistered-manual", changes: { registered: false }, actions: ["delete"] },
            { tag: "registered-local", changes: { discovered: true, manual: false }, actions: ["pin"] },
            { tag: "unregistered-local", changes: { discovered: true, manual: false, registered: false }, actions: ["hide"] },
            { tag: "discovered-manual", changes: { discovered: true }, actions: ["pin", "delete"] },
            { tag: "remote-psn", changes: { manual: false, address: "", duid: "sample-psn-id" }, actions: ["pin"] }
        ];
    }

    function test_actionsMatchHostEligibility(data) {
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host(data.changes)] });
        const details = openDetails(screen);
        for (const action of ["pin", "wake", "delete", "hide"]) {
            const button = findChild(details.contentItem, action + "Action");
            compare(!!button && button.visible && button.enabled, data.actions.includes(action), action);
        }
        verify(!findChild(screen, "consoleManageButton"));
        verify(!findChild(screen, "consoleActionsMenu"));
        details.close();
    }

    function test_removalAfterAcceptanceRestoresValidFocus() {
        const target = host();
        const other = host({ address: "192.0.2.8", mac: "00:11:22:33:44:66" });
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [target, other] });
        dialogSpy.target = screen;
        screen.deleteRequested.connect(sourceIndex => {
            compare(sourceIndex, 0);
            screen.hosts = [other];
        });
        const details = openDetails(screen);
        mouseClick(findChild(details.contentItem, "deleteAction"));
        tryCompare(dialogSpy, "count", 1);
        verify(dialogSpy.signalArguments[0][1]());
        tryCompare(findChild(screen, "connectButton"), "activeFocus", true);
        compare(screen.selectedIdentity, screen.identityOf(other));
    }

    function test_controllerManagementUsesSameEligibility_data() {
        return [
            { tag: "pin", action: "pin", key: Qt.Key_PageUp, changes: {}, expected: 1 },
            { tag: "no-pin", action: "pin", key: Qt.Key_PageUp, changes: { registered: false }, expected: 0 },
            { tag: "wake", action: "wake", key: Qt.Key_Yes, changes: {}, expected: 1 },
            { tag: "no-local-wake", action: "wake", key: Qt.Key_Yes, changes: { discovered: true }, expected: 0 },
            { tag: "no-psn-wake", action: "wake", key: Qt.Key_Yes, changes: { duid: "sample-psn-id", manual: false }, expected: 0 },
            { tag: "delete", action: "delete", key: Qt.Key_No, changes: {}, expected: 1 },
            { tag: "hide", action: "hide", key: Qt.Key_No, changes: { discovered: true, manual: false, registered: false }, expected: 1 },
            { tag: "no-hide", action: "hide", key: Qt.Key_No, changes: { discovered: true, manual: false }, expected: 0 }
        ];
    }

    function test_controllerManagementUsesSameEligibility(data) {
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host(data.changes)] });
        dialogSpy.target = screen;
        actionSpy.signalName = data.action + "Requested";
        actionSpy.target = screen;
        connectSpy.target = screen;
        screen.forceActiveFocus(Qt.TabFocusReason);
        keyClick(data.key);
        if (data.action !== "wake") {
            compare(dialogSpy.count, data.expected);
            if (data.expected) {
                compare(dialogSpy.signalArguments[0][0], data.action);
                verify(dialogSpy.signalArguments[0][1]("0123"));
            }
        }
        compare(actionSpy.count, data.expected);
        compare(connectSpy.count, 0);
    }

    function test_allManualActionsFitNarrowDetails() {
        const screen = createTemporaryObject(screenComponent, testCase, {
            width: 800, height: 600, hosts: [host()]
        });
        const details = openDetails(screen);
        waitForRendering(details.contentItem);
        for (const name of ["pinAction", "wakeAction", "deleteAction", "closeDetailsButton"]) {
            const button = findChild(details.contentItem, name);
            verify(button && button.visible && button.enabled, name);
            const position = button.mapToItem(screen, 0, 0);
            verify(position.x >= 0 && position.y >= 0, name);
            verify(position.x + button.width <= screen.width, name + " horizontal bounds");
            verify(position.y + button.height <= screen.height, name + " vertical bounds");
        }
        details.close();
    }
}
