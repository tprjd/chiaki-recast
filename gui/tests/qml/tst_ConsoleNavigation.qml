import QtQuick
import QtQuick.Controls
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

    function test_sidebarArrowsDoNotMoveConsoleSelection() {
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [host("Living room", "01"), host("Bedroom", "02")]
        });
        const settings = findChild(screen, "settingsButton");
        settings.forceActiveFocus(Qt.TabFocusReason);
        const selected = screen.selectedIdentity;
        keyClick(Qt.Key_Right);
        compare(screen.selectedIdentity, selected);
        verify(settings.activeFocus);
    }

    function descendantsNamed(item, name) {
        let items = item.objectName === name ? [item] : [];
        for (const child of item.children || [])
            items = items.concat(descendantsNamed(child, name));
        return items;
    }

    function test_tabOrderKeepsEveryActionVisible_data() {
        return [{ tag: "desktop", width: 1280, height: 720 },
            { tag: "narrow", width: 800, height: 600 }];
    }

    function test_tabOrderKeepsEveryActionVisible(data) {
        const hosts = [];
        for (let index = 0; index < 7; index++) hosts.push(host("Console " + index, "mac" + index));
        const screen = createTemporaryObject(screenComponent, testCase, {
            width: data.width, height: data.height, hosts, psnAvailable: true, steamAvailable: true
        });
        waitForRendering(screen);
        const order = ["consolesButton", "settingsButton", "steamShortcutButton", "quitButton",
            "addConsoleButton", "consoleSearch", "refreshPsnButton"].map(name => findChild(screen, name));
        for (const card of descendantsNamed(screen, "consoleCard"))
            order.push(findChild(card, "connectButton"), findChild(card, "consoleActionsButton"));
        order.push(findChild(screen, "discoveryButton"));
        order[0].forceActiveFocus(Qt.TabFocusReason);
        for (let index = 0; index < order.length; index++) {
            const control = order[index];
            verify(control.activeFocus, "Tab reaches " + control.objectName + " at " + index);
            verify(control.Accessible.name.length > 0 || control.text.length > 0);
            const position = control.mapToItem(screen, 0, 0);
            verify(position.y >= 0 && position.y + control.height <= screen.height,
                "Focused " + control.objectName + " is visible at " + position.y);
            keyClick(Qt.Key_Tab);
        }
        for (let index = order.length - 1; index >= 0; index--) {
            keyClick(Qt.Key_Tab, Qt.ShiftModifier);
            verify(order[index].activeFocus, "Shift+Tab reaches " + index);
        }
    }

    function test_selectedConsoleHintsMatchAvailableActions_data() {
        return [
            { tag: "local", changes: {}, expected: ["PIN"] },
            { tag: "manual", changes: { discovered: false, manual: true }, expected: ["PIN", "Wake", "Delete"] },
            { tag: "unregistered", changes: { registered: false }, expected: ["Hide"] },
            { tag: "remote", changes: { discovered: false, duid: "remote-id" }, expected: ["PIN"] }
        ];
    }

    function test_selectedConsoleHintsMatchAvailableActions(data) {
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [Object.assign(host("Living room", "01"), data.changes)]
        });
        const hints = findChild(screen, "controllerHints");
        verify(hints && hints.visible, "Selected-console controller hints are visible outside details");
        compare(descendantsNamed(hints, "controllerHintLabel").map(item => item.text), data.expected);
        verify(findChild(screen, "connectButton").icon.source.toString().includes("#cross"));
        screen.hosts = [];
        verify(!hints.visible);
    }

    Component {
        id: focusSinkComponent
        Item { focus: true }
    }
    Component {
        id: managementDialogComponent
        Popup {
            width: 260
            height: 120
            modal: true
            focus: true
            closePolicy: Popup.CloseOnEscape
        }
    }

    function test_managementDialogKeepsFocusAfterDetailsClose() {
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host("Living room", "01")] });
        const popup = createTemporaryObject(managementDialogComponent, screen);
        const details = findChild(screen, "consoleDetails");
        const opener = findChild(screen, "consoleActionsButton");
        let cancelDialog = null;
        screen.managementRequested.connect((action, accept, cancel) => {
            verify(!details.visible, "Details must finish closing before the next modal opens");
            cancelDialog = cancel;
            popup.open();
        });
        popup.closed.connect(() => cancelDialog());
        commandSpy.signalName = "quitRequested";
        commandSpy.target = screen;
        for (let attempt = 0; attempt < 2; attempt++) {
            mouseClick(opener);
            tryCompare(details, "opened", true);
            mouseClick(findChild(details.contentItem, "pinAction"));
            tryCompare(popup, "opened", true);
            verify(popup.activeFocus);
            keyClick(Qt.Key_Escape);
            tryCompare(popup, "visible", false);
            tryCompare(opener, "activeFocus", true);
            compare(commandSpy.count, 0, "Escape must close the active dialog without opening Quit");
        }
    }

    function test_returnFocusPreservesOpenerOrUsesRemainingCard_data() {
        return [{ tag: "settings", opener: "settingsButton", remove: false },
            { tag: "card", opener: "consoleActionsButton", remove: false },
            { tag: "removed-card", opener: "consoleActionsButton", remove: true }];
    }

    function test_returnFocusPreservesOpenerOrUsesRemainingCard(data) {
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [host("Living room", "01"), host("Bedroom", "02")]
        });
        const opener = findChild(screen, data.opener);
        opener.forceActiveFocus(Qt.TabFocusReason);
        screen.rememberFocus();
        const sink = createTemporaryObject(focusSinkComponent, testCase);
        sink.forceActiveFocus();
        if (data.remove) screen.hosts = [host("Bedroom", "02")];
        screen.restoreFocus();
        if (data.remove) {
            const action = findChild(screen, "connectButton");
            verify(action.activeFocus);
            compare(action.Accessible.name, "Connect to Bedroom");
        } else verify(opener.activeFocus);
    }

    function test_managementDialogReturnsToDetailsOpener() {
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host("Living room", "01")] });
        const opener = findChild(screen, "consoleActionsButton");
        mouseClick(opener);
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        commandSpy.signalName = "managementRequested";
        commandSpy.target = screen;
        mouseClick(findChild(details.contentItem, "pinAction"));
        tryCompare(commandSpy, "count", 1);
        tryCompare(details, "visible", false);
        const sink = createTemporaryObject(focusSinkComponent, testCase);
        sink.forceActiveFocus();
        commandSpy.signalArguments[0][2]();
        tryCompare(opener, "activeFocus", true);
    }

    function test_controllerShortcutsInDetails_data() {
        return [
            { tag: "pin", changes: {}, key: Qt.Key_PageUp, action: "pin", signal: "managementRequested", count: 1 },
            { tag: "no-pin", changes: { registered: false }, key: Qt.Key_PageUp, signal: "managementRequested", count: 0 },
            { tag: "wake", changes: { manual: true, discovered: false }, key: Qt.Key_Yes, signal: "wakeRequested", count: 1 },
            { tag: "no-wake", changes: {}, key: Qt.Key_Yes, signal: "wakeRequested", count: 0 },
            { tag: "delete", changes: { manual: true }, key: Qt.Key_No, action: "delete", signal: "managementRequested", count: 1 },
            { tag: "hide", changes: { registered: false }, key: Qt.Key_No, action: "hide", signal: "managementRequested", count: 1 }
        ];
    }

    function test_controllerShortcutsInDetails(data) {
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [Object.assign(host("Living room", "01"), data.changes)]
        });
        commandSpy.signalName = data.signal;
        commandSpy.target = screen;
        connectSpy.target = screen;
        mouseClick(findChild(screen, "consoleActionsButton"));
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        keyClick(data.key);
        if (data.count) {
            tryCompare(commandSpy, "count", data.count);
            if (data.action) {
                compare(commandSpy.signalArguments[0][0], data.action);
                commandSpy.signalArguments[0][2]();
            }
        } else compare(commandSpy.count, 0);
        compare(connectSpy.count, 0);
        details.close();
    }

    function test_detailsKeepTabAndArrowInputInsidePopup() {
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [Object.assign(host("Living room", "01"), { manual: true, discovered: false }), host("Bedroom", "02")]
        });
        connectSpy.target = screen;
        mouseClick(findChild(screen, "consoleActionsButton"));
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        const selected = screen.selectedIdentity;
        const close = findChild(details.contentItem, "closeDetailsButton");
        verify(close.activeFocus);
        keyClick(Qt.Key_Tab);
        verify(findChild(details.contentItem, "pinAction").activeFocus);
        keyClick(Qt.Key_Tab, Qt.ShiftModifier);
        verify(close.activeFocus);
        for (const key of [Qt.Key_Left, Qt.Key_Right]) keyClick(key);
        compare(screen.selectedIdentity, selected);
        verify(close.activeFocus);
        keyClick(Qt.Key_Return);
        tryCompare(details, "visible", false);
        compare(connectSpy.count, 0);
    }

    function test_keyboardFocusChangesRenderedControl_data() {
        return ["consolesButton", "settingsButton", "steamShortcutButton", "quitButton", "consoleSearch",
            "addConsoleButton", "refreshPsnButton", "connectButton", "consoleActionsButton", "discoveryButton"]
            .map(control => ({ tag: control, control, popup: false, changes: {} }))
            .concat(["closeDetailsButton", "pinAction", "wakeAction", "deleteAction", "hideAction"]
                .map(control => ({ tag: control, control, popup: true,
                    changes: control === "hideAction" ? { registered: false }
                        : { discovered: false, manual: true } })));
    }

    function test_keyboardFocusChangesRenderedControl(data) {
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [Object.assign(host("Living room", "01"), data.changes)], psnAvailable: true, steamAvailable: true
        });
        if (data.popup) {
            mouseClick(findChild(screen, "consoleActionsButton"));
            tryCompare(findChild(screen, "consoleDetails"), "opened", true);
        }
        const details = findChild(screen, "consoleDetails");
        const control = findChild(data.popup ? details.contentItem : screen, data.control);
        const sink = data.popup ? findChild(details.contentItem,
            data.control === "closeDetailsButton" ? "pinAction" : "closeDetailsButton")
            : createTemporaryObject(focusSinkComponent, testCase);
        sink.forceActiveFocus();
        waitForRendering(control);
        const unfocused = grabImage(screen);
        control.forceActiveFocus(Qt.TabFocusReason);
        waitForRendering(control);
        verify(control.activeFocus);
        const position = control.mapToItem(screen, 0, 0);
        function hasVisibleChange() {
            const focused = grabImage(screen);
            for (let y = Math.ceil(position.y); y < position.y + control.height; y += 3)
                for (let x = Math.ceil(position.x); x < position.x + control.width; x += 3)
                    if (focused.red(x, y) !== unfocused.red(x, y)
                        || focused.green(x, y) !== unfocused.green(x, y)
                        || focused.blue(x, y) !== unfocused.blue(x, y)) return true;
            return false;
        }
        tryVerify(hasVisibleChange, 1000,
            "Keyboard focus must change rendered pixels");
        const focused = grabImage(screen);
        if (data.control === "connectButton")
            verify(focused.red(Math.floor(position.x + 1), Math.floor(position.y + control.height / 2)) > 200,
                "The primary action has a bright focus border over its usual blue background");
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
            { tag: "delete", changes: { manual: true, discovered: false }, item: "deleteAction", action: "delete" },
            { tag: "hide", changes: { registered: false }, item: "hideAction", action: "hide" }
        ];
    }

    function test_managementAccess(data) {
        const target = Object.assign(host("Living room", "01"), data.changes);
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [target] });
        commandSpy.signalName = data.action === "wake" ? "wakeRequested" : "managementRequested";
        commandSpy.target = screen;
        connectSpy.target = screen;
        mouseClick(findChild(screen, "consoleActionsButton"));
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        const action = findChild(details.contentItem, data.item);
        verify(action.visible && action.enabled);
        if (data.keyboard) {
            action.forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Return);
        } else {
            mouseClick(action);
        }
        tryCompare(commandSpy, "count", 1);
        if (data.action === "wake") compare(commandSpy.signalArguments[0][0], 0);
        else {
            compare(commandSpy.signalArguments[0][0], data.action);
            verify(typeof commandSpy.signalArguments[0][1] === "function");
            commandSpy.signalArguments[0][2]();
        }
        compare(connectSpy.count, 0);
    }

    function test_escapeClosesDetailsAndRestoresOpener() {
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host("Living room", "01")] });
        commandSpy.signalName = "quitRequested";
        commandSpy.target = screen;
        const opener = findChild(screen, "consoleActionsButton");
        mouseClick(opener);
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
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
