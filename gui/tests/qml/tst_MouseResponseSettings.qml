import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "MouseResponseSettings"
    width: 900; height: 600
    visible: true
    when: windowShown
    Component { id: harnessComponent; SettingsHarness { width: 900; height: 600 } }

    function test_editAndSearchAtMinimumSize() {
        const h = createTemporaryObject(harnessComponent, testCase);
        h.screen.currentCategory = 5;
        const field = findChild(h, "mouseHorizontalSensitivity");
        verify(field, "Mouse response is available in Keyboard and mouse settings");
        verify(!field.enabled);
        const mode = findChild(h, "mouseCameraEnabled");
        mode.forceActiveFocus();
        keyClick(Qt.Key_Space);
        verify(field.enabled);
        field.forceActiveFocus();
        keyClick(Qt.Key_Return);
        keyClick(Qt.Key_A, Qt.ControlModifier);
        field.text = "2.25";
        keyClick(Qt.Key_Tab);
        compare(h.settings.mouseHorizontalSensitivity, 2.25);
        field.forceActiveFocus();
        keyClick(Qt.Key_Return);
        keyClick(Qt.Key_A, Qt.ControlModifier);
        field.text = "999";
        keyClick(Qt.Key_Tab);
        compare(h.settings.mouseHorizontalSensitivity, 2.25);
        compare(field.text, "2.25");
        findChild(h, "keyboardMouseTabs").currentIndex = 1;
        const search = findChild(h, "settingsSearch");
        search.text = "Horizontal sensitivity";
        search.forceActiveFocus();
        keyClick(Qt.Key_Return);
        tryCompare(field, "activeFocus", true);
        compare(findChild(h, "keyboardMouseTabs").currentIndex, 0);
        const point = field.mapToItem(h, 0, 0);
        verify(point.x >= 0 && point.x + field.width <= h.width);
        verify(point.y >= 0 && point.y + field.height <= h.height);
        compare(findChild(h, "keyboardMouseSaved").text, "Saved · Applies the next time you press F8.");
        wait(250);
        compare(mode.checked, true);
        compare(h.settings.mouseCameraEnabled, true);
        waitForRendering(h);
        grabImage(h).save("build/mouse-response-minimum.png");
    }
}
