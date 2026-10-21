import QtQuick
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "RegistrationAccount"
    width: 1280; height: 720
    visible: true; when: windowShown
    Component { id: harnessComponent; RegistrationAccountHarness {} }
    function field(h, name) { return findChild(h.screen, "registration" + name); }
    function control(h, name) { return findChild(h.current, name); }
    function activate(item) { item.forceActiveFocus(Qt.TabFocusReason); tryVerify(() => item.activeFocus && item.visualFocus); keyClick(Qt.Key_Return); }
    function test_publicLookupExplainsItsService() {
        const h = createTemporaryObject(harnessComponent, testCase);
        activate(field(h, "Lookup"));
        tryVerify(() => control(h, "lookupHeading") !== null);
        verify(control(h, "lookupExplanation").text.indexOf("psn.flipscreen.games") >= 0);
        verify(control(h, "lookupExplanation").text.indexOf("anyone") >= 0);
        verify(control(h, "lookupExplanation").text.indexOf("credentials") >= 0);
        compare(h.backend.signIns, 0);
    }
    function test_lookupErrorsCanRetryAndSuccessReturnsToRegistration() {
        const h = createTemporaryObject(harnessComponent, testCase);
        field(h, "Host").text = "original-console.local";
        field(h, "Pin").text = "00123456";
        activate(field(h, "Lookup"));
        tryVerify(() => control(h, "lookupUsername") && control(h, "lookupUsername").activeFocus);
        control(h, "lookupSubmit").clicked();
        verify(control(h, "lookupError").text.length > 0);
        compare(h.lookupRequests.length, 0);
        control(h, "lookupUsername").text = "  public-user  ";
        for (const response of [{status: 0, text: ""}, {status: 200, text: "invalid"},
                                {status: 200, text: '{}'}, {status: 200, text: '{"encoded_id":42}'}]) {
            control(h, "lookupSubmit").clicked();
            const count = h.lookupRequests.length;
            verify(!control(h, "lookupSubmit").enabled, "Lookup must show active progress");
            control(h, "lookupSubmit").clicked();
            compare(h.lookupRequests.length, count);
            h.lookupCallbacks[count - 1](response.status, response.text);
            verify(control(h, "lookupError").text.length > 0);
            verify(control(h, "lookupSubmit").enabled);
        }
        control(h, "lookupSubmit").clicked();
        const callback = h.lookupCallbacks[h.lookupCallbacks.length - 1];
        callback(200, '{"encoded_id":"EjRWeJCrze8="}');
        tryVerify(() => field(h, "AccountId").activeFocus);
        compare(field(h, "AccountId").text, "EjRWeJCrze8=");
        compare(field(h, "Host").text, "original-console.local");
        compare(field(h, "Pin").text, "00123456");
        compare(h.lookupRequests[0], "public-user");
        compare(h.backend.signIns, 0);
        compare(h.backend.settings.psnAccountId, "");
        callback(200, '{"encoded_id":"AAAAAAAAAAA="}');
        compare(field(h, "AccountId").text, "EjRWeJCrze8=");
    }
    function test_signInReturnsDirectlyWithStorageExplanation() {
        const h = createTemporaryObject(harnessComponent, testCase);
        field(h, "Host").text = "original-console.local";
        activate(field(h, "Login"));
        tryVerify(() => control(h, "psnStorage") !== null);
        verify(control(h, "psnStorage").text.indexOf("saves PSN authentication credentials") >= 0);
        control(h, "psnSubmit").clicked();
        verify(control(h, "psnCloseExplanation").visible);
        compare(h.backend.signIns, 1);
        h.backend.finish("EjRWeJCrze8=");
        tryVerify(() => field(h, "AccountId").activeFocus);
        compare(field(h, "AccountId").text, "EjRWeJCrze8=");
        compare(field(h, "Host").text, "original-console.local");
        compare(h.closes, 1);
        h.backend.finish("AAAAAAAAAAA=");
        compare(field(h, "AccountId").text, "EjRWeJCrze8=");
    }
    function openLookup(h) {
        activate(field(h, "Lookup"));
        tryVerify(() => control(h, "lookupUsername").activeFocus);
        control(h, "lookupUsername").text = "public-user";
        control(h, "lookupSubmit").clicked();
    }
    function test_closedAndReplacedLookupResultsCannotChangeRegistration() {
        const h = createTemporaryObject(harnessComponent, testCase);
        openLookup(h);
        const first = h.lookupCallbacks[0];
        control(h, "lookupCancel").clicked();
        tryVerify(() => field(h, "AccountId").activeFocus);
        openLookup(h);
        first(200, '{"encoded_id":"AAAAAAAAAAA="}');
        compare(h.closes, 1);
        compare(field(h, "AccountId").text, "");
        h.replaceAccount();
        h.lookupCallbacks[1](200, '{"encoded_id":"EjRWeJCrze8="}');
        compare(h.current.objectName, "replacement");
        compare(field(h, "AccountId").text, "");
        compare(h.closes, 1);
        h.replaceRegistration();
        h.lookupCallbacks[1](200, '{"encoded_id":"EjRWeJCrze8="}');
        compare(field(h, "AccountId").text, "");
        compare(h.closes, 1);
    }
    function test_changedProfileCannotApplyPublicLookupEvenAfterSwitchBack() {
        const h = createTemporaryObject(harnessComponent, testCase);
        openLookup(h);
        h.backend.settings.currentProfile = "profile-b";
        h.backend.settings.currentProfile = "profile-a";
        h.lookupCallbacks[0](200, '{"encoded_id":"EjRWeJCrze8="}');
        compare(field(h, "AccountId").text, "");
        compare(h.closes, 0);
        verify(control(h, "lookupError").text.indexOf("profile changed") >= 0);
        verify(!control(h, "lookupSubmit").enabled);
        control(h, "lookupCancel").clicked();
        compare(h.closes, 1);
    }
    function test_pendingSignInCannotCompleteReplacementRegistration() {
        const h = createTemporaryObject(harnessComponent, testCase);
        activate(field(h, "Login"));
        tryVerify(() => control(h, "psnRedirect").activeFocus);
        control(h, "psnSubmit").clicked();
        control(h, "psnClose").clicked();
        h.replaceRegistration();
        activate(field(h, "Login"));
        tryVerify(() => control(h, "psnClose").activeFocus);
        h.backend.finish("EjRWeJCrze8=");
        compare(field(h, "AccountId").text, "");
        compare(h.closes, 1);
        verify(control(h, "psnStatus").text.indexOf("ready") >= 0);
        control(h, "psnClose").clicked();
        compare(h.closes, 2);
    }
    function test_replacedSignInCannotNavigateOrTakeFocus() {
        const h = createTemporaryObject(harnessComponent, testCase);
        activate(field(h, "Login"));
        tryVerify(() => control(h, "psnRedirect").activeFocus);
        control(h, "psnSubmit").clicked();
        const oldClose = control(h, "psnClose");
        h.replaceAccount();
        h.current.forceActiveFocus(Qt.TabFocusReason);
        h.backend.finish("EjRWeJCrze8=");
        wait(1);
        compare(h.current.objectName, "replacement");
        compare(h.closes, 0);
        verify(!oldClose.activeFocus);
        compare(field(h, "AccountId").text, "");
    }
    function test_changedProfileSignInCanCloseWithoutApplyingAccount() {
        const h = createTemporaryObject(harnessComponent, testCase);
        activate(field(h, "Login"));
        tryVerify(() => control(h, "psnRedirect").activeFocus);
        control(h, "psnSubmit").clicked();
        h.backend.settings.currentProfile = "profile-b";
        h.backend.finish("EjRWeJCrze8=");
        compare(field(h, "AccountId").text, "");
        control(h, "psnClose").clicked();
        compare(h.closes, 1);
    }
    function test_stalledLookupCanRetryAndIgnoresLateResult() {
        const h = createTemporaryObject(harnessComponent, testCase);
        openLookup(h);
        tryVerify(() => control(h, "lookupSubmit").enabled, 31000);
        verify(control(h, "lookupError").text.indexOf("timed out") >= 0);
        control(h, "lookupSubmit").clicked();
        h.lookupCallbacks[0](200, '{"encoded_id":"AAAAAAAAAAA="}');
        compare(h.closes, 0);
        h.lookupCallbacks[1](200, '{"encoded_id":"EjRWeJCrze8="}');
        tryVerify(() => field(h, "AccountId").activeFocus);
        compare(field(h, "AccountId").text, "EjRWeJCrze8=");
    }
    function test_layoutPrivacyAndKeyboard_data() {
        return [{tag: "900x600", width: 900, height: 600}, {tag: "1280x720", width: 1280, height: 720}];
    }
    function test_layoutPrivacyAndKeyboard(data) {
        const h = createTemporaryObject(harnessComponent, testCase, {width: data.width, height: data.height});
        activate(field(h, "Lookup"));
        tryVerify(() => control(h, "lookupUsername").activeFocus);
        h.backend.settings.streamerMode = true;
        compare(control(h, "lookupUsername").echoMode, TextInput.Password);
        h.backend.settings.streamerMode = false;
        compare(control(h, "lookupUsername").echoMode, TextInput.Normal);
        const explanation = control(h, "lookupExplanation");
        const normal = explanation.text;
        explanation.text = normal.repeat(15);
        waitForRendering(h);
        verify(explanation.height >= explanation.implicitHeight - 1, "The full lookup explanation must remain readable");
        for (const name of ["lookupUsername", "lookupSubmit", "lookupCancel"]) {
            const item = control(h, name);
            item.forceActiveFocus(Qt.TabFocusReason);
            tryVerify(() => item.activeFocus);
            const scroll = control(h, "lookupScroll");
            tryVerify(() => {
                const point = item.mapToItem(scroll, 0, 0);
                return point.x >= -1 && point.y >= -1 && point.x + item.width <= scroll.width + 1 && point.y + item.height <= scroll.height + 1;
            }, 1000, name + " remains reachable with long text");
        }
        explanation.text = normal;
        control(h, "lookupUsername").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Down);
        tryVerify(() => control(h, "lookupSubmit").activeFocus);
        verify(control(h, "lookupSubmit").visualFocus);
        keyClick(Qt.Key_Return);
        verify(control(h, "lookupError").visible);
        keyClick(Qt.Key_Menu);
        compare(h.lookupRequests.length, 0);
        keyClick(Qt.Key_Escape);
        tryVerify(() => field(h, "AccountId").activeFocus);
    }
}
