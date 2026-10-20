import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "RegistrationForm"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; RegistrationHarness {} }
    function field(harness, name) { return findChild(harness.screen, "registration" + name); }
    function enter(control, text) {
        control.forceActiveFocus(Qt.TabFocusReason);
        if (control.readOnly) keyClick(Qt.Key_Return);
        keyClick(Qt.Key_A, Qt.ControlModifier);
        for (let i = 0; i < text.length; i++) keyClick(text[i]);
        keyClick(Qt.Key_Return);
    }
    function activate(control) {
        control.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
    }
    function choose(combo, index) {
        activate(combo);
        tryCompare(combo.popup, "visible", true);
        keyClick(Qt.Key_Home);
        for (let i = 0; i < index; i++) keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", false);
    }
    function complete(harness) {
        enter(field(harness, "AccountId"), "  EjRWeJCrze8=  ");
        enter(field(harness, "Pin"), "00123456");
    }
    function test_routeDefaultsAndSavedAccount() {
        const harness = createTemporaryObject(harnessComponent, testCase, {screenActive: false,
            initialHost: "255.255.255.255", initialPs5: false});
        harness.settings.psnAccountId = "EjRWeJCrze8=";
        harness.screenActive = true;
        compare(findChild(harness, "registrationHost").text, "255.255.255.255");
        compare(findChild(harness, "registrationConsole").currentIndex, 3);
        compare(findChild(harness, "registrationAccountId").text, "EjRWeJCrze8=");
        verify(!findChild(harness, "registrationLogin").visible);
        verify(!findChild(harness, "registrationLookup").visible);
        compare(harness.backend.requests.length, 0);
        compare(harness.actions.length, 0);
    }
    function test_accountRoutesKeepDetails_data() {
        return [{tag: "sign-in", login: true}, {tag: "public-lookup", login: false}];
    }
    function test_accountRoutesKeepDetails(data) {
        const harness = createTemporaryObject(harnessComponent, testCase);
        enter(field(harness, "Host"), "  games.example  ");
        enter(field(harness, "AccountId"), "deliberate edit");
        enter(field(harness, "Pin"), "00123456");
        enter(field(harness, "ConsolePin"), "0042");
        activate(field(harness, data.login ? "Login" : "Lookup"));
        compare(harness.actions[0].login, data.login);
        harness.settings.psnAccountId = "stale saved value";
        harness.returnFromAccount("");
        tryVerify(() => field(harness, "AccountId").activeFocus);
        compare(field(harness, "AccountId").text, "deliberate edit");
        // A canceled account screen must not write or submit registration.
        compare(harness.backend.requests.length, 0);
        harness.settings.psnAccountId = "";
        activate(field(harness, data.login ? "Login" : "Lookup"));
        harness.returnFromAccount("EjRWeJCrze8=");
        tryVerify(() => field(harness, "AccountId").activeFocus);
        compare(field(harness, "AccountId").text, "EjRWeJCrze8=");
        compare(field(harness, "Host").text, "  games.example  ");
        compare(field(harness, "Pin").text, "00123456");
        compare(field(harness, "ConsolePin").text, "0042");
        compare(harness.backend.requests.length, 0);
    }
    function test_consoleTargets_data() {
        return [{tag: "ps5", index: 0, target: 1000100}, {tag: "ps4-before7", index: 1, target: 800},
            {tag: "ps4-before8", index: 2, target: 900}, {tag: "ps4-current", index: 3, target: 1000}];
    }
    function test_consoleTargets(data) {
        const harness = createTemporaryObject(harnessComponent, testCase);
        complete(harness);
        choose(field(harness, "Console"), data.index);
        const legacy = data.index === 1;
        compare(field(harness, "OnlineId").visible, legacy);
        compare(field(harness, "AccountId").visible, !legacy);
        if (legacy) enter(field(harness, "OnlineId"), "  CaseSensitiveUser  ");
        enter(field(harness, "Host"), "  255.255.255.255  ");
        enter(field(harness, "ConsolePin"), "0042");
        verify(field(harness, "ConsoleHelp").text.indexOf(data.index === 0 ? "Link Device" : "Add Device") >= 0);
        activate(field(harness, "Submit"));
        compare(harness.backend.requests.length, 1);
        const request = harness.backend.requests[0];
        compare(request.target, data.target);
        compare(request.host, "255.255.255.255");
        compare(request.identity, legacy ? "CaseSensitiveUser" : "EjRWeJCrze8=");
        compare(request.pin, "00123456");
        compare(request.consolePin, "0042");
        compare(request.broadcast, true);
        const log = findChild(harness, "registrationLog");
        tryCompare(log, "visible", true);
        tryVerify(() => findChild(harness, "registrationLogText").activeFocus);
        harness.backend.callback("controlled diagnostic", true, false);
        compare(findChild(harness, "registrationLogText").text, "controlled diagnostic\n");
        harness.backend.callback("", true, true);
        compare(log.standardButtons, Dialog.Close);
        keyClick(Qt.Key_Return);
        tryCompare(log, "visible", false);
        compare(harness.actions[0].name, "close");
        compare(harness.backend.requests.length, 1);
    }
    function test_hostnameAndOptionalPin() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        complete(harness);
        enter(field(harness, "Host"), "  games.example  ");
        activate(field(harness, "Submit"));
        compare(harness.backend.requests[0].host, "games.example");
        compare(harness.backend.requests[0].broadcast, false);
        compare(harness.backend.requests[0].consolePin, "");
        tryVerify(() => findChild(harness, "registrationLogText").activeFocus);
        keyClick(Qt.Key_Escape);
        tryVerify(() => harness.actions.length > 0);
        compare(harness.actions[0].name, "close");
    }
    function test_invalidDetails_data() {
        return [{tag: "address", name: "Host", text: "   ", message: "Enter a console address."},
            {tag: "identity", name: "AccountId", text: "   ", message: "Enter your PSN Account ID."},
            {tag: "online-identity", name: "OnlineId", text: "   ", message: "Enter your PSN Online ID."},
            {tag: "short-pin", name: "Pin", text: "1234567", message: "Enter exactly eight digits."},
            {tag: "short-console-pin", name: "ConsolePin", text: "001", message: "Enter exactly four digits, or leave this empty."}];
    }
    function test_invalidDetails(data) {
        const harness = createTemporaryObject(harnessComponent, testCase);
        complete(harness);
        if (data.name === "OnlineId") choose(field(harness, "Console"), 1);
        enter(field(harness, data.name), data.text);
        verify(!field(harness, "Submit").enabled);
        const helpName = data.name === "OnlineId" ? "AccountId" : data.name;
        verify(findChild(harness, "registration" + helpName + "Help").text.indexOf(data.message) >= 0);
        keyClick(Qt.Key_Menu);
        compare(harness.backend.requests.length, 0);
    }
    function test_nativeRejectionLeavesFormEditable() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        complete(harness);
        harness.backend.acceptRegistration = false;
        activate(field(harness, "Submit"));
        compare(harness.backend.requests.length, 1);
        verify(!findChild(harness, "registrationLog").visible);
        verify(field(harness, "Submit").enabled);
        verify(field(harness, "Error").text.indexOf("eight bytes encoded as Base64") >= 0);
        enter(field(harness, "AccountId"), "fixed account");
        compare(field(harness, "AccountId").text, "fixed account");
    }
    function test_navigationDoesNotSubmit_data() {
        return [{tag: "back", name: "Back"}, {tag: "cancel", name: "Cancel"}, {tag: "escape", name: ""}];
    }
    function test_navigationDoesNotSubmit(data) {
        const harness = createTemporaryObject(harnessComponent, testCase);
        complete(harness);
        if (data.name) activate(field(harness, data.name));
        else keyClick(Qt.Key_Escape);
        compare(harness.actions[0].name, "close");
        compare(harness.backend.requests.length, 0);
    }
    function test_popupEscapeAndTextEditing() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        activate(field(harness, "Console"));
        tryCompare(field(harness, "Console").popup, "visible", true);
        keyClick(Qt.Key_Escape);
        tryCompare(field(harness, "Console").popup, "visible", false);
        compare(harness.actions.length, 0);
        const host = field(harness, "Host");
        host.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        keyClick(Qt.Key_Home);
        keyClick(Qt.Key_Right);
        compare(host.cursorPosition, 1);
        verify(host.activeFocus);
        keyClick(Qt.Key_Escape);
        verify(host.readOnly);
        compare(harness.actions.length, 0);
        keyClick(Qt.Key_Escape);
        compare(harness.actions[0].name, "close");
    }
    function test_privacyAndLayout_data() {
        return [{tag: "normal", width: 1280, height: 720}, {tag: "minimum", width: 900, height: 600}];
    }
    function test_privacyAndLayout(data) {
        const harness = createTemporaryObject(harnessComponent, testCase, {width: data.width, height: data.height});
        complete(harness);
        harness.settings.streamerMode = true;
        enter(field(harness, "OnlineId"), "SamplePlayer");
        for (const name of ["Host", "AccountId", "OnlineId", "Pin", "ConsolePin"])
            compare(field(harness, name).echoMode, TextInput.Password);
        waitForRendering(harness);
        for (const name of ["Console", "Host", "AccountId", "Login", "Lookup", "Pin", "ConsolePin", "Cancel", "Submit"]) {
            const control = field(harness, name);
            control.forceActiveFocus(Qt.TabFocusReason);
            wait(20);
            const point = control.mapToItem(harness, 0, 0);
            verify(point.x >= 0 && point.x + control.width <= harness.width + 1, name + " fits horizontally");
            verify(point.y >= 0 && point.y + control.height <= harness.height + 1, name + " scrolls into view");
            verify(control.Accessible.name.length > 0, name + " has an accessible name");
        }
        field(harness, "Host").forceActiveFocus(Qt.TabFocusReason);
        wait(30);
        const form = field(harness, "FormPanel").mapToItem(harness, 0, 0);
        const help = field(harness, "Instructions").mapToItem(harness, 0, 0);
        verify(help.x > form.x + field(harness, "FormPanel").width);
        compare(harness.backend.requests.length, 0);
    }
}
