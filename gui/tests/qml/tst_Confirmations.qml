import QtQuick
import QtQuick.Controls
import QtTest
import "../../src/qml"
import "fixtures"

TestCase {
    id: tests
    name: "Confirmations"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    property int accepted: 0
    property int cancelled: 0
    property int behindClicks: 0
    Component { id: dialogComponent; ConfirmDialog {} }
    Component { id: harnessComponent; ConfirmationHarness { width: tests.width; height: tests.height } }
    Button { id: opener; text: "Open"; onClicked: tests.behindClicks++ }
    function openDialog(properties) {
        accepted = 0; cancelled = 0;
        opener.forceActiveFocus(Qt.TabFocusReason);
        const dialog = createTemporaryObject(dialogComponent, tests, Object.assign({
            title: "Delete console", text: "Delete this console?",
            callback: () => accepted++, rejectCallback: () => cancelled++,
            restoreFocusItem: opener
        }, properties || {}));
        verify(dialog);
        dialog.open();
        tryCompare(dialog, "opened", true);
        return dialog;
    }
    function test_cancelHasInitialFocusAndEnterCancels() {
        const dialog = openDialog();
        const cancel = findChild(dialog.footer, "confirmationCancel");
        verify(cancel, "The confirmation has an explicit Cancel action");
        tryCompare(cancel, "activeFocus", true);
        verify(cancel.visualFocus, "Cancel has visible keyboard focus when the dialog opens");
        keyClick(Qt.Key_Return);
        tryCompare(dialog, "visible", false);
        compare(accepted, 0);
        compare(cancelled, 1);
        tryCompare(opener, "activeFocus", true);
        tryCompare(opener, "visualFocus", true);
    }
    function test_explicitActions_data() {
        return ["Quit", "Delete console", "Hide console", "Unhide console"].map(label => ({tag: label, label: label}));
    }
    function test_explicitActions(data) {
        const dialog = openDialog({actionText: data.label});
        const action = findChild(dialog.footer, "confirmationAction");
        compare(action.text, data.label);
        keyClick(Qt.Key_Up);
        tryCompare(action, "activeFocus", true);
        verify(action.visualFocus);
        keyClick(Qt.Key_Return);
        dialog.accept();
        tryCompare(dialog, "visible", false);
        compare(accepted, 1);
        compare(cancelled, 0);
    }
    function test_escapeHasNoActionOrInputLeak() {
        const dialog = openDialog({actionText: "Delete console"});
        findChild(dialog.footer, "confirmationAction").forceActiveFocus(Qt.TabFocusReason);
        const previous = behindClicks;
        keyClick(Qt.Key_Escape);
        tryCompare(dialog, "visible", false);
        compare(accepted, 0);
        compare(cancelled, 1);
        compare(behindClicks, previous);
    }
    function test_enterDoesNotActivateDisabledAction() {
        const dialog = openDialog({actionText: "Delete console"});
        const action = findChild(dialog.footer, "confirmationAction");
        action.enabled = false;
        keyClick(Qt.Key_Up);
        keyClick(Qt.Key_Return);
        tryCompare(dialog, "visible", false);
        compare(accepted, 0);
    }
    function test_reopeningResetsFocus() {
        const dialog = openDialog({actionText: "Delete console"});
        const action = findChild(dialog.footer, "confirmationAction");
        action.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(dialog, "visible", false);
        compare(accepted, 1);
        dialog.callback = () => accepted++;
        dialog.rejectCallback = () => cancelled++;
        dialog.restoreFocusItem = opener;
        dialog.open();
        tryCompare(dialog, "opened", true);
        tryCompare(findChild(dialog.footer, "confirmationCancel"), "activeFocus", true);
        keyClick(Qt.Key_Return);
        tryCompare(dialog, "visible", false);
        compare(accepted, 1);
        compare(cancelled, 1);
    }
    function test_guardedCommands_data() {
        const rows = [];
        for (const action of ["delete", "hide", "registeredDelete", "unhide"])
            for (const change of ["reordered", "removed", "duplicate", "profile", "cancel", "escape"])
                rows.push({tag: action + "-" + change, action: action, change: change});
        return rows;
    }
    function test_guardedCommands(data) {
        const settingsMode = ["registeredDelete", "unhide"].includes(data.action);
        const harness = createTemporaryObject(harnessComponent, tests, {settingsMode: settingsMode});
        const target = {display: true, discovered: data.action === "hide", manual: data.action !== "hide",
            registered: data.action !== "hide", name: "Living room", mac: "001122334455", address: "192.0.2.1", ps5: true};
        const other = Object.assign({}, target, {mac: "aabbccddeeff", address: "192.0.2.2", name: "Bedroom"});
        harness.backend.hosts = [target, other];
        harness.settings.registeredHosts = [target, other];
        harness.backend.hiddenHosts = [target, other];
        waitForRendering(harness);
        harness.settings.actions = [];
        if (settingsMode) {
            const button = findChild(harness.screen, data.action === "unhide" ? "unhideConsole0" : "deleteRegisteredConsole0");
            button.forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Return);
        } else {
            mouseClick(findChild(harness.screen, "consoleActionsButton"));
            const details = findChild(harness.screen, "consoleDetails");
            tryCompare(details, "opened", true);
            mouseClick(findChild(details.contentItem, data.action === "hide" ? "hideAction" : "deleteAction"));
        }
        const dialog = harness.dialog;
        tryCompare(dialog, "opened", true);
        const action = findChild(dialog.footer, "confirmationAction");
        compare(action.text, data.action === "hide" ? "Hide console" : data.action === "unhide" ? "Unhide console" : "Delete console");
        tryCompare(findChild(dialog.footer, "confirmationCancel"), "activeFocus", true);
        const current = data.change === "reordered" ? [other, Object.assign({}, target, {name: "Renamed"})]
            : data.change === "duplicate" ? [target, target] : data.change === "removed" ? [other] : [target, other];
        harness.backend.hosts = current;
        harness.settings.registeredHosts = current;
        harness.backend.hiddenHosts = current;
        if (data.change === "profile") {
            harness.settings.currentProfile = "Other";
            harness.settings.currentProfile = "Test profile";
        }
        if (data.change === "escape") keyClick(Qt.Key_Escape);
        else {
            if (data.change !== "cancel") action.forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Return);
        }
        dialog.accept();
        tryCompare(dialog, "visible", false);
        const commands = harness.actions.concat(harness.settings.actions);
        compare(commands.length, data.change === "reordered" ? 1 : 0);
        if (commands.length) {
            if (data.action === "delete" || data.action === "registeredDelete") compare(commands[0].args[0], 1);
            else compare(commands[0].args[0], target.mac);
            if (data.action === "hide") compare(commands[0].args[1], "Renamed");
        }
    }
    function test_longTextActionsRemainReachable_data() {
        return [{tag: "900", width: 900, height: 600}, {tag: "1280", width: 1280, height: 720}];
    }
    function test_longTextActionsRemainReachable(data) {
        tests.width = data.width; tests.height = data.height;
        const dialog = openDialog({title: "Delete console with a long translated title", text: "Long console instructions. ".repeat(120),
            actionText: "Delete this console and its saved registration information"});
        verify(dialog.height <= data.height - 32);
        for (const name of ["confirmationCancel", "confirmationAction"]) {
            const button = findChild(dialog.footer, name);
            button.forceActiveFocus(Qt.TabFocusReason);
            waitForRendering(button);
            verify(button.visualFocus);
            const point = button.mapToItem(tests, 0, 0);
            verify(point.x >= 0 && point.x + button.width <= tests.width);
            verify(point.y >= 0 && point.y + button.height <= tests.height);
        }
        keyClick(Qt.Key_Escape);
        tryCompare(dialog, "visible", false);
    }
    function test_longTextScrollsFromFocusedActions_data() {
        const rows = [];
        for (const width of [900, 1280])
            for (const secondary of [false, true]) rows.push({tag: width + (secondary ? "-reminder" : "-confirmation"), width, secondary});
        return rows;
    }
    function test_longTextScrollsFromFocusedActions(data) {
        tests.width = data.width; tests.height = data.width === 900 ? 600 : 720;
        const dialog = openDialog({title: "Long translated instructions", text: "Read the complete instructions before you choose an action. ".repeat(120),
            actionText: "Set up now", secondaryActionText: data.secondary ? "Don't ask again" : ""});
        const scroll = dialog.contentItem;
        verify(scroll.contentHeight > scroll.height);
        const cancel = findChild(dialog.footer, "confirmationCancel");
        tryCompare(cancel, "activeFocus", true);
        verify(cancel.visualFocus);
        // Page keys also come from the controller shoulder buttons.
        keyClick(Qt.Key_PageDown);
        tryVerify(() => scroll.contentItem.contentY > 0, 1000, "Long instructions scroll while Cancel keeps focus");
        verify(cancel.activeFocus && cancel.visualFocus);
        const lower = scroll.contentItem.contentY;
        keyClick(Qt.Key_PageUp);
        tryVerify(() => scroll.contentItem.contentY < lower);
        const names = ["confirmationAction", "confirmationCancel"];
        if (data.secondary) names.push("confirmationSecondaryAction");
        for (const name of names) {
            const action = findChild(dialog.footer, name);
            for (let count = 0; count < 8 && !action.activeFocus; count++) keyClick(Qt.Key_Tab);
            verify(action.activeFocus && action.visualFocus, "Tab reaches " + name);
            keyClick(Qt.Key_PageDown);
            verify(scroll.contentItem.contentY > 0);
        }
        compare(accepted, 0); compare(cancelled, 0);
        keyClick(Qt.Key_Escape);
        tryCompare(dialog, "visible", false);
        compare(accepted, 0); compare(cancelled, 1);
    }
}
