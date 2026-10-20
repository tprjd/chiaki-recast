import QtQuick
import QtTest
import "../../src/qml"

TestCase {
    id: testCase
    name: "ConsoleAppearance"
    width: 1280
    height: 720
    visible: true
    when: windowShown

    Component { id: screenComponent; ConsoleLibrary { width: 1280; height: 720 } }
    SignalSpy { id: connectSpy; signalName: "connectRequested" }

    function host(name, mac) {
        return { display: true, discovered: true, registered: true, state: "ready",
            name, mac, ps5: true, address: "192.0.2.7" };
    }

    function cleanup() { connectSpy.target = null; connectSpy.clear(); }

    function test_selectedBorderHasSameThicknessAboveAndBelowArtwork() {
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host("Living room", "01")] });
        waitForRendering(screen);
        const card = findChild(screen, "consoleCard");
        const position = card.mapToItem(screen, 0, 0);
        const image = grabImage(screen);
        function blueWidth(y) {
            let pixels = 0;
            for (let x = Math.ceil(position.x); x < position.x + 6; x++)
                if (image.red(x, y) > 70 && image.green(x, y) > 140 && image.blue(x, y) > 210) pixels++;
            return pixels;
        }
        const artBorder = blueWidth(Math.round(position.y + 90));
        const bodyBorder = blueWidth(Math.round(position.y + 240));
        verify(bodyBorder >= 2, "Selected card has a visible border");
        compare(artBorder, bodyBorder, "Artwork must not cover the selected border");
    }

    function test_secondaryButtonsHaveVisibleOutlines_data() {
        return ["settingsButton", "steamShortcutButton", "quitButton", "refreshPsnButton",
            "discoveryButton", "consoleActionsButton", "pinShortcutButton"].map(control => ({ tag: control, control }));
    }

    function test_secondaryButtonsHaveVisibleOutlines(data) {
        const screen = createTemporaryObject(screenComponent, testCase, {
            hosts: [host("Living room", "01")], psnAvailable: true, steamAvailable: true
        });
        const button = findChild(screen, data.control);
        verify(button && button.visible && button.enabled, data.control);
        screen.forceActiveFocus();
        waitForRendering(screen);
        const image = grabImage(screen);
        const position = button.mapToItem(screen, 0, 0);
        const y = Math.floor(position.y + button.height / 2);
        let outline = false;
        for (let x = Math.ceil(position.x); x < position.x + 4; x++)
            if (image.red(x, y) > 50 && image.green(x, y) > 60 && image.blue(x, y) > 70) outline = true;
        verify(outline, data.control + " must look clickable without focus or hover");
    }

    function test_detailsPrimaryActionResolvesCurrentTargetAfterClosing() {
        const first = host("Living room", "01");
        const second = host("Bedroom", "02");
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [first, second] });
        mouseClick(findChild(screen, "consoleActionsButton"));
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        screen.hosts = [second, Object.assign({}, first)];
        connectSpy.target = screen;
        screen.connectRequested.connect(() => verify(!details.visible, "Close details before connecting"));
        const connect = findChild(details.contentItem, "detailsConnectButton");
        verify(connect && connect.visible && connect.enabled, "Prototype primary action is available in details");
        mouseClick(connect);
        tryCompare(connectSpy, "count", 1);
        compare(connectSpy.signalArguments[0][0], 1);
        compare(connectSpy.signalArguments[0][1], "Living room");
    }

    function test_detailsDimLibraryAndBlockBackgroundCommands() {
        const screen = createTemporaryObject(screenComponent, testCase, { hosts: [host("Living room", "01")] });
        const settings = findChild(screen, "settingsButton");
        const position = settings.mapToItem(screen, 0, 0);
        waitForRendering(screen);
        const before = grabImage(screen);
        mouseClick(findChild(screen, "consoleActionsButton"));
        const details = findChild(screen, "consoleDetails");
        tryCompare(details, "opened", true);
        const backdrop = findChild(screen, "detailsBackdrop");
        verify(backdrop.visible && backdrop.width === screen.width && backdrop.height === screen.height);
        waitForRendering(screen);
        const after = grabImage(screen);
        let dimmed = false;
        for (let y = Math.ceil(position.y); y < position.y + settings.height; y++)
            for (let x = Math.ceil(position.x); x < position.x + settings.width; x++)
                if (before.blue(x, y) > 150 && after.blue(x, y) < before.blue(x, y) / 2) dimmed = true;
        verify(dimmed, "The library is visibly dimmed behind details");
        let settingsOpened = false;
        screen.settingsRequested.connect(() => { settingsOpened = true; });
        mouseClick(settings);
        verify(!settingsOpened, "A modal blocks commands behind it");
        details.close();
        tryCompare(backdrop, "visible", false);
    }
}
