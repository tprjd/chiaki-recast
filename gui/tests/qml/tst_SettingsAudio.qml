import QtQuick
import QtQuick.Controls
import QtTest
import "fixtures"

TestCase {
    id: testCase
    name: "SettingsAudio"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    Component { id: harnessComponent; SettingsHarness {} }


    function choose(combo, index) {
        combo.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", true);
        keyClick(Qt.Key_Home);
        for (let i = 0; i < index; ++i) keyClick(Qt.Key_Down);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", false);
    }

    function audioHarness(properties) {
        const harness = createTemporaryObject(harnessComponent, testCase, properties || {});
        harness.screen.currentCategory = 3;
        return harness;
    }

    function test_sliderWrites_data() {
        return [
            {tag: "buffer-min", name: "audioBufferSize", initial: 9600, maximum: false, expected: 1920, text: "10 ms"},
            {tag: "buffer-max", name: "audioBufferSize", initial: 9600, maximum: true, expected: 19200, text: "100 ms"},
            {tag: "volume-min", name: "audioVolume", initial: 64, maximum: false, expected: 0, text: "0% volume"},
            {tag: "volume-max", name: "audioVolume", initial: 64, maximum: true, expected: 128, text: "100% volume"},
            {tag: "noise-min", name: "noiseSuppressLevel", initial: 6, maximum: false, expected: 0, text: "0 dB"},
            {tag: "noise-max", name: "noiseSuppressLevel", initial: 6, maximum: true, expected: 60, text: "60 dB"},
            {tag: "echo-min", name: "echoSuppressLevel", initial: 30, maximum: false, expected: 0, text: "0 dB"},
            {tag: "echo-max", name: "echoSuppressLevel", initial: 30, maximum: true, expected: 60, text: "60 dB"},
            {tag: "wifi-min", name: "wifiDroppedNotif", initial: 3, maximum: false, expected: 0, text: ">= 0% dropped packets"},
            {tag: "wifi-max", name: "wifiDroppedNotif", initial: 3, maximum: true, expected: 100, text: ">= 100% dropped packets"},
            {tag: "loss-min", name: "packetLossReportedMax", initial: 5, maximum: false, expected: 0, text: "0% packet loss"},
            {tag: "loss-max", name: "packetLossReportedMax", initial: 5, maximum: true, expected: 100, text: "100% packet loss"}
        ];
    }
    function test_sliderWrites(data) {
        const harness = audioHarness();
        harness.settings.speechProcessing = true;
        harness.settings[data.name] = data.initial;
        harness.settings.changeCount = 0;
        const control = findChild(harness, data.name);
        verify(control && control.visible);
        control.forceActiveFocus(Qt.TabFocusReason);
        wait(10);
        const x = data.maximum ? control.width - 1 : 1;
        mouseClick(control, x, control.height / 2);
        compare(harness.settings[data.name], data.expected);
        compare(harness.settings.changeCount, 1);
        compare(findChild(harness, data.name + "Value").text, data.text);
        mouseClick(control, x, control.height / 2);
        compare(harness.settings.changeCount, 1, "Bound does not produce another write");
    }

    function test_sliderKeyboardSteps_data() {
        return [
            {tag: "buffer", name: "audioBufferSize", initial: 0, expected: 11520, text: "60 ms"},
            {tag: "volume", name: "audioVolume", initial: 64, expected: 65, text: "51% volume"},
            {tag: "noise", name: "noiseSuppressLevel", initial: 6, expected: 7, text: "7 dB"},
            {tag: "echo", name: "echoSuppressLevel", initial: 30, expected: 31, text: "31 dB"},
            {tag: "wifi", name: "wifiDroppedNotif", initial: 3, expected: 4, text: ">= 4% dropped packets"},
            {tag: "loss", name: "packetLossReportedMax", initial: 5, expected: 6, text: "6% packet loss"}
        ];
    }
    function test_sliderKeyboardSteps(data) {
        const harness = audioHarness();
        harness.settings.speechProcessing = true;
        harness.settings[data.name] = data.initial;
        harness.settings.changeCount = 0;
        const control = findChild(harness, data.name);
        control.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Right);
        compare(harness.settings[data.name], data.expected);
        compare(harness.settings.changeCount, 1);
        compare(findChild(harness, data.name + "Value").text, data.text);
    }

    function test_checkBoxWrites_data() {
        return [{tag: "microphone", name: "startMicUnmuted"},
            {tag: "speech", name: "speechProcessing"},
            {tag: "recovery", name: "iDROnFECFailureEnabled"},
            {tag: "statistics", name: "showStreamStats"}];
    }
    function test_checkBoxWrites(data) {
        const harness = audioHarness();
        const control = findChild(harness, data.name);
        verify(control && control.visible);
        harness.settings.changeCount = 0;
        control.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Space);
        compare(harness.settings[data.name], true);
        compare(harness.settings.changeCount, 1);
        if (data.name === "speechProcessing") {
            verify(findChild(harness, "noiseSuppressLevel").visible);
            verify(findChild(harness, "echoSuppressLevel").visible);
        }
        keyClick(Qt.Key_Space);
        compare(harness.settings[data.name], false);
        compare(harness.settings.changeCount, 2);
        if (data.name === "speechProcessing") {
            verify(!findChild(harness, "noiseSuppressLevel").visible);
            verify(!findChild(harness, "echoSuppressLevel").visible);
        }
    }

    function test_devices_data() {
        return [{tag: "output", name: "audioOutDevice", list: "availableAudioOutDevices", peer: "audioInDevice"},
            {tag: "input", name: "audioInDevice", list: "availableAudioInDevices", peer: "audioOutDevice"}];
    }
    function test_devices(data) {
        const harness = audioHarness();
        harness.settings[data.list] = ["First device", "Second device"];
        harness.settings[data.name] = "Missing device";
        harness.settings[data.peer] = "Unchanged peer";
        const control = findChild(harness, data.name);
        compare(control.currentIndex, 0);
        harness.settings.changeCount = 0;
        choose(control, 2);
        compare(harness.settings[data.name], "Second device");
        compare(harness.settings[data.peer], "Unchanged peer");
        compare(harness.settings.changeCount, 1);
        harness.settings[data.list] = ["Second device", "First device"];
        compare(control.currentIndex, 1);
        choose(control, 0);
        compare(harness.settings[data.name], "");
        harness.settings[data.list] = [];
        compare(control.count, 1);
        compare(control.currentIndex, 0);
        compare(harness.settings[data.peer], "Unchanged peer");
    }

    function test_profileValuesRefreshWithoutWrites() {
        const harness = audioHarness();
        harness.settings.audioBufferSize = 0;
        compare(findChild(harness, "audioBufferSize").value, 5);
        compare(findChild(harness, "audioBufferSizeValue").text, "50 ms");
        harness.settings.changeCount = 0;
        harness.screen.currentCategory = 0;
        harness.screen.currentCategory = 3;
        compare(harness.settings.audioBufferSize, 0);
        compare(harness.settings.changeCount, 0);
        harness.settings.currentProfile = "Other test profile";
        harness.settings.audioBufferSize = 3840;
        harness.settings.audioVolume = 32;
        harness.settings.changeCount = 0;
        waitForRendering(harness);
        compare(findChild(harness, "audioBufferSizeValue").text, "20 ms");
        compare(findChild(harness, "audioVolumeValue").text, "25% volume");
        compare(harness.settings.changeCount, 0);
    }

    function test_allControlsFitOnFocus_data() {
        return [{tag: "desktop", width: 1280, height: 720},
            {tag: "minimum", width: 900, height: 600}];
    }
    function test_allControlsFitOnFocus(data) {
        const harness = audioHarness({width: data.width, height: data.height});
        harness.settings.speechProcessing = true;
        harness.settings.availableAudioOutDevices = ["A very long translated audio output device name that must remain inside the narrow settings page"];
        harness.settings.audioOutDevice = harness.settings.availableAudioOutDevices[0];
        waitForRendering(harness);
        for (const name of ["audioOutDevice", "audioInDevice", "audioBufferSize", "audioVolume", "startMicUnmuted", "speechProcessing", "noiseSuppressLevel", "echoSuppressLevel", "wifiDroppedNotif", "packetLossReportedMax", "iDROnFECFailureEnabled", "showStreamStats"]) {
            const control = findChild(harness, name);
            verify(control && control.visible, name + " exists");
            control.forceActiveFocus(Qt.TabFocusReason);
            wait(10);
            const point = control.mapToItem(harness, 0, 0);
            verify(point.x >= 0 && point.x + control.width <= harness.width, name + " fits width");
            verify(point.y >= 0 && point.y + control.height <= harness.height, name + " is reachable on focus");
            const value = findChild(harness, name + "Value");
            if (value) {
                const flick = findChild(harness, "audiowifiFlick");
                const valuePoint = value.mapToItem(flick, 0, 0);
                verify(valuePoint.y >= 0 && valuePoint.y + value.height <= flick.height, name + " keeps units visible on focus");
            }
        }
        const output = findChild(harness, "audioOutDevice");
        output.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(output.popup, "visible", true);
        verify(output.popup.width <= harness.width, "Device popup fits width");
        keyClick(Qt.Key_Escape);
        compare(harness.actions.length, 0);
    }

    function findDeviceCombo(item, device) {
        if (item.model && item.model.indexOf && item.model.indexOf(device) >= 0 && item.popup)
            return item;
        for (const child of item.children || []) {
            const found = findDeviceCombo(child, device);
            if (found) return found;
        }
        return null;
    }

    function test_deviceListChangeCancelsPendingChoice() {
        const harness = createTemporaryObject(harnessComponent, testCase);
        harness.settings.availableAudioOutDevices = ["Speakers A", "Headphones B"];
        harness.settings.audioOutDevice = "Speakers A";
        mouseClick(findChild(harness, "settingsCategory3"));
        const combo = findDeviceCombo(harness, "Speakers A");
        verify(combo);
        combo.forceActiveFocus(Qt.TabFocusReason);
        keyClick(Qt.Key_Return);
        tryCompare(combo.popup, "visible", true);
        keyClick(Qt.Key_End);
        harness.settings.changeCount = 0;
        harness.settings.availableAudioOutDevices = ["Replacement C"];
        tryCompare(combo.popup, "visible", false);
        compare(harness.settings.audioOutDevice, "Speakers A");
        compare(combo.currentIndex, 0);
        compare(harness.settings.changeCount, 1); // Only the injected list changed.
    }

    function test_deviceControlFitsNarrowPage() {
        const harness = createTemporaryObject(harnessComponent, testCase, {width: 900, height: 600});
        harness.screen.currentCategory = 3;
        waitForRendering(harness);
        const combo = findDeviceCombo(harness, "Speakers");
        verify(combo);
        const point = combo.mapToItem(harness, 0, 0);
        verify(point.x >= 0 && point.x + combo.width <= harness.width, "Output device fits narrow page");
    }

    function test_unsupportedSpeechHasNoWarningsOrWrites() {
        const harness = createTemporaryObject(harnessComponent, testCase, {screenActive: false});
        harness.settings.speechProcessing = undefined;
        harness.settings.noiseSuppressLevel = undefined;
        harness.settings.echoSuppressLevel = undefined;
        harness.settings.changeCount = 0;
        failOnWarning(/Unable to assign/);
        harness.screenActive = true;
        waitForRendering(harness);
        mouseClick(findChild(harness, "settingsCategory3"));
        compare(harness.settings.changeCount, 0);
        verify(!findChild(harness, "speechProcessing").visible);
        verify(!findChild(harness, "noiseSuppressLevel").visible);
        verify(!findChild(harness, "echoSuppressLevel").visible);
    }
}


