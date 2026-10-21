import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: tests
    name: "Disconnect"
    width: 1280; height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; DisconnectHarness { anchors.fill: parent } }
    function test_enterInitiallyCancels() {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.opener.forceActiveFocus(Qt.TabFocusReason);
        harness.prompt.open();
        const dialog = findChild(harness.prompt, "confirmationDialog");
        tryCompare(dialog, "opened", true);
        const cancel = findChild(dialog.footer, "confirmationCancel");
        tryCompare(cancel, "activeFocus", true);
        verify(cancel.visualFocus);
        keyClick(Qt.Key_Return);
        tryCompare(harness.prompt, "opened", false);
        compare(harness.backend.commands.length, 0);
        compare(harness.grabs, 1);
        compare(harness.releases, 1);
        tryCompare(harness.opener, "activeFocus", true);
    }
    function test_choices_data() {
        const rows = [];
        for (const separate of [false, true])
            for (const choice of ["sleep", "disconnect", "cancel", "escape", "close"])
                rows.push({tag: (separate ? "window-" : "inline-") + choice, separate, choice});
        return rows;
    }
    function test_choices(data) {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.backend.window.runtimeRendererBackend = data.separate ? 1 : 0;
        harness.backend.settings.disconnectAction = 2;
        harness.prompt.open();
        harness.prompt.open();
        const dialog = findChild(harness.prompt, "confirmationDialog");
        const window = findChild(harness.prompt, "streamDisconnectWindow");
        tryCompare(dialog, "opened", true);
        compare(window.visible, data.separate);
        const sleep = findChild(dialog.footer, "confirmationAction");
        const disconnect = findChild(dialog.footer, "confirmationSecondaryAction");
        const cancel = findChild(dialog.footer, "confirmationCancel");
        compare(sleep.text, "Disconnect and sleep");
        compare(disconnect.text, "Disconnect");
        compare(cancel.text, "Cancel");
        tryCompare(cancel, "activeFocus", true);
        verify(cancel.visualFocus);
        if (data.choice === "close") {
            if (data.separate) window.close();
            else dialog.close();
        } else {
            if (data.choice === "sleep") keyClick(Qt.Key_Down);
            else if (data.choice === "disconnect") keyClick(Qt.Key_Up);
            keyClick(data.choice === "escape" ? Qt.Key_Escape : Qt.Key_Return);
        }
        dialog.accept();
        tryCompare(harness.prompt, "opened", false);
        tryCompare(window, "visible", false);
        compare(harness.backend.commands.length, data.choice === "sleep" || data.choice === "disconnect" ? 1 : 0);
        if (harness.backend.commands.length) compare(harness.backend.commands[0], data.choice === "sleep");
        compare(harness.backend.settings.disconnectAction, 2);
        compare(harness.grabs, 1);
        compare(harness.releases, 1);
        tryCompare(harness.opener, "activeFocus", true);
        harness.prompt.open();
        tryCompare(dialog, "opened", true);
        tryCompare(cancel, "activeFocus", true);
        keyClick(Qt.Key_Return);
        tryCompare(harness.prompt, "opened", false);
        compare(harness.grabs, 2);
        compare(harness.releases, 2);
        compare(harness.backend.commands.length, data.choice === "sleep" || data.choice === "disconnect" ? 1 : 0);
    }
    function test_longTextAtSupportedSizes_data() {
        return [{tag: "inline-900", width: 900, height: 600}, {tag: "inline-1280", width: 1280, height: 720},
            {tag: "window-900", width: 900, height: 600, separate: true}, {tag: "window-1280", width: 1280, height: 720, separate: true}];
    }
    function test_longTextAtSupportedSizes(data) {
        tests.width = data.width; tests.height = data.height;
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.backend.window.runtimeRendererBackend = data.separate ? 1 : 0;
        harness.prompt.open();
        const dialog = findChild(harness.prompt, "confirmationDialog");
        const window = findChild(harness.prompt, "streamDisconnectWindow");
        tryCompare(dialog, "opened", true);
        dialog.title = "Disconnect the current console session safely";
        dialog.text = "Long translated instructions for ending the current Remote Play session. ".repeat(70);
        dialog.actionText = "Disconnect this Remote Play session and put the console into sleep mode";
        dialog.secondaryActionText = "Disconnect this Remote Play session and keep the console running";
        waitForRendering(tests);
        for (const name of ["confirmationCancel", "confirmationAction", "confirmationSecondaryAction"]) {
            const button = findChild(dialog.footer, name);
            button.forceActiveFocus(Qt.TabFocusReason);
            const owner = data.separate ? window.contentItem : tests;
            const position = button.mapToItem(owner, 0, 0);
            verify(button.visualFocus);
            verify(position.x >= 0 && position.x + button.width <= owner.width);
            verify(position.y >= 0 && position.y + button.height <= owner.height);
        }
        keyClick(Qt.Key_Escape);
        tryCompare(harness.prompt, "opened", false);
        compare(harness.backend.commands.length, 0);
    }
    function test_tabNavigationAndDisabledAction_data() {
        return [{tag: "inline"}, {tag: "window", separate: true}];
    }
    function test_tabNavigationAndDisabledAction(data) {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.backend.window.runtimeRendererBackend = data.separate ? 1 : 0;
        harness.prompt.open();
        const dialog = findChild(harness.prompt, "confirmationDialog");
        tryCompare(dialog, "opened", true);
        const sleep = findChild(dialog.footer, "confirmationAction");
        const disconnect = findChild(dialog.footer, "confirmationSecondaryAction");
        const cancel = findChild(dialog.footer, "confirmationCancel");
        tryCompare(cancel, "activeFocus", true);
        keyClick(Qt.Key_Tab);
        tryCompare(sleep, "activeFocus", true);
        keyClick(Qt.Key_Tab);
        tryCompare(disconnect, "activeFocus", true);
        keyClick(Qt.Key_Backtab);
        tryCompare(sleep, "activeFocus", true);
        sleep.enabled = false;
        cancel.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Down);
        tryCompare(disconnect, "activeFocus", true);
        keyClick(Qt.Key_Escape);
        tryCompare(harness.prompt, "opened", false);
        compare(harness.backend.commands.length, 0);
    }
}
