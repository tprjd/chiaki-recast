import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "SettingsStream"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; SettingsHarness {} }
    readonly property var routes: ["LocalPS4", "RemotePS4", "LocalPS5", "RemotePS5"]

    function openStream(width, height) {
        const harness = createTemporaryObject(harnessComponent, testCase,
            { width: width || 1280, height: height || 720, screenActive: false });
        for (const route of routes) {
            harness.settings["resolution" + route] = 3;
            harness.settings["fps" + route] = 60;
            harness.settings["bitrate" + route] = 0;
        }
        harness.settings.resolutionLocalPS5 = 4;
        harness.settings.codecLocalPS5 = 1;
        harness.settings.codecRemotePS5 = 1;
        harness.settings.changeCount = 0;
        harness.screenActive = true;
        harness.screen.currentCategory = 2;
        return harness;
    }
    function choose(combo, index) {
        verify(combo);
        combo.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", true);
        keyClick(Qt.Key_Home);
        for (let i = 0; i < index; i++) keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", false);
    }

    function test_resolutionOnlyUpdatesSelectedRoute_data() {
        return routes.map(route => ({tag: route, route: route}));
    }
    function test_resolutionOnlyUpdatesSelectedRoute(data) {
        const harness = openStream();
        for (const route of routes) harness.settings["bitrate" + route] = 23000;
        choose(findChild(harness, "consoleSelection"), data.route.endsWith("PS4") ? 0 : 1);
        harness.settings.changeCount = 0;
        const resolution = findChild(harness, "resolution" + data.route);
        compare(resolution.count, 4);
        choose(resolution, 0);
        compare(harness.settings["resolution" + data.route], 1);
        compare(harness.settings["bitrate" + data.route], 0);
        compare(harness.settings.changeCount, 2);
        for (const route of routes.filter(route => route !== data.route)) {
            compare(harness.settings["bitrate" + route], 23000);
            compare(harness.settings["resolution" + route], route === "LocalPS5" ? 4 : 3);
        }
    }
    function test_frameRateOnlyUpdatesSelectedRoute_data() {
        return routes.map(route => ({tag: route, route: route}));
    }
    function test_frameRateOnlyUpdatesSelectedRoute(data) {
        const harness = openStream();
        choose(findChild(harness, "consoleSelection"), data.route.endsWith("PS4") ? 0 : 1);
        harness.settings.changeCount = 0;
        const fps = findChild(harness, "fps" + data.route);
        compare(fps.count, 2);
        choose(fps, 0);
        compare(harness.settings["fps" + data.route], 30);
        compare(harness.settings.changeCount, 1);
        for (const route of routes.filter(route => route !== data.route))
            compare(harness.settings["fps" + route], 60);
        choose(fps, 1);
        compare(harness.settings["fps" + data.route], 60);
        compare(harness.settings.changeCount, 2);
    }

    function test_bitrateConversionsAndLimits_data() {
        return routes.map(route => ({tag: route, route: route}));
    }
    function test_bitrateConversionsAndLimits(data) {
        const harness = openStream();
        choose(findChild(harness, "consoleSelection"), data.route.endsWith("PS4") ? 0 : 1);
        const bitrate = findChild(harness, "bitrate" + data.route);
        compare(bitrate.from, 2);
        compare(bitrate.to, 100);
        compare(bitrate.stepSize, 1);
        harness.settings["bitrate" + data.route] = 2000;
        bitrate.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Left);
        compare(harness.settings["bitrate" + data.route], 2000);
        keyClick(Qt.Key_Right);
        compare(harness.settings["bitrate" + data.route], 3000);
        harness.settings["bitrate" + data.route] = 99000;
        keyClick(Qt.Key_Right);
        compare(harness.settings["bitrate" + data.route], 100000);
        keyClick(Qt.Key_Right);
        compare(harness.settings["bitrate" + data.route], 100000);
        for (const route of routes.filter(route => route !== data.route))
            compare(harness.settings["bitrate" + route], 0);
        harness.settings["bitrate" + data.route] = 15500;
        compare(bitrate.value, 15.5);
        verify(findChild(harness, "bitrateDescription" + data.route).text.includes("15.5 Mbps"));
    }

    function test_automaticBitrateNeverWritesOverride_data() {
        const cases = [];
        for (const route of routes)
            for (let resolution = 1; resolution <= 4; resolution++)
                cases.push({tag: route + resolution, route: route, resolution: resolution,
                    expected: [2, 6, 10, 15][resolution - 1]});
        return cases;
    }
    function test_automaticBitrateNeverWritesOverride(data) {
        const harness = openStream();
        harness.settings["resolution" + data.route] = data.resolution;
        choose(findChild(harness, "consoleSelection"), data.route.endsWith("PS4") ? 0 : 1);
        harness.settings.changeCount = 0;
        const bitrate = findChild(harness, "bitrate" + data.route);
        compare(bitrate.value, data.expected);
        verify(findChild(harness, "bitrateDescription" + data.route).text.includes("Automatic:"));
        bitrate.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Tab);
        harness.screen.currentCategory = 0;
        harness.screen.currentCategory = 2;
        compare(harness.settings["bitrate" + data.route], 0);
        compare(harness.settings.changeCount, 0);
    }

    function test_codecOnlyAvailableForPs5() {
        const harness = openStream();
        for (const route of ["LocalPS5", "RemotePS5"]) {
            const codec = findChild(harness, "codec" + route);
            compare(codec.count, 3);
            harness.settings.changeCount = 0;
            choose(codec, 2);
            compare(harness.settings["codec" + route], 2);
            compare(harness.settings.changeCount, 1);
            choose(codec, 0);
            compare(harness.settings["codec" + route], 0);
        }
        harness.settings.changeCount = 0;
        choose(findChild(harness, "consoleSelection"), 0);
        for (const route of ["LocalPS4", "RemotePS4"]) {
            verify(!findChild(harness, "codec" + route).visible);
            compare(harness.settings["codec" + route], undefined);
        }
        choose(findChild(harness, "consoleSelection"), 1);
        compare(harness.settings.changeCount, 0);
    }

    function test_profileRefreshUpdatesBoundValuesWithoutWrites() {
        const harness = openStream();
        const bitrate = findChild(harness, "bitrateLocalPS5");
        bitrate.forceActiveFocus(Qt.TabFocusReason);
        harness.settings.currentProfile = "Another synthetic profile";
        harness.settings.resolutionLocalPS5 = 2;
        harness.settings.fpsLocalPS5 = 30;
        harness.settings.bitrateLocalPS5 = 0;
        harness.settings.codecLocalPS5 = 0;
        harness.settings.changeCount = 0;
        keyClick(Qt.Key_Tab);
        wait(10);
        compare(bitrate.value, 6);
        compare(findChild(harness, "resolutionLocalPS5").currentIndex, 1);
        compare(findChild(harness, "fpsLocalPS5").currentIndex, 0);
        compare(findChild(harness, "codecLocalPS5").currentIndex, 0);
        compare(harness.settings.changeCount, 0);
    }

    function test_openAndConsoleNavigationDoNotWrite() {
        const harness = openStream();
        compare(harness.settings.changeCount, 0);
        choose(findChild(harness, "consoleSelection"), 0);
        choose(findChild(harness, "consoleSelection"), 1);
        const resolution = findChild(harness, "resolutionLocalPS5");
        resolution.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Right);
        verify(findChild(harness, "resolutionRemotePS5").activeFocus);
        compare(harness.settings.changeCount, 0);
    }

    function test_responsiveStream_data() {
        return [{tag: "desktop", width: 1280, height: 720},
            {tag: "narrow", width: 800, height: 600},
            {tag: "minimum", width: 390, height: 600}];
    }
    function test_responsiveStream(data) {
        const harness = openStream(data.width, data.height);
        waitForRendering(harness);
        for (const consoleIndex of [0, 1]) {
            choose(findChild(harness, "consoleSelection"), consoleIndex);
            for (const route of consoleIndex === 0 ? ["LocalPS4", "RemotePS4"] : ["LocalPS5", "RemotePS5"]) {
                const fields = consoleIndex === 0 ? ["resolution", "fps", "bitrate"] : ["resolution", "fps", "bitrate", "codec"];
                for (const field of fields) {
                    const control = findChild(harness, field + route);
                    control.forceActiveFocus(Qt.TabFocusReason);
                    wait(10);
                    const point = control.mapToItem(harness, 0, 0);
                    verify(point.x >= 0 && point.x + control.width <= harness.width, control.objectName + " fits width");
                    verify(point.y >= 0 && point.y + control.height <= harness.height, control.objectName + " is visible on focus");
                    verify(control.Accessible.name.length > 0);
                }
            }
        }
    }

}
