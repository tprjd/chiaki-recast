import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"
import "../../src/qml"

TestCase {
    id: tests
    name: "InformationDialogs"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    Component { id: settingsComponent; SettingsHarness { width: tests.width; height: tests.height } }
    Component { id: rendererLoader; Loader { source: "../../src/qml/RendererFallbackDialog.qml" } }
    Button { id: fallbackFocus; text: "Current view" }
    property int behindClicks: 0
    Button { id: behind; anchors.centerIn: parent; text: "Behind"; onClicked: tests.behindClicks++ }
    QtObject { id: navigation; function restoreMainFocus() { fallbackFocus.forceActiveFocus(Qt.TabFocusReason); } }
    function test_rendererReportsActualFallback() {
        const loader = createTemporaryObject(rendererLoader, tests);
        tryCompare(loader, "status", Loader.Ready);
        const dialog = loader.item;
        dialog.reason = "Synthetic device does not support the required Vulkan extension.";
        dialog.restoreFocusItem = behind;
        dialog.navigation = navigation;
        dialog.open();
        tryCompare(dialog, "opened", true);
        const message = findChild(dialog.contentItem, "confirmationMessage");
        verify(message.text.includes("switched to OpenGL"));
        verify(message.text.includes(dialog.reason));
        compare(message.textFormat, Text.PlainText);
        const close = findChild(dialog.footer, "confirmationCancel");
        compare(close.text, "Close");
        verify(!findChild(dialog.footer, "confirmationAction").visible);
        verify(!findChild(dialog.footer, "confirmationSecondaryAction").visible);
        tryCompare(close, "activeFocus", true);
        verify(close.visualFocus);
        const count = behindClicks;
        mouseClick(behind);
        compare(behindClicks, count);
        verify(dialog.visible);
        mouseClick(close);
        tryCompare(dialog, "visible", false);
        tryCompare(behind, "activeFocus", true);
    }
    function test_aboutClosesAndRestoresFocus() {
        verify(Qt.application.version.length > 0, "The test supplies an application version");
        const harness = createTemporaryObject(settingsComponent, tests);
        harness.screen.currentCategory = 8;
        const opener = findChild(harness, "aboutSettings");
        verify(opener);
        opener.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        const dialog = findChild(harness, "aboutDialog");
        tryCompare(dialog, "opened", true);
        const close = findChild(dialog.footer, "confirmationCancel");
        verify(close, "About has one explicit Close action");
        compare(close.text, "Close");
        tryCompare(close, "activeFocus", true);
        verify(close.visualFocus);
        keyClick(Qt.Key_Return);
        tryCompare(dialog, "visible", false);
        tryCompare(opener, "activeFocus", true);
    }
    function test_supportedLayoutsAndDismissal_data() {
        const rows = [];
        for (const kind of ["about", "renderer"])
            for (const size of [{width: 900, height: 600}, {width: 1280, height: 720}])
                for (const input of ["escape", "controller", "enter", "click"])
                    rows.push({tag: kind + "-" + size.width + "-" + input, kind: kind, width: size.width, height: size.height, input: input});
        return rows;
    }
    function test_supportedLayoutsAndDismissal(data) {
        tests.width = data.width; tests.height = data.height;
        let dialog;
        let opener;
        if (data.kind === "about") {
            const harness = createTemporaryObject(settingsComponent, tests);
            harness.screen.currentCategory = 8;
            opener = findChild(harness, "aboutSettings");
            opener.forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Return);
            dialog = findChild(harness, "aboutDialog");
            const message = findChild(dialog.contentItem, "confirmationMessage");
            for (const phrase of ["chiaki-ng", "Street Pea", "version " + Qt.application.version, "Chiaki", "Florian Markl", "2.1.1", "GNU Affero General Public License version 3", "Free Software Foundation", "WITHOUT ANY WARRANTY", "MERCHANTABILITY", "FITNESS FOR A PARTICULAR PURPOSE", "GNU General Public License"])
                verify(message.text.includes(phrase), "About retains " + phrase);
            const logo = findChild(dialog.contentItem, "informationLogo");
            tryCompare(logo, "status", Image.Ready);
            verify(logo.visible);
        } else {
            const loader = createTemporaryObject(rendererLoader, tests);
            tryCompare(loader, "status", Loader.Ready);
            dialog = loader.item;
            dialog.reason = "Synthetic device does not support the required Vulkan extension.";
            opener = fallbackFocus;
            dialog.restoreFocusItem = opener;
            dialog.open();
        }
        tryCompare(dialog, "opened", true);
        if (data.input === "click") {
            waitForRendering(tests);
            grabImage(tests).save(Qt.resolvedUrl("../../../build/information-" + data.kind + "-" + data.width + "-normal.png").toString().replace("file:///", ""));
        }
        dialog.title += " with a longer translated information title";
        const previousHeight = dialog.contentItem.contentHeight;
        dialog.text += " Long translated information remains available to read.".repeat(120);
        tryVerify(() => dialog.contentItem.contentHeight > previousHeight);
        const close = findChild(dialog.footer, "confirmationCancel");
        tryCompare(close, "activeFocus", true);
        verify(close.visualFocus);
        verify(dialog.width <= data.width - 32);
        verify(dialog.height <= data.height - 32);
        const point = close.mapToItem(tests, 0, 0);
        verify(point.x >= 0 && point.x + close.width <= tests.width);
        verify(point.y >= 0 && point.y + close.height <= tests.height);
        const scroll = dialog.contentItem;
        tryVerify(() => scroll.contentHeight > scroll.height, 5000, "Long information can scroll");
        scroll.contentItem.contentY = scroll.contentHeight - scroll.height;
        waitForRendering(tests);
        if (data.input === "click") {
            const path = Qt.resolvedUrl("../../../build/information-" + data.kind + "-" + data.width + ".png").toString().replace("file:///", "");
            grabImage(tests).save(path);
        }
        if (data.input === "escape") keyClick(Qt.Key_Escape);
        else if (data.input === "controller") keyClick(Qt.Key_Yes);
        else if (data.input === "enter") keyClick(Qt.Key_Return);
        else mouseClick(close);
        tryCompare(dialog, "visible", false);
        tryCompare(opener, "activeFocus", true);
    }
    function test_aboutReopensAndRestoresFocusAgain() {
        const harness = createTemporaryObject(settingsComponent, tests);
        harness.screen.currentCategory = 8;
        const opener = findChild(harness, "aboutSettings");
        opener.forceActiveFocus(Qt.TabFocusReason);
        for (let i = 0; i < 2; i++) {
            keyClick(Qt.Key_Return);
            const dialog = findChild(harness, "aboutDialog");
            tryCompare(dialog, "opened", true);
            keyClick(Qt.Key_Escape);
            tryCompare(dialog, "visible", false);
            tryCompare(opener, "activeFocus", true);
            tryCompare(opener, "visualFocus", true);
        }
    }
    function test_longNoticeCanScrollWithKeyboard() {
        const loader = createTemporaryObject(rendererLoader, tests);
        tryCompare(loader, "status", Loader.Ready);
        const dialog = loader.item;
        dialog.reason = "Long fallback reason. ".repeat(200);
        dialog.open();
        tryCompare(dialog, "opened", true);
        const scroll = dialog.contentItem;
        tryVerify(() => scroll.contentHeight > scroll.height);
        keyClick(Qt.Key_Up);
        tryCompare(scroll, "activeFocus", true);
        verify(scroll.visualFocus);
        keyClick(Qt.Key_Down);
        keyClick(Qt.Key_PageDown);
        tryVerify(() => scroll.contentItem.contentY > 0, 5000, "Keyboard can scroll the reason");
        keyClick(Qt.Key_Yes);
        tryCompare(dialog, "visible", false);
    }
    function test_missingOpenerUsesCurrentViewFocus() {
        const loader = createTemporaryObject(rendererLoader, tests);
        tryCompare(loader, "status", Loader.Ready);
        const dialog = loader.item;
        dialog.navigation = navigation;
        dialog.restoreFocusItem = behind;
        dialog.open();
        tryCompare(dialog, "opened", true);
        behind.enabled = false;
        keyClick(Qt.Key_Escape);
        tryCompare(dialog, "visible", false);
        tryCompare(fallbackFocus, "activeFocus", true);
        behind.enabled = true;
    }

}
