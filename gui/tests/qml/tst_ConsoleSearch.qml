import QtQuick
import QtTest
import "../../src/qml"

TestCase {
    id: testCase
    name: "ConsoleSearch"
    width: 1280
    height: 720
    visible: true
    when: windowShown

    Component { id: screenComponent; ConsoleLibrary { width: 1280; height: 720 } }
    SignalSpy { id: connectSpy; signalName: "connectRequested" }
    SignalSpy { id: commandSpy }

    function host(name, mac, ps5) {
        return { display: true, discovered: true, registered: true, state: "ready",
            name, mac, ps5, address: "192.0.2.7" };
    }

    function searchFor(screen, text) {
        const search = findChild(screen, "consoleSearch");
        verify(search);
        search.text = text;
        return search;
    }

    function cleanup() {
        for (const spy of [connectSpy, commandSpy]) {
            spy.target = null;
            spy.clear();
        }
    }

    function descendantsNamed(item, name) {
        let items = item.objectName === name ? [item] : [];
        for (const child of item.children || [])
            items = items.concat(descendantsNamed(child, name));
        return items;
    }

    function consoleNames(item) {
        return descendantsNamed(item, "consoleName").map(label => label.text);
    }

    function test_searchFindsLaterSourceHost() {
        const hidden = Object.assign(host("Bedroom hidden", "00", false), { display: false });
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [hidden, host("Living room", "01", true), host("Bedroom", "02", false)]
        });
        searchFor(screen, "  bEDrOOm  ");
        compare(findChild(screen, "consoleName").text, "Bedroom");
        connectSpy.target = screen;
        mouseClick(findChild(screen, "connectButton"));
        compare(connectSpy.count, 1);
        compare(connectSpy.signalArguments[0][0], 2);
        compare(connectSpy.signalArguments[0][1], "Bedroom");
    }

    function test_nameAndKnownTypeSearch_data() {
        return [
            { tag: "spaces", query: "  lIVIng    ROOM  ", expected: ["Living room"] },
            { tag: "ps5", query: "ps5", expected: ["Living room"] },
            { tag: "ps4", query: "PS4", expected: ["Bedroom"] },
            { tag: "full-type", query: "  pLaySTation  5 ", expected: ["Living room"] },
            { tag: "name-and-type", query: "PS5 living", expected: ["Living room"] },
            { tag: "no-match", query: "garage", expected: [] },
            { tag: "space-only", query: "   ", expected: ["Living room", "Bedroom", "Studio"] }
        ];
    }

    function test_nameAndKnownTypeSearch(data) {
        const hidden = Object.assign(host("Hidden PS5", "00", true), { display: false });
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [hidden, host("Living room", "01", true), host("Bedroom", "02", false), host("Studio", "03", undefined)]
        });
        const search = searchFor(screen, data.query);
        compare(consoleNames(screen), data.expected);
        search.clear();
        compare(consoleNames(screen), ["Living room", "Bedroom", "Studio"]);
    }

    function test_streamerSearchUsesMaskedConsoleName() {
        const target = host("192.0.2.7", "01", undefined);
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [target], streamerMode: true });
        searchFor(screen, "manual");
        compare(consoleNames(screen), ["Manual console"]);
        searchFor(screen, "192.0.2.7");
        compare(consoleNames(screen), []);
    }

    function test_selectionFollowsFilterReorderAndRemoval() {
        const first = host("Living room", "01", true);
        const second = host("Bedroom", "02", false);
        const third = host("Studio", "03", true);
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [first, second, third] });
        screen.selectedIdentity = screen.identityOf(second);
        const savedIdentity = screen.selectedIdentity;
        screen.hosts = [third, second, first];
        compare(screen.selectedIdentity, savedIdentity);
        compare(screen.selectedTarget.sourceIndex, 1);
        searchFor(screen, "bedroom");
        compare(screen.selectedIdentity, savedIdentity);
        searchFor(screen, "studio");
        compare(screen.selectedIdentity, screen.identityOf(third));
        searchFor(screen, "");
        compare(screen.selectedIdentity, screen.identityOf(third));
        screen.hosts = [second, first];
        compare(screen.selectedIdentity, screen.identityOf(second));
        connectSpy.target = screen;
        screen.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(connectSpy.count, 1);
        compare(connectSpy.signalArguments[0][0], 0);
        compare(connectSpy.signalArguments[0][1], "Bedroom");
    }

    function test_noMatchesClearsSelectionAndKeyboardCannotConnect() {
        const first = host("Living room", "01", true);
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [first] });
        searchFor(screen, "garage");
        compare(screen.selectedIdentity, "");
        compare(screen.selectedTarget, null);
        screen.forceActiveFocus(Qt.TabFocusReason);
        connectSpy.target = screen;
        for (const key of [Qt.Key_Left, Qt.Key_Right, Qt.Key_Up, Qt.Key_Down, Qt.Key_Return])
            keyClick(key);
        compare(connectSpy.count, 0);
    }

    function test_selectionAndFocusSkipAmbiguousOrMissingIdentities() {
        const ambiguous = host("Duplicate", "01", true);
        const valid = host("Studio", "03", true);
        const missing = { display: true, name: "Unknown", registered: true, discovered: true };
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [ambiguous, missing, Object.assign({}, ambiguous), valid]
        });
        compare(screen.selectedIdentity, screen.identityOf(valid));
        screen.moveSelection(-1);
        compare(screen.selectedIdentity, screen.identityOf(valid));
        screen.restoreCardFocus(screen.identityOf(ambiguous));
        const focus = screen.Window.window.activeFocusItem;
        verify(focus.Accessible.name.indexOf("Studio") >= 0);
    }

    function test_countsFollowVisibleFilteredAndLiveHosts() {
        const hidden = Object.assign(host("Hidden", "00", true), { display: false });
        const first = host("Living room", "01", true);
        const second = host("Bedroom", "02", false);
        const third = host("Studio", "03", undefined);
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [hidden, first, second, third] });
        const count = findChild(screen, "consoleCount");
        verify(count && count.visible);
        compare(count.text, "3 consoles");
        const search = searchFor(screen, "ps5");
        compare(count.text, "1 of 3 consoles");
        screen.hosts = [third, second, first, hidden];
        compare(count.text, "1 of 3 consoles");
        screen.hosts = [third, second, hidden];
        compare(count.text, "0 of 2 consoles");
        search.clear();
        compare(count.text, "2 consoles");
        screen.hosts = [second, hidden];
        compare(count.text, "1 console");
        search.text = "bed";
        compare(count.text, "1 of 1 console");
        search.clear();
        screen.hosts = [hidden];
        compare(count.text, "0 consoles");
    }

    function test_emptyState_data() {
        const hidden = Object.assign(host("Hidden", "00", true), { display: false });
        return [
            { tag: "empty", hosts: [], discovery: true, query: "", title: "No consoles found", action: "Add console", signal: "addConsoleRequested" },
            { tag: "all-hidden", hosts: [hidden], discovery: true, query: "", title: "No consoles found", action: "Add console", signal: "addConsoleRequested" },
            { tag: "discovery-off", hosts: [], discovery: false, query: "", title: "Discovery is off", action: "Enable discovery", signal: "discoveryToggled" },
            { tag: "no-matches", hosts: [host("Living room", "01", true)], discovery: true, query: "garage", title: "No matching consoles", action: "Clear search", signal: "" },
            { tag: "no-matches-discovery-off", hosts: [host("Living room", "01", true)], discovery: false, query: "garage", title: "No matching consoles", action: "Clear search", signal: "" }
        ];
    }

    function test_emptyState(data) {
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: data.hosts, discoveryEnabled: data.discovery
        });
        const search = searchFor(screen, data.query);
        const title = findChild(screen, "emptyStateTitle");
        const action = findChild(screen, "emptyStateAction");
        verify(title && title.visible);
        verify(action && action.visible && action.enabled);
        compare(title.text, data.title);
        compare(action.text, data.action);
        connectSpy.target = screen;
        if (data.signal) {
            commandSpy.signalName = data.signal;
            commandSpy.target = screen;
        }
        mouseClick(action);
        compare(connectSpy.count, 0);
        if (data.signal) {
            compare(commandSpy.count, 1);
            if (data.signal === "discoveryToggled")
                compare(commandSpy.signalArguments[0][0], true);
        } else {
            compare(search.text, "");
            verify(search.activeFocus);
            verify(!title.visible);
        }
    }

    function test_searchRemainsEditableWithoutConsoleActions() {
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [host("Living room", "01", true), host("Bedroom", "02", false)]
        });
        const search = searchFor(screen, "");
        search.forceActiveFocus(Qt.TabFocusReason);
        connectSpy.target = screen;
        commandSpy.signalName = "managementRequested";
        commandSpy.target = screen;
        for (const letter of "room") keyClick(letter);
        compare(search.text, "room");
        const selected = screen.selectedIdentity;
        keyClick(Qt.Key_Home);
        compare(search.cursorPosition, 0);
        keyClick(Qt.Key_Right);
        compare(search.cursorPosition, 1);
        keyClick(Qt.Key_End);
        keyClick(Qt.Key_Left, Qt.ShiftModifier);
        compare(search.selectedText, "m");
        keyClick(Qt.Key_Delete);
        compare(search.text, "roo");
        keyClick("m");
        for (const key of [Qt.Key_Up, Qt.Key_Down, Qt.Key_PageUp, Qt.Key_PageDown, Qt.Key_Return, Qt.Key_Enter])
            keyClick(key);
        compare(connectSpy.count, 0);
        compare(commandSpy.count, 0);
        compare(screen.selectedIdentity, selected);
        verify(search.activeFocus && !search.readOnly);
        keyClick(Qt.Key_Backspace);
        compare(search.text, "roo");
    }

    function test_escapeClearsSearchBeforeQuit() {
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host("Bedroom", "02", false)] });
        const search = searchFor(screen, "bed");
        search.forceActiveFocus(Qt.TabFocusReason);
        commandSpy.signalName = "quitRequested";
        commandSpy.target = screen;
        keyClick(Qt.Key_Escape);
        compare(search.text, "");
        verify(search.activeFocus);
        compare(commandSpy.count, 0);
        keyClick(Qt.Key_Escape);
        compare(commandSpy.count, 1);
    }

    function test_liveUpdatesPreserveSearchFocusAndCursor() {
        const first = host("Living room", "01", true);
        const second = host("Bedroom", "02", false);
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [first, second] });
        const search = searchFor(screen, "room");
        search.forceActiveFocus(Qt.TabFocusReason);
        search.cursorPosition = 2;
        screen.hosts = [second, first, host("Studio", "03", undefined)];
        compare(search.text, "room");
        compare(search.cursorPosition, 2);
        verify(search.activeFocus && !search.readOnly);
    }

    function test_filteredCardsKeepNormalWidthAndLeftPacking_data() {
        return [{ tag: "desktop", width: 1280, height: 720 }, { tag: "narrow", width: 800, height: 600 }];
    }

    function test_filteredCardsKeepNormalWidthAndLeftPacking(data) {
        const screen = createTemporaryObject(screenComponent, testCase, {
            width: data.width, height: data.height,
            hosts: [host("Living room", "01", true), host("Bedroom", "02", false), host("Studio", "03", true)]
        });
        waitForRendering(screen);
        const normalWidth = findChild(screen, "consoleCard").width;
        const search = searchFor(screen, "ps5");
        waitForRendering(screen);
        const cards = descendantsNamed(screen, "consoleCard");
        compare(cards.length, 2);
        fuzzyCompare(cards[0].width, normalWidth, 1);
        fuzzyCompare(cards[1].width, normalWidth, 1);
        fuzzyCompare(cards[1].x - cards[0].x - cards[0].width, 18, 1);
        search.text = "studio";
        waitForRendering(screen);
        const card = findChild(screen, "consoleCard");
        const scroll = findChild(screen, "libraryScroll");
        fuzzyCompare(card.width, normalWidth, 1);
        fuzzyCompare(card.mapToItem(scroll, 0, 0).x, 0, 1);
    }

    function test_tabLeavesSearchAndShiftTabReturns() {
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [host("Bedroom", "02", false)], psnAvailable: true
        });
        const search = searchFor(screen, "bed");
        search.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Tab);
        verify(findChild(screen, "refreshPsnButton").activeFocus);
        keyClick(Qt.Key_Tab, Qt.ShiftModifier);
        verify(search.activeFocus && !search.readOnly);
    }

    function test_profileAndAllHiddenUpdatesClearStaleSelection() {
        const target = host("Bedroom", "02", false);
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [target], profileKey: "first" });
        searchFor(screen, "room");
        const previous = screen.selectedIdentity;
        screen.profileKey = "second";
        verify(screen.selectedIdentity !== previous);
        compare(screen.resolveTarget(previous), null);
        screen.hosts = [Object.assign({}, target, { display: false })];
        compare(screen.selectedIdentity, "");
        compare(screen.selectedTarget, null);
        screen.forceActiveFocus(Qt.TabFocusReason);
        connectSpy.target = screen;
        for (const key of [Qt.Key_Left, Qt.Key_Right, Qt.Key_Up, Qt.Key_Down, Qt.Key_Return])
            keyClick(key);
        compare(connectSpy.count, 0);
        screen.hosts = [Object.assign({}, target)];
        compare(screen.selectedIdentity, screen.identityOf(target));
        verify(!screen.connectToTarget(previous));
        keyClick(Qt.Key_Return);
        compare(connectSpy.count, 1);
        compare(connectSpy.signalArguments[0][1], "Bedroom");
    }

    function test_searchAndEmptyStateFitNarrowScreen() {
        const screen = createTemporaryObject(screenComponent, testCase, {
            width: 800, height: 600, hosts: [host("Bedroom", "02", false)], psnAvailable: true
        });
        searchFor(screen, "garage");
        waitForRendering(screen);
        for (const name of ["consoleSearch", "consoleCount", "emptyStateTitle", "emptyStateAction"]) {
            const item = findChild(screen, name);
            const position = item.mapToItem(screen, 0, 0);
            verify(item.visible && item.width > 0 && item.height > 0, name);
            verify(position.x >= 0 && position.y >= 0, name);
            verify(position.x + item.width <= screen.width, name);
            verify(position.y + item.height <= screen.height, name);
        }
    }
}
