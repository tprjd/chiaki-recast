import QtQuick
import QtTest
import "../../src/qml"

TestCase {
    id: testCase
    name: "ConsoleNavigation"
    width: 1280
    height: 720
    visible: true
    when: windowShown

    Component { id: screenComponent; ConsoleLibrary { width: 1280; height: 720 } }
    SignalSpy { id: commandSpy }
    SignalSpy { id: connectSpy; signalName: "connectRequested" }

    function cleanup() {
        commandSpy.target = null;
        commandSpy.clear();
        connectSpy.target = null;
        connectSpy.clear();
    }

    function test_mainCommands_data() {
        return [
            { tag: "settings", button: "settingsButton", signal: "settingsRequested" },
            { tag: "add", button: "addConsoleButton", signal: "addConsoleRequested" },
            { tag: "quit", button: "quitButton", signal: "quitRequested" }
        ];
    }

    function test_mainCommands(data) {
        const screen = createTemporaryObject(screenComponent, testCase);
        commandSpy.signalName = data.signal;
        commandSpy.target = screen;
        const button = findChild(screen, data.button);
        verify(button && button.visible && button.enabled);
        mouseClick(button);
        compare(commandSpy.count, 1);
    }

    function test_optionalCommands_data() {
        return [
            { tag: "psn", property: "psnAvailable", button: "refreshPsnButton", signal: "refreshPsnRequested" },
            { tag: "steam", property: "steamAvailable", button: "steamShortcutButton", signal: "steamShortcutRequested" }
        ];
    }

    function test_optionalCommands(data) {
        const screen = createTemporaryObject(screenComponent, testCase);
        const button = findChild(screen, data.button);
        verify(button && !button.visible);
        screen[data.property] = true;
        verify(button.visible && button.enabled);
        commandSpy.signalName = data.signal;
        commandSpy.target = screen;
        mouseClick(button);
        compare(commandSpy.count, 1);
        screen[data.property] = false;
        verify(!button.visible);
    }

    function test_discoveryToggle_data() {
        return [{ tag: "enable", enabled: false }, { tag: "disable", enabled: true }];
    }

    function test_discoveryToggle(data) {
        const screen = createTemporaryObject(screenComponent, testCase, { discoveryEnabled: data.enabled });
        commandSpy.signalName = "discoveryToggled";
        commandSpy.target = screen;
        mouseClick(findChild(screen, "discoveryButton"));
        compare(commandSpy.count, 1);
        compare(commandSpy.signalArguments[0][0], !data.enabled);
    }

    function host(name, mac) {
        return { display: true, discovered: true, registered: true, state: "ready",
            name, mac, address: "192.0.2.7", ps5: true };
    }

    function test_layout_data() {
        return [{ tag: "desktop", width: 1280, height: 720, collapsed: false },
            { tag: "narrow", width: 800, height: 600, collapsed: true }];
    }

    function test_layout(data) {
        const hosts = [host("Living room", "01"), host("Bedroom", "02"), host("Studio", "03")];
        const screen = createTemporaryObject(screenComponent, testCase, {
            width: data.width, height: data.height, hosts, psnAvailable: true, steamAvailable: true
        });
        waitForRendering(screen);
        compare(screen.collapsedSidebar, data.collapsed);
        for (const name of ["settingsButton", "addConsoleButton", "refreshPsnButton", "steamShortcutButton", "quitButton"]) {
            const button = findChild(screen, name);
            const position = button.mapToItem(screen, 0, 0);
            verify(button.visible && button.width > 0 && button.height > 0);
            verify(position.x >= 0 && position.y >= 0, name);
            verify(position.x + button.width <= screen.width, name + " horizontal bounds");
            verify(position.y + button.height <= screen.height, name + " vertical bounds");
        }
        const card = findChild(screen, "consoleCard");
        const action = findChild(card, "connectButton");
        const actionPosition = action.mapToItem(card, 0, 0);
        verify(card.width >= 270);
        verify(actionPosition.y + action.height <= card.height);
        const scroll = findChild(screen, "libraryScroll");
        if (data.collapsed)
            verify(scroll.contentHeight > scroll.height, "Additional cards remain reachable by scrolling");
    }

    function test_controllerSelectionUsesVisibleHostIdentity() {
        const hidden = Object.assign(host("Hidden", "00"), { display: false });
        const first = host("Living room", "01");
        const second = host("Bedroom", "02");
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [hidden, first, second] });
        commandSpy.signalName = "connectRequested";
        commandSpy.target = screen;
        screen.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Right);
        keyClick(Qt.Key_Return);
        compare(commandSpy.count, 1);
        compare(commandSpy.signalArguments[0][0], 2);
        compare(commandSpy.signalArguments[0][1], "Bedroom");
    }

    function test_primaryActionHasVisibleBackground() {
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host("Living room", "01")] });
        const button = findChild(screen, "connectButton");
        waitForRendering(button);
        const image = grabImage(button);
        verify(image.red(5, Math.floor(button.height / 2)) > 0.3,
            "Connect has a visible blue background with the supported software renderer");
    }

    function test_consoleBodyIsReadableOnDarkCard() {
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host("Living room", "01")] });
        const artwork = findChild(screen, "consoleArtwork");
        tryCompare(artwork, "status", Image.Ready);
        const image = grabImage(artwork);
        verify(image.red(Math.floor(artwork.width / 2), Math.floor(artwork.height / 2)) > 0.8,
            "The console body contrasts with the dark artwork panel");
    }

    function test_emptyNavigationTerminates() {
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [Object.assign(host("Hidden", "00"), { display: false })]
        });
        commandSpy.signalName = "connectRequested";
        commandSpy.target = screen;
        screen.forceActiveFocus(Qt.TabFocusReason);
        for (const key of [Qt.Key_Left, Qt.Key_Right, Qt.Key_Up, Qt.Key_Down, Qt.Key_Return])
            keyClick(key);
        compare(commandSpy.count, 0);
    }

    function test_managementAccess_data() {
        return [
            { tag: "pin", changes: {}, item: "pinAction", action: "pin", keyboard: true },
            { tag: "wake", changes: { manual: true, discovered: false }, item: "wakeAction", action: "wake" },
            { tag: "delete", changes: { manual: true, discovered: false }, item: "removeAction", action: "delete" },
            { tag: "hide", changes: { registered: false }, item: "removeAction", action: "hide" }
        ];
    }

    function test_managementAccess(data) {
        const target = Object.assign(host("Living room", "01"), data.changes);
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [target] });
        commandSpy.signalName = "managementRequested";
        commandSpy.target = screen;
        connectSpy.target = screen;
        mouseClick(findChild(screen, "consoleActionsButton"));
        tryCompare(findChild(screen, "consoleDetails"), "opened", true);
        mouseClick(findChild(screen, "consoleManageButton"));
        const menu = findChild(screen, "consoleActionsMenu");
        tryCompare(menu, "opened", true);
        const action = findChild(screen, data.item);
        verify(action.visible && action.enabled);
        if (data.keyboard) {
            action.forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Return);
        } else {
            mouseClick(action);
        }
        tryCompare(commandSpy, "count", 1);
        compare(commandSpy.signalArguments[0][0], data.action);
        compare(commandSpy.signalArguments[0][1], screen.identityOf(target));
        compare(connectSpy.count, 0);
    }

    function test_escapeClosesMenuAndRestoresOpener() {
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host("Living room", "01")] });
        commandSpy.signalName = "quitRequested";
        commandSpy.target = screen;
        const opener = findChild(screen, "consoleActionsButton");
        mouseClick(opener);
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        const manage = findChild(screen, "consoleManageButton");
        mouseClick(manage);
        const menu = findChild(screen, "consoleActionsMenu");
        tryCompare(menu, "opened", true);
        keyClick(Qt.Key_Escape);
        tryCompare(menu, "opened", false);
        tryCompare(manage, "activeFocus", true);
        verify(details.opened);
        keyClick(Qt.Key_Escape);
        tryCompare(details, "opened", false);
        tryCompare(opener, "activeFocus", true);
        compare(commandSpy.count, 0);
    }

    function test_controllerCommands_data() {
        return [
            { tag: "settings", key: Qt.Key_Menu, signal: "settingsRequested", options: {}, count: 1 },
            { tag: "add", key: Qt.Key_F2, signal: "addConsoleRequested", options: {}, count: 1 },
            { tag: "psn", key: Qt.Key_PageDown, signal: "refreshPsnRequested", options: { psnAvailable: true }, count: 1 },
            { tag: "no-psn", key: Qt.Key_PageDown, signal: "refreshPsnRequested", options: {}, count: 0 },
            { tag: "steam", key: Qt.Key_F1, signal: "steamShortcutRequested", options: { steamAvailable: true }, count: 1 },
            { tag: "no-steam", key: Qt.Key_F1, signal: "steamShortcutRequested", options: {}, count: 0 }
        ];
    }

    function test_controllerCommands(data) {
        const screen = createTemporaryObject(screenComponent, testCase, data.options);
        commandSpy.signalName = data.signal;
        commandSpy.target = screen;
        screen.forceActiveFocus(Qt.TabFocusReason);
        keyClick(data.key);
        compare(commandSpy.count, data.count);
    }
}
