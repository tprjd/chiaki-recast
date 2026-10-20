import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"
import "../../src/qml"

TestCase {
    id: testCase
    name: "SettingsDisplay"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    Component {
        id: displayHarness
        Item {
            id: root
            width: 1280; height: 720
            property alias settings: fakeSettings
            property alias screen: loader.item
            property bool screenActive: true
            property int closeCount: 0
            function closeDialog() { closeCount++; }
            FakeSettings { id: fakeSettings }
            QtObject { id: fakeBackend; property var settings: fakeSettings }
            Loader { id: loader; anchors.fill: parent; active: root.screenActive; sourceComponent: DisplaySettingsDialog { backend: fakeBackend; navigation: root } }
        }
    }
    Component {
        id: navigationHarness
        Item {
            id: root
            width: 900; height: 600
            function closeDialog() { pages.pop(); }
            function showDisplaySettingsDialog() { pages.push(displayComponent); }
            SettingsHarness { id: collaborators; visible: false; screenActive: false }
            StackView {
                id: pages
                objectName: "settingsStack"
                anchors.fill: parent
                initialItem: SettingsDialog { backend: collaborators.backend; navigation: root }
            }
            Component { id: displayComponent; DisplaySettingsDialog { backend: collaborators.backend; navigation: root } }
        }
    }
    function choose(combo, index) {
        combo.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", true);
        keyClick(Qt.Key_Home);
        for (let i = 0; i < index; i++) keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", false);
    }
    function test_displayModes() {
        const harness = createTemporaryObject(displayHarness, testCase);
        verify(harness);
        const prim = findChild(harness, "displayTargetPrim");
        verify(prim);
        compare(prim.count, 18);
        choose(prim, 17);
        compare(harness.settings.displayTargetPrim, 17);
    }

    function test_allModesAndNumericValues() {
        const harness = createTemporaryObject(displayHarness, testCase);
        const prim = findChild(harness, "displayTargetPrim");
        const transfer = findChild(harness, "displayTargetTrc");
        compare(prim.count, 18);
        compare(transfer.count, 17);
        for (let i = 0; i < 18; i++) { choose(prim, i); compare(harness.settings.displayTargetPrim, i); }
        for (let i = 0; i < 17; i++) { choose(transfer, i); compare(harness.settings.displayTargetTrc, i); }
        const peak = findChild(harness, "displayTargetPeak");
        const peakChoice = findChild(harness, "targetPeakChoice");
        verify(!peak.visible);
        choose(peakChoice, 1);
        compare(harness.settings.displayTargetPeak, 1000);
        verify(peak.visible);
        peak.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Right);
        compare(harness.settings.displayTargetPeak, 1010);
        compare(peak.from, 10); compare(peak.to, 10000); compare(peak.stepSize, 10);
        mouseClick(peak, peak.width - peak.rightPadding, peak.height / 2);
        compare(harness.settings.displayTargetPeak, 10000);
        mouseClick(peak, peak.leftPadding, peak.height / 2);
        compare(harness.settings.displayTargetPeak, 10);
        choose(peakChoice, 0);
        compare(harness.settings.displayTargetPeak, 0);
        verify(!peak.visible);
        const contrast = findChild(harness, "displayTargetContrast");
        const contrastChoice = findChild(harness, "targetContrastChoice");
        verify(!contrast.visible);
        choose(contrastChoice, 1);
        compare(harness.settings.displayTargetContrast, -1);
        verify(!contrast.visible);
        choose(contrastChoice, 2);
        compare(harness.settings.displayTargetContrast, 1000);
        verify(contrast.visible);
        contrast.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Right);
        compare(harness.settings.displayTargetContrast, 2000);
        compare(contrast.from, 10); compare(contrast.to, 1000000); compare(contrast.stepSize, 1000);
        mouseClick(contrast, contrast.width - contrast.rightPadding, contrast.height / 2);
        compare(harness.settings.displayTargetContrast, 1000000);
        mouseClick(contrast, contrast.leftPadding, contrast.height / 2);
        compare(harness.settings.displayTargetContrast, 10);
        choose(contrastChoice, 0);
        compare(harness.settings.displayTargetContrast, 0);
        verify(!contrast.visible);
    }
    function test_noWritesOnConstructionAndProfileUpdates() {
        const harness = createTemporaryObject(displayHarness, testCase, {screenActive: false});
        harness.settings.displayTargetPeak = 0;
        harness.settings.displayTargetContrast = -1;
        harness.settings.changeCount = 0;
        harness.screenActive = true;
        waitForRendering(harness);
        compare(harness.settings.changeCount, 0);
        harness.settings.currentProfile = "Other";
        harness.settings.displayTargetPeak = 3000;
        harness.settings.displayTargetContrast = 9000;
        harness.settings.changeCount = 0;
        waitForRendering(harness);
        compare(findChild(harness, "displayTargetPeak").value, 3000);
        compare(findChild(harness, "displayTargetContrast").value, 9000);
        compare(harness.settings.changeCount, 0);
        findChild(harness, "displayTargetPrim").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        keyClick(Qt.Key_Escape);
        tryCompare(findChild(harness, "displayTargetPrim").popup, "visible", false);
        wait(100);
        compare(harness.closeCount, 0);
        keyClick(Qt.Key_Escape);
        compare(harness.closeCount, 1);
    }
    function test_responsiveDisplay_data() {
        return [{tag: "desktop", width: 1280, height: 720}, {tag: "minimum", width: 900, height: 600}];
    }
    function test_responsiveDisplay(data) {
        const harness = createTemporaryObject(displayHarness, testCase, {width: data.width, height: data.height});
        harness.settings.displayTargetPeak = 1000;
        harness.settings.displayTargetContrast = 1000;
        waitForRendering(harness);
        for (const name of ["displayTargetPrim", "displayTargetTrc", "targetPeakChoice", "displayTargetPeak", "targetContrastChoice", "displayTargetContrast"]) {
            const control = findChild(harness, name);
            control.forceActiveFocus(Qt.TabFocusReason);
            wait(20);
            const point = control.mapToItem(harness, 0, 0);
            verify(point.x >= 0 && point.x + control.width <= harness.width, name + " fits width");
            verify(point.y >= 0 && point.y + control.height <= harness.height, name + " stays visible on focus");
        }
    }
    function test_backRestoresVideoOpener() {
        const harness = createTemporaryObject(navigationHarness, testCase);
        const pages = findChild(harness, "settingsStack");
        mouseClick(findChild(harness, "settingsCategory1"));
        const opener = findChild(harness, "displaySettings");
        opener.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(pages, "depth", 2);
        tryCompare(pages, "busy", false);
        const prim = findChild(harness, "displayTargetPrim");
        prim.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Escape);
        tryCompare(pages, "depth", 1);
        tryCompare(pages, "busy", false);
        tryCompare(opener, "activeFocus", true);
    }
}
