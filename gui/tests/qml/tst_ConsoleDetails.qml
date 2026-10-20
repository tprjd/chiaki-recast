import QtQuick
import QtQuick.Controls
import QtTest
import "../../src/qml"

TestCase {
    id: testCase
    name: "ConsoleDetails"
    width: 1280
    height: 720
    visible: true
    when: windowShown

    Component { id: screenComponent; ConsoleLibrary { width: 1280; height: 720 } }
    SignalSpy { id: connectSpy; signalName: "connectRequested" }
    SignalSpy { id: quitSpy; signalName: "quitRequested" }

    function host() {
        return { display: true, discovered: true, registered: true, state: "ready",
            name: "Living room", address: "192.0.2.7", mac: "00:11:22:33:44:55",
            ps5: true, app: "Sample game", titleId: "PPSA00001" };
    }

    function cleanup() {
        connectSpy.target = null;
        connectSpy.clear();
        quitSpy.target = null;
        quitSpy.clear();
    }

    function displayedText(item) {
        let result = "";
        if (item.visible && item.text !== undefined) result += item.text + "\n";
        for (const child of item.children || []) result += displayedText(child);
        return result;
    }

    function accessibleAndTooltipText(item) {
        let result = "";
        if (item.Accessible) result += item.Accessible.name + "\n" + item.Accessible.description + "\n";
        if (item.ToolTip) result += item.ToolTip.text + "\n";
        for (const child of item.children || []) result += accessibleAndTooltipText(child);
        for (const child of item.data || [])
            if (child.contentItem && child.text !== undefined)
                result += child.text + "\n" + accessibleAndTooltipText(child.contentItem);
        return result;
    }

    function test_detailsShowAvailableFieldsWithoutConnecting() {
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host()] });
        connectSpy.target = screen;
        const opener = findChild(screen, "consoleActionsButton");
        verify(opener && opener.visible && opener.enabled);
        mouseClick(opener);
        const details = findChild(screen, "consoleDetails");
        verify(details);
        tryCompare(details, "opened", true);
        const text = displayedText(details.contentItem);
        for (const value of ["Living room", "Address: 192.0.2.7", "ID: 00:11:22:33:44:55",
                            "Registration: Registered", "Route: Local network",
                            "App: Sample game", "Title ID: PPSA00001"])
            verify(text.indexOf(value) >= 0, value);
        compare(connectSpy.count, 0);
        details.close();
    }

    function test_hostMarkupRemainsLiteralText() {
        const name = '<a href="test:console">Room</a>';
        const app = '<a href="test:app">Game</a>';
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [Object.assign(host(), { name, app })]
        });
        const nameLabel = findChild(screen, "consoleName");
        function verifyLiteralLabel(label, expected) {
            verify(label);
            compare(label.text, expected);
            compare(label.Accessible.name, expected);
            waitForRendering(label);
            let link = "";
            for (let y = 1; y < label.height && !link; y += 4)
                for (let x = 1; x < label.width && !link; x += 4)
                    link = label.linkAt(x, y);
            compare(link, "", "Host text must not create a rendered hyperlink");
            compare(label.textFormat, Text.PlainText);
        }
        function labelWithText(item, text) {
            if (item.text === text && typeof item.linkAt === "function") return item;
            for (const child of item.children || []) {
                const label = labelWithText(child, text);
                if (label) return label;
            }
            return null;
        }
        verifyLiteralLabel(nameLabel, name);
        mouseMove(nameLabel, 5, 5);
        const nameTip = nameLabel.data.find(item => item.objectName === "consoleNameToolTip");
        tryCompare(nameTip, "opened", true);
        verifyLiteralLabel(nameTip.contentItem, name);
        const opener = findChild(screen, "consoleActionsButton");
        opener.hoverEnabled = true;
        mouseMove(opener, opener.width / 2, opener.height / 2);
        const detailsTip = opener.data.find(item => item.objectName === "consoleDetailsToolTip");
        tryCompare(detailsTip, "opened", true);
        verifyLiteralLabel(detailsTip.contentItem, "Details for " + name);
        mouseClick(opener);
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        verifyLiteralLabel(labelWithText(details.contentItem, name), name);
        verifyLiteralLabel(labelWithText(details.contentItem, "App: " + app), "App: " + app);
        details.close();
    }

    function test_streamerModeMasksEveryPrivateIdentifier_data() {
        return [
            { tag: "local", changes: {} },
            { tag: "manual-address-name", changes: { discovered: false, manual: true, name: "192.0.2.7" } },
            { tag: "missing-name", changes: { discovered: false, manual: true, name: "" } },
            { tag: "psn", changes: { discovered: false, address: "", mac: "", duid: "private-psn-id" } }
        ];
    }

    function test_streamerModeMasksEveryPrivateIdentifier(data) {
        const target = Object.assign(host(), data.changes);
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [target] });
        mouseClick(findChild(screen, "consoleActionsButton"));
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        screen.streamerMode = true;
        wait(0);
        const text = displayedText(screen) + displayedText(details.contentItem)
            + accessibleAndTooltipText(screen) + accessibleAndTooltipText(details.contentItem);
        for (const value of ["192.0.2.7", "00:11:22:33:44:55", "private-psn-id"])
            verify(text.indexOf(value) < 0, "Streamer mode exposed " + value);
        verify(displayedText(details.contentItem).indexOf("ID: hidden") >= 0);
        screen.streamerMode = false;
        wait(0);
        const expected = target.duid || target.mac;
        verify(displayedText(details.contentItem).indexOf("ID: " + expected) >= 0);
        details.close();
    }

    function test_detailsOmitUnavailableFields() {
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [{ display: true, manual: true, address: "192.0.2.9" }]
        });
        mouseClick(findChild(screen, "consoleActionsButton"));
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        const text = displayedText(details.contentItem);
        verify(text.indexOf("192.0.2.9") >= 0);
        verify(text.indexOf("Route: Manual address") >= 0);
        for (const missing of ["ID:", "Registration:", "App:", "Title ID:", "undefined", "null"])
            verify(text.indexOf(missing) < 0, missing);
        details.close();
    }

    function test_liveDetailsFollowIdentityAndCloseWhenTargetIsLost_data() {
        return [{ tag: "removed", change: "removed" }, { tag: "ambiguous", change: "ambiguous" },
            { tag: "hidden", change: "hidden" }, { tag: "profile", change: "profile" }];
    }

    function test_liveDetailsFollowIdentityAndCloseWhenTargetIsLost(data) {
        const target = host();
        const other = Object.assign(host(), { name: "Bedroom", mac: "00:11:22:33:44:66" });
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [target, other] });
        connectSpy.target = screen;
        mouseClick(findChild(screen, "consoleActionsButton"));
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        const renamed = Object.assign({}, target, { name: "Studio", address: "192.0.2.18", app: "Another game" });
        screen.hosts = [other, renamed];
        wait(0);
        verify(details.opened);
        const text = displayedText(details.contentItem);
        for (const value of ["Studio", "192.0.2.18", "Another game"])
            verify(text.indexOf(value) >= 0, value);
        verify(text.indexOf("Bedroom") < 0);
        if (data.change === "removed") screen.hosts = [other];
        if (data.change === "ambiguous") screen.hosts = [renamed, Object.assign({}, renamed), other];
        if (data.change === "hidden") screen.hosts = [Object.assign({}, renamed, { display: false }), other];
        if (data.change === "profile") screen.profileKey = "Other profile";
        tryCompare(details, "opened", false);
        compare(connectSpy.count, 0);
    }

    function test_keyboardOpenAndCloseRestoresFocusWithoutConnecting() {
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host()] });
        connectSpy.target = screen;
        quitSpy.target = screen;
        const opener = findChild(screen, "consoleActionsButton");
        opener.forceActiveFocus(Qt.TabFocusReason);
        tryCompare(opener, "visualFocus", true);
        keyClick(Qt.Key_Return);
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        const close = findChild(screen, "closeDetailsButton");
        tryCompare(close, "activeFocus", true);
        keyClick(Qt.Key_Escape);
        tryCompare(details, "opened", false);
        tryCompare(opener, "activeFocus", true);
        tryCompare(opener, "visualFocus", true);
        keyClick(Qt.Key_Return);
        tryCompare(details, "opened", true);
        tryCompare(close, "activeFocus", true);
        keyClick(Qt.Key_Return);
        tryCompare(details, "opened", false);
        tryCompare(opener, "activeFocus", true);
        compare(connectSpy.count, 0);
        compare(quitSpy.count, 0);
    }

    function test_removalRestoresFocusToRemainingControl_data() {
        return [{ tag: "remaining-card", remaining: true }, { tag: "empty-library", remaining: false }];
    }

    function test_removalRestoresFocusToRemainingControl(data) {
        const other = Object.assign(host(), { name: "Bedroom", mac: "00:11:22:33:44:66" });
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host(), other] });
        mouseClick(findChild(screen, "consoleActionsButton"));
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        screen.hosts = data.remaining ? [other] : [];
        tryCompare(details, "opened", false);
        const nextControl = findChild(screen, data.remaining ? "connectButton" : "addConsoleButton");
        tryCompare(nextControl, "activeFocus", true);
        verify(nextControl.enabled && nextControl.visible);
    }

    function test_longDetailsRemainInsideNarrowWindow() {
        const target = Object.assign(host(), {
            name: "Living room console with a long name ".repeat(8),
            app: "Sample game with a long title ".repeat(8)
        });
        const screen = createTemporaryObject(screenComponent, testCase, { width: 800, height: 600, hosts: [target] });
        mouseClick(findChild(screen, "consoleActionsButton"));
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        waitForRendering(details.contentItem);
        const position = details.contentItem.mapToItem(testCase, 0, 0);
        verify(position.x >= 0 && position.y >= 0, "Details start inside the window");
        verify(details.height <= screen.height, "Details fit vertically in the narrow window");
        const close = findChild(screen, "closeDetailsButton");
        const closePosition = close.mapToItem(testCase, 0, 0);
        verify(closePosition.y + close.height <= screen.height, "Close remains reachable");
        const scroll = findChild(screen, "detailsScroll");
        const content = scroll.contentItem;
        verify(content.contentHeight > content.height, "Long details have a scrollable information area");
        mouseWheel(scroll, scroll.width / 2, scroll.height / 2, 0, -1200);
        tryVerify(() => content.contentY > 0, 1000, "The user can scroll through long details");
        details.close();
    }
}
