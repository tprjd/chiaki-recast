import QtQuick
import QtQuick.Controls
import QtTest
import "../../src/qml"
import "fixtures"

TestCase {
    id: tests
    name: "SteamShortcut"
    width: 1280; height: 720
    visible: true
    when: windowShown
    property var events: []
    property var calls: []
    property int closes: 0
    property int reminders: 0
    property int signIns: 0
    property var reminderAction
    property alias settings: settingsState
    QtObject {
        id: settingsState
        property string currentProfile: ""
        property bool streamerMode: false
        property bool addSteamShortcutAsk: true
        property bool remotePlayAsk: false
        property string psnRefreshToken: ""
        property string psnAuthToken: ""
        property string psnAuthTokenExpiry: ""
        property string psnAccountId: ""
        property string chosenPath: "C:/synthetic/Steam"
        function chooseSteamBasePath() { return chosenPath; }
    }
    QtObject {
        id: fakeBackend
        property alias settings: settingsState
        property bool throwOnCreate: false
        function createSteamShortcut(name, options, callback, path) {
            if (throwOnCreate) throw new Error("Synthetic dispatch failure");
            calls.push({name, options, path}); events.push(callback);
        }
    }
    Component { id: component; SteamShortcutDialog {} }
    Component { id: harnessComponent; SteamShortcutHarness {} }
    function open(profile = "Guest") {
        events = []; calls = []; closes = 0; reminders = 0; signIns = 0; reminderAction = null;
        fakeBackend.throwOnCreate = false;
        settings.psnRefreshToken = ""; settings.psnAuthToken = ""; settings.psnAuthTokenExpiry = ""; settings.psnAccountId = "";
        settings.currentProfile = profile; settings.streamerMode = false;
        settings.addSteamShortcutAsk = true; settings.remotePlayAsk = false;
        settings.chosenPath = "C:/synthetic/Steam";
        return createTemporaryObject(component, tests, {width: tests.width, height: tests.height,
            backend: fakeBackend, navigation: {closeDialog: () => closes++,
                showRemindDialog: (title, message, remote, action) => { reminders++; reminderAction = action; },
                showPSNTokenDialog: () => signIns++}});
    }
    function test_defaultsAndNameValidation() {
        const dialog = open();
        verify(dialog);
        const name = findChild(dialog, "steamName");
        compare(name.text, "chiaki-ng Guest");
        compare(findChild(dialog, "steamOptions").text, "--profile=Guest");
        name.text = "  ";
        verify(!findChild(dialog, "steamCreate").enabled);
        keyClick(Qt.Key_Menu);
        compare(calls.length, 0);
    }
    function captureEvidence(dialog, suffix) {
        const url = String(Qt.resolvedUrl("../../../.scratch/psn-dialog-refresh/evidence/ticket03-" + suffix + ".png"));
        grabImage(dialog).save(decodeURIComponent(url.slice(Qt.platform.os === "windows" ? 8 : 7)));
    }
    function submit(dialog) {
        findChild(dialog, "steamCreate").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryVerify(() => calls.length > 0);
    }
    function test_optionalInputsAndTerminalSuccess() {
        const dialog = open("");
        compare(findChild(dialog, "steamName").text, "chiaki-ng");
        compare(findChild(dialog, "steamOptions").text, "");
        findChild(dialog, "steamName").text = "  My shortcut  ";
        findChild(dialog, "steamOptions").text = " --profile=custom ";
        mouseClick(findChild(dialog, "steamChoosePath"));
        compare(findChild(dialog, "steamPath").text, "C:/synthetic/Steam");
        submit(dialog);
        compare(calls[0].name, "My shortcut");
        compare(calls[0].options, "--profile=custom");
        compare(calls[0].path, "C:/synthetic/Steam");
        events[0]("Artwork saved", true, false);
        verify(settings.addSteamShortcutAsk);
        verify(!findChild(dialog, "steamDone").visible);
        events[0]("Shortcut written", true, true);
        verify(!settings.addSteamShortcutAsk);
        verify(findChild(dialog, "steamResultMessage").text.indexOf("Restart Steam yourself") >= 0);
        tryCompare(findChild(dialog, "steamDone"), "activeFocus", true);
        verify(!findChild(dialog, "steamDetails").visible);
        keyClick(Qt.Key_Return);
        compare(closes, 1);
        keyClick(Qt.Key_Escape);
        compare(closes, 1);
    }
    function test_partialProgressFailureRetryAndLateCallbacks() {
        const dialog = open();
        submit(dialog);
        events[0]("Artwork saved in synthetic folder", true, false);
        events[0]("Controller configuration not found", false, true);
        verify(settings.addSteamShortcutAsk);
        compare(findChild(dialog, "steamResultHeading").text, "Steam shortcut was not completed");
        verify(findChild(dialog, "steamResultMessage").text.indexOf("Some files may already have changed") >= 0);
        verify(findChild(dialog, "steamResultMessage").text.indexOf("Restart") < 0);
        tryCompare(findChild(dialog, "steamEdit"), "activeFocus", true);
        mouseClick(findChild(dialog, "steamDetailsToggle"));
        verify(findChild(dialog, "steamDetails").text.indexOf("Controller configuration not found") >= 0);
        events[0]("Wrong repeated success", true, true);
        verify(settings.addSteamShortcutAsk);
        waitForRendering(dialog);
        mouseClick(findChild(dialog, "steamRetry"));
        tryVerify(() => calls.length === 2);
        verify(!findChild(dialog, "steamDetails").visible);
        mouseClick(findChild(dialog, "steamDetailsToggle"));
        compare(findChild(dialog, "steamDetails").text, "");
        events[0]("Old result", true, true);
        verify(!findChild(dialog, "steamDone").visible);
        events[1]("Retry succeeded", true, true);
        verify(findChild(dialog, "steamDone").visible);
        verify(!settings.addSteamShortcutAsk);
    }
    function test_editRetainsInputAndPickerCancelRetainsFolder() {
        const dialog = open();
        mouseClick(findChild(dialog, "steamChoosePath"));
        settings.chosenPath = "";
        mouseClick(findChild(dialog, "steamChoosePath"));
        compare(findChild(dialog, "steamPath").text, "C:/synthetic/Steam");
        findChild(dialog, "steamName").text = "Keep this name";
        submit(dialog);
        events[0]("Write failed", false, true);
        waitForRendering(dialog);
        mouseClick(findChild(dialog, "steamEdit"));
        compare(findChild(dialog, "steamName").text, "Keep this name");
        compare(findChild(dialog, "steamOptions").text, "--profile=Guest");
        compare(findChild(dialog, "steamPath").text, "C:/synthetic/Steam");
        tryCompare(findChild(dialog, "steamName"), "activeFocus", true);
    }
    function test_activeWriteHasNoCancellationOrDuplicateSubmission() {
        const dialog = open();
        submit(dialog);
        verify(!findChild(dialog, "steamCancel").visible);
        verify(!findChild(dialog, "steamClose").visible);
        verify(!findChild(dialog, "steamBack").enabled);
        keyClick(Qt.Key_Escape);
        keyClick(Qt.Key_Menu);
        compare(closes, 0);
        compare(calls.length, 1);
        verify(findChild(dialog, "steamResultMessage").text.indexOf("cannot be cancelled") >= 0);
        events[0]("Failed", false, true);
        waitForRendering(dialog);
        mouseClick(findChild(dialog, "steamClose"));
        compare(closes, 1);
        events[0]("Late success", true, true);
        verify(settings.addSteamShortcutAsk);
    }
    function test_streamerModeMasksProfileOptionsFolderAndDetails() {
        const dialog = open();
        mouseClick(findChild(dialog, "steamChoosePath"));
        settings.streamerMode = true;
        compare(findChild(dialog, "steamName").echoMode, TextInput.Password);
        compare(findChild(dialog, "steamOptions").echoMode, TextInput.Password);
        compare(findChild(dialog, "steamPath").text, "Folder hidden");
        submit(dialog);
        events[0]("Synthetic user folder", false, true);
        mouseClick(findChild(dialog, "steamDetailsToggle"));
        compare(findChild(dialog, "steamDetails").text, "Technical details hidden in streamer mode.");
    }
    function test_keyboardAndControllerActions() {
        const dialog = open();
        const name = findChild(dialog, "steamName");
        const options = findChild(dialog, "steamOptions");
        tryCompare(name, "activeFocus", true);
        keyClick(Qt.Key_Down);
        tryCompare(options, "activeFocus", true);
        keyClick(Qt.Key_Tab);
        tryCompare(findChild(dialog, "steamChoosePath"), "activeFocus", true);
        keyClick(Qt.Key_Return);
        compare(findChild(dialog, "steamPath").text, "C:/synthetic/Steam");
        keyClick(Qt.Key_Menu);
        tryVerify(() => calls.length === 1);
    }
    function test_layoutActionsRemainReachable_data() {
        return [{tag: "minimum", w: 900, h: 600}, {tag: "large", w: 1280, h: 720}];
    }
    function test_layoutActionsRemainReachable(data) {
        const dialog = open("A long synthetic profile name used for translated text layout");
        dialog.width = data.w; dialog.height = data.h;
        findChild(dialog, "steamCreate").text = "Create this Steam shortcut with the selected profile";
        waitForRendering(dialog);
        captureEvidence(dialog, data.tag + "-form");
        for (const target of ["steamName", "steamOptions", "steamChoosePath", "steamCreate", "steamCancel"]) {
            const item = findChild(dialog, target);
            item.forceActiveFocus(Qt.TabFocusReason);
            tryVerify(() => {
                const pos = item.mapToItem(dialog, 0, 0);
                return pos.x >= 0 && pos.x + item.width <= dialog.width && pos.y >= 0 && pos.y + item.height <= dialog.height;
            }, 1000, "Focused Steam action must scroll into view: " + target);
            if (item.background && item.background.border) compare(item.background.border.width, 2);
        }
        mouseClick(findChild(dialog, "steamCreate"));
        tryVerify(() => calls.length === 1);
        events[0]("Failure with long details. ".repeat(80), false, true);
        mouseClick(findChild(dialog, "steamDetailsToggle"));
        const retry = findChild(dialog, "steamRetry");
        retry.forceActiveFocus(Qt.TabFocusReason);
        tryVerify(() => {
            const pos = retry.mapToItem(dialog, 0, 0);
            return pos.y >= 0 && pos.y + retry.height <= dialog.height;
        });
        waitForRendering(dialog);
        captureEvidence(dialog, data.tag + "-failure");
    }

    function test_returnFocusToLibraryOpener_data() { return [{tag: "cancel", success: false}, {tag: "done", success: true}]; }
    function test_returnFocusToLibraryOpener(data) {
        open().destroy();
        wait(0);
        const harness = createTemporaryObject(harnessComponent, tests, {width: 900, height: 600, backend: fakeBackend});
        const opener = findChild(harness.library, "steamShortcutButton");
        opener.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryVerify(() => !!harness.dialog);
        let action = findChild(harness.dialog, "steamCancel");
        if (data.success) {
            submit(harness.dialog);
            events[0]("Created", true, true);
            action = findChild(harness.dialog, "steamDone");
            tryCompare(action, "activeFocus", true);
        }
        action.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryVerify(() => !harness.dialog);
        tryCompare(opener, "activeFocus", true);
    }
    function test_nextReminderIsEligibleAndOnlyAppearsAfterClosing_data() {
        return [{tag: "reminder", reminder: true, eligible: true},
            {tag: "manual", reminder: false, eligible: true},
            {tag: "disabled", reminder: true, eligible: false}];
    }
    function test_nextReminderIsEligibleAndOnlyAppearsAfterClosing(data) {
        const dialog = open();
        dialog.fromReminder = data.reminder;
        settings.remotePlayAsk = data.eligible;
        submit(dialog);
        events[0]("Written", true, true);
        compare(reminders, 0);
        tryCompare(findChild(dialog, "steamDone"), "activeFocus", true);
        keyClick(Qt.Key_Return);
        compare(closes, 1);
        compare(reminders, data.reminder && data.eligible ? 1 : 0);
        keyClick(Qt.Key_Escape);
        compare(closes, 1);
        if (reminderAction) { reminderAction(); compare(signIns, 1); }
    }

    function test_progressAndSuccessLayouts_data() { return test_layoutActionsRemainReachable_data(); }
    function test_progressAndSuccessLayouts(data) {
        const dialog = open();
        dialog.width = data.w; dialog.height = data.h;
        findChild(dialog, "steamCreate").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Menu);
        tryVerify(() => calls.length === 1);
        waitForRendering(dialog);
        captureEvidence(dialog, data.tag + "-progress");
        events[0]("Created", true, true);
        const done = findChild(dialog, "steamDone");
        tryCompare(done, "activeFocus", true);
        waitForRendering(dialog);
        const pos = done.mapToItem(dialog, 0, 0);
        verify(pos.y >= 0 && pos.y + done.height <= dialog.height);
        captureEvidence(dialog, data.tag + "-success");
    }

    function test_dispatchExceptionHasARecoverableResult() {
        const dialog = open();
        fakeBackend.throwOnCreate = true;
        findChild(dialog, "steamCreate").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(findChild(dialog, "steamEdit"), "activeFocus", true);
        verify(settings.addSteamShortcutAsk);
        verify(findChild(dialog, "steamResultMessage").text.indexOf("Check the details") >= 0);
        fakeBackend.throwOnCreate = false;
        mouseClick(findChild(dialog, "steamRetry"));
        tryVerify(() => calls.length === 1);
        events[0]("Succeeded after correcting folder", true, true);
        verify(!settings.addSteamShortcutAsk);
    }
    function test_destroyedFormCannotApplyLateSuccess() {
        const dialog = open();
        submit(dialog);
        dialog.destroy();
        wait(0);
        events[0]("Late result after removal", true, true);
        verify(settings.addSteamShortcutAsk);
        compare(closes, 0);
    }
    function test_existingPsnCredentialsSuppressTheNextReminder() {
        const dialog = open();
        dialog.fromReminder = true;
        settings.remotePlayAsk = true;
        settings.psnRefreshToken = "synthetic"; settings.psnAuthToken = "synthetic";
        settings.psnAuthTokenExpiry = "synthetic"; settings.psnAccountId = "synthetic";
        const cancel = findChild(dialog, "steamCancel");
        cancel.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(closes, 1);
        compare(reminders, 0);
        verify(!settings.remotePlayAsk);
    }
}
