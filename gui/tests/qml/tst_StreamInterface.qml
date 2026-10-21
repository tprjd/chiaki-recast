import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: tests
    name: "StreamInterface"
    width: 900; height: 600
    visible: true
    when: windowShown
    Component { id: harnessComponent; StreamHarness { anchors.fill: parent } }
    function routes() { return [{tag: "inline", separate: false}, {tag: "window", separate: true}]; }
    function menuHost(h, separate) { return separate ? findChild(h.screen, "streamMenuWindow") : h.screen; }
    function openMenu(h, separate) {
        h.nativeHost = separate;
        h.window.runtimeRendererBackend = separate ? 1 : 0;
        h.opener.forceActiveFocus(Qt.TabFocusReason);
        h.window.menuRequested();
        wait(300);
        const host = menuHost(h, separate);
        const close = findChild(host, "streamResume");
        verify(close);
        tryCompare(close, "activeFocus", true);
        return host;
    }
    function test_menuRoundTrip_data() { return routes(); }
    function test_menuRoundTrip(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        verify(h);
        const host = openMenu(h, data.separate);
        keyClick(Qt.Key_Escape);
        tryCompare(h.window, "releases", 1);
        wait(300);
        compare(h.window.grabs, 1);
        compare(h.window.releases, 1);
        if (data.separate) compare(host.visible, false);
        else tryCompare(findChild(h.screen, "streamMenuInline"), "visible", false);
        tryCompare(findChild(h.screen, data.separate ? "streamStatsWindow" : "streamStatsInline"), "visible", true);
        tryCompare(h.opener, "activeFocus", true);
    }
    function test_closeNativeWindow_data() { return [{tag: "window", separate: true}]; }
    function test_closeNativeWindow(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        const host = openMenu(h, true);
        host.close();
        tryCompare(h.window, "releases", 1);
        wait(300);
        compare(host.visible, false);
        compare(h.window.grabs, 1);
        compare(h.window.releases, 1);
    }
    function test_disconnectEntry_data() {
        const rows = [];
        for (const separate of [false, true])
            for (const choice of ["cancel", "escape", "disconnect", "sleep"])
                rows.push({tag: (separate ? "window-" : "inline-") + choice, separate, choice});
        return rows;
    }
    function test_disconnectEntry(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        const host = openMenu(h, data.separate);
        findChild(host, "streamDisconnect").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        const prompt = findChild(h.screen, "streamDisconnectDialog");
        const dialog = findChild(prompt, "confirmationDialog");
        tryCompare(dialog, "opened", true);
        const cancel = findChild(dialog.footer, "confirmationCancel");
        tryCompare(cancel, "activeFocus", true);
        wait(300);
        tryCompare(cancel, "activeFocus", true);
        compare(h.window.grabs, 2);
        compare(h.window.releases, 1);
        h.window.menuRequested();
        if (data.choice === "disconnect") keyClick(Qt.Key_Up);
        if (data.choice === "sleep") keyClick(Qt.Key_Down);
        keyClick(data.choice === "escape" ? Qt.Key_Escape : Qt.Key_Return);
        tryCompare(prompt, "opened", false);
        wait(300);
        compare(h.window.releases, 2);
        compare(h.backend.commands.length, data.choice === "disconnect" || data.choice === "sleep" ? 1 : 0);
        if (h.backend.commands.length) compare(h.backend.commands[0].sleep, data.choice === "sleep");
    }

    function test_controls_data() { return routes(); }
    function test_controls(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        const host = openMenu(h, data.separate);
        const volume = findChild(host, "streamVolume");
        volume.forceActiveFocus(Qt.TabFocusReason);
        h.settings.audioVolume = 1;
        keyClick(Qt.Key_Down);
        compare(h.settings.audioVolume, 0);
        compare(volume.value, 0);
        h.settings.audioVolume = 127;
        keyClick(Qt.Key_Up);
        compare(h.settings.audioVolume, 128);
        h.settings.audioVolume = 64;
        compare(volume.value, 64);
        keyClick(Qt.Key_Up);
        compare(h.settings.audioVolume, 65);
        const mic = findChild(host, "streamMicrophone");
        compare(mic.checked, false);
        mic.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(h.backend.session.muted, false);
        compare(mic.checked, true);
        keyClick(Qt.Key_Return);
        compare(h.backend.session.muted, true);
        h.backend.session.connected = false;
        compare(mic.enabled, false);
        h.backend.session.connected = true;
        compare(mic.enabled, true);
        findChild(host, "streamPicture").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        const zoom = findChild(host, "streamZoom");
        const factor = findChild(host, "streamZoomFactor");
        const stretch = findChild(host, "streamStretch");
        zoom.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(h.window.videoMode, 2);
        compare(factor.visible, true);
        factor.forceActiveFocus(Qt.TabFocusReason);
        h.window.ZoomFactor = -0.99;
        keyClick(Qt.Key_Down);
        compare(h.window.ZoomFactor, -1);
        compare(h.settings.sZoomFactor, -1);
        h.window.ZoomFactor = 3.99;
        keyClick(Qt.Key_Up);
        compare(h.window.ZoomFactor, 4);
        compare(h.settings.sZoomFactor, 4);
        stretch.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(h.window.videoMode, 1);
        compare(factor.visible, false);
        findChild(host, "streamFit").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(h.window.videoMode, 0);
        compare(stretch.checked, false);
        keyClick(Qt.Key_Escape);
        findChild(host, "streamQuality").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        for (const row of [{name: "streamDefault", value: 1}, {name: "streamHighQuality", value: 2},
                {name: "streamSpatial", value: 3}, {name: "streamAdvancedSpatial", value: 4}, {name: "streamCustom", value: 5}]) {
            const button = findChild(findChild(host, "streamQualityPopup").contentItem, row.name);
            button.forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Return);
            compare(h.window.videoPreset, row.value);
            compare(h.settings.videoPreset, row.value);
            compare(button.checked, true);
            compare(findChild(host, "streamRendererSettings").visible, row.value === 5);
        }
        keyClick(Qt.Key_Escape);
        keyClick(Qt.Key_Escape);
        tryCompare(h.window, "releases", 1);
    }
    function test_settingsActions_data() {
        const rows = [];
        for (const separate of [false, true])
            for (const renderer of [false, true]) rows.push({tag: (separate ? "window-" : "inline-") + renderer, separate, renderer});
        return rows;
    }
    function test_settingsActions(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        h.window.videoPreset = data.renderer ? 5 : 1;
        const host = openMenu(h, data.separate);
        findChild(host, data.renderer ? "streamQuality" : "streamPicture").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        const button = findChild(host, data.renderer ? "streamRendererSettings" : "streamDisplaySettings");
        button.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(h, "actions", [data.renderer ? "renderer" : "display"]);
        compare(h.actions[0], data.renderer ? "renderer" : "display");
        compare(h.window.releases, 1);
        wait(300);
        if (data.separate) compare(host.visible, false);
        compare(h.window.releases, 1);
    }
    function test_sessionRemoval_data() { return routes(); }
    function test_sessionRemoval(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        const host = openMenu(h, data.separate);
        h.backend.session = null;
        tryCompare(h.window, "releases", 1);
        wait(300);
        if (data.separate) compare(host.visible, false);
        compare(findChild(host, "streamMicrophone").enabled, false);
        compare(h.backend.commands.length, 0);
    }

    function test_exclusions_data() {
        const rows = [];
        for (const separate of [false, true])
            for (const condition of ["pin", "connecting", "failure"])
                rows.push({tag: (separate ? "window-" : "inline-") + condition, separate, condition});
        return rows;
    }
    function test_exclusions(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        h.window.runtimeRendererBackend = data.separate ? 1 : 0;
        if (data.condition === "pin") h.backend.sessionPinRequest = {id: "pin-1"};
        else h.backend.connectionAttempt = {presented: true, id: "1", phase: data.condition, route: "local", targetValid: true, cleanupPending: false};
        const baseline = h.window.grabs;
        h.window.menuRequested();
        wait(300);
        compare(h.window.grabs, baseline);
        if (data.separate) compare(menuHost(h, true).visible, false);
        compare(h.backend.commands.length, 0);
    }
    function test_repeatAndDestruction_data() { return routes(); }
    function test_repeatAndDestruction(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        openMenu(h, data.separate);
        h.window.menuRequested();
        h.window.menuRequested();
        wait(300);
        compare(h.window.grabs, 1);
        compare(h.window.releases, 1);
        h.window.menuRequested();
        wait(300);
        compare(h.window.grabs, 2);
        h.screenActive = false;
        tryCompare(h.window, "releases", 2);
        compare(h.screen, null);
    }

    function test_sessionReplacement_data() { return routes(); }
    function test_sessionReplacement(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        const host = openMenu(h, data.separate);
        const oldSession = h.backend.session;
        const replacement = createTemporaryQmlObject('import QtQuick; QtObject { property bool connected: true; property bool muted: false; property bool cantDisplay: false; property string host: "Replacement"; property real measuredBitrate: 0; property real averagePacketLoss: 0; property int framesLost: 0 }', h);
        h.backend.session = replacement;
        const mic = findChild(host, "streamMicrophone");
        compare(mic.checked, true);
        mic.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(replacement.muted, true);
        compare(oldSession.muted, true);
        h.backend.session = null;
        replacement.destroy();
        wait(300);
        compare(mic.enabled, false);
        compare(h.window.releases, 1);
        compare(h.backend.commands.length, 0);
    }

    function test_shortcutDuringClosure_data() { return routes(); }
    function test_shortcutDuringClosure(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        const host = openMenu(h, data.separate);
        keyClick(Qt.Key_Escape);
        h.window.menuRequested();
        wait(300);
        compare(h.window.grabs, 1);
        compare(h.window.releases, 1);
        if (data.separate) compare(host.visible, false);
    }
    function test_configuredDisconnect_data() {
        return [{tag: "inline-stop", separate: false, policy: 0}, {tag: "inline-sleep", separate: false, policy: 1},
            {tag: "window-stop", separate: true, policy: 0}, {tag: "window-sleep", separate: true, policy: 1}];
    }
    function test_configuredDisconnect(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        h.settings.disconnectAction = data.policy;
        const host = openMenu(h, data.separate);
        findChild(host, "streamDisconnect").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(h.backend.commands.length, 1);
        compare(h.backend.commands[0].sleep, data.policy === 1);
        keyClick(Qt.Key_Return);
        compare(h.backend.commands.length, 1);
        compare(findChild(h.screen, "streamDisconnectDialog").opened, false);
    }

}
