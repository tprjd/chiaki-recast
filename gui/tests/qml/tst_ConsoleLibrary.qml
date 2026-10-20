import QtQuick
import QtQuick.Controls
import QtTest
import "../../src/qml"

TestCase {
    id: testCase
    name: "ConsoleLibrary"
    width: 1280
    height: 720
    visible: true
    when: windowShown

    Component {
        id: libraryComponent
        ConsoleLibrary { width: 1280; height: 720 }
    }

    SignalSpy { id: connectSpy; signalName: "connectRequested" }

    function readyHost(name) {
        return { display: true, registered: true, discovered: true,
            state: "ready", name: name, address: "192.0.2.7",
            mac: "00:11:22:33:44:55", ps5: true };
    }

    function cleanup() {
        connectSpy.target = null;
        connectSpy.clear();
    }

    function atSource(host, index) {
        const hosts = [];
        for (let i = 0; i < index; i++)
            hosts.push({ display: false });
        hosts.push(host);
        return hosts;
    }

    function namedConnectButton(item, name) {
        if (item.objectName === "connectButton" && item.Accessible.name.indexOf(name) >= 0)
            return item;
        for (const child of item.children || []) {
            const button = namedConnectButton(child, name);
            if (button)
                return button;
        }
        return null;
    }

    function test_readyConsoleConnectsToSourceOnce() {
        const library = createTemporaryObject(libraryComponent, testCase, {
            hosts: atSource(readyHost("Living room"), 7)
        });
        verify(library);
        connectSpy.target = library;
        compare(findChild(library, "consoleName").text, "Living room");
        compare(findChild(library, "consoleStatus").text, "Online");
        const connectButton = findChild(library, "connectButton");
        verify(connectButton.enabled);
        mouseClick(connectButton);
        compare(connectSpy.count, 1);
        compare(connectSpy.signalArguments[0][0], 7);
        compare(connectSpy.signalArguments[0][1], "Living room");
    }

    function test_enterConnectsOnceWithVisibleFocus() {
        const library = createTemporaryObject(libraryComponent, testCase, {
            hosts: atSource(readyHost("Living room"), 7)
        });
        connectSpy.target = library;
        const connectButton = findChild(library, "connectButton");
        connectButton.forceActiveFocus(Qt.TabFocusReason);
        verify(connectButton.visualFocus);
        keyClick(Qt.Key_Return);
        compare(connectSpy.count, 1);
        compare(connectSpy.signalArguments[0][0], 7);
    }

    function test_standbyConsoleWakesAndConnectsWithNickname() {
        const host = readyHost("Living room");
        host.state = "standby";
        const library = createTemporaryObject(libraryComponent, testCase, {
            hosts: atSource(host, 4)
        });
        connectSpy.target = library;
        compare(findChild(library, "consoleStatus").text, "In standby");
        const button = findChild(library, "connectButton");
        compare(button.text, "Wake and connect");
        mouseClick(button);
        compare(connectSpy.count, 1);
        compare(connectSpy.signalArguments[0][0], 4);
        compare(connectSpy.signalArguments[0][1], "Living room");
        compare(findChild(library, "consoleStatus").text, "In standby");
    }

    function test_hostCase_data() {
        return [
            { tag: "ready", changes: {}, status: "Online", route: "Local network", action: "Connect", nickname: "Living room", automatic: false },
            { tag: "standby", changes: { state: "standby" }, status: "In standby", route: "Local network", action: "Wake and connect", nickname: "Living room", automatic: false },
            { tag: "manual", changes: { discovered: false, manual: true, state: "unknown" }, status: "Status unknown", route: "Manual address", action: "Connect", nickname: "", automatic: false },
            { tag: "manual-ready-is-unknown", changes: { discovered: false, manual: true }, status: "Status unknown", route: "Manual address", action: "Connect", nickname: "", automatic: false },
            { tag: "manual-registration", changes: { discovered: false, manual: true, registered: false }, status: "Not registered", route: "Manual address", action: "Register", nickname: "", automatic: false },
            { tag: "discovered-registration", changes: { registered: false, state: "standby" }, status: "Not registered", route: "Local network", action: "Register", nickname: "Living room", automatic: false },
            { tag: "automatic-registration", changes: { registered: false, duid: "psn-id" }, status: "Not registered", route: "Local network", action: "Register", nickname: "Living room", automatic: true },
            { tag: "psn", changes: { discovered: false, manual: false, duid: "psn-id", address: "" }, status: "PSN", route: "Remote connection via PSN", action: "Connect", nickname: "", automatic: false },
            { tag: "missing-state", changes: { state: undefined }, status: "Status unknown", route: "Local network", action: "Connect", nickname: "Living room", automatic: false },
            { tag: "unknown-state", changes: { state: "unrecognized" }, status: "Status unknown", route: "Local network", action: "Connect", nickname: "Living room", automatic: false }
        ];
    }

    function test_hostCase(data) {
        const host = Object.assign(readyHost("Living room"), data.changes);
        const library = createTemporaryObject(libraryComponent, testCase, {
            hosts: atSource(host, 6)
        });
        connectSpy.target = library;
        compare(findChild(library, "consoleStatus").text, data.status);
        compare(findChild(library, "consoleRoute").text, data.route);
        compare(findChild(library, "automaticRegistration").visible, data.automatic);
        const button = findChild(library, "connectButton");
        compare(button.text, data.action);
        verify(button.enabled);
        mouseClick(button);
        compare(connectSpy.count, 1);
        compare(connectSpy.signalArguments[0][0], 6);
        compare(connectSpy.signalArguments[0][1], data.nickname);
    }

    function test_hiddenAndDuplicateSuppressedHostsHaveNoCards() {
        const hidden = Object.assign(readyHost("Hidden"), { display: false });
        const duplicate = Object.assign(readyHost("Duplicate"), { display: false, discovered: false, manual: true });
        const library = createTemporaryObject(libraryComponent, testCase, {
            hosts: [hidden, readyHost("Living room"), duplicate]
        });
        const oneCard = createTemporaryObject(libraryComponent, testCase, {
            hosts: [readyHost("Living room")], visible: false
        });
        compare(findChild(library, "consoleName").text, "Living room");
        compare(findChild(library, "libraryScroll").contentHeight, findChild(oneCard, "libraryScroll").contentHeight);
        connectSpy.target = library;
        mouseClick(findChild(library, "connectButton"));
        compare(connectSpy.count, 1);
        compare(connectSpy.signalArguments[0][0], 1);

        library.hosts = [hidden, duplicate];
        compare(findChild(library, "connectButton"), null);
        const scroll = findChild(library, "libraryScroll");
        verify(scroll.contentHeight < scroll.height);
    }

    function test_savedTargetFollowsHostAcrossReorderAndRename() {
        const first = readyHost("Living room");
        const second = Object.assign(readyHost("Living room"), { mac: "00:11:22:33:44:66" });
        const library = createTemporaryObject(libraryComponent, testCase, { hosts: [first, second] });
        connectSpy.target = library;
        const target = library.identityOf(first);
        library.hosts = [second, Object.assign({}, first, { name: "Bedroom" })];
        verify(library.connectToTarget(target));
        compare(connectSpy.count, 1);
        compare(connectSpy.signalArguments[0][0], 1);
        compare(connectSpy.signalArguments[0][1], "Bedroom");
    }

    function test_invalidTarget_data() {
        const host = readyHost("Living room");
        return [
            { tag: "removed", host, current: [] },
            { tag: "replaced-at-old-index", host, current: [Object.assign({}, host, { mac: "00:11:22:33:44:66" })] },
            { tag: "manual-address-reused", host: Object.assign({}, host, { discovered: false, manual: true }), current: [Object.assign({}, host, { discovered: false, manual: true, mac: "00:11:22:33:44:66" })] },
            { tag: "ambiguous", host, current: [host, Object.assign({}, host)] },
            { tag: "hidden", host, current: [Object.assign({}, host, { display: false })] },
            { tag: "missing-identity", host: { display: true, discovered: true, registered: true, name: "Console" }, current: [] },
        ];
    }

    function test_invalidTarget(data) {
        const library = createTemporaryObject(libraryComponent, testCase, { hosts: [data.host] });
        const target = library.identityOf(data.host);
        library.hosts = data.current;
        connectSpy.target = library;
        verify(!library.connectToTarget(target));
        compare(connectSpy.count, 0);
    }

    function test_profileSwitchInvalidatesSavedTarget() {
        const host = readyHost("Living room");
        const library = createTemporaryObject(libraryComponent, testCase, { hosts: [host], profileKey: "first" });
        verify(library);
        const target = library.identityOf(host);
        library.profileKey = "second";
        library.profileKey = "first";
        connectSpy.target = library;
        verify(!library.connectToTarget(target));
        compare(connectSpy.count, 0);
        mouseClick(findChild(library, "connectButton"));
        compare(connectSpy.count, 1);
    }

    function test_missingOrInvalidTypeHasNeutralLabel_data() {
        return [{ tag: "missing", ps5: undefined }, { tag: "invalid", ps5: "false" }];
    }

    function test_missingOrInvalidTypeHasNeutralLabel(data) {
        const host = Object.assign(readyHost("Living room"), { ps5: data.ps5 });
        const library = createTemporaryObject(libraryComponent, testCase, { hosts: [host] });
        const type = findChild(library, "consoleType");
        verify(type && type.visible);
        compare(type.text, "Console");
        compare(findChild(library, "consoleArtwork").source.toString(), "");
        verify(findChild(library, "connectButton").enabled);
    }

    function test_unconfirmedPowerHasNoOnlineLight_data() {
        return [
            { tag: "manual", changes: { discovered: false, manual: true } },
            { tag: "psn", changes: { discovered: false, duid: "psn-id", address: "" } },
            { tag: "missing-state", changes: { state: undefined } },
            { tag: "unknown-state", changes: { state: "unrecognized" } }
        ];
    }

    function test_unconfirmedPowerHasNoOnlineLight(data) {
        const host = Object.assign(readyHost("Living room"), data.changes);
        const library = createTemporaryObject(libraryComponent, testCase, { hosts: [host] });
        const artwork = findChild(library, "consoleArtwork");
        compare(artwork.source.toString(), "image://svg/console-ps5#console");
        tryCompare(artwork, "status", Image.Ready);
    }

    function test_cardUsesCurrentHostList_data() {
        return [
            { tag: "discovered", host: readyHost("Living room"), other: Object.assign(readyHost("Living room"), { mac: "00:11:22:33:44:66" }), nickname: "Bedroom" },
            { tag: "manual", host: { display: true, manual: true, discovered: false, registered: false, name: "192.0.2.7", address: "192.0.2.7", state: "unknown", duid: "" }, other: { display: true, manual: true, discovered: false, registered: false, name: "192.0.2.8", address: "192.0.2.8", state: "unknown", duid: "" }, nickname: "" },
            { tag: "psn", host: { display: true, manual: false, discovered: false, registered: true, name: "Living room", address: "", duid: "psn-first", ps5: true }, other: { display: true, manual: false, discovered: false, registered: true, name: "Living room", address: "", duid: "psn-second", ps5: true }, nickname: "" }
        ];
    }

    function test_cardUsesCurrentHostList(data) {
        const library = createTemporaryObject(libraryComponent, testCase, {
            hosts: [data.host, data.other]
        });
        library.hosts = [data.other, Object.assign({}, data.host, { name: "Bedroom" })];

        connectSpy.target = library;
        mouseClick(namedConnectButton(library, "Bedroom"));
        compare(connectSpy.count, 1);
        compare(connectSpy.signalArguments[0][0], 1);
        compare(connectSpy.signalArguments[0][1], data.nickname);
    }

    function test_registrationActionFitsInsideLongNameCard() {
        const host = Object.assign(readyHost("Living room PlayStation with a long name"), {
            registered: false, duid: "psn-first"
        });
        const library = createTemporaryObject(libraryComponent, testCase, { hosts: [host] });
        waitForRendering(library);
        const button = findChild(library, "connectButton");
        const card = findChild(library, "consoleCard");
        const position = button.mapToItem(card, 0, 0);
        verify(position.y + button.height <= card.height,
            "The primary action must fit inside its card");
    }

    function test_streamerModeMasksAddressName() {
        const library = createTemporaryObject(libraryComponent, testCase, {
            hosts: [readyHost("192.0.2.7")], streamerMode: true
        });
        const nameLabel = findChild(library, "consoleName");
        compare(nameLabel.text, "Manual console");
        compare(nameLabel.Accessible.name, "Manual console");
        const nameTip = nameLabel.data.find(item => item.objectName === "consoleNameToolTip");
        compare(nameTip.text, "Manual console");
        compare(findChild(library, "connectButton").Accessible.name,
            "Connect to Manual console");
    }
}
