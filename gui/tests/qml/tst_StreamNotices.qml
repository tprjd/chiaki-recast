import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtTest
import "fixtures"

TestCase {
    id: tests
    name: "StreamNotices"
    width: 1280; height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; StreamHarness { anchors.fill: parent } }
    function routes() { return [{tag: "inline", separate: false}, {tag: "window", separate: true}]; }
    function passiveHost(h, separate) { return findChild(h.screen, separate ? "streamPassiveNoticesWindow" : "streamPassiveNoticesInline"); }
    function test_networkThresholdAndAudio_data() { return routes(); }
    function test_networkThresholdAndAudio(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        h.nativeHost = data.separate;
        h.window.runtimeRendererBackend = data.separate ? 1 : 0;
        wait(20);
        (data.separate ? h.window : tests.Window.window).requestActivate();
        wait(20);
        h.opener.forceActiveFocus(Qt.TabFocusReason);
        const host = passiveHost(h, data.separate);
        verify(host);
        h.backend.session.averagePacketLoss = 0.029;
        compare(host.visible, false);
        h.backend.session.averagePacketLoss = 0.03;
        compare(host.visible, false);
        h.backend.session.averagePacketLoss = 0.032;
        tryCompare(host, "visible", true);
        const loss = findChild(host, "streamNetworkNotice");
        verify(loss);
        compare(loss.title, "Unstable connection");
        compare(loss.message, "Packet loss 3.2%");
        verify(loss.width < 250, "Compact warning");
        h.settings.audioVideoDisabled = 1;
        const audio = findChild(host, "streamAudioNotice");
        tryCompare(audio, "visible", true);
        compare(audio.title, "Audio disabled");
        verify(audio.message.indexOf("Settings → General → Audio/Video") >= 0);
        tryVerify(() => audio.y >= loss.y + loss.height);
        compare(h.window.grabs, 0);
        compare(h.opener.focus, true);
        compare(loss.activeFocus, false);
        compare(audio.activeFocus, false);
        h.backend.session.averagePacketLoss = 0;
        tryCompare(loss, "visible", false);
        h.settings.audioVideoDisabled = 0;
        wait(30);
        tryCompare(host, "visible", false);
        compare(h.backend.commands.length, 0);
        h.settings.wifiDroppedNotif = 5;
        h.backend.session.averagePacketLoss = 0.05;
        compare(host.visible, false);
        h.backend.session.averagePacketLoss = 0.051;
        tryCompare(host, "visible", true);
        h.backend.session.averagePacketLoss = 0;
        tryCompare(host, "visible", false);
        h.backend.session.averagePacketLoss = Infinity;
        compare(host.visible, false);
        h.backend.session.averagePacketLoss = NaN;
        compare(host.visible, false);
    }
    function test_blockedActionAndClearance_data() { return routes(); }
    function test_blockedActionAndClearance(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        h.nativeHost = data.separate;
        h.window.runtimeRendererBackend = data.separate ? 1 : 0;
        wait(20);
        (data.separate ? h.window : tests.Window.window).requestActivate();
        wait(20);
        h.opener.forceActiveFocus(Qt.TabFocusReason);
        h.backend.session.cantDisplay = true;
        const host = findChild(h.screen, data.separate ? "streamActionNoticeWindow" : "streamActionNoticeInline");
        verify(host);
        tryCompare(host, "visible", true);
        const notice = findChild(host, "streamActionNotice");
        compare(notice.title, "Content blocked by the console");
        compare(notice.message, "This screen cannot be shown through Remote Play.");
        const button = findChild(notice, "streamNoticeAction");
        tryCompare(button, "activeFocus", true);
        compare(button.text, "Go to console home");
        wait(30);
        compare(button.activeFocus, true);
        const area = data.separate ? host.contentItem : h.screen;
        const focused = grabImage(area);
        const edge = button.mapToItem(area, 1, Math.floor(button.height / 2));
        verify(focused.red(Math.round(edge.x), Math.round(edge.y)) > 200
            && focused.green(Math.round(edge.x), Math.round(edge.y)) > 200, "Visible focus outline");
        keyClick(Qt.Key_Return);
        compare(h.backend.commands.length, 1);
        compare(h.backend.commands[0].name, "home");
        keyClick(Qt.Key_Escape);
        compare(h.backend.commands.length, 2);
        compare(h.window.grabs, 1);
        h.backend.session.cantDisplay = false;
        tryCompare(host, "visible", false);
        compare(h.window.releases, 1);
        tryCompare(h.opener, "activeFocus", true);
        button.clicked();
        compare(h.backend.commands.length, 2);
    }
    function test_disabledVideoMenuAndPriority_data() {
        const rows = [];
        for (const separate of [false, true])
            for (const flags of [2, 3]) rows.push({tag: (separate ? "window-" : "inline-") + flags, separate, flags});
        return rows;
    }
    function test_disabledVideoMenuAndPriority(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        h.nativeHost = data.separate;
        h.window.runtimeRendererBackend = data.separate ? 1 : 0;
        wait(30);
        (data.separate ? h.window : tests.Window.window).requestActivate();
        wait(20);
        h.opener.forceActiveFocus(Qt.TabFocusReason);
        compare(h.opener.activeFocus, true);
        h.settings.audioVideoDisabled = data.flags;
        const host = findChild(h.screen, data.separate ? "streamActionNoticeWindow" : "streamActionNoticeInline");
        tryCompare(host, "visible", true);
        const notice = findChild(host, "streamActionNotice");
        compare(notice.title, data.flags === 3 ? "Audio and video disabled" : "Video disabled");
        verify(notice.message.indexOf("Still connected.") === 0);
        verify(notice.message.indexOf("Settings → General → Audio/Video") >= 0);
        const action = findChild(notice, "streamNoticeAction");
        compare(action.text, "Open stream menu");
        wait(30);
        keyClick(Qt.Key_Return);
        tryCompare(host, "visible", false);
        const menu = findChild(h.screen, data.separate ? "streamMenuWindow" : "streamMenuInline");
        tryCompare(menu, "visible", true);
        compare(h.window.grabs - h.window.releases, 1);
        wait(300);
        keyClick(Qt.Key_Escape);
        tryCompare(menu, "visible", false);
        tryCompare(host, "visible", true);
        tryCompare(action, "activeFocus", true);
        compare(h.window.grabs - h.window.releases, 1);
        h.backend.sessionPinRequest = {id: "pin"};
        tryCompare(host, "visible", false);
        compare(h.window.grabs, h.window.releases);
        action.clicked();
        compare(menu.visible, false);
        h.backend.sessionPinRequest = {id: ""};
        tryCompare(host, "visible", true);
        h.settings.audioVideoDisabled = 0;
        wait(30);
        tryCompare(host, "visible", false);
        tryCompare(h.opener, "activeFocus", true);
        compare(h.window.grabs, h.window.releases);
        compare(h.backend.commands.length, 0);
    }
    function test_warningTimingAndSessionReplacement_data() { return routes(); }
    function test_warningTimingAndSessionReplacement(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        h.nativeHost = data.separate;
        h.window.runtimeRendererBackend = data.separate ? 1 : 0;
        const passive = passiveHost(h, data.separate);
        h.backend.session.averagePacketLoss = 0.04;
        tryCompare(passive, "visible", true);
        wait(100);
        compare(passive.visible, true);
        wait(800);
        compare(passive.visible, false);
        h.backend.session.averagePacketLoss = 0.05;
        tryCompare(passive, "visible", true);
        const oldSession = h.backend.session;
        const replacement = createTemporaryQmlObject('import QtQuick; QtObject { property bool connected: true; property bool muted: false; property bool cantDisplay: false; property string host: "Replacement"; property real measuredBitrate: 0; property real averagePacketLoss: 0; property int framesLost: 0 }', h);
        h.backend.session = replacement;
        compare(passive.visible, false);
        oldSession.cantDisplay = true;
        const actionHost = findChild(h.screen, data.separate ? "streamActionNoticeWindow" : "streamActionNoticeInline");
        compare(actionHost.visible, false);
        replacement.cantDisplay = true;
        tryCompare(actionHost, "visible", true);
        const button = findChild(actionHost, "streamNoticeAction");
        h.backend.session = null;
        tryCompare(actionHost, "visible", false);
        compare(passive.visible, false);
        button.clicked();
        compare(h.backend.commands.length, 0);
        compare(h.window.grabs, h.window.releases);
        replacement.destroy();
        h.screenActive = false;
        compare(h.window.grabs, h.window.releases);
    }
    function test_layout_data() {
        const rows = [];
        for (const separate of [false, true])
            for (const size of [[900, 600], [1280, 720], [1920, 1080]])
                for (const background of ["dark", "bright", "detail"])
                    rows.push({tag: (separate ? "window-" : "inline-") + size[0] + "-" + background,
                        separate, width: size[0], height: size[1], background});
        return rows;
    }
    function test_layout(data) {
        tests.width = data.width; tests.height = data.height;
        const h = createTemporaryObject(harnessComponent, tests);
        h.nativeHost = data.separate;
        h.window.runtimeRendererBackend = data.separate ? 1 : 0;
        h.settings.showStreamStats = false;
        h.window.x = 115; h.window.y = 77;
        wait(20);
        (data.separate ? h.window : tests.Window.window).requestActivate();
        wait(20);
        h.opener.forceActiveFocus(Qt.TabFocusReason);
        const backdrop = createTemporaryQmlObject('import QtQuick; Rectangle { anchors.fill: parent; z: -1; property bool detail: false; color: "#080b11"; Repeater { model: parent.detail ? 48 : 0; Rectangle { required property int index; width: parent ? parent.width : 0; height: 12; y: index * 23; color: index % 2 ? "#718a98" : "#d8d1ae" } } }', h.screen);
        backdrop.color = data.background === "bright" ? "#f7f5ef" : "#080b11";
        backdrop.detail = data.background === "detail";
        h.backend.session.cantDisplay = true;
        h.backend.session.averagePacketLoss = 0.05;
        h.settings.audioVideoDisabled = 1;
        const host = findChild(h.screen, data.separate ? "streamActionNoticeWindow" : "streamActionNoticeInline");
        const notice = findChild(host, "streamActionNotice");
        const passive = passiveHost(h, data.separate);
        tryCompare(host, "visible", true);
        tryCompare(passive, "visible", true);
        wait(380);
        const image = grabImage(data.separate ? host.contentItem : h.screen);
        image.save("build/notice-" + data.tag + ".png");
        findChild(notice, "streamNoticeTitle").text = "Content blocked by the console. ".repeat(4);
        findChild(notice, "streamNoticeMessage").text = "This screen cannot be shown through Remote Play. ".repeat(6);
        const action = findChild(notice, "streamNoticeAction");
        action.text = "Go to console home and return to supported content. ".repeat(3);
        wait(80);
        compare(action.contentItem.truncated, false);
        verify(action.contentItem.paintedWidth <= action.contentItem.width + 1);
        verify(action.contentItem.paintedHeight <= action.contentItem.height + 1);
        verify(action.height >= action.contentItem.implicitHeight);
        verify(host.height < data.height - 110);
        verify(host.width <= data.width - 36);
        if (data.separate) {
            compare(host.transientParent, h.window);
            compare(passive.transientParent, h.window);
            compare(host.x, 115 + Math.round((data.width - host.width) / 2));
            compare(host.y + host.height, 77 + data.height - 72);
            verify(passive.flags & Qt.WindowTransparentForInput);
            verify(passive.flags & Qt.WindowDoesNotAcceptFocus);
            h.window.x += 91; h.window.y += 43;
            tryVerify(() => host.y + host.height === 120 + data.height - 72);
            tryVerify(() => passive.x + passive.width === 206 + data.width - 18);
        } else {
            compare(host.y + host.height, data.height - 72);
            compare(passive.x + passive.width, data.width - 18);
        }
        grabImage(data.separate ? host.contentItem : h.screen).save("build/notice-" + data.tag + "-long.png");
        h.backend.session.cantDisplay = false;
        h.backend.session.averagePacketLoss = 0.06;
        const network = findChild(passive, "streamNetworkNotice");
        const audio = findChild(passive, "streamAudioNotice");
        network.title = "Unstable connection. ".repeat(3);
        network.message = "Packet loss 6.0% measured during this session. ".repeat(3);
        audio.title = "Audio disabled. ".repeat(3);
        audio.message = "Settings → General → Audio/Video. ".repeat(3);
        wait(80);
        verify(passive.height < data.height - 36);
        verify(audio.y >= network.y + network.height);
        grabImage(data.separate ? passive.contentItem : h.screen).save("build/notice-" + data.tag + "-passive-long.png");
        h.screenActive = false;
        compare(h.window.grabs, h.window.releases);
        tests.width = 1280; tests.height = 720;
    }
    function test_higherPriorityAndDestruction_data() {
        const rows = [];
        for (const separate of [false, true])
            for (const priority of ["connecting", "failure", "disconnect", "destroy"])
                rows.push({tag: (separate ? "window-" : "inline-") + priority, separate, priority});
        return rows;
    }
    function test_higherPriorityAndDestruction(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        h.nativeHost = data.separate;
        h.window.runtimeRendererBackend = data.separate ? 1 : 0;
        h.backend.session.cantDisplay = true;
        h.settings.audioVideoDisabled = 1;
        const host = findChild(h.screen, data.separate ? "streamActionNoticeWindow" : "streamActionNoticeInline");
        const passive = passiveHost(h, data.separate);
        tryCompare(host, "visible", true);
        compare(passive.visible, true);
        const action = findChild(host, "streamNoticeAction");
        if (data.priority === "destroy") {
            h.screenActive = false;
            compare(h.window.grabs, h.window.releases);
            return;
        }
        if (data.priority === "disconnect") h.backend.sessionStopDialogRequested();
        else h.backend.connectionAttempt = {presented: true, id: "1", phase: data.priority, route: "local", targetValid: true, cleanupPending: false};
        tryCompare(host, "visible", false);
        compare(passive.visible, false);
        compare(h.window.grabs - h.window.releases, 1);
        action.clicked();
        compare(h.backend.commands.length, 0);
        if (data.priority === "disconnect") {
            const dialog = findChild(findChild(h.screen, "streamDisconnectDialog"), "confirmationDialog");
            dialog.choose("cancel");
            tryCompare(host, "visible", true);
            tryCompare(action, "activeFocus", true);
        } else {
            h.backend.connectionAttempt = {presented: true, id: "1", phase: "ready", route: "local", targetValid: true, cleanupPending: false};
            tryCompare(host, "visible", true);
        }
        h.backend.session.connected = false;
        tryCompare(host, "visible", false);
        compare(passive.visible, false);
        action.clicked();
        compare(h.backend.commands.length, 0);
        compare(h.window.grabs, h.window.releases);
    }
    function test_blockedNoticeYieldsToMenu_data() { return routes(); }
    function test_blockedNoticeYieldsToMenu(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        h.nativeHost = true;
        h.window.runtimeRendererBackend = data.separate ? 1 : 0;
        wait(20);
        h.window.requestActivate();
        wait(20);
        h.opener.forceActiveFocus(Qt.TabFocusReason);
        h.backend.session.cantDisplay = true;
        h.settings.audioVideoDisabled = 1;
        h.backend.session.averagePacketLoss = 0.05;
        const host = findChild(h.screen, data.separate ? "streamActionNoticeWindow" : "streamActionNoticeInline");
        const passive = passiveHost(h, data.separate);
        tryCompare(host, "visible", true);
        tryCompare(passive, "visible", true);
        h.window.menuRequested();
        tryCompare(host, "visible", false);
        compare(passive.visible, false);
        compare(h.window.grabs - h.window.releases, 1);
        wait(300);
        keyClick(Qt.Key_Escape);
        tryCompare(host, "visible", true);
        compare(passive.visible, true);
        h.settings.audioVideoDisabled = 2;
        compare(findChild(host, "streamActionNotice").title, "Video disabled");
        h.settings.audioVideoDisabled = 0;
        compare(findChild(host, "streamActionNotice").title, "Content blocked by the console");
        h.backend.session.cantDisplay = false;
        tryCompare(h.opener, "activeFocus", true);
        compare(h.window.grabs, h.window.releases);
        compare(h.backend.commands.length, 0);
    }
}
