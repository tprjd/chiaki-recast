import QtQuick
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "PsnSetup"
    width: 1280; height: 720
    visible: true; when: windowShown
    Component { id: harnessComponent; PsnSetupHarness {} }
    function control(h, name) { return findChild(h.screen, "psn" + name); }
    function test_redirectIsMaskedAndStreamerModeHidesReveal() {
        const h = createTemporaryObject(harnessComponent, testCase);
        verify(h !== null);
        compare(control(h, "Redirect").echoMode, TextInput.Password);
        compare(control(h, "Redirect").Accessible.name, "Sign-in redirect URL");
        control(h, "Reveal").clicked();
        compare(control(h, "Redirect").echoMode, TextInput.Normal);
        h.backend.settings.streamerMode = true;
        compare(control(h, "Redirect").echoMode, TextInput.Password);
        verify(!control(h, "Reveal").enabled);
        h.backend.settings.streamerMode = false;
        compare(control(h, "Redirect").echoMode, TextInput.Password);
    }
    function test_onlyFinalResultCompletesAndSubmissionCannotRepeat() {
        const h = createTemporaryObject(harnessComponent, testCase);
        control(h, "Redirect").text = "synthetic redirect";
        control(h, "Submit").clicked();
        compare(control(h, "Status").text, "Completing PSN sign-in");
        compare(control(h, "Close").text, "Close");
        verify(control(h, "CloseExplanation").text.indexOf("save credentials") >= 0);
        h.screen.submit("repeat");
        compare(h.backend.commands.filter(command => command === "submit").length, 1);
        h.backend.change({phase: "account"});
        compare(control(h, "Status").text, "Retrieving your Account ID");
        h.backend.change({phase: "failure", message: "Check your network.", errorText: "Safe diagnostic"});
        compare(control(h, "Status").text, "PSN sign-in did not finish");
        verify(!control(h, "Details").visible);
        control(h, "DetailsToggle").clicked();
        verify(control(h, "Details").visible);
        compare(control(h, "Details").textFormat, Text.PlainText);
        verify(control(h, "Fresh").visible);
        control(h, "Submit").clicked();
        verify(!control(h, "Details").visible);
        h.backend.change({phase: "success", accountId: "synthetic"});
        compare(control(h, "Status").text, "PSN remote access is ready");
        compare(h.actions.length, 0);
        tryVerify(() => control(h, "Close").activeFocus);
        keyClick(Qt.Key_Escape);
        compare(h.actions.length, 1);
    }
    function test_closeBeforeSubmissionAndLateResultsDoNotNavigate() {
        const h = createTemporaryObject(harnessComponent, testCase);
        compare(control(h, "Close").text, "Cancel");
        control(h, "Close").clicked();
        h.screen.submit("ignored");
        compare(h.backend.commands.filter(command => command === "submit").length, 0);
        compare(h.actions.length, 1);
        const active = createTemporaryObject(harnessComponent, testCase);
        active.screen.submit("synthetic");
        control(active, "Close").clicked();
        active.backend.change({phase: "success"});
        wait(1);
        compare(active.actions.length, 1);
        verify(!active.screen.activeFocus);
    }
    function test_expiryCopyPasteAndFreshSignIn() {
        const h = createTemporaryObject(harnessComponent, testCase);
        h.screen.expired = true;
        compare(control(h, "Heading").text, "Renew PSN credentials");
        verify(control(h, "Explanation").text.indexOf("expired") >= 0);
        control(h, "Copy").clicked();
        control(h, "Paste").clicked();
        compare(control(h, "Redirect").text, "https://example.invalid/sign-in");
        compare(h.backend.commands.filter(command => command === "submit").length, 0);
        h.screen.submit("synthetic");
        h.backend.change({phase: "failure", message: "Start fresh."});
        control(h, "Fresh").clicked();
        compare(control(h, "Redirect").text, "");
        verify(control(h, "Submit").visible);
        compare(control(h, "Status").text, "Sign in with your browser");
    }
    function test_supportedSizesAndLongText_data() {
        return [{tag: "900x600", width: 900, height: 600}, {tag: "1280x720", width: 1280, height: 720}];
    }
    function test_supportedSizesAndLongText(data) {
        const h = createTemporaryObject(harnessComponent, testCase, {width: data.width, height: data.height});
        tryVerify(() => control(h, "Redirect").activeFocus);
        control(h, "Storage").text = "Long translated credential storage explanation ".repeat(25);
        for (const name of ["Copy", "Redirect", "Paste", "Reveal", "Submit", "Close"]) {
            const item = control(h, name);
            item.forceActiveFocus(Qt.TabFocusReason);
            const scroll = control(h, "Scroll");
            tryVerify(() => item.activeFocus);
            tryVerify(() => {
                const point = item.mapToItem(scroll, 0, 0);
                return point.y >= -1 && point.y + item.height <= scroll.height + 1 && point.x >= -1 && point.x + item.width <= scroll.width + 1;
            }, 500, name + " must remain reachable");
        }
    }
    function test_embeddedBrowserRecoveryAndFallback() {
        const fallback = createTemporaryObject(harnessComponent, testCase, {embedded: true});
        verify(control(fallback, "Redirect").visible);
        compare(fallback.backend.commands[0], "browser");
        const partial = createTemporaryObject(harnessComponent, testCase, {embedded: true, browserAvailable: true, browserHintsFail: true});
        verify(control(partial, "Redirect").visible);
        compare(partial.screen.browser, null);
        control(partial, "Close").clicked();
        compare(partial.actions.length, 1);
        const h = createTemporaryObject(harnessComponent, testCase, {embedded: true, browserAvailable: true});
        verify(!control(h, "Redirect").visible);
        compare(h.backend.commands.length, 0);
        control(h, "Reload").clicked();
        compare(h.browserCommands.slice(0,3).join(","), "cookies,cache,reload");
        h.screen.browser.navigationRequested({url: "https://remoteplay.dl.playstation.net/remoteplay/redirect?code=synthetic", reject: function() {}, accept: function() { fail("Redirect was not intercepted"); }});
        compare(control(h, "Status").text, "Completing PSN sign-in");
        h.backend.change({phase: "failure", message: "Start fresh."});
        control(h, "Fresh").clicked();
        verify(control(h, "BrowserHost").visible);
        control(h, "External").clicked();
        verify(control(h, "Redirect").visible);
    }
    function test_rejectedInputStaysEditableAndDetailsHideInStreamerMode() {
        const h = createTemporaryObject(harnessComponent, testCase);
        h.backend.rejectRedirect = true;
        control(h, "Redirect").text = "invalid input";
        control(h, "Submit").clicked();
        compare(control(h, "Status").text, "PSN sign-in did not finish");
        verify(control(h, "Failure").text.indexOf("complete") >= 0);
        verify(control(h, "Redirect").visible && control(h, "Redirect").enabled);
        verify(control(h, "Redirect").text.length > 0);
        h.backend.change({errorText: "Safe details"});
        control(h, "DetailsToggle").clicked();
        compare(control(h, "Details").text, "Safe details");
        h.backend.settings.streamerMode = true;
        compare(control(h, "Details").text, "Details are hidden in streamer mode.");
    }
}
