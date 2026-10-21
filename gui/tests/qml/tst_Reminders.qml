import QtQuick
import QtQuick.Controls
import QtTest
import "../../src/qml"
import "fixtures"

TestCase {
    id: tests
    name: "Reminders"
    width: 1280; height: 720
    visible: true
    when: windowShown
    property int setups: 0
    Component { id: reminderComponent; RemindDialog {} }
    Component { id: harnessComponent; RemindersHarness { width: tests.width; height: tests.height } }
    Button { id: opener; text: "Open" }
    function test_escapeLeavesPreferenceUnchanged() {
        const settings = {remotePlayAsk: true, addSteamShortcutAsk: true};
        const dialog = createTemporaryObject(reminderComponent, tests, {
            backend: {settings: settings}, navigation: {restoreMainFocus: () => {}}
        });
        verify(dialog);
        dialog.showReminder("PSN sign-in", "Set up remote access.", true, () => setups++, opener);
        tryCompare(dialog, "opened", true);
        const later = findChild(dialog.footer, "confirmationCancel");
        compare(later.text, "Later");
        tryCompare(later, "activeFocus", true);
        verify(later.visualFocus);
        keyClick(Qt.Key_Escape);
        tryCompare(dialog, "visible", false);
        compare(settings.remotePlayAsk, true);
        compare(setups, 0);
        tryCompare(opener, "activeFocus", true);
    }
    function test_startupChoices_data() {
        const cases = [];
        for (const remote of [false, true])
            for (const action of ["setup", "later", "never", "escape"])
                cases.push({tag: (remote ? "psn" : "steam") + "-" + action, remote: remote, action: action});
        return cases;
    }
    function test_startupChoices(data) {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.settings.remotePlayAsk = data.remote;
        harness.settings.addSteamShortcutAsk = !data.remote;
        harness.settings.changeCount = 0;
        harness.started = true;
        tryCompare(harness.dialog, "opened", true);
        compare(harness.dialog.remotePlay, data.remote);
        const setup = findChild(harness.dialog.footer, "confirmationAction");
        const never = findChild(harness.dialog.footer, "confirmationSecondaryAction");
        const later = findChild(harness.dialog.footer, "confirmationCancel");
        compare(setup.text, "Set up now");
        compare(never.text, "Don't ask again");
        compare(later.text, "Later");
        tryCompare(later, "activeFocus", true);
        verify(later.visualFocus);
        if (data.action === "setup") keyClick(Qt.Key_Down);
        else if (data.action === "never") keyClick(Qt.Key_Up);
        keyClick(data.action === "escape" ? Qt.Key_Escape : Qt.Key_Return);
        harness.dialog.accept();
        tryCompare(harness.dialog, "visible", false);
        compare(harness.routes.length, data.action === "setup" ? 1 : 0);
        if (harness.routes.length) {
            compare(harness.routes[0].name, data.remote ? "psn" : "steam");
            if (data.remote) compare(harness.routes[0].remote, false);
            else compare(harness.routes[0].reminder, true);
        }
        compare(data.remote ? harness.settings.remotePlayAsk : harness.settings.addSteamShortcutAsk, data.action !== "never");
        compare(harness.settings.changeCount, data.action === "never" ? 1 : 0);
        if (data.action !== "setup") tryCompare(findChild(harness, "addConsoleButton"), "activeFocus", true);
    }
    function test_steamThenPsn_data() {
        return ["later", "never", "escape"].map(action => ({tag: action, action: action}));
    }
    function test_steamThenPsn(data) {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.settings.remotePlayAsk = true;
        harness.settings.addSteamShortcutAsk = true;
        harness.settings.changeCount = 0;
        harness.started = true;
        tryCompare(harness.dialog, "opened", true);
        compare(harness.prompts, ["steam"]);
        if (data.action === "never") keyClick(Qt.Key_Up);
        keyClick(data.action === "escape" ? Qt.Key_Escape : Qt.Key_Return);
        tryCompare(harness.dialog, "remotePlay", true);
        tryCompare(harness.dialog, "opened", true);
        compare(harness.prompts, ["steam", "psn"]);
        compare(harness.routes.length, 0);
        compare(harness.settings.addSteamShortcutAsk, data.action !== "never");
        compare(harness.settings.remotePlayAsk, true);
        compare(harness.settings.changeCount, data.action === "never" ? 1 : 0);
        tryCompare(findChild(harness.dialog.footer, "confirmationCancel"), "activeFocus", true);
        keyClick(Qt.Key_Escape);
        tryCompare(harness.dialog, "visible", false);
        compare(harness.prompts.length, 2);
        compare(harness.routes.length, 0);
        tryCompare(findChild(harness, "addConsoleButton"), "activeFocus", true);
        harness.pages.push(Qt.createQmlObject('import QtQuick; Item {}', harness), {}, StackView.Immediate);
        harness.pages.pop(null, StackView.Immediate);
        wait(50);
        compare(harness.prompts.length, 2);
        compare(harness.dialog.visible, false);
    }
    function test_startupEligibility_data() {
        return [
            {tag: "auto-connect", auto: true, direct: false, steam: true, available: true, psn: true, credentials: false, expected: ""},
            {tag: "direct-stream", auto: false, direct: true, steam: true, available: true, psn: true, credentials: false, expected: ""},
            {tag: "steam-unavailable", auto: false, direct: false, steam: true, available: false, psn: true, credentials: false, expected: "psn"},
            {tag: "steam-first", auto: false, direct: false, steam: true, available: true, psn: true, credentials: true, expected: "steam"},
            {tag: "no-preferences", auto: false, direct: false, steam: false, available: true, psn: false, credentials: false, expected: ""},
            {tag: "psn-complete", auto: false, direct: false, steam: false, available: true, psn: true, credentials: true, expected: ""},
            {tag: "psn-incomplete", auto: false, direct: false, steam: false, available: true, psn: true, credentials: false, expected: "psn"}
        ];
    }
    function test_startupEligibility(data) {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.settings.remotePlayAsk = data.psn;
        harness.settings.addSteamShortcutAsk = data.steam;
        harness.backend.autoConnect = data.auto;
        harness.backend.window.directStream = data.direct;
        if (!data.available) harness.backend.createSteamShortcut = undefined;
        // Synthetic completion state uses the existing credential-presence contract.
        harness.settings.psnRefreshToken = "synthetic-refresh";
        harness.settings.psnAuthToken = "synthetic-auth";
        harness.settings.psnAuthTokenExpiry = "synthetic-expiry";
        harness.settings.psnAccountId = data.credentials ? "synthetic-account" : "";
        harness.settings.changeCount = 0;
        harness.started = true;
        wait(100);
        compare(harness.prompts, data.expected ? [data.expected] : []);
        compare(harness.dialog.visible, !!data.expected);
        compare(harness.routes.length, 0);
        const completePsn = data.psn && data.credentials && !data.steam && !data.auto && !data.direct;
        compare(harness.settings.remotePlayAsk, completePsn ? false : data.psn);
        compare(harness.settings.addSteamShortcutAsk, data.steam);
        compare(harness.settings.changeCount, completePsn ? 1 : 0);
        if (data.expected) {
            tryCompare(harness.dialog, "opened", true);
            keyClick(Qt.Key_Escape);
            tryCompare(harness.dialog, "visible", false);
        }
    }
    function test_duplicatePromptKeepsOriginalSetupRoute() {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.settings.remotePlayAsk = false;
        harness.settings.addSteamShortcutAsk = true;
        harness.started = true;
        tryCompare(harness.dialog, "opened", true);
        compare(harness.showRemindDialog("Replacement", "Replacement", true, () => fail("A duplicate prompt replaced the setup route")), false);
        compare(harness.dialog.remotePlay, false);
        compare(harness.prompts, ["steam"]);
        keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        harness.dialog.accept();
        tryCompare(harness.dialog, "visible", false);
        compare(harness.routes, [{name: "steam", reminder: true}]);
        compare(harness.settings.addSteamShortcutAsk, true);
    }
    function test_controllerLaterShortcut() {
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.settings.remotePlayAsk = true;
        harness.started = true;
        tryCompare(harness.dialog, "opened", true);
        keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Yes);
        tryCompare(harness.dialog, "visible", false);
        compare(harness.settings.remotePlayAsk, true);
        compare(harness.routes.length, 0);
    }
    function test_longTextAndPrivacy_data() {
        return [{tag: "900", width: 900, height: 600}, {tag: "1280", width: 1280, height: 720}];
    }
    function test_longTextAndPrivacy(data) {
        tests.width = data.width; tests.height = data.height;
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.settings.streamerMode = true;
        harness.settings.remotePlayAsk = true;
        harness.started = true;
        tryCompare(harness.dialog, "opened", true);
        verify(!harness.dialog.text.includes("synthetic"));
        harness.dialog.text = "Long translated setup instructions. ".repeat(100);
        findChild(harness.dialog.footer, "confirmationAction").text = "Set up the remote connection and the account preferences now";
        findChild(harness.dialog.footer, "confirmationSecondaryAction").text = "Do not ask again about these remote connection settings";
        waitForRendering(harness);
        for (const name of ["confirmationAction", "confirmationSecondaryAction", "confirmationCancel"]) {
            const button = findChild(harness.dialog.footer, name);
            button.forceActiveFocus(Qt.TabFocusReason);
            verify(button.visualFocus);
            const position = button.mapToItem(tests, 0, 0);
            verify(position.x >= 0 && position.x + button.width <= tests.width);
            verify(position.y >= 0 && position.y + button.height <= tests.height);
        }
        keyClick(Qt.Key_Escape);
        tryCompare(harness.dialog, "visible", false);
        compare(harness.settings.remotePlayAsk, true);
        compare(harness.routes.length, 0);
    }
}
