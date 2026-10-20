import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"
import "../../src/qml"

TestCase {
    id: testCase
    name: "SettingsColorMapping"
    width: 1280; height: 720
    visible: true
    when: windowShown
    Component {
        id: colorHarness
        Item {
            id: root
            width: 1280; height: 720
            property alias settings: fakeSettings
            property alias screen: loader.item
            property bool screenActive: true
            property int closes: 0
            function closeDialog() { closes++; }
            FakeSettings { id: fakeSettings }
            QtObject { id: fakeBackend; property var settings: fakeSettings }
            Loader {
                id: loader
                anchors.fill: parent
                active: root.screenActive
                sourceComponent: PlaceboColorMappingDialog { backend: fakeBackend; navigation: root }
            }
        }
    }
    Component {
        id: navigationHarness
        Item {
            id: root
            width: 900; height: 600
            function closeDialog() { pages.pop(); }
            function showPlaceboSettingsDialog() { pages.push(rendererComponent); }
            function showPlaceboColorMappingDialog() { pages.push(colorComponent); }
            SettingsHarness { id: collaborators; visible: false; screenActive: false }
            Component.onCompleted: {
                collaborators.backend.window.videoPreset = 5;
                collaborators.settings.placeboColorMappingEnabled = true;
            }
            StackView {
                id: pages
                objectName: "settingsStack"
                anchors.fill: parent
                initialItem: SettingsDialog { backend: collaborators.backend; navigation: root }
            }
            Component { id: rendererComponent; PlaceboSettingsDialog { backend: collaborators.backend; navigation: root } }
            Component { id: colorComponent; PlaceboColorMappingDialog { backend: collaborators.backend; navigation: root } }
        }
    }
    function test_constructionAndBack() {
        const h = createTemporaryObject(colorHarness, testCase, {screenActive: false});
        h.settings.placeboGamutMappingFunction = 1;
        h.settings.placeboToneMappingFunction = 1;
        h.settings.placeboGamutMappingPerceptualDeadzone = 0.33;
        h.settings.changeCount = 0;
        h.screenActive = true;
        wait(20);
        compare(h.settings.changeCount, 0);
        const func = findChild(h, "placeboGamutMappingFunction");
        verify(func);
        func.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Escape);
        compare(h.closes, 1);
    }
    function test_sectionsAndFinalValue_data() {
        return [{tag: "desktop", w: 1280, h: 720}, {tag: "minimum", w: 900, h: 600}];
    }
    function test_sectionsAndFinalValue(data) {
        const h = createTemporaryObject(colorHarness, testCase, {width: data.w, height: data.h});
        h.settings.placeboGamutMappingFunction = 1;
        h.settings.placeboToneMappingFunction = 1;
        h.settings.changeCount = 0;
        for (let i = 0; i < 5; ++i) {
            const section = findChild(h, "colorMappingSection" + i);
            verify(section, "Color mapping section " + i);
            mouseClick(section);
            wait(20);
            verify(findChild(h, "colorMappingGroup" + i).visible);
            for (const row of test_settings_data()) {
                if (row.section !== i) continue;
                const control = findChild(h, row.key);
                if (!control.visible) continue;
                control.forceActiveFocus(Qt.TabFocusReason);
                assertVisibleBounds(h, control, row.key);
                if (row.kind === "Slider") assertVisibleBounds(h, findChild(h, row.key + "Value"), row.key + " value");
            }
        }
        findChild(h, "placeboToneMappingContrastRecovery").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Down);
        tryCompare(findChild(h, "placeboToneMappingContrastSmoothness"), "activeFocus", true);
        const value = findChild(h, "placeboToneMappingContrastSmoothnessValue");
        const viewport = findChild(h, "colorMappingFlick");
        const point = value.mapToItem(viewport, 0, 0);
        verify(point.y >= 0 && point.y + value.height <= viewport.height);
        compare(h.settings.changeCount, 0);
    }
    function choose(combo, index) {
        combo.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", true);
        keyClick(Qt.Key_Home);
        for (let i = 0; i < index; ++i) keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", false);
    }
    function test_settings_data() {
        return [
            {"tag": "placeboGamutMappingFunction", "key": "placeboGamutMappingFunction", "section": 0, "kind": "ComboBox", "options": ["Clip", "Perceptual", "Soft Clip", "Relative", "Saturation", "Absolute", "Desaturate", "Darken", "Highlight", "Linear"]},
            {"tag": "placeboGamutMappingPerceptualDeadzone", "key": "placeboGamutMappingPerceptualDeadzone", "section": 0, "kind": "Slider", "functions": [1], "from": 0.0, "to": 1.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboGamutMappingPerceptualStrength", "key": "placeboGamutMappingPerceptualStrength", "section": 0, "kind": "Slider", "functions": [1], "from": 0.0, "to": 1.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboGamutMappingColorimetricGamma", "key": "placeboGamutMappingColorimetricGamma", "section": 0, "kind": "Slider", "functions": [3, 5, 7], "from": 0.0, "to": 10.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboGamutMappingSoftClipKnee", "key": "placeboGamutMappingSoftClipKnee", "section": 0, "kind": "Slider", "functions": [1, 2], "from": 0.0, "to": 1.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboGamutMappingSoftClipDesat", "key": "placeboGamutMappingSoftClipDesat", "section": 0, "kind": "Slider", "functions": [2], "from": 0.0, "to": 1.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboGamutMappingLut3dSizeI", "key": "placeboGamutMappingLut3dSizeI", "section": 1, "kind": "Slider", "from": 0.0, "to": 1024.0, "stepSize": 1.0, "decimals": 0},
            {"tag": "placeboGamutMappingLut3dSizeC", "key": "placeboGamutMappingLut3dSizeC", "section": 1, "kind": "Slider", "from": 0.0, "to": 1024.0, "stepSize": 1.0, "decimals": 0},
            {"tag": "placeboGamutMappingLut3dSizeH", "key": "placeboGamutMappingLut3dSizeH", "section": 1, "kind": "Slider", "from": 0.0, "to": 1024.0, "stepSize": 1.0, "decimals": 0},
            {"tag": "placeboGamutMappingLut3dTricubicEnabled", "key": "placeboGamutMappingLut3dTricubicEnabled", "section": 1, "kind": "CheckBox"},
            {"tag": "placeboGamutMappingGamutExpansionEnabled", "key": "placeboGamutMappingGamutExpansionEnabled", "section": 1, "kind": "CheckBox"},
            {"tag": "placeboToneMappingFunction", "key": "placeboToneMappingFunction", "section": 2, "kind": "ComboBox", "options": ["Clip", "Spline", "St2094-40", "St-2094-10", "Bt2390", "Bt2446a", "Reinhard", "Mobius", "Hable", "Gamma", "Linear", "Linear Light"]},
            {"tag": "placeboToneMappingKneeAdaptation", "key": "placeboToneMappingKneeAdaptation", "section": 2, "kind": "Slider", "functions": [1, 2, 3], "from": 0.0, "to": 1.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboToneMappingKneeMinimum", "key": "placeboToneMappingKneeMinimum", "section": 2, "kind": "Slider", "functions": [1, 2, 3], "from": 0.0, "to": 0.5, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboToneMappingKneeMaximum", "key": "placeboToneMappingKneeMaximum", "section": 2, "kind": "Slider", "functions": [1, 2, 3], "from": 0.5, "to": 1.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboToneMappingKneeDefault", "key": "placeboToneMappingKneeDefault", "section": 2, "kind": "Slider", "from": 0.0, "to": 1.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboToneMappingKneeOffset", "key": "placeboToneMappingKneeOffset", "section": 2, "kind": "Slider", "functions": [4], "from": 0.5, "to": 2.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboToneMappingSlopeTuning", "key": "placeboToneMappingSlopeTuning", "section": 3, "kind": "Slider", "functions": [1], "from": 0.0, "to": 10.0, "stepSize": 0.1, "decimals": 1},
            {"tag": "placeboToneMappingSlopeOffset", "key": "placeboToneMappingSlopeOffset", "section": 3, "kind": "Slider", "functions": [1], "from": 0.0, "to": 1.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboToneMappingSplineContrast", "key": "placeboToneMappingSplineContrast", "section": 3, "kind": "Slider", "functions": [1], "from": 0.0, "to": 1.5, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboToneMappingReinhardContrast", "key": "placeboToneMappingReinhardContrast", "section": 3, "kind": "Slider", "functions": [6], "from": 0.0, "to": 1.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboToneMappingLinearKnee", "key": "placeboToneMappingLinearKnee", "section": 3, "kind": "Slider", "functions": [7, 9], "from": 0.0, "to": 1.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboToneMappingExposure", "key": "placeboToneMappingExposure", "section": 3, "kind": "Slider", "functions": [10, 11], "from": 0.0, "to": 10.0, "stepSize": 0.1, "decimals": 1},
            {"tag": "placeboToneMappingInverseToneMappingEnabled", "key": "placeboToneMappingInverseToneMappingEnabled", "section": 4, "kind": "CheckBox"},
            {"tag": "placeboToneMappingMetadata", "key": "placeboToneMappingMetadata", "section": 4, "kind": "ComboBox", "options": ["Any", "None", "HDR10", "HDR10 Plus", "Cie_y"]},
            {"tag": "placeboToneMappingToneLutSize", "key": "placeboToneMappingToneLutSize", "section": 4, "kind": "Slider", "from": 0.0, "to": 4096.0, "stepSize": 1.0, "decimals": 0},
            {"tag": "placeboToneMappingContrastRecovery", "key": "placeboToneMappingContrastRecovery", "section": 4, "kind": "Slider", "from": 0.0, "to": 2.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboToneMappingContrastSmoothness", "key": "placeboToneMappingContrastSmoothness", "section": 4, "kind": "Slider", "from": 1.0, "to": 32.0, "stepSize": 0.1, "decimals": 1}
        ];
    }
    function assertVisibleBounds(h, control, name) {
        const viewport = findChild(h, "colorMappingFlick");
        const point = control.mapToItem(viewport, 0, 0);
        verify(point.x >= 0 && point.x + control.width <= viewport.width + 1, name + " width");
        verify(point.y >= 0 && point.y + control.height <= viewport.height + 1, name + " height");
    }
    function test_settings(data) {
        const h = createTemporaryObject(colorHarness, testCase, {width: 900, height: 600});
        if (data.functions) {
            const key = data.section < 2 ? "placeboGamutMappingFunction" : "placeboToneMappingFunction";
            h.settings[key] = data.functions[0];
        }
        mouseClick(findChild(h, "colorMappingSection" + data.section));
        wait(20);
        const control = findChild(h, data.key);
        verify(control && control.visible, data.key);
        control.forceActiveFocus(Qt.TabFocusReason);
        wait(10);
        assertVisibleBounds(h, control, data.key);
        if (data.kind === "ComboBox") {
            compare(control.count, data.options.length);
            for (let i = 0; i < data.options.length; ++i) {
                compare(control.textAt(i), data.options[i]);
                choose(control, i);
                compare(h.settings[data.key], i);
            }
        } else if (data.kind === "CheckBox") {
            const original = h.settings[data.key];
            keyClick(Qt.Key_Space);
            compare(h.settings[data.key], !original);
            keyClick(Qt.Key_Space);
            compare(h.settings[data.key], original);
        } else {
            compare(control.from, data.from);
            compare(control.to, data.to);
            compare(control.stepSize, data.stepSize);
            mouseClick(control, control.width - control.rightPadding, control.height / 2);
            verify(Math.abs(h.settings[data.key] - data.to) < 0.00001);
            mouseClick(control, control.leftPadding, control.height / 2);
            verify(Math.abs(h.settings[data.key] - data.from) < 0.00001);
            keyClick(Qt.Key_Right);
            verify(Math.abs(h.settings[data.key] - (data.from + data.stepSize)) < 0.00001);
            const value = findChild(h, data.key + "Value");
            compare(value.text, Number(h.settings[data.key]).toFixed(data.decimals));
            assertVisibleBounds(h, value, data.key + " displayed value");
        }
    }
    function test_functionConditions_data() {
        return [{tag: "gamut", section: 0, key: "placeboGamutMappingFunction", count: 10},
                {tag: "tone", section: 2, key: "placeboToneMappingFunction", count: 12}];
    }
    function test_functionConditions(data) {
        const h = createTemporaryObject(colorHarness, testCase, {width: 900, height: 600});
        for (let i = 0; i < data.count; ++i) {
            mouseClick(findChild(h, "colorMappingSection" + data.section));
            wait(10);
            h.settings.changeCount = 0;
            choose(findChild(h, data.key), i);
            compare(h.settings.changeCount, i === 0 ? 0 : 1);
            for (const row of test_settings_data()) {
                if (row.section !== data.section && !(data.section === 2 && row.section === 3)) continue;
                mouseClick(findChild(h, "colorMappingSection" + row.section));
                wait(5);
                const control = findChild(h, row.key);
                const applicable = !row.functions || row.functions.indexOf(i) >= 0;
                compare(control.visible, applicable, row.key + " for function " + i);
                if (applicable) {
                    control.forceActiveFocus(Qt.TabFocusReason);
                    assertVisibleBounds(h, control, row.key);
                    if (row.kind === "Slider") assertVisibleBounds(h, findChild(h, row.key + "Value"), row.key + " value");
                }
            }
            compare(h.settings.changeCount, i === 0 ? 0 : 1);
        }
    }
    function test_popupCancellationAndProfileRefresh() {
        const h = createTemporaryObject(colorHarness, testCase);
        const func = findChild(h, "placeboGamutMappingFunction");
        func.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(func.popup, "visible", true);
        keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Escape);
        tryCompare(func.popup, "visible", false);
        compare(h.settings.placeboGamutMappingFunction, 0);
        compare(h.closes, 0);
        h.settings.placeboGamutMappingFunction = 1;
        h.settings.placeboGamutMappingPerceptualDeadzone = 0.42;
        h.settings.currentProfile = "Other profile";
        h.settings.changeCount = 0;
        wait(20);
        compare(func.currentIndex, 1);
        compare(findChild(h, "placeboGamutMappingPerceptualDeadzone").value, 0.42);
        keyClick(Qt.Key_PageDown);
        verify(findChild(h, "colorMappingGroup1").visible);
        keyClick(Qt.Key_PageUp);
        verify(findChild(h, "colorMappingGroup0").visible);
        compare(h.settings.changeCount, 0);
    }
    function test_backRestoresRendererAndVideo() {
        const h = createTemporaryObject(navigationHarness, testCase);
        const pages = findChild(h, "settingsStack");
        mouseClick(findChild(h, "settingsCategory1"));
        const rendererOpener = findChild(h, "customRendererSettings");
        rendererOpener.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(pages, "depth", 2);
        tryCompare(pages, "busy", false);
        mouseClick(findChild(h, "rendererSection6"));
        const colorOpener = findChild(h, "openPlaceboColorMapping");
        colorOpener.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(pages, "depth", 3);
        tryCompare(pages, "busy", false);
        findChild(h, "placeboGamutMappingFunction").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Escape);
        tryCompare(pages, "depth", 2);
        tryCompare(pages, "busy", false);
        tryCompare(colorOpener, "activeFocus", true);
        keyClick(Qt.Key_Escape);
        tryCompare(pages, "depth", 1);
        tryCompare(pages, "busy", false);
        tryCompare(rendererOpener, "activeFocus", true);
    }
}
