import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"
import "../../src/qml"

TestCase {
    id: testCase
    name: "SettingsSidebar"
    width: 1280; height: 720
    visible: true
    when: windowShown
    Component { id: mainHarness; SettingsHarness {} }
    Component {
        id: linkedHarness
        Item {
            width: 900; height: 600
            property alias screen: loader.item
            property Component page
            SettingsHarness { id: collaborators; visible: false; screenActive: false }
            Loader { id: loader; anchors.fill: parent; sourceComponent: page }
            Component { id: renderer; PlaceboSettingsDialog { backend: collaborators.backend; navigation: collaborators } }
            Component { id: color; PlaceboColorMappingDialog { backend: collaborators.backend; navigation: collaborators } }
            property Component rendererPage: renderer
            property Component colorPage: color
        }
    }
    function categoryHeading(item, text) {
        if (item.text === text && item.font && item.font.pixelSize === 24) return item;
        for (const child of item.children || []) {
            const heading = categoryHeading(child, text);
            if (heading) return heading;
        }
        return null;
    }
    function test_categoryHeadings() {
        const h = createTemporaryObject(mainHarness, testCase, {width: 900, height: 600});
        for (const page of [{index: 6, title: "Controllers"}, {index: 8, title: "Config"}]) {
            findChild(h, "settingsCategory" + page.index).forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Return);
            const heading = categoryHeading(h.screen, page.title);
            verify(heading && heading.visible, page.title + " category heading is visible");
            const point = heading.mapToItem(h, 0, 0);
            verify(point.y >= 80 && point.y + heading.height <= h.height, "Heading fits the page");
        }
    }
    function test_longColorCheckbox() {
        const h = createTemporaryObject(linkedHarness, testCase, {width: 900, height: 600});
        h.page = h.colorPage;
        findChild(h, "colorMappingSection1").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        const box = findChild(h, "placeboGamutMappingLut3dTricubicEnabled");
        box.text = "[QA] Enable Tricubic Interpolation for 3DLUTs extra extra extra";
        box.forceActiveFocus(Qt.TabFocusReason);
        wait(20);
        verify(box.contentItem.lineCount > 1, "Complete long checkbox text wraps");
        verify(box.contentItem.height >= box.contentItem.paintedHeight, "Checkbox text fits");
        const point = box.mapToItem(h, 0, 0);
        verify(point.y >= 80 && point.y + box.height <= h.height, "Checkbox stays reachable");
        const before = box.checked;
        keyClick(Qt.Key_Space);
        compare(box.checked, !before);
    }
    function test_longNames_data() {
        const rows = [];
        for (const size of [{w: 900, h: 600}, {w: 1280, h: 720}]) {
            for (const page of [{page: "main", prefix: "settingsCategory", count: 9},
                                {page: "renderer", prefix: "rendererSection", count: 7},
                                {page: "color", prefix: "colorMappingSection", count: 5}])
                rows.push({tag: page.page + size.w, page: page.page, prefix: page.prefix, count: page.count, w: size.w, h: size.h});
        }
        return rows;
    }
    function test_longNames(data) {
        const h = createTemporaryObject(data.page === "main" ? mainHarness : linkedHarness, testCase, {width: data.w, height: data.h});
        if (data.page !== "main") h.page = data.page === "renderer" ? h.rendererPage : h.colorPage;
        wait(20);
        for (let i = 0; i < data.count; ++i) {
            const button = findChild(h, data.prefix + i);
            // A translated name can contain long words and need several lines.
            button.text = "Expanded translated category name " + "Sigmoidization".repeat(3);
        }
        wait(20);
        for (let i = 0; i < data.count; ++i) {
            const button = findChild(h, data.prefix + i);
            button.forceActiveFocus(Qt.TabFocusReason);
            wait(20);
            verify(!button.contentItem.truncated, data.prefix + i + " shows the complete name");
            verify(button.contentItem.lineCount > 1, "Long names wrap");
            verify(button.contentItem.height >= button.contentItem.paintedHeight, "All text lines fit");
            const point = button.mapToItem(h, 0, 0);
            verify(point.y >= 80 && point.y + button.height <= h.height, data.prefix + i + " remains reachable");
            keyClick(Qt.Key_Return);
            if (data.page === "main") compare(h.screen.currentCategory, i);
            else compare(h.screen.currentSection, i);
        }
    }
}
