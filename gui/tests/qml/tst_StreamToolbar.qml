import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: tests
    name: "StreamToolbar"
    width: 900; height: 600
    visible: true
    when: windowShown
    Component { id: harnessComponent; StreamHarness { anchors.fill: parent } }
    function routes() { return [{tag: "inline", separate: false}, {tag: "window", separate: true}]; }
    function openMenu(h, separate) {
        h.nativeHost = separate;
        h.window.runtimeRendererBackend = separate ? 1 : 0;
        h.opener.forceActiveFocus(Qt.TabFocusReason);
        h.window.menuRequested();
        wait(300);
        return separate ? findChild(h.screen, "streamMenuWindow") : h.screen;
    }
    function test_resume_data() { return routes(); }
    function test_resume(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        const host = openMenu(h, data.separate);
        const resume = findChild(host, "streamResume");
        verify(resume, "The toolbar names Resume as its first action");
        tryCompare(resume, "activeFocus", true);
        const captureRoot = data.separate ? host.contentItem : h.screen;
        const focused = grabImage(captureRoot);
        const position = resume.mapToItem(captureRoot, 0, 0);
        focused.save("build/resume-focused-" + data.separate + ".png");
        let outline = false;
        const y = Math.round(position.y + resume.height / 2);
        for (let x = Math.ceil(position.x); x < position.x + 20; x++)
            if (focused.blue(x, y) > focused.red(x, y) + 40) outline = true;
        verify(outline, "Initial Resume focus has a visible blue outline");
        compare(resume.text, "Resume");
        compare(findChild(host, "streamVolumePercentage").text, "50%");
        compare(findChild(host, "streamMicrophone").text, "Muted");
        compare(findChild(host, "streamPicture").text, "Fit");
        compare(findChild(host, "streamQuality").text, "Default");
        keyClick(Qt.Key_Return);
        tryCompare(h.window, "releases", 1);
        wait(300);
        compare(h.backend.commands.length, 0);
        tryCompare(h.opener, "activeFocus", true);
    }
    function test_pictureFlyout_data() { return routes(); }
    function test_pictureFlyout(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        const host = openMenu(h, data.separate);
        const opener = findChild(host, "streamPicture");
        opener.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        const popup = findChild(host, "streamPicturePopup");
        verify(popup, "Picture controls open above the toolbar");
        tryCompare(popup, "opened", true);
        const zoom = findChild(host, "streamZoom");
        zoom.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(h.window.videoMode, 2);
        compare(opener.text, "Zoom");
        compare(findChild(host, "streamZoomFactor").visible, true);
        keyClick(Qt.Key_Escape);
        tryCompare(popup, "opened", false);
        tryCompare(opener, "activeFocus", true);
        compare(h.window.releases, 0);
        keyClick(Qt.Key_Escape);
        tryCompare(h.window, "releases", 1);
    }
    function test_qualityFlyout_data() { return routes(); }
    function test_qualityFlyout(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        const host = openMenu(h, data.separate);
        const opener = findChild(host, "streamQuality");
        opener.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        const popup = findChild(host, "streamQualityPopup");
        verify(popup, "Quality controls open above the toolbar");
        tryCompare(popup, "opened", true);
        for (const row of [{name: "streamDefault", value: 1, label: "Default"}, {name: "streamHighQuality", value: 2, label: "High quality"},
                {name: "streamSpatial", value: 3, label: "HQ + Spatial"}, {name: "streamAdvancedSpatial", value: 4, label: "HQ + Advanced spatial"}, {name: "streamCustom", value: 5, label: "Custom"}]) {
            const button = findChild(popup.contentItem, row.name);
            button.forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Return);
            compare(h.window.videoPreset, row.value);
            compare(h.settings.videoPreset, row.value);
            compare(opener.text, row.label);
            compare(button.checked, true);
            compare(findChild(host, "streamRendererSettings").visible, row.value === 5);
        }
        keyClick(Qt.Key_Escape);
        tryCompare(popup, "opened", false);
        tryCompare(opener, "activeFocus", true);
        compare(h.window.releases, 0);
        keyClick(Qt.Key_Escape);
        tryCompare(h.window, "releases", 1);
    }
    function test_flyoutSwitchAndToggle_data() { return routes(); }
    function test_flyoutSwitchAndToggle(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        const host = openMenu(h, data.separate);
        const picture = findChild(host, "streamPicture");
        const quality = findChild(host, "streamQuality");
        const picturePopup = findChild(host, "streamPicturePopup");
        const qualityPopup = findChild(host, "streamQualityPopup");
        mouseClick(picture);
        tryCompare(picturePopup, "opened", true);
        mouseClick(quality);
        tryCompare(picturePopup, "opened", false);
        tryCompare(qualityPopup, "opened", true);
        mouseClick(quality);
        tryCompare(qualityPopup, "opened", false);
        tryCompare(quality, "activeFocus", true);
        compare(h.window.releases, 0);
        mouseClick(picture);
        tryCompare(picturePopup, "opened", true);
        h.window.menuRequested();
        tryCompare(picturePopup, "opened", false);
        tryCompare(h.window, "releases", 1);
        wait(300);
        if (data.separate) compare(host.visible, false);
    }
    function test_keyboardAndControllerNavigation_data() { return routes(); }
    function test_keyboardAndControllerNavigation(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        const host = openMenu(h, data.separate);
        for (const name of ["streamVolume", "streamMicrophone", "streamPicture", "streamQuality", "streamDisconnect"]) {
            keyClick(Qt.Key_Right);
            const control = findChild(host, name);
            tryCompare(control, "activeFocus", true);
            verify(control.visualFocus);
        }
        keyClick(Qt.Key_Tab);
        tryCompare(findChild(host, "streamResume"), "activeFocus", true);
        h.backend.session.connected = false;
        keyClick(Qt.Key_Tab);
        tryCompare(findChild(host, "streamVolume"), "activeFocus", true);
        keyClick(Qt.Key_Tab);
        tryCompare(findChild(host, "streamPicture"), "activeFocus", true);
        keyClick(Qt.Key_Return);
        tryCompare(findChild(host, "streamFit"), "activeFocus", true);
        keyClick(Qt.Key_Right);
        keyClick(Qt.Key_Return);
        compare(h.window.videoMode, 2);
        keyClick(Qt.Key_Down);
        tryCompare(findChild(host, "streamZoomFactor"), "activeFocus", true);
        keyClick(Qt.Key_Up);
        compare(h.settings.sZoomFactor, 0.01);
        keyClick(Qt.Key_Right);
        tryCompare(findChild(host, "streamDisplaySettings"), "activeFocus", true);
    }
    function test_priorityClosesFlyout_data() {
        const rows = [];
        for (const separate of [false, true])
            for (const condition of ["pin", "connecting", "failure"])
                rows.push({tag: (separate ? "window-" : "inline-") + condition, separate, condition});
        return rows;
    }
    function test_priorityClosesFlyout(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        const host = openMenu(h, data.separate);
        mouseClick(findChild(host, "streamPicture"));
        const popup = findChild(host, "streamPicturePopup");
        tryCompare(popup, "opened", true);
        if (data.condition === "pin") h.backend.sessionPinRequest = {id: "pin-1"};
        else h.backend.connectionAttempt = {presented: true, id: "1", phase: data.condition, route: "local", targetValid: true, cleanupPending: false};
        tryCompare(popup, "opened", false);
        tryCompare(h.window, "releases", 1);
        wait(300);
        compare(data.separate ? host.visible : findChild(h.screen, "streamMenuInline").visible, false);
        compare(h.backend.commands.length, 0);
    }
    function test_layout_data() {
        const rows = [];
        for (const separate of [false, true])
            for (const size of [{width: 900, height: 600}, {width: 1280, height: 720}, {width: 1920, height: 1080}])
                for (const background of ["dark", "bright", "detailed"])
                    rows.push({tag: (separate ? "window" : "inline") + "-" + size.width + "-" + background, separate, size, background});
        return rows;
    }
    function test_linkedSettingsOwnFocus_data() {
        const rows = [];
        for (const separate of [false, true])
            for (const condition of ["none", "video", "blocked"])
                rows.push({tag: (separate ? "window-" : "inline-") + condition, separate, condition});
        return rows;
    }
    function test_linkedSettingsOwnFocus(data) {
        const h = createTemporaryObject(harnessComponent, tests);
        h.settings.audioVideoDisabled = data.condition === "video" ? 2 : 0;
        h.backend.session.cantDisplay = data.condition === "blocked";
        h.window.videoPreset = data.condition === "blocked" ? 5 : 1;
        const host = openMenu(h, data.separate);
        mouseClick(findChild(host, data.condition === "blocked" ? "streamQuality" : "streamPicture"));
        const action = findChild(host, data.condition === "blocked" ? "streamRendererSettings" : "streamDisplaySettings");
        action.forceActiveFocus(Qt.TabFocusReason);
        tryCompare(action, "activeFocus", true);
        keyClick(Qt.Key_Return);
        tryCompare(h, "streamSettingsOpen", true);
        wait(350);
        compare(h.settingsFocusItem.activeFocus, true, "Settings keeps focus after menu closure");
        compare(h.window.grabs - h.window.releases, 1, "Only settings owns input");
        compare(findChild(h.screen, data.separate ? "streamActionNoticeWindow" : "streamActionNoticeInline").visible, false);
        h.closeStreamSettings();
        if (data.condition !== "none") {
            const notice = findChild(h.screen, data.separate ? "streamActionNoticeWindow" : "streamActionNoticeInline");
            tryCompare(notice, "visible", true);
            tryCompare(findChild(notice, "streamNoticeAction"), "activeFocus", true);
            compare(h.window.grabs - h.window.releases, 1);
        } else compare(h.window.grabs, h.window.releases);
    }
    function test_shortcutClosesFlyoutAndMenu() {
        const h = createTemporaryObject(harnessComponent, tests);
        const host = openMenu(h, true);
        mouseClick(findChild(host, "streamPicture"));
        const popup = findChild(host, "streamPicturePopup");
        tryCompare(popup, "opened", true);
        tryCompare(findChild(host, "streamFit"), "activeFocus", true);
        keyClick(Qt.Key_O, Qt.ControlModifier);
        tryCompare(host, "visible", false);
        compare(h.window.grabs, h.window.releases);
    }
    function test_layout(data) {
        tests.width = data.size.width; tests.height = data.size.height;
        const h = createTemporaryObject(harnessComponent, tests);
        h.nativeHost = data.separate;
        createTemporaryQmlObject('import QtQuick; Rectangle { anchors.fill: parent; z: -1; color: "' + (data.background === "bright" ? "#f5f5f5" : "#151a22") + '"; Repeater { model: ' + (data.background === "detailed" ? '30' : '0') + '; Rectangle { required property int index; x: index * 70; y: index % 2 * 190; width: 60; height: 350; color: index % 2 ? "#6b8595" : "#c3b3a4" } } }', h);
        const host = openMenu(h, data.separate);
        const toolbar = data.separate ? host : findChild(h.screen, "streamMenuInline");
        compare(toolbar.width, 820);
        verify(toolbar.height >= 78 && toolbar.height < 200, "Labels fit in a compact toolbar");
        if (!data.separate) {
            compare(toolbar.x, (tests.width - 820) / 2);
            compare(tests.height - toolbar.y - toolbar.height, 18);
        }
        for (const name of ["streamResume", "streamVolume", "streamMicrophone", "streamPicture", "streamQuality", "streamDisconnect"]) {
            const control = findChild(host, name);
            const p = control.mapToItem(data.separate ? host.contentItem : toolbar, 0, 0);
            verify(p.x >= 0 && p.x + control.width <= toolbar.width + 1);
            verify(p.y >= 0 && p.y + control.height <= toolbar.height + 1);
        }
        h.window.videoPreset = 4;
        const volumeLabel = findChild(host, "streamVolumeLabel");
        const percentage = findChild(host, "streamVolumePercentage");
        verify(volumeLabel.contentWidth <= volumeLabel.width + 1, "Translated Volume label wraps before the percentage");
        verify(volumeLabel.mapToGlobal(volumeLabel.width, 0).x <= percentage.mapToGlobal(0, 0).x);
        const captureRoot = data.separate ? host.contentItem : h;
        const prefix = "build/toolbar-" + data.tag;
        grabImage(captureRoot).save(prefix + ".png");
        mouseClick(findChild(host, "streamPicture"));
        const picture = findChild(host, "streamPicturePopup");
        tryCompare(picture, "opened", true);
        tryCompare(findChild(host, "streamFit"), "activeFocus", true);
        findChild(host, "streamZoom").forceActiveFocus(Qt.TabFocusReason);
        tryCompare(findChild(host, "streamZoom"), "activeFocus", true);
        keyClick(Qt.Key_Return);
        compare(h.window.videoMode, 2);
        compare(picture.opened, true);
        if (data.separate) compare(picture.nativeHost.visible, true);
        compare(picture.width, 320);
        wait(50);
        verify(picture.contentItem.mapToGlobal(0, picture.contentItem.height + picture.bottomPadding).y <= findChild(host, "streamResume").mapToGlobal(0, 0).y);
        verify(picture.height + toolbar.height + 27 < tests.height);
        grabImage(data.separate ? picture.nativeHost.contentItem : h).save(prefix + "-picture.png");
        mouseClick(findChild(host, "streamQuality"));
        const quality = findChild(host, "streamQualityPopup");
        tryCompare(quality, "opened", true);
        h.window.videoPreset = 5;
        compare(quality.width, 320);
        wait(50);
        verify(quality.contentItem.mapToGlobal(0, quality.contentItem.height + quality.bottomPadding).y <= findChild(host, "streamResume").mapToGlobal(0, 0).y);
        verify(quality.height + toolbar.height + 27 < tests.height);
        grabImage(data.separate ? quality.nativeHost.contentItem : h).save(prefix + "-quality.png");
        keyClick(Qt.Key_Escape);
        keyClick(Qt.Key_Escape);
        wait(300);
    }
}
