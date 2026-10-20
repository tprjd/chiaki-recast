import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"
import "../../src/qml"

TestCase {
    id: testCase
    name: "SettingsRemote"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; SettingsHarness {} }
    Component {
        id: loginFlowComponent
        Item {
            id: flow
            width: 900
            height: 600
            property alias settings: fixture.settings
            property alias actions: fixture.actions
            property alias stack: stack
            property alias screen: initialSettings
            function showPSNTokenDialog(remote) {
                fixture.record("login", [remote]);
                stack.push(loginComponent);
            }
            function closeDialog() { stack.pop(); }
            SettingsHarness { id: fixture; screenActive: false; visible: false }
            StackView {
                id: stack
                anchors.fill: parent
                initialItem: SettingsDialog { id: initialSettings; backend: fixture.backend; navigation: flow }
            }
            Component {
                id: loginComponent
                FocusScope {
                    objectName: "fakePsnLogin"
                    StackView.onActivated: forceActiveFocus()
                    Keys.onEscapePressed: stack.pop()
                }
            }
        }
    }
    readonly property var credentials: ["psnRefreshToken", "psnAuthToken", "psnAuthTokenExpiry", "psnAccountId"]
    function remoteHarness(properties) {
        const harness = createTemporaryObject(harnessComponent, testCase, properties || {});
        mouseClick(findChild(harness, "settingsCategory7"));
        return harness;
    }
    function test_credentialConditions_data() {
        const cases = [];
        for (let mask = 0; mask < 16; mask++) cases.push({tag: "credentials-" + mask, mask: mask});
        return cases;
    }
    function test_credentialConditions(data) {
        const harness = remoteHarness();
        for (let index = 0; index < credentials.length; index++)
            harness.settings[credentials[index]] = data.mask & (1 << index) ? "synthetic-private-" + index : "";
        compare(findChild(harness, "openPsnLogin").visible, data.mask !== 15);
        compare(findChild(harness, "resetPsnTokens").visible, data.mask === 15);
        compare(harness.actions.length, 0);
    }
    function test_clearChangesOnlyCredentialsAndRestoresLoginFocus() {
        const harness = remoteHarness();
        for (const property of credentials) harness.settings[property] = "synthetic-private-" + property;
        harness.settings.portGuessingEnabled = true;
        harness.settings.portGuessCount = 37;
        harness.settings.portGuessSocketCount = 261;
        harness.settings.changeCount = 0;
        const clear = findChild(harness, "resetPsnTokens");
        clear.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        for (const property of credentials) compare(harness.settings[property], "");
        compare(harness.settings.changeCount, 4);
        compare(harness.settings.portGuessingEnabled, true);
        compare(harness.settings.portGuessCount, 37);
        compare(harness.settings.portGuessSocketCount, 261);
        verify(findChild(harness, "openPsnLogin").activeFocus);
        compare(harness.actions.length, 0);
    }
    function test_accountRefreshKeepsUsableFocus() {
        const harness = remoteHarness();
        const login = findChild(harness, "openPsnLogin");
        const clear = findChild(harness, "resetPsnTokens");
        login.forceActiveFocus(Qt.TabFocusReason);
        for (const property of credentials) harness.settings[property] = "synthetic-private-" + property;
        tryCompare(clear, "activeFocus", true);
        harness.settings.psnAuthToken = "";
        tryCompare(login, "activeFocus", true);
        const slider = findChild(harness, "portGuessCount");
        slider.forceActiveFocus(Qt.TabFocusReason);
        harness.settings.psnAuthToken = "synthetic-refreshed-token";
        wait(10);
        verify(slider.activeFocus);
    }
    function test_loginDispatchDoesNotChangeSettings() {
        const harness = remoteHarness();
        harness.settings.psnAccountId = "synthetic-partial-account";
        harness.settings.changeCount = 0;
        findChild(harness, "openPsnLogin").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(harness.actions.length, 1);
        compare(harness.actions[0].name, "login");
        compare(harness.actions[0].args[0], false);
        compare(harness.settings.changeCount, 0);
        compare(harness.settings.psnAccountId, "synthetic-partial-account");
    }
    function test_loginReturnFocus_data() {
        return [{tag: "cancel", authenticated: false}, {tag: "success", authenticated: true}];
    }
    function test_loginReturnFocus(data) {
        const flow = createTemporaryObject(loginFlowComponent, testCase);
        mouseClick(findChild(flow.screen, "settingsCategory7"));
        findChild(flow.screen, "openPsnLogin").forceActiveFocus(Qt.TabFocusReason);
        flow.settings.changeCount = 0;
        keyClick(Qt.Key_Return);
        tryCompare(flow.stack, "depth", 2);
        tryCompare(flow.stack, "busy", false);
        compare(flow.actions.length, 1);
        compare(flow.actions[0].name, "login");
        compare(flow.actions[0].args[0], false);
        compare(flow.settings.changeCount, 0);
        if (data.authenticated)
            for (const property of credentials) flow.settings[property] = "synthetic-private-" + property;
        flow.settings.changeCount = 0;
        verify(flow.stack.currentItem.activeFocus);
        keyClick(Qt.Key_Escape);
        tryCompare(flow.stack, "depth", 1);
        tryCompare(flow.stack, "busy", false);
        tryCompare(findChild(flow.screen, data.authenticated ? "resetPsnTokens" : "openPsnLogin"), "activeFocus", true);
        compare(flow.settings.changeCount, 0);
    }
    function test_portGuessToggleAndKeyboardSteps() {
        const harness = remoteHarness();
        harness.settings.portGuessCount = 40;
        harness.settings.portGuessSocketCount = 250;
        harness.settings.changeCount = 0;
        findChild(harness, "portGuessingEnabled").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Space);
        verify(harness.settings.portGuessingEnabled);
        compare(harness.settings.changeCount, 1);
        for (const name of ["portGuessCount", "portGuessSocketCount"]) {
            const value = harness.settings[name];
            findChild(harness, name).forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Right);
            compare(harness.settings[name], value + 1);
            keyClick(Qt.Key_Left);
            compare(harness.settings[name], value);
        }
        compare(harness.settings.changeCount, 5);
    }
    function test_openingBrowsingAndProfileRefreshDoNotWrite() {
        const harness = createTemporaryObject(harnessComponent, testCase, {screenActive: false});
        harness.settings.portGuessCount = 75;
        harness.settings.portGuessSocketCount = 250;
        harness.settings.changeCount = 0;
        harness.screenActive = true;
        mouseClick(findChild(harness, "settingsCategory7"));
        compare(harness.settings.changeCount, 0);
        compare(findChild(harness, "portGuessCount").value, 75);
        compare(findChild(harness, "portGuessSocketCount").value, 250);
        harness.settings.currentProfile = "Another profile";
        harness.settings.portGuessCount = 12;
        harness.settings.portGuessSocketCount = 93;
        harness.settings.changeCount = 0;
        mouseClick(findChild(harness, "settingsCategory0"));
        mouseClick(findChild(harness, "settingsCategory7"));
        compare(harness.settings.changeCount, 0);
        compare(findChild(harness, "portGuessCount").value, 12);
        compare(findChild(harness, "portGuessSocketCount").value, 93);
        compare(harness.actions.length, 0);
    }
    function publicText(item) {
        let result = "";
        if (item.visible) result += (item.text || "") + " " + item.Accessible.name + " ";
        for (const child of item.children || []) result += publicText(child);
        return result;
    }
    function test_privacyAndMinimumLayout_data() {
        return [{tag: "desktop", width: 1280, height: 720}, {tag: "minimum", width: 900, height: 600}];
    }
    function test_privacyAndMinimumLayout(data) {
        const harness = remoteHarness({width: data.width, height: data.height});
        for (const property of credentials) harness.settings[property] = "synthetic-private-" + property;
        for (const streamer of [false, true]) {
            harness.settings.streamerMode = streamer;
            verify(!publicText(harness.screen).includes("synthetic-private"));
        }
        const toggle = findChild(harness, "portGuessingEnabled");
        toggle.text = "A deliberately long translated port guessing option that must wrap within the settings group at the supported minimum window width.";
        wait(10);
        verify(toggle.contentItem.paintedWidth <= toggle.contentItem.width);
        for (const name of ["resetPsnTokens", "portGuessingEnabled", "portGuessCount", "portGuessSocketCount"]) {
            const control = findChild(harness, name);
            control.forceActiveFocus(Qt.TabFocusReason);
            wait(20);
            const point = control.mapToItem(harness, 0, 0);
            verify(point.x >= 0 && point.x + control.width <= harness.width, name + " horizontal bounds");
            verify(point.y >= 0 && point.y + control.height <= harness.height, name + " vertical bounds");
        }
    }
    function test_sliderWrites_data() {
        return [
            {tag: "port-min", name: "portGuessCount", initial: 40, max: false, value: 0, text: "0 guesses"},
            {tag: "port-max", name: "portGuessCount", initial: 40, max: true, value: 75, text: "75 guesses"},
            {tag: "socket-min", name: "portGuessSocketCount", initial: 250, max: false, value: 0, text: "0 sockets"},
            {tag: "socket-max", name: "portGuessSocketCount", initial: 250, max: true, value: 500, text: "500 sockets"}
        ];
    }
    function test_sliderWrites(data) {
        const harness = remoteHarness();
        harness.settings[data.name] = data.initial;
        harness.settings.changeCount = 0;
        const slider = findChild(harness, data.name);
        verify(slider);
        slider.forceActiveFocus(Qt.TabFocusReason);
        wait(10);
        mouseClick(slider, data.max ? slider.width - 1 : 1, slider.height / 2);
        compare(harness.settings[data.name], data.value);
        compare(harness.settings.changeCount, 1);
        compare(findChild(harness, data.name + "Value").text, data.text);
    }
}
