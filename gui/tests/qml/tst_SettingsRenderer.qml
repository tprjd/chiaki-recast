import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"
import "../../src/qml"

TestCase {
    id: testCase
    name: "SettingsRenderer"
    width: 1280; height: 720
    visible: true
    when: windowShown
    Component {
        id: rendererHarness
        Item {
            id: root
            width: 1280; height: 720
            property alias settings: fakeSettings
            property alias screen: loader.item
            property bool screenActive: true
            property var actions: []
            function record(name) { actions = actions.concat([name]); }
            function closeDialog() { record("close"); }
            function showPlaceboColorMappingDialog() { record("colorMapping"); }
            FakeSettings {
                id: fakeSettings
                function exportPlaceboSettings() { root.record("export"); }
                function importPlaceboSettings() { root.record("import"); }
            }
            QtObject {
                id: fakeBackend
                property var settings: fakeSettings
                function openPlaceboOptionsLink() { root.record("documentation"); return "https://example.invalid/renderer"; }
            }
            Loader {
                id: loader
                anchors.fill: parent
                active: root.screenActive
                sourceComponent: PlaceboSettingsDialog { backend: fakeBackend; navigation: root }
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
            Component.onCompleted: collaborators.backend.window.videoPreset = 5
            StackView {
                id: pages
                objectName: "settingsStack"
                anchors.fill: parent
                initialItem: SettingsDialog { backend: collaborators.backend; navigation: root }
            }
            Component { id: rendererComponent; PlaceboSettingsDialog { backend: collaborators.backend; navigation: root } }
            Component { id: colorComponent; DialogView { title: "Color mapping fixture"; buttonVisible: false; function close() { root.closeDialog(); } Button { text: "Back"; onClicked: root.closeDialog() } } }
        }
    }
    function test_configActionsAndConstruction() {
        const h = createTemporaryObject(rendererHarness, testCase, {screenActive: false});
        verify(h);
        h.settings.changeCount = 0;
        h.screenActive = true;
        wait(20);
        compare(h.settings.changeCount, 0);
        const docs = findChild(h, "openRendererDocumentation");
        verify(docs);
        mouseClick(docs);
        compare(h.actions, ["documentation"]);
        verify(!docs.visible);
        verify(findChild(h, "copyRendererDocumentationUrl").visible);
        verify(!findChild(h, "rendererDocumentationUrl").visible);
        mouseClick(findChild(h, "copyRendererDocumentationUrl"));
        const pasteTarget = createTemporaryQmlObject('import QtQuick; TextInput { width: 100; height: 30 }', testCase);
        pasteTarget.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_V, Qt.ControlModifier);
        compare(pasteTarget.text, "https://example.invalid/renderer");
        pasteTarget.destroy();
        mouseClick(findChild(h, "exportPlaceboSettings"));
        mouseClick(findChild(h, "importPlaceboSettings"));
        compare(h.actions, ["documentation", "export", "import"]);
        keyClick(Qt.Key_Escape);
        compare(h.actions[3], "close");
    }
    function test_groupsFit_data() { return [{tag: "desktop", w: 1280, h: 720}, {tag: "minimum", w: 900, h: 600}]; }
    function test_groupsFit(data) {
        const h = createTemporaryObject(rendererHarness, testCase, {width: data.w, height: data.h});
        h.settings.placeboDebandEnabled = true;
        h.settings.placeboSigmoidEnabled = true;
        h.settings.placeboColorAdjustmentEnabled = true;
        h.settings.placeboPeakDetectionEnabled = true;
        h.settings.placeboColorMappingEnabled = true;
        h.settings.changeCount = 0;
        for (let i = 0; i < 7; ++i) {
            const section = findChild(h, "rendererSection" + i);
            verify(section, "Renderer section " + i);
            mouseClick(section);
            wait(20);
            const group = findChild(h, "rendererGroup" + i);
            verify(group.visible);
            verify(group.width <= h.width);
            for (const row of test_settings_data()) {
                if (row.section !== i) continue;
                const control = findChild(h, row.key);
                if (!control.visible) continue;
                control.forceActiveFocus(Qt.TabFocusReason);
                wait(5);
                const point = control.mapToItem(h, 0, 0);
                verify(point.x >= 0 && point.x + control.width <= h.width + 1, row.key + " width");
                verify(point.y >= 80 && point.y + control.height <= h.height, row.key + " focus");
            }
        }
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
    function enableCustomGroups(h) {
        h.settings.placeboDebandEnabled = true;
        h.settings.placeboSigmoidEnabled = true;
        h.settings.placeboColorAdjustmentEnabled = true;
        h.settings.placeboPeakDetectionEnabled = true;
        h.settings.placeboColorMappingEnabled = true;
    }
    function test_settings_data() {
        return [
            {"tag": "placeboUpscaler", "key": "placeboUpscaler", "section": 1, "kind": "ComboBox", "options": ["Custom", "Nearest", "Bilinear", "Oversample", "Bicubic", "Gaussian", "Catmull Rom", "Lanczos", "EwaLanczos", "EwaLanczosSharp", "EwaLanczosSharpest", "FSR", "FSRCNNX x2 8-0-4-1", "FSRCNNX x2 16-0-4-1"]},
            {"tag": "placeboPlaneUpscaler", "key": "placeboPlaneUpscaler", "section": 1, "kind": "ComboBox", "options": ["Chosen Upscaler", "Nearest", "Bilinear", "Oversample", "Bicubic", "Gaussian", "Catmull Rom", "Lanczos", "EwaLanczos", "EwaLanczosSharp", "EwaLanczosSharpest"]},
            {"tag": "placeboDownscaler", "key": "placeboDownscaler", "section": 1, "kind": "ComboBox", "options": ["Custom", "Box", "Hermite", "Bilinear", "Bicubic", "Gaussian", "Catmull Rom", "Mitchell", "Lanczos"]},
            {"tag": "placeboPlaneDownscaler", "key": "placeboPlaneDownscaler", "section": 1, "kind": "ComboBox", "options": ["Chosen Downscaler", "Box", "Hermite", "Bilinear", "Bicubic", "Gaussian", "Catmull Rom", "Mitchell", "Lanczos"]},
            {"tag": "placeboFrameMixer", "key": "placeboFrameMixer", "section": 1, "kind": "ComboBox", "options": ["None", "Oversample", "Hermite", "Linear", "Cubic"]},
            {"tag": "placeboAntiringingStrength", "key": "placeboAntiringingStrength", "section": 1, "kind": "Slider", "from": 0.0, "to": 1.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboDebandEnabled", "key": "placeboDebandEnabled", "section": 2, "kind": "CheckBox"},
            {"tag": "placeboDebandPreset", "key": "placeboDebandPreset", "section": 2, "kind": "ComboBox", "options": ["Custom", "Default"]},
            {"tag": "placeboDebandIterations", "key": "placeboDebandIterations", "section": 2, "kind": "Slider", "from": 0.0, "to": 16.0, "stepSize": 1.0, "decimals": 0},
            {"tag": "placeboDebandThreshold", "key": "placeboDebandThreshold", "section": 2, "kind": "Slider", "from": 0.0, "to": 1000.0, "stepSize": 0.1, "decimals": 1},
            {"tag": "placeboDebandRadius", "key": "placeboDebandRadius", "section": 2, "kind": "Slider", "from": 0.0, "to": 1000.0, "stepSize": 0.1, "decimals": 1},
            {"tag": "placeboDebandGrain", "key": "placeboDebandGrain", "section": 2, "kind": "Slider", "from": 0.0, "to": 1000.0, "stepSize": 0.1, "decimals": 1},
            {"tag": "placeboSigmoidEnabled", "key": "placeboSigmoidEnabled", "section": 3, "kind": "CheckBox"},
            {"tag": "placeboSigmoidPreset", "key": "placeboSigmoidPreset", "section": 3, "kind": "ComboBox", "options": ["Custom", "Default"]},
            {"tag": "placeboSigmoidCenter", "key": "placeboSigmoidCenter", "section": 3, "kind": "Slider", "from": 0.0, "to": 1.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboSigmoidSlope", "key": "placeboSigmoidSlope", "section": 3, "kind": "Slider", "from": 1.0, "to": 20.0, "stepSize": 0.1, "decimals": 1},
            {"tag": "placeboColorAdjustmentEnabled", "key": "placeboColorAdjustmentEnabled", "section": 4, "kind": "CheckBox"},
            {"tag": "placeboColorAdjustmentPreset", "key": "placeboColorAdjustmentPreset", "section": 4, "kind": "ComboBox", "options": ["Custom", "Neutral"]},
            {"tag": "placeboColorAdjustmentBrightness", "key": "placeboColorAdjustmentBrightness", "section": 4, "kind": "Slider", "from": -1.0, "to": 1.0, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboColorAdjustmentContrast", "key": "placeboColorAdjustmentContrast", "section": 4, "kind": "Slider", "from": 0.0, "to": 100.0, "stepSize": 0.1, "decimals": 1},
            {"tag": "placeboColorAdjustmentSaturation", "key": "placeboColorAdjustmentSaturation", "section": 4, "kind": "Slider", "from": 0.0, "to": 100.0, "stepSize": 0.1, "decimals": 1},
            {"tag": "placeboColorAdjustmentHue", "key": "placeboColorAdjustmentHue", "section": 4, "kind": "Slider", "from": 0.0, "to": 6.28, "stepSize": 0.01, "decimals": 2},
            {"tag": "placeboColorAdjustmentGamma", "key": "placeboColorAdjustmentGamma", "section": 4, "kind": "Slider", "from": 0.0, "to": 100.0, "stepSize": 0.1, "decimals": 1},
            {"tag": "placeboColorAdjustmentTemperature", "key": "placeboColorAdjustmentTemperature", "section": 4, "kind": "Slider", "from": -1.143, "to": 5.286, "stepSize": 0.001, "decimals": 3},
            {"tag": "placeboPeakDetectionEnabled", "key": "placeboPeakDetectionEnabled", "section": 5, "kind": "CheckBox"},
            {"tag": "placeboPeakDetectionPreset", "key": "placeboPeakDetectionPreset", "section": 5, "kind": "ComboBox", "options": ["Custom", "Default", "High Quality"]},
            {"tag": "placeboPeakDetectionPeakSmoothingPeriod", "key": "placeboPeakDetectionPeakSmoothingPeriod", "section": 5, "kind": "Slider", "from": 0.0, "to": 1000.0, "stepSize": 0.1, "decimals": 1},
            {"tag": "placeboPeakDetectionSceneThresholdLow", "key": "placeboPeakDetectionSceneThresholdLow", "section": 5, "kind": "Slider", "from": 0.0, "to": 100.0, "stepSize": 0.1, "decimals": 1},
            {"tag": "placeboPeakDetectionSceneThresholdHigh", "key": "placeboPeakDetectionSceneThresholdHigh", "section": 5, "kind": "Slider", "from": 0.0, "to": 100.0, "stepSize": 0.1, "decimals": 1},
            {"tag": "placeboPeakDetectionPeakPercentile", "key": "placeboPeakDetectionPeakPercentile", "section": 5, "kind": "Slider", "from": 0.0, "to": 100.0, "stepSize": 0.001, "decimals": 3},
            {"tag": "placeboPeakDetectionBlackCutoff", "key": "placeboPeakDetectionBlackCutoff", "section": 5, "kind": "Slider", "from": 0.0, "to": 100.0, "stepSize": 0.1, "decimals": 1},
            {"tag": "placeboPeakDetectionAllowDelayedPeak", "key": "placeboPeakDetectionAllowDelayedPeak", "section": 5, "kind": "CheckBox"},
            {"tag": "placeboColorMappingEnabled", "key": "placeboColorMappingEnabled", "section": 6, "kind": "CheckBox"},
            {"tag": "placeboColorMappingPreset", "key": "placeboColorMappingPreset", "section": 6, "kind": "ComboBox", "options": ["Custom", "Default", "High Quality"]}
        ];
    }
    function test_settings(data) {
        const h = createTemporaryObject(rendererHarness, testCase, {width: 900, height: 600});
        enableCustomGroups(h);
        mouseClick(findChild(h, "rendererSection" + data.section));
        wait(20);
        const control = findChild(h, data.key);
        verify(control && control.visible, data.key);
        control.forceActiveFocus(Qt.TabFocusReason);
        wait(10);
        const point = control.mapToItem(h, 0, 0);
        verify(point.x >= 0 && point.x + control.width <= h.width + 1, data.key + " horizontal bounds");
        verify(point.y >= 80 && point.y + control.height <= h.height, data.key + " focus bounds");
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
            compare(findChild(h, data.key + "Value").text, Number(h.settings[data.key]).toFixed(data.decimals));
        }
    }
    function test_presetsAndConditions_data() {
        return [
            {tag: "deband", section: 2, enabled: "placeboDebandEnabled", preset: "placeboDebandPreset", custom: "placeboDebandIterations"},
            {tag: "sigmoid", section: 3, enabled: "placeboSigmoidEnabled", preset: "placeboSigmoidPreset", custom: "placeboSigmoidCenter"},
            {tag: "color", section: 4, enabled: "placeboColorAdjustmentEnabled", preset: "placeboColorAdjustmentPreset", custom: "placeboColorAdjustmentBrightness"},
            {tag: "peak", section: 5, enabled: "placeboPeakDetectionEnabled", preset: "placeboPeakDetectionPreset", custom: "placeboPeakDetectionPeakPercentile"},
            {tag: "mapping", section: 6, enabled: "placeboColorMappingEnabled", preset: "placeboColorMappingPreset", custom: "openPlaceboColorMapping"}
        ];
    }
    function test_presetsAndConditions(data) {
        const h = createTemporaryObject(rendererHarness, testCase);
        mouseClick(findChild(h, "rendererSection" + data.section));
        wait(20);
        const enabled = findChild(h, data.enabled);
        const preset = findChild(h, data.preset);
        const custom = findChild(h, data.custom);
        verify(!preset.visible && !custom.visible);
        mouseClick(enabled);
        verify(preset.visible && custom.visible);
        choose(preset, 1);
        verify(!custom.visible);
        choose(preset, 0);
        verify(custom.visible);
        enabled.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Space);
        verify(!preset.visible && !custom.visible);
    }
    function test_popupCancellationAndProfileRefresh() {
        const h = createTemporaryObject(rendererHarness, testCase);
        mouseClick(findChild(h, "rendererSection1"));
        const upscaler = findChild(h, "placeboUpscaler");
        upscaler.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(upscaler.popup, "visible", true);
        keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Escape);
        tryCompare(upscaler.popup, "visible", false);
        compare(h.actions.length, 0);
        compare(h.settings.placeboUpscaler, 0);
        h.settings.placeboUpscaler = 10;
        h.settings.placeboColorAdjustmentTemperature = 0.333;
        h.settings.currentProfile = "Other profile";
        h.settings.changeCount = 0;
        wait(20);
        compare(upscaler.currentIndex, 10);
        compare(findChild(h, "placeboColorAdjustmentTemperature").value, 0.333);
        compare(h.settings.changeCount, 0);
        keyClick(Qt.Key_PageDown);
        verify(findChild(h, "rendererGroup2").visible);
        keyClick(Qt.Key_PageUp);
        verify(findChild(h, "rendererGroup1").visible);
    }
    function test_colorMappingRoute() {
        const h = createTemporaryObject(rendererHarness, testCase);
        h.settings.placeboColorMappingEnabled = true;
        mouseClick(findChild(h, "rendererSection6"));
        wait(20);
        mouseClick(findChild(h, "openPlaceboColorMapping"));
        compare(h.actions, ["colorMapping"]);
    }
    function test_returnRestoresCallers() {
        const h = createTemporaryObject(navigationHarness, testCase);
        const pages = findChild(h, "settingsStack");
        mouseClick(findChild(h, "settingsCategory1"));
        const opener = findChild(h, "customRendererSettings");
        opener.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(pages, "depth", 2);
        tryCompare(pages, "busy", false);
        mouseClick(findChild(h, "rendererSection6"));
        const enabled = findChild(h, "placeboColorMappingEnabled");
        mouseClick(enabled);
        const colorOpener = findChild(h, "openPlaceboColorMapping");
        colorOpener.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(pages, "depth", 3);
        tryCompare(pages, "busy", false);
        keyClick(Qt.Key_Escape);
        tryCompare(pages, "depth", 2);
        tryCompare(pages, "busy", false);
        tryCompare(colorOpener, "activeFocus", true);
        keyClick(Qt.Key_Escape);
        tryCompare(pages, "depth", 1);
        tryCompare(pages, "busy", false);
        tryCompare(opener, "activeFocus", true);
    }
}
