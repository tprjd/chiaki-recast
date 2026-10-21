import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "KeyboardMouseBindingsLayout"
    width: 900; height: 600
    visible: true
    when: windowShown
    Component { id: harnessComponent; SettingsHarness { width: 900; height: 600 } }

    function isVisibleIn(item, viewport) {
        const point = item.mapToItem(viewport, 0, 0);
        return point.x >= -1 && point.x + item.width <= viewport.width + 1
            && point.y >= -1 && point.y + item.height <= viewport.height + 1;
    }

    function test_longLabelsAndVisibleKeyboardSearchFocus() {
        const h = createTemporaryObject(harnessComponent, testCase);
        const rows = [
            {action: 4, name: "Cross jump action with a long translated description", group: 1,
                primary: Qt.Key_Space, alternative: -Qt.MiddleButton,
                primaryName: "Space keyboard button with a long input name",
                alternativeName: "Middle mouse button with a long input name"},
            {action: 5, name: "Circle crouch action with a long translated description", group: 1,
                primary: Qt.Key_Control, alternative: -Qt.BackButton,
                primaryName: "Control keyboard button with a long input name",
                alternativeName: "Back mouse button with a long input name"},
            {action: 21, name: "PS system menu with a long translated description", group: 4,
                primary: Qt.Key_F1, alternative: -Qt.ForwardButton,
                primaryName: "F1 keyboard button with a long input name",
                alternativeName: "Forward mouse button with a long input name"}
        ];
        h.settings.keyboardMouseBindings = rows;
        h.settings.mouseCameraEnabled = true;
        h.screen.currentCategory = 5;
        const tabs = findChild(h, "keyboardMouseTabs");
        tabs.currentIndex = 1;
        const flick = findChild(h, "keysFlick");
        tryVerify(() => findChild(h, "keyboardMouseBinding4_0") !== null);
        waitForRendering(h);

        let primaryX = -1;
        let alternativeX = -1;
        for (const row of rows) {
            const primary = findChild(h, "keyboardMouseBinding" + row.action + "_0");
            const alternative = findChild(h, "keyboardMouseBinding" + row.action + "_1");
            const label = primary.parent.children.find(item => item.text === row.name);
            verify(label, "The action label is present");
            for (const item of [label, primary, alternative]) {
                const point = item.mapToItem(flick, 0, 0);
                verify(item.width > 0 && point.x >= -1 && point.x + item.width <= flick.width + 1,
                    row.name + " stays within the settings columns");
            }
            verify(label.x + label.width <= primary.x + 1);
            verify(primary.x + primary.width <= alternative.x + 1);
            if (primaryX < 0) {
                primaryX = primary.x;
                alternativeX = alternative.x;
            }
            fuzzyCompare(primary.x, primaryX, 1);
            fuzzyCompare(alternative.x, alternativeX, 1);
            primary.forceActiveFocus(Qt.TabFocusReason);
            tryVerify(() => isVisibleIn(primary, flick));
            keyClick(Qt.Key_Tab);
            tryCompare(alternative, "activeFocus", true);
            tryVerify(() => isVisibleIn(alternative, flick));
        }

        tabs.currentIndex = 0;
        const search = findChild(h, "settingsSearch");
        search.text = rows[2].name + " alternative binding";
        search.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryVerify(() => findChild(h, "keyboardMouseBinding21_1") !== null);
        const target = findChild(h, "keyboardMouseBinding21_1");
        tryCompare(tabs, "currentIndex", 1);
        tryCompare(target, "activeFocus", true);
        tryCompare(target, "visualFocus", true);
        tryVerify(() => isVisibleIn(target, flick));
        verify(!findChild(h, "keyboardMouseBindingDialog").opened);
        wait(250);
        waitForRendering(h);
        grabImage(h).save("build/keyboard-mouse-bindings-minimum.png");
    }
}
