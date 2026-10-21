import QtQuick
import QtQuick.Window
import QtTest
import "fixtures"

TestCase {
    id: tests
    name: "StreamStatistics"
    width: 1280; height: 720
    visible: true
    when: windowShown
    property bool detailed: false
    Rectangle {
        id: backdrop
        anchors.fill: parent
        color: "#101820"
        Repeater {
            model: 30
            Rectangle { required property int index; width: backdrop.width; height: 12; y: index * 38; color: "#c3a66b"; visible: tests.detailed }
        }
    }
    Component { id: harnessComponent; StreamHarness { anchors.fill: parent } }
    function cleanup() { tests.width = 1280; tests.height = 720; backdrop.color = "#101820"; tests.detailed = false; }
    function routes() { return [{tag: "inline", separate: false}, {tag: "window", separate: true}]; }
    function showStats(separate) {
        const h = createTemporaryObject(harnessComponent, tests);
        verify(h);
        h.nativeHost = separate;
        h.window.runtimeRendererBackend = separate ? 1 : 0;
        tryCompare(findChild(h.screen, separate ? "streamStatsWindow" : "streamStatsInline"), "visible", true);
        return h;
    }
    function statsHost(h) { return h.window.runtimeRendererBackend === 1 ? findChild(h.screen, "streamStatsWindow").contentItem : h.screen; }
    function metric(h, name) {
        tryVerify(() => findChild(statsHost(h), "streamStats" + name));
        const label = findChild(statsHost(h), "streamStats" + name);
        verify(label, name + " is displayed");
        return label;
    }
    function test_liveValues_data() { return routes(); }
    function test_liveValues(data) {
        const h = showStats(data.separate);
        compare(metric(h, "Bitrate").text, "15.2 Mbps");
        compare(metric(h, "Loss").text, "1.0 %");
        compare(metric(h, "Dropped").text, "3");
        compare(metric(h, "Lost").text, "4");
        compare(metric(h, "Queue").text, "1.2");
        compare(metric(h, "Age").text, "16 ms");
        h.backend.session.measuredBitrate = 23.46;
        h.backend.session.averagePacketLoss = 0.035;
        h.backend.session.framesLost = 27;
        h.window.droppedFrames = 12;
        h.window.queueDepthAverage = 2.46;
        h.window.pendingFrameAge = 0.039;
        compare(metric(h, "Bitrate").text, "23.5 Mbps");
        compare(metric(h, "Loss").text, "3.5 %");
        compare(metric(h, "Dropped").text, "12");
        compare(metric(h, "Lost").text, "27");
        compare(metric(h, "Queue").text, "2.5");
        compare(metric(h, "Age").text, "39 ms");
    }
    function test_zeroAndFiniteValues_data() { return routes(); }
    function test_zeroAndFiniteValues(data) {
        const h = showStats(data.separate);
        h.backend.session.measuredBitrate = 0;
        h.backend.session.averagePacketLoss = 0;
        h.backend.session.framesLost = 0;
        h.window.droppedFrames = 0;
        h.window.queueDepthAverage = 0;
        h.window.pendingFrameAge = 0;
        const expected = {Bitrate: "0.0 Mbps", Loss: "0.0 %", Dropped: "0", Lost: "0", Queue: "0.0", Age: "0 ms"};
        for (const name in expected) compare(metric(h, name).text, expected[name]);
        for (const value of [NaN, Infinity, -Infinity]) {
            h.backend.session.measuredBitrate = value;
            h.backend.session.averagePacketLoss = value;
            h.window.queueDepthAverage = value;
            h.window.pendingFrameAge = value;
            for (const name in expected) compare(metric(h, name).text, expected[name]);
        }
    }
    function test_visibilityAndTeardown_data() { return routes(); }
    function test_visibilityAndTeardown(data) {
        const h = showStats(data.separate);
        const host = findChild(h.screen, data.separate ? "streamStatsWindow" : "streamStatsInline");
        h.settings.showStreamStats = false;
        tryCompare(host, "visible", false);
        h.settings.showStreamStats = true;
        tryCompare(host, "visible", true);
        h.settings.audioVideoDisabled = 1;
        compare(host.visible, true);
        for (const disabled of [2, 3]) {
            h.settings.audioVideoDisabled = disabled;
            tryCompare(host, "visible", false);
        }
        h.settings.audioVideoDisabled = 0;
        tryCompare(host, "visible", true);
        for (const phase of ["connecting", "failure"]) {
            h.backend.connectionAttempt = {presented: true, id: "1", phase, route: "local", targetValid: true, cleanupPending: false};
            tryCompare(host, "visible", false);
        }
        h.backend.connectionAttempt = {presented: true, id: "1", phase: "ready", route: "local", targetValid: true, cleanupPending: false};
        tryCompare(host, "visible", true);
        h.opener.forceActiveFocus(Qt.TabFocusReason);
        h.window.menuRequested();
        tryCompare(host, "visible", false);
        wait(300);
        keyClick(Qt.Key_Escape);
        compare(host.visible, false, "Statistics stay hidden during menu closure");
        wait(300);
        tryCompare(host, "visible", true);
        const old = h.backend.session;
        h.backend.session = null;
        tryCompare(host, "visible", false);
        h.backend.session = old;
        tryCompare(host, "visible", true);
        h.screenActive = false;
        compare(h.screen, null);
        tryVerify(() => !findChild(h, "streamStatsWindow"), 1000, "Destroying the stream removes its native statistics window");
    }
    function test_sessionReplacement_data() { return routes(); }
    function test_sessionReplacement(data) {
        const h = showStats(data.separate);
        const old = h.backend.session;
        const replacement = createTemporaryQmlObject('import QtQuick; QtObject { property bool connected: true; property bool muted: true; property bool cantDisplay: false; property string host: "Replacement"; property real measuredBitrate: 7.3; property real averagePacketLoss: 0; property int framesLost: 0 }', h);
        h.backend.session = replacement;
        compare(metric(h, "Bitrate").text, "7.3 Mbps");
        compare(metric(h, "Loss").text, "0.0 %");
        compare(metric(h, "Lost").text, "0");
        old.measuredBitrate = 99;
        compare(metric(h, "Bitrate").text, "7.3 Mbps", "The old session cannot update the readout");
        h.backend.session = null;
        replacement.destroy();
        tryCompare(findChild(h.screen, data.separate ? "streamStatsWindow" : "streamStatsInline"), "visible", false);
    }
    function test_layout_data() {
        const rows = [];
        for (const separate of [false, true])
            for (const size of [[900, 600], [1280, 720], [1920, 1080]])
                rows.push({tag: (separate ? "window-" : "inline-") + size[0], separate, w: size[0], h: size[1]});
        return rows;
    }
    function test_layout(data) {
        tests.width = data.w; tests.height = data.h;
        const h = showStats(data.separate);
        h.backend.session.measuredBitrate = 12345.6;
        h.backend.session.averagePacketLoss = 1;
        h.backend.session.framesLost = 2147483647;
        h.window.droppedFrames = 2147483647;
        h.window.queueDepthAverage = 1234.5;
        h.window.pendingFrameAge = 999.999;
        const names = ["Bitrate", "Loss", "Dropped", "Lost", "Queue", "Age"];
        const expected = ["12345.6 Mbps", "100.0 %", "2147483647", "2147483647", "1234.5", "999999 ms"];
        for (const name of names) metric(h, name + "Label").text = "Long translated metric label " + name;
        waitForRendering(statsHost(h));
        const row = findChild(statsHost(h), "streamStatsReadout");
        const origin = row.mapToItem(statsHost(h), 0, 0);
        compare(origin.x, 18);
        compare(statsHost(h).height - origin.y - row.height, 18);
        verify(row.width <= data.w - 36);
        verify(row.height <= 36, "The readout stays one compact row");
        let previous = -1;
        for (let index = 0; index < names.length; index++) {
            const value = metric(h, names[index]);
            compare(value.text, expected[index]);
            const p = value.mapToItem(row, 0, 0);
            verify(p.x > previous, "Metrics keep their approved order");
            verify(value.width >= value.contentWidth, names[index] + " value remains complete");
            verify(p.x >= 0 && p.x + value.width <= row.width, names[index] + " stays inside the row");
            verify(p.y >= 0 && p.y + value.height <= row.height);
            verify(value.Accessible.name.length > value.text.length);
            previous = p.x + value.width;
        }
        if (Qt.platform.pluginName === "offscreen") {
            const image = grabImage(data.separate ? statsHost(h) : tests);
            image.save("build/stream-statistics-" + data.tag + ".png");
        }
        if (data.separate) {
            const host = findChild(h.screen, "streamStatsWindow");
            const before = Qt.point(host.x, host.y);
            h.window.x += 31;
            h.window.y += 17;
            tryCompare(host, "x", before.x + 31);
            tryCompare(host, "y", before.y + 17);
        }
    }
    function test_passive_data() { return routes(); }
    function test_passive(data) {
        if (data.separate && Qt.platform.pluginName === "offscreen")
            skip("The offscreen platform activates passive windows. Run this case with the Windows platform.");
        const h = showStats(data.separate);
        if (data.separate) h.window.requestActivate();
        else tests.Window.window.requestActivate();
        wait(20);
        h.opener.forceActiveFocus(Qt.TabFocusReason);
        h.settings.showStreamStats = false;
        h.settings.showStreamStats = true;
        wait(50);
        compare(h.opener.activeFocus, true);
        compare(h.window.grabs, 0);
        compare(h.window.releases, 0);
        keyClick(Qt.Key_Space);
        compare(h.gameClicks, 1, "The game keeps keyboard input");
        if (data.separate) {
            const host = findChild(h.screen, "streamStatsWindow");
            verify(host.flags & Qt.WindowTransparentForInput);
            verify(host.flags & Qt.WindowDoesNotAcceptFocus);
        } else {
            const row = findChild(h.screen, "streamStatsReadout");
            const p = row.mapToItem(h, row.width / 2, row.height / 2);
            mouseClick(h, p.x, p.y);
            compare(h.gameClicks, 2, "A click on statistics reaches the game");
        }
        compare(h.backend.commands.length, 0);
    }
    function test_backgroundContrast_data() {
        const rows = [];
        for (const separate of [false, true])
            for (const background of ["dark", "bright", "detailed"])
                rows.push({tag: (separate ? "window-" : "inline-") + background, separate, background});
        return rows;
    }
    function test_backgroundContrast(data) {
        if (Qt.platform.pluginName !== "offscreen")
            skip("Pixel checks use the offscreen renderer. Native window capture is checked in the full application.");
        backdrop.color = data.background === "bright" ? "#ffffff" : "#101820";
        tests.detailed = data.background === "detailed";
        const h = showStats(data.separate);
        const value = metric(h, "Bitrate");
        const row = findChild(statsHost(h), "streamStatsReadout");
        waitForRendering(statsHost(h));
        const image = grabImage(data.separate ? statsHost(h) : tests);
        const scale = image.width / (data.separate ? statsHost(h).width : tests.width);
        const p = row.mapToItem(data.separate ? statsHost(h) : tests, 2, row.height / 2);
        p.x *= scale; p.y *= scale;
        if (data.separate) {
            const alpha = image.alpha(Math.round(p.x), Math.round(p.y));
            verify(alpha >= 160 && alpha <= 170, "The separate background keeps about 65 percent alpha");
        } else {
            const red = image.red(Math.round(p.x), Math.round(p.y));
            verify(red < 105, "The dark backplate preserves text contrast even over bright video");
            if (data.background === "bright") verify(red > 80, "Bright video remains visible through the backplate");
        }
        const vp = value.mapToItem(data.separate ? statsHost(h) : tests, 0, 0);
        vp.x *= scale; vp.y *= scale;
        let opaqueText = false;
        for (let y = Math.ceil(vp.y); y < vp.y + value.height * scale; y++)
            for (let x = Math.ceil(vp.x); x < vp.x + value.width * scale; x++)
                if (image.red(x, y) > 225 && image.green(x, y) > 225 && image.blue(x, y) > 225 && image.alpha(x, y) > 245) opaqueText = true;
        verify(opaqueText, "Values stay opaque and readable");
        image.save("build/stream-statistics-" + data.tag + ".png");
    }
}
