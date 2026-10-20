import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "SettingsSearch"
    width: 1280; height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; SettingsHarness {} }
    Component { id: linkedHarnessComponent; SettingsSearchHarness {} }
    function searchFor(h, query) {
        const search = findChild(h, "settingsSearch");
        search.text = query;
        wait(10);
        verify(findChild(h, "settingsSearchResults").count > 0, query);
        search.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
    }
    function test_linkedDisplayAndNestedColorRouting() {
        const h = createTemporaryObject(linkedHarnessComponent, testCase);
        h.backend.window.videoPreset = 5;
        h.settings.placeboColorMappingEnabled = true;
        h.settings.placeboColorMappingPreset = 0;
        h.settings.changeCount = 0;
        searchFor(h, "Target peak value");
        tryCompare(h.pages, "depth", 2);
        tryCompare(findChild(h.pages.currentItem, "targetPeakChoice"), "activeFocus", true);
        compare(h.settings.displayTargetPeak, 0);
        keyClick(Qt.Key_Escape);
        tryCompare(h.pages, "depth", 1);
        tryCompare(findChild(h.pages.currentItem, "displaySettings"), "activeFocus", true);
        searchFor(h, "LUT 3D size h");
        tryCompare(h.pages, "depth", 3);
        tryCompare(h.pages.currentItem, "objectName", "placeboColorMappingDialog");
        tryCompare(findChild(h.pages.currentItem, "placeboGamutMappingLut3dSizeH"), "activeFocus", true);
        keyClick(Qt.Key_Escape);
        tryCompare(h.pages, "depth", 2);
        tryCompare(findChild(h.pages.currentItem, "openPlaceboColorMapping"), "activeFocus", true);
        keyClick(Qt.Key_Escape);
        tryCompare(h.pages, "depth", 1);
        tryCompare(findChild(h.pages.currentItem, "customRendererSettings"), "activeFocus", true);
        compare(h.settings.changeCount, 0);
    }
    function test_controllerSearchRetainsSelectionAndDoesNotCaptureTarget() {
        const h = createTemporaryObject(linkedHarnessComponent, testCase);
        searchFor(h, "MIC alternate input");
        const capture = findChild(h, "controllerCapture");
        tryCompare(capture, "opened", true);
        compare(h.pages.depth, 1);
        compare(h.actions.filter(action => action.name === "beginMapping").length, 1);
        h.backend.currentControllerMapping = [{buttonValue: 1073741824, buttonName: "MIC", physicalButton: ["PRIVATE_INPUT", "PRIVATE_ALT"]}];
        h.backend.controllerMappingInProgress = true;
        tryCompare(h.pages, "depth", 2);
        tryCompare(findChild(h.pages.currentItem, "controllerMap1073741824Slot1"), "activeFocus", true);
        verify(!findChild(h.pages.currentItem, "controllerButtonCapture").visible);
        compare(h.actions.filter(action => action.name === "selectButton").length, 0);
        keyClick(Qt.Key_Escape);
        tryCompare(h.pages, "depth", 1);
        tryCompare(findChild(h.pages.currentItem, "controllerMappingChange"), "activeFocus", true);
    }
    function test_keyboardEmptyClearAndCategorySelection() {
        const h = createTemporaryObject(harnessComponent, testCase);
        const search = findChild(h, "settingsSearch");
        const results = findChild(h, "settingsSearchResults");
        search.text = "   ";
        verify(!results.visible);
        search.text = "no_such_setting";
        tryCompare(results, "count", 0);
        verify(findChild(h, "settingsSearchEmpty").visible);
        search.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        compare(search.text, "no_such_setting");
        keyClick(Qt.Key_Escape);
        compare(search.text, "");
        compare(h.actions.filter(action => action.name === "close").length, 0);
        search.text = "shortcut";
        keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Down);
        compare(results.currentIndex, 1);
        keyClick(Qt.Key_Up);
        compare(results.currentIndex, 0);
        keyClick(Qt.Key_Up);
        tryCompare(search, "activeFocus", true);
        keyClick(Qt.Key_PageDown);
        compare(h.screen.currentCategory, 0);
        mouseClick(findChild(h, "settingsCategory8"));
        compare(search.text, "");
        compare(h.screen.currentCategory, 8);
        search.text = "latency";
        mouseClick(findChild(h, "clearSettingsSearch"));
        compare(search.text, "");
        compare(h.screen.currentCategory, 8);
        tryCompare(search, "activeFocus", true);
    }
    function selectTarget(h, query, target) {
        if (h.pages) tryVerify(() => !h.pages.busy);
        const search = findChild(h, "settingsSearch");
        search.text = query;
        wait(1);
        const entries = h.pages ? h.pages.currentItem.searchResults : h.screen.searchResults;
        const index = entries.findIndex(row => row.target === target);
        verify(index >= 0, target + " is indexed");
        const results = findChild(h, "settingsSearchResults");
        results.positionViewAtIndex(index, ListView.Contain);
        wait(10);
        const button = findChild(results, "settingsSearchResult" + index);
        verify(button, target + " has a result");
        mouseClick(button);
    }
    function assertInViewport(h, target) {
        wait(10);
        const point = target.mapToItem(h, 0, 0);
        verify(target.visible && target.enabled, target.objectName + " is available");
        verify(point.x >= 0 && point.x + target.width <= h.width + 1, target.objectName + " fits width");
        verify(point.y >= 0 && point.y + target.height <= h.height + 1, target.objectName + " fits height");
    }
    function test_fullMainInventoryAndKeyboardTargets_data() {
        return [{tag: "desktop", width: 1280, height: 720}, {tag: "minimum", width: 900, height: 600}];
    }
    function test_fullMainInventoryAndKeyboardTargets(data) {
        const h = createTemporaryObject(harnessComponent, testCase, {width: data.width, height: data.height});
        h.settings.steamDeckHaptics = false;
        h.settings.verticalDeck = false;
        h.settings.speechProcessing = true;
        h.settings.streamMenuEnabled = true;
        h.settings.dpadTouchEnabled = true;
        h.settings.windowType = 1;
        h.backend.window.videoPreset = 5;
        h.settings.registeredHosts = [{mac: "PRIVATE_MAC", name: "PRIVATE_CONSOLE", ps5: true}];
        h.backend.hiddenHosts = [{mac: "PRIVATE_HIDDEN_MAC", name: "PRIVATE_HIDDEN_CONSOLE", ps5: true}];
        const keys = [];
        for (let i = 0; i < 26; ++i) keys.push({buttonValue: Math.pow(2,i), buttonName: "Key target " + i, keyName: "PRIVATE_KEY"});
        h.settings.controllerMapping = keys;
        h.settings.changeCount = 0;
        h.settings.actions = [];
        const targets = [
            "disconnectAction", "suspendAction", "audioVideoDisabled", "steamDeckHaptics", "verticalDeck", "streamerMode", "openLogDirectory", "streamMenuEnabled",
            "streamMenuShortcut1", "streamMenuShortcut2", "streamMenuShortcut3", "streamMenuShortcut4",
            "decoder", "useZeroCopy", "windowType", "customResolutionWidth", "customResolutionHeight", "fullscreenDoubleClick", "hideCursor", "vSyncEnabled", "videoPreset", "queueDepthLimit", "rendererBackend", "customRendererSettings", "displaySettings", "consoleSelection",
            "audioOutDevice", "audioInDevice", "audioBufferSize", "audioVolume", "startMicUnmuted", "speechProcessing", "noiseSuppressLevel", "echoSuppressLevel", "wifiDroppedNotif", "packetLossReportedMax", "iDROnFECFailureEnabled", "showStreamStats",
            "registerConsole", "registeredConsoleInfo0", "autoConnectConsole0", "deleteRegisteredConsole0", "hiddenConsoleInfo0", "unhideConsole0", "keyboardEnabled", "mouseTouchEnabled", "resetAllKeys",
            "controllerMappingChange", "controllerMappingReset", "allowJoystickBackgroundEvents", "buttonsByPosition", "dpadTouchEnabled", "dpadTouchIncrement", "dpadTouchShortcut1", "dpadTouchShortcut2", "dpadTouchShortcut3", "dpadTouchShortcut4", "rumbleHapticsIntensity", "hapticOverride",
            "openPsnLogin", "portGuessingEnabled", "portGuessCount", "portGuessSocketCount", "manageProfiles", "exportSettings", "importSettings", "logSanitize", "logVerbose", "aboutSettings"
        ];
        for (const console of ["PS4", "PS5"]) for (const route of ["Local", "Remote"])
            for (const name of console === "PS5" ? ["resolution", "fps", "bitrate", "codec"] : ["resolution", "fps", "bitrate"])
                targets.push(name + route + console);
        for (const key of keys) targets.push("keyMapping" + key.buttonValue);
        for (const target of targets) {
            const search = findChild(h, "settingsSearch");
            const terms = ["General", "Video", "Stream", "Audio/Wifi", "Consoles", "Keys", "Controllers", "Remote", "Config"];
            let query = "";
            for (const term of terms) {
                search.text = term;
                if (h.screen.searchResults.some(row => row.target === target)) { query = term; break; }
            }
            verify(query.length > 0, target + " appears in a category search");
            selectTarget(h, query, target);
            const item = findChild(h.screen, target);
            tryCompare(item, "activeFocus", true);
            assertInViewport(h, item);
            const valueLabel = findChild(h.screen, target + "Value");
            if (valueLabel && valueLabel.visible) assertInViewport(h, valueLabel);
            if (target.startsWith("bitrate")) assertInViewport(h, findChild(h.screen, "bitrateDescription" + target.substring(7)));
        }
        compare(h.settings.changeCount, 0);
        compare(h.settings.actions.filter(action => action.name !== "refreshAudioDevices").length, 0);
        compare(h.actions.length, 0);
        for (const index of [0, 8]) {
            const category = findChild(h, "settingsCategory" + index);
            category.forceActiveFocus(Qt.TabFocusReason);
            assertInViewport(h, category);
        }
    }
    function test_liveConditionsAndPrivacy() {
        const h = createTemporaryObject(harnessComponent, testCase);
        const search = findChild(h, "settingsSearch");
        search.text = "Steam Deck haptics";
        compare(h.screen.searchResults.length, 0);
        h.settings.steamDeckHaptics = false;
        tryCompare(findChild(h, "settingsSearchResults"), "count", 1);
        h.settings.steamDeckHaptics = undefined;
        tryCompare(findChild(h, "settingsSearchResults"), "count", 0);
        searchFor(h, "Noise suppression level");
        tryCompare(findChild(h, "speechProcessing"), "activeFocus", true);
        compare(h.settings.speechProcessing, false);
        h.settings.speechProcessing = undefined;
        search.text = "noise suppression level";
        compare(h.screen.searchResults.length, 0);
        searchFor(h, "Custom resolution height");
        tryCompare(findChild(h, "windowType"), "activeFocus", true);
        compare(h.settings.windowType, 3);
        h.settings.currentProfile = "UNIQUE_PRIVATE_PROFILE";
        h.settings.profiles = ["default", "UNIQUE_PRIVATE_PROFILE", "create new profile"];
        h.settings.registeredHosts = [{mac: "UNIQUE_PRIVATE_MAC", name: "UNIQUE_PRIVATE_CONSOLE"}];
        h.backend.hiddenHosts = [{mac: "UNIQUE_PRIVATE_HIDDEN_MAC", name: "UNIQUE_PRIVATE_HIDDEN"}];
        h.settings.decoder = "UNIQUE_PRIVATE_DECODER";
        h.settings.audioOutDevice = "UNIQUE_PRIVATE_AUDIO";
        h.settings.logDirectory = "UNIQUE_PRIVATE_PATH";
        h.settings.psnAuthToken = "UNIQUE_PRIVATE_AUTH";
        h.settings.psnRefreshToken = "UNIQUE_PRIVATE_REFRESH";
        h.settings.psnAuthTokenExpiry = "UNIQUE_PRIVATE_EXPIRY";
        h.settings.psnAccountId = "UNIQUE_PRIVATE_ACCOUNT";
        h.settings.controllerMapping = [{buttonName: "Cross", buttonValue: 1, keyName: "UNIQUE_PRIVATE_KEY"}];
        h.backend.currentControllerMapping = [{buttonName: "Cross", buttonValue: 1, physicalButton: ["UNIQUE_PRIVATE_INPUT", "UNIQUE_PRIVATE_ALT"]}];
        for (const mode of [false, true]) {
            h.settings.streamerMode = mode;
            h.settings.changeCount = 0;
            search.text = "UNIQUE_PRIVATE";
            compare(h.screen.searchResults.length, 0);
            for (const query of ["General", "Video", "Consoles", "Keys", "Controllers", "Remote", "Config"]) {
                search.text = query;
                for (const entry of h.screen.searchResults)
                    verify(JSON.stringify(entry).indexOf("UNIQUE_PRIVATE") < 0);
                const results = findChild(h, "settingsSearchResults");
                for (let i = 0; i < Math.min(results.count, 3); ++i) {
                    const button = findChild(results, "settingsSearchResult" + i);
                    if (!button) continue;
                    verify(button.Accessible.name.indexOf("UNIQUE_PRIVATE") < 0);
                    verify(button.Accessible.description.indexOf("UNIQUE_PRIVATE") < 0);
                }
            }
            compare(h.settings.changeCount, 0);
        }
        searchFor(h, "Clear PSN tokens");
        tryCompare(findChild(h, "resetPsnTokens"), "activeFocus", true);
        h.settings.psnAccountId = "";
        searchFor(h, "Clear PSN tokens");
        tryCompare(findChild(h, "openPsnLogin"), "activeFocus", true);
    }
    function test_fullLinkedInventory_data() {
        return [{tag: "desktop", width: 1280, height: 720}, {tag: "minimum", width: 900, height: 600}];
    }
    function test_fullLinkedInventory(data) {
        const h = createTemporaryObject(linkedHarnessComponent, testCase, {width: data.width, height: data.height});
        h.backend.window.videoPreset = 5;
        for (const group of ["Deband", "Sigmoid", "ColorAdjustment", "PeakDetection", "ColorMapping"]) {
            h.settings["placebo" + group + "Enabled"] = true;
            h.settings["placebo" + group + "Preset"] = 0;
        }
        h.settings.displayTargetPeak = 1000;
        h.settings.displayTargetContrast = 1000;
        const targets = [
            "displayTargetPrim", "displayTargetTrc", "targetPeakChoice", "displayTargetPeak", "targetContrastChoice", "displayTargetContrast",
            "openRendererDocumentation", "copyRendererDocumentationUrl", "exportPlaceboSettings", "importPlaceboSettings",
            "placeboUpscaler", "placeboPlaneUpscaler", "placeboDownscaler", "placeboPlaneDownscaler", "placeboFrameMixer", "placeboAntiringingStrength",
            "placeboDebandEnabled", "placeboDebandPreset", "placeboDebandIterations", "placeboDebandThreshold", "placeboDebandRadius", "placeboDebandGrain",
            "placeboSigmoidEnabled", "placeboSigmoidPreset", "placeboSigmoidCenter", "placeboSigmoidSlope",
            "placeboColorAdjustmentEnabled", "placeboColorAdjustmentPreset", "placeboColorAdjustmentBrightness", "placeboColorAdjustmentContrast", "placeboColorAdjustmentSaturation", "placeboColorAdjustmentHue", "placeboColorAdjustmentGamma", "placeboColorAdjustmentTemperature",
            "placeboPeakDetectionEnabled", "placeboPeakDetectionPreset", "placeboPeakDetectionPeakSmoothingPeriod", "placeboPeakDetectionSceneThresholdLow", "placeboPeakDetectionSceneThresholdHigh", "placeboPeakDetectionPeakPercentile", "placeboPeakDetectionBlackCutoff", "placeboPeakDetectionAllowDelayedPeak",
            "placeboColorMappingEnabled", "placeboColorMappingPreset", "openPlaceboColorMapping",
            "placeboGamutMappingFunction", "placeboGamutMappingPerceptualDeadzone", "placeboGamutMappingPerceptualStrength", "placeboGamutMappingColorimetricGamma", "placeboGamutMappingSoftClipKnee", "placeboGamutMappingSoftClipDesat",
            "placeboGamutMappingLut3dSizeI", "placeboGamutMappingLut3dSizeC", "placeboGamutMappingLut3dSizeH", "placeboGamutMappingLut3dTricubicEnabled", "placeboGamutMappingGamutExpansionEnabled",
            "placeboToneMappingFunction", "placeboToneMappingKneeAdaptation", "placeboToneMappingKneeMinimum", "placeboToneMappingKneeMaximum", "placeboToneMappingKneeDefault", "placeboToneMappingKneeOffset",
            "placeboToneMappingSlopeTuning", "placeboToneMappingSlopeOffset", "placeboToneMappingSplineContrast", "placeboToneMappingReinhardContrast", "placeboToneMappingLinearKnee", "placeboToneMappingExposure",
            "placeboToneMappingInverseToneMappingEnabled", "placeboToneMappingMetadata", "placeboToneMappingToneLutSize", "placeboToneMappingContrastRecovery", "placeboToneMappingContrastSmoothness"
        ];
        for (const target of targets) {
            h.settings.placeboGamutMappingFunction = target === "placeboGamutMappingColorimetricGamma" ? 3 : target === "placeboGamutMappingSoftClipDesat" ? 2 : 1;
            h.settings.placeboToneMappingFunction = target === "placeboToneMappingKneeOffset" ? 4 : target === "placeboToneMappingReinhardContrast" ? 6 : target === "placeboToneMappingLinearKnee" ? 7 : target === "placeboToneMappingExposure" ? 10 : 1;
            h.settings.changeCount = 0;
            selectTarget(h, "Video", target);
            tryVerify(() => h.pages.depth > 1 && !h.pages.busy);
            const expected = target === "copyRendererDocumentationUrl" ? "openRendererDocumentation" : target;
            tryCompare(findChild(h.pages.currentItem, expected), "activeFocus", true);
            assertInViewport(h, findChild(h.pages.currentItem, expected));
            const valueLabel = findChild(h.pages.currentItem, expected + "Value");
            if (valueLabel && valueLabel.visible) assertInViewport(h, valueLabel);
            compare(h.settings.changeCount, 0, target + " search does not write");
            while (h.pages.depth > 1) { keyClick(Qt.Key_Escape); tryVerify(() => !h.pages.busy); }
        }
        for (const target of ["profileSelection", "newProfileName", "deleteProfile", "profileAction"]) {
            selectTarget(h, "Config", target);
            tryCompare(h.pages, "depth", 2);
            tryCompare(findChild(h.pages.currentItem, "profileSelection"), "activeFocus", true);
            keyClick(Qt.Key_Escape);
            tryCompare(h.pages, "depth", 1);
            tryCompare(findChild(h.pages.currentItem, "manageProfiles"), "activeFocus", true);
        }
        compare(h.actions.length, 0);
    }
    function test_linkedPrerequisitesAndProfileChanges() {
        const h = createTemporaryObject(linkedHarnessComponent, testCase, {width: 900, height: 600});
        h.settings.changeCount = 0;
        searchFor(h, "Perceptual deadzone");
        compare(h.pages.depth, 1);
        tryCompare(findChild(h.pages.currentItem, "videoPreset"), "activeFocus", true);
        h.backend.window.videoPreset = 5;
        h.settings.placeboColorMappingEnabled = false;
        h.settings.changeCount = 0;
        searchFor(h, "Perceptual deadzone");
        tryCompare(h.pages, "depth", 2);
        tryCompare(findChild(h.pages.currentItem, "placeboColorMappingEnabled"), "activeFocus", true);
        compare(h.settings.changeCount, 0);
        keyClick(Qt.Key_Escape);
        tryCompare(h.pages, "depth", 1);
        h.settings.placeboColorMappingEnabled = true;
        h.settings.placeboColorMappingPreset = 0;
        h.settings.placeboGamutMappingFunction = 0;
        h.settings.changeCount = 0;
        searchFor(h, "Perceptual deadzone");
        tryCompare(h.pages, "depth", 3);
        tryCompare(findChild(h.pages.currentItem, "placeboGamutMappingFunction"), "activeFocus", true);
        compare(h.settings.placeboGamutMappingFunction, 0);
        compare(h.settings.changeCount, 0);
        h.settings.placeboColorMappingEnabled = false;
        keyClick(Qt.Key_Escape);
        tryCompare(h.pages, "depth", 2);
        tryCompare(findChild(h.pages.currentItem, "placeboColorMappingEnabled"), "activeFocus", true);
        keyClick(Qt.Key_Escape);
        tryCompare(h.pages, "depth", 1);
        const search = findChild(h, "settingsSearch");
        h.settings.controllerMapping = [{buttonValue: 1, buttonName: "Cross", keyName: "old"}];
        search.text = "Keyboard key for Cross";
        compare(h.pages.currentItem.searchResults.length, 1);
        h.settings.currentProfile = "Changed profile";
        h.settings.controllerMapping = [{buttonValue: 2, buttonName: "Moon", keyName: "new"}];
        tryCompare(findChild(h, "settingsSearchResults"), "count", 0);
        search.text = "Keyboard key for Moon";
        search.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(findChild(h.pages.currentItem, "keyMapping2"), "activeFocus", true);
    }
    function test_profileRoutes() {
        const h = createTemporaryObject(linkedHarnessComponent, testCase, {width: 900, height: 600});
        for (const target of ["profileSelection", "newProfileName", "deleteProfile", "profileAction"]) {
            selectTarget(h, "Config", target);
            tryCompare(h.pages, "depth", 2);
            keyClick(Qt.Key_Escape);
            tryCompare(h.pages, "depth", 1);
        }
    }
    function test_searchExplainsOuterPrerequisitesAndNativeNames() {
        const h = createTemporaryObject(harnessComponent, testCase);
        const search = findChild(h, "settingsSearch");
        search.text = "Renderer";
        verify(h.screen.searchResults.some(row => row.target === "placeboUpscaler"), "Linked page names are searchable context");
        search.text = "Perceptual deadzone";
        verify(h.screen.searchResults[0].description.indexOf("Custom render preset") >= 0);
        h.backend.window.videoPreset = 5;
        h.settings.placeboColorMappingEnabled = false;
        verify(h.screen.searchResults[0].description.indexOf("Color mapping enabled") >= 0);
        for (const query of ["Audio/Video", "Use zero copy", "Start mic unmuted", "Request IDR frame on FEC failure", "Login to PSN", "Force STUN port guessing", "Export settings to file"]) {
            search.text = query;
            verify(h.screen.searchResults.length > 0, query + " uses the native setting name");
        }
        search.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Escape);
        keyClick(Qt.Key_A);
        keyClick(Qt.Key_U);
        keyClick(Qt.Key_D);
        keyClick(Qt.Key_I);
        keyClick(Qt.Key_O);
        compare(search.text, "audio");
        verify(h.screen.searchResults.length > 0);
        keyClick(Qt.Key_Tab);
        tryCompare(findChild(h, "clearSettingsSearch"), "activeFocus", true);
    }
    function test_allControllerPrimaryAndAlternateRoutes() {
        const h = createTemporaryObject(linkedHarnessComponent, testCase, {width: 900, height: 600});
        const mapping = [];
        const nativeIds = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072,
            67108864, 134217728, 268435456, 536870912, 1073741824];
        for (let i = 0; i < nativeIds.length; ++i)
            mapping.push({buttonValue: nativeIds[i], buttonName: "Target " + i, physicalButton: ["PRIVATE_PHYSICAL", "PRIVATE_ALTERNATE"]});
        h.backend.currentControllerMapping = mapping;
        h.settings.changeCount = 0;
        for (const row of mapping) for (let slot = 0; slot < 2; ++slot) {
            const target = "controllerMap" + row.buttonValue + "Slot" + slot;
            selectTarget(h, "Controllers", target);
            tryCompare(findChild(h, "controllerCapture"), "opened", true);
            h.backend.controllerMappingInProgress = true;
            tryCompare(h.pages, "depth", 2);
            tryCompare(findChild(h.pages.currentItem, target), "activeFocus", true);
            assertInViewport(h, findChild(h.pages.currentItem, target));
            verify(!findChild(h.pages.currentItem, "controllerButtonCapture").visible);
            compare(h.actions.filter(action => action.name === "selectButton").length, 0);
            keyClick(Qt.Key_Escape);
            tryCompare(h.pages, "depth", 1);
        }
        compare(h.settings.changeCount, 0);
        compare(h.actions.filter(action => action.name === "updateButton" || action.name === "applyMapping").length, 0);
    }
    function test_mappingCancelSteamProfileAndDelayedModel() {
        const h = createTemporaryObject(linkedHarnessComponent, testCase);
        for (const cancel of ["cancel", "steam", "profile"]) {
            searchFor(h, "MIC alternate input");
            tryCompare(findChild(h, "controllerCapture"), "opened", true);
            if (cancel === "steam") {
                h.backend.controllerMappingSteamControllerSelected();
                tryCompare(findChild(h, "steamControllerNotice"), "opened", true);
                keyClick(Qt.Key_Escape);
            } else if (cancel === "profile") h.settings.currentProfile = "Other profile";
            else keyClick(Qt.Key_Escape);
            tryCompare(findChild(h, "controllerCapture"), "visible", false);
            compare(h.pages.depth, 1);
            tryCompare(findChild(h.pages.currentItem, "controllerMappingChange"), "activeFocus", true);
        }
        searchFor(h, "Cross primary input");
        tryCompare(findChild(h, "controllerCapture"), "opened", true);
        h.backend.controllerMappingInProgress = true;
        tryCompare(h.pages, "depth", 2);
        h.backend.currentControllerMapping = [{buttonValue: 1, buttonName: "Cross", physicalButton: ["A", "B"]}];
        tryCompare(findChild(h.pages.currentItem, "controllerMap1Slot0"), "activeFocus", true);
        keyClick(Qt.Key_Return);
        tryCompare(findChild(h.pages.currentItem, "controllerButtonCapture"), "opened", true);
        h.backend.controllerMappingButtonSelected("new input");
        tryCompare(findChild(h.pages.currentItem, "controllerButtonCapture"), "visible", false);
        compare(h.actions.filter(action => action.name === "updateButton").length, 1);
        compare(h.actions.filter(action => action.name === "updateButton")[0].args[0], 1);
    }
    function test_searchPreservesNativeActionsAndEdits() {
        const h = createTemporaryObject(harnessComponent, testCase);
        searchFor(h, "Audio volume");
        const beforeVolume = h.settings.audioVolume;
        keyClick(Qt.Key_Left);
        verify(h.settings.audioVolume < beforeVolume);
        searchFor(h, "PS5 Remote Bitrate");
        keyClick(Qt.Key_Right);
        verify(h.settings.bitrateRemotePS5 > 0);
        compare(h.settings.bitrateLocalPS5, 0);
        compare(h.settings.bitrateRemotePS4, 0);
        h.settings.actions = [];
        searchFor(h, "Export settings to file");
        compare(h.settings.actions.length, 0);
        keyClick(Qt.Key_Return);
        compare(h.settings.actions[0].name, "exportSettings");
        searchFor(h, "Registered console information");
        tryCompare(findChild(h, "registeredConsolesEmpty"), "activeFocus", true);
        searchFor(h, "Unhide console");
        tryCompare(findChild(h, "hiddenConsolesEmpty"), "activeFocus", true);
        h.settings.registeredHosts = [{mac: "A", name: "First"}, {mac: "B", name: "Second"}];
        const search = findChild(h, "settingsSearch");
        search.text = "Delete registered console";
        h.settings.registeredHosts = [{mac: "B", name: "Second"}, {mac: "A", name: "First"}];
        search.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(findChild(h, "deleteRegisteredConsole0"), "activeFocus", true);
        compare(h.actions.filter(action => action.name === "confirm").length, 0);
        keyClick(Qt.Key_Return);
        const confirmation = h.actions.filter(action => action.name === "confirm")[0];
        verify(confirmation);
        h.settings.registeredHosts = [{mac: "A", name: "First"}, {mac: "B", name: "Second"}];
        confirmation.args[2]();
        compare(h.settings.actions[h.settings.actions.length - 1].args[0], 1);
    }
    function test_conditionChangeKeepsUsableSearchFocus() {
        const h = createTemporaryObject(harnessComponent, testCase);
        const search = findChild(h, "settingsSearch");
        search.text = "Speech processing";
        search.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Down);
        verify(findChild(h, "settingsSearchResult0").activeFocus);
        h.settings.speechProcessing = undefined;
        tryCompare(search, "activeFocus", true);
        compare(h.screen.currentCategory, 0);
        keyClick(Qt.Key_Escape);
        compare(search.text, "");
        compare(h.actions.filter(action => action.name === "close").length, 0);
    }
    function test_mappingControlsAndMissingTarget() {
        const h = createTemporaryObject(linkedHarnessComponent, testCase);
        h.backend.currentControllerMapping = [{buttonValue: 2, buttonName: "Moon", physicalButton: ["A", "B"]}];
        for (const row of [
                ["Analog stick and L2/R2 mapping", "enableAnalogStickMapping", false],
                ["Apply controller mapping", "controllerMappingApply", true],
                ["Cancel controller mapping", "controllerMappingCancel", false],
                ["Cross primary input", "enableAnalogStickMapping", false],
                ["Apply controller mapping", "enableAnalogStickMapping", false]]) {
            tryVerify(() => !h.pages.busy);
            h.backend.controllerMappingAltered = row[2];
            searchFor(h, row[0]);
            tryCompare(findChild(h, "controllerCapture"), "opened", true);
            h.backend.controllerMappingInProgress = true;
            tryCompare(h.pages, "depth", 2);
            tryCompare(findChild(h.pages.currentItem, row[1]), "activeFocus", true);
            compare(h.actions.filter(action => action.name === "selectButton" || action.name === "applyMapping").length, 0);
            keyClick(Qt.Key_Escape);
            tryCompare(h.pages, "depth", 1);
        }
    }
    function test_consoleActionUsesFirstCurrentEligibleNativeControl() {
        const h = createTemporaryObject(harnessComponent, testCase);
        const duplicateHosts = [{mac: "DUP", name: "Duplicate1"}, {mac: "DUP", name: "Duplicate2"}, {mac: "VALID", name: "Eligible"}];
        h.settings.registeredHosts = duplicateHosts;
        h.backend.hiddenHosts = duplicateHosts;
        h.settings.changeCount = 0;
        for (const row of [["Auto-connect console", "autoConnectConsole2"], ["Delete registered console", "deleteRegisteredConsole2"], ["Unhide console", "unhideConsole2"]]) {
            searchFor(h, row[0]);
            tryCompare(findChild(h, row[1]), "activeFocus", true);
        }
        compare(h.actions.length, 0);
        compare(h.settings.changeCount, 0);
        const search = findChild(h, "settingsSearch");
        search.text = "Delete registered console";
        h.settings.registeredHosts = [{mac: "OTHER", name: "Replacement"}];
        search.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(findChild(h, "deleteRegisteredConsole0"), "activeFocus", true);
        compare(h.actions.length, 0);
    }
    function test_noArgumentRendererEntryAndOrdinaryReturnKeepSection() {
        const h = createTemporaryObject(linkedHarnessComponent, testCase, {width: 900, height: 600});
        h.backend.window.videoPreset = 5;
        findChild(h, "settingsCategory1").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        findChild(h, "customRendererSettings").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(h.pages, "depth", 2);
        tryVerify(() => !h.pages.busy);
        compare(h.pages.currentItem.currentSection, 0);
        findChild(h.pages.currentItem, "rendererSection1").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        const opener = findChild(h.pages.currentItem, "placeboAntiringingStrength");
        opener.forceActiveFocus(Qt.TabFocusReason);
        h.showDisplaySettingsDialog();
        tryCompare(h.pages, "depth", 3);
        tryVerify(() => !h.pages.busy);
        keyClick(Qt.Key_Escape);
        tryCompare(h.pages, "depth", 2);
        tryVerify(() => !h.pages.busy);
        compare(h.pages.currentItem.currentSection, 1);
        tryCompare(opener, "activeFocus", true);
        h.settings.placeboDebandEnabled = true;
        h.settings.placeboDebandPreset = 0;
        findChild(h.pages.currentItem, "rendererSection2").forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        findChild(h.pages.currentItem, "placeboDebandIterations").forceActiveFocus(Qt.TabFocusReason);
        h.showDisplaySettingsDialog();
        tryCompare(h.pages, "depth", 3);
        tryVerify(() => !h.pages.busy);
        h.settings.placeboDebandEnabled = false;
        keyClick(Qt.Key_Escape);
        tryCompare(h.pages, "depth", 2);
        tryVerify(() => !h.pages.busy);
        compare(h.pages.currentItem.currentSection, 2);
        tryCompare(findChild(h.pages.currentItem, "placeboDebandEnabled"), "activeFocus", true);
    }
    function test_returnActivatesResultWithoutVisualFocus() {
        const h = createTemporaryObject(linkedHarnessComponent, testCase);
        h.backend.window.videoPreset = 5;
        h.settings.placeboColorAdjustmentEnabled = true;
        h.settings.placeboColorAdjustmentPreset = 0;
        const search = findChild(h, "settingsSearch");
        search.text = "Brightness";
        const results = findChild(h, "settingsSearchResults");
        results.currentIndex = h.pages.currentItem.searchResults.findIndex(row => row.target === "placeboColorAdjustmentBrightness");
        results.positionViewAtIndex(results.currentIndex, ListView.Contain);
        wait(10);
        const result = results.currentItem;
        result.forceActiveFocus(Qt.MouseFocusReason);
        verify(result.activeFocus);
        verify(!result.visualFocus);
        keyClick(Qt.Key_Return);
        tryCompare(h.pages, "depth", 2);
        tryCompare(findChild(h.pages.currentItem, "placeboColorAdjustmentBrightness"), "activeFocus", true);
        compare(search.text, "");
    }
    function test_escapeDuringKeyCaptureOpeningKeepsSettings() {
        const h = createTemporaryObject(harnessComponent, testCase);
        h.settings.controllerMapping = [{buttonValue: 1, buttonName: "Cross", keyName: "Existing"}];
        searchFor(h, "Keyboard key for Cross");
        mouseClick(findChild(h, "keyMapping1"));
        const capture = findChild(h, "keyCaptureDialog");
        verify(capture.visible);
        verify(!capture.opened, "Escape is sent during the native popup opening transition");
        keyClick(Qt.Key_Escape);
        tryCompare(capture, "visible", false);
        compare(h.actions.filter(action => action.name === "close").length, 0);
        compare(h.settings.controllerMapping[0].keyName, "Existing");
        compare(h.settings.actions.filter(action => action.name === "changeControllerKey").length, 0);
        tryCompare(findChild(h, "keyMapping1"), "activeFocus", true);
    }
    function test_keyboardResultsScrollFullyIntoView() {
        const h = createTemporaryObject(harnessComponent, testCase, {width: 900, height: 600});
        const search = findChild(h, "settingsSearch");
        search.text = "Brightness";
        const results = findChild(h, "settingsSearchResults");
        verify(results.count > 3);
        search.forceActiveFocus(Qt.TabFocusReason);
        for (let index = 0; index < results.count; ++index) keyClick(Qt.Key_Down);
        compare(results.currentIndex, results.count - 1);
        tryVerify(function() {
            const row = results.currentItem;
            if (!row || !row.activeFocus) return false;
            const point = row.mapToItem(results, 0, 0);
            return point.y >= -1 && point.y + row.height <= results.height + 1;
        }, 5000, "The entire focused result becomes visible after the native list movement settles");
    }
    function test_freshPrerequisiteAfterCaptureAndWindowResize() {
        const h = createTemporaryObject(linkedHarnessComponent, testCase, {startAtLibrary: true});
        h.settings.controllerMapping = [{buttonValue: 1, buttonName: "Cross", keyName: "Existing"}];
        h.showSettingsDialog();
        tryVerify(() => !h.pages.busy);
        for (let attempt = 0; attempt < 3; ++attempt) {
            if (!attempt) searchFor(h, "Keyboard key for Cross");
            mouseClick(findChild(h, "keyMapping1"));
            verify(findChild(h, "keyCaptureDialog").visible);
            keyClick(Qt.Key_Escape);
            tryCompare(findChild(h, "keyCaptureDialog"), "visible", false);
        }
        const search = findChild(h, "settingsSearch");
        search.text = "target";
        search.forceActiveFocus(Qt.TabFocusReason);
        for (let index = 0; index < 3; ++index) keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Escape);
        keyClick(Qt.Key_Escape);
        tryCompare(h.pages, "depth", 1);
        tryVerify(() => !h.pages.busy);
        h.width = 900; h.height = 600;
        h.showSettingsDialog();
        tryVerify(() => !h.pages.busy);
        const freshSearch = findChild(h, "settingsSearch");
        freshSearch.text = "Brightness";
        const resultIndex = h.screen.searchResults.findIndex(row => row.target === "placeboColorAdjustmentBrightness");
        freshSearch.forceActiveFocus(Qt.TabFocusReason);
        for (let index = 0; index <= resultIndex; ++index) keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        const control = findChild(h, "videoPreset");
        const flick = findChild(h, "videoFlick");
        tryCompare(control, "activeFocus", true);
        wait(500);
        const point = control.mapToItem(flick, 0, 0);
        verify(point.y >= 0 && point.y + control.height <= flick.height,
            "Fresh Video prerequisite stays fully inside its viewport after layout settles");
        compare(h.pages.depth, 2);
    }
    function test_everyCategoryAndStreamRoute() {
        const h = createTemporaryObject(harnessComponent, testCase);
        const search = findChild(h, "settingsSearch");
        const cases = [
            ["Action on suspend", "suspendAction", 0], ["Hardware decoder", "decoder", 1],
            ["PS4 Remote Frame rate", "fpsRemotePS4", 2], ["Microphone startup", "startMicUnmuted", 3],
            ["Register a new console", "registerConsole", 4], ["Reset all keys", "resetAllKeys", 5],
            ["Background controller input", "allowJoystickBackgroundEvents", 6],
            ["Port guess count", "portGuessCount", 7], ["Sanitize logs", "logSanitize", 8]
        ];
        h.settings.changeCount = 0;
        for (const row of cases) {
            search.text = row[0];
            wait(10);
            verify(findChild(h, "settingsSearchResults").count > 0, row[0]);
            search.forceActiveFocus(Qt.TabFocusReason);
            keyClick(Qt.Key_Return);
            tryCompare(findChild(h, row[1]), "activeFocus", true);
            compare(h.screen.currentCategory, row[2]);
        }
        compare(h.settings.changeCount, 0);
        compare(h.screen.selectedConsole, 0);
    }
    function test_searchFindsAndRevealsAcrossCategoriesWithoutWrites() {
        const h = createTemporaryObject(harnessComponent, testCase);
        const search = findChild(h, "settingsSearch");
        verify(search, "Settings has a search field");
        h.settings.changeCount = 0;
        search.text = "  AUDIO VOLUME  ";
        const results = findChild(h, "settingsSearchResults");
        tryCompare(results, "count", 1);
        search.forceActiveFocus();
        keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        tryCompare(findChild(h, "audioVolume"), "activeFocus", true);
        compare(h.screen.currentCategory, 3);
        compare(search.text, "");
        compare(h.settings.changeCount, 0);
    }
}
