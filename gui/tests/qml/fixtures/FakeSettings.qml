import QtQuick

// Synthetic settings for production-QML screen tests. No native storage is used.
QtObject {
    id: settings
    property int changeCount: 0
    property bool mouseCameraSupported: true
    property bool mouseCameraEnabled: false
    property real mouseHorizontalSensitivity: 1
    property real mouseVerticalSensitivity: 1
    property real mouseDeadZoneCompensation: 0
    property bool mouseInvertVertical: false
    property var keyboardMouseBindings: []
    readonly property real mouseSensitivityMinimum: 0.05
    readonly property real mouseSensitivityMaximum: 8
    readonly property real mouseCompensationMaximum: 0.5
    onMouseCameraEnabledChanged: changeCount++
    property var actions: []
    function record(name, args) { actions = actions.concat([{name: name, args: args || []}]); }
    function resetKeyboardMouseDefaults() {
        mouseCameraEnabled = false;
        mouseHorizontalSensitivity = 1;
        mouseVerticalSensitivity = 1;
        mouseDeadZoneCompensation = 0;
        mouseInvertVertical = false;
        keyboardMouseBindings = [];
    }
    property bool remotePlayAsk: false
    onRemotePlayAskChanged: changeCount++
    property bool addSteamShortcutAsk: false
    onAddSteamShortcutAskChanged: changeCount++
    property bool hideCursor: false
    onHideCursorChanged: changeCount++
    property int audioVideoDisabled: 0
    onAudioVideoDisabledChanged: changeCount++
    property int resolutionLocalPS4: 0
    onResolutionLocalPS4Changed: changeCount++
    property int resolutionRemotePS4: 0
    onResolutionRemotePS4Changed: changeCount++
    property int resolutionLocalPS5: 0
    onResolutionLocalPS5Changed: changeCount++
    property int resolutionRemotePS5: 0
    onResolutionRemotePS5Changed: changeCount++
    property int disconnectAction: 2
    onDisconnectActionChanged: changeCount++
    property int suspendAction: 0
    onSuspendActionChanged: changeCount++
    property bool logVerbose: false
    onLogVerboseChanged: changeCount++
    property bool logSanitize: false
    onLogSanitizeChanged: changeCount++
    property bool vSyncEnabled: true
    onVSyncEnabledChanged: changeCount++
    property int rumbleHapticsIntensity: 0
    onRumbleHapticsIntensityChanged: changeCount++
    property var steamDeckHaptics: undefined
    onSteamDeckHapticsChanged: changeCount++
    property var verticalDeck: undefined
    onVerticalDeckChanged: changeCount++
    property bool buttonsByPosition: false
    onButtonsByPositionChanged: changeCount++
    property bool startMicUnmuted: false
    onStartMicUnmutedChanged: changeCount++
    property var speechProcessing: false
    onSpeechProcessingChanged: changeCount++
    property var noiseSuppressLevel: 6
    onNoiseSuppressLevelChanged: changeCount++
    property var echoSuppressLevel: 30
    onEchoSuppressLevelChanged: changeCount++
    property bool showStreamStats: false
    onShowStreamStatsChanged: changeCount++
    property bool streamerMode: false
    onStreamerModeChanged: changeCount++
    property real hapticOverride: 0
    onHapticOverrideChanged: changeCount++
    property int displayTargetContrast: 0
    onDisplayTargetContrastChanged: changeCount++
    property int displayTargetPeak: 0
    onDisplayTargetPeakChanged: changeCount++
    property int displayTargetPrim: 0
    onDisplayTargetPrimChanged: changeCount++
    property int displayTargetTrc: 0
    onDisplayTargetTrcChanged: changeCount++
    property bool fullscreenDoubleClick: false
    onFullscreenDoubleClickChanged: changeCount++
    property int fpsLocalPS4: 0
    onFpsLocalPS4Changed: changeCount++
    property int fpsRemotePS4: 0
    onFpsRemotePS4Changed: changeCount++
    property int fpsLocalPS5: 0
    onFpsLocalPS5Changed: changeCount++
    property int fpsRemotePS5: 0
    onFpsRemotePS5Changed: changeCount++
    property int bitrateLocalPS4: 0
    onBitrateLocalPS4Changed: changeCount++
    property int bitrateRemotePS4: 0
    onBitrateRemotePS4Changed: changeCount++
    property int bitrateLocalPS5: 0
    onBitrateLocalPS5Changed: changeCount++
    property int bitrateRemotePS5: 0
    onBitrateRemotePS5Changed: changeCount++
    property int codecLocalPS5: 0
    onCodecLocalPS5Changed: changeCount++
    property int codecRemotePS5: 0
    onCodecRemotePS5Changed: changeCount++
    property int audioBufferSize: 19200
    onAudioBufferSizeChanged: changeCount++
    property int audioVolume: 100
    onAudioVolumeChanged: changeCount++
    property string audioInDevice: ""
    onAudioInDeviceChanged: changeCount++
    property string audioOutDevice: ""
    onAudioOutDeviceChanged: changeCount++
    property string decoder: ""
    onDecoderChanged: changeCount++
    property bool useZeroCopy: false
    onUseZeroCopyChanged: changeCount++
    property int windowType: 3
    onWindowTypeChanged: changeCount++
    property int customResolutionWidth: 0
    onCustomResolutionWidthChanged: changeCount++
    property int customResolutionHeight: 0
    onCustomResolutionHeightChanged: changeCount++
    property int videoPreset: 0
    onVideoPresetChanged: changeCount++
    property int rendererBackend: 0
    onRendererBackendChanged: changeCount++
    property real sZoomFactor: 0
    onSZoomFactorChanged: changeCount++
    property int packetLossReportedMax: 0
    onPacketLossReportedMaxChanged: changeCount++
    property int queueDepthLimit: 0
    onQueueDepthLimitChanged: changeCount++
    property string autoConnectMac: ""
    onAutoConnectMacChanged: changeCount++
    property bool allowJoystickBackgroundEvents: false
    onAllowJoystickBackgroundEventsChanged: changeCount++
    property string logDirectory: "C:/synthetic/logs"
    onLogDirectoryChanged: changeCount++
    property var availableDecoders: ["Auto", "d3d11va"]
    onAvailableDecodersChanged: changeCount++
    property var availableAudioInDevices: ["Microphone"]
    onAvailableAudioInDevicesChanged: changeCount++
    property var availableAudioOutDevices: ["Speakers"]
    onAvailableAudioOutDevicesChanged: changeCount++
    property var registeredHosts: []
    onRegisteredHostsChanged: changeCount++
    property var profiles: []
    onProfilesChanged: changeCount++
    property string currentProfile: "Test profile"
    onCurrentProfileChanged: changeCount++
    property var controllerMapping: []
    onControllerMappingChanged: changeCount++
    property int wifiDroppedNotif: 0
    onWifiDroppedNotifChanged: changeCount++
    property bool portGuessingEnabled: false
    onPortGuessingEnabledChanged: changeCount++
    property int portGuessCount: 0
    onPortGuessCountChanged: changeCount++
    property int portGuessSocketCount: 0
    onPortGuessSocketCountChanged: changeCount++
    property string psnRefreshToken: ""
    onPsnRefreshTokenChanged: changeCount++
    property string psnAuthToken: ""
    onPsnAuthTokenChanged: changeCount++
    property string psnAuthTokenExpiry: ""
    onPsnAuthTokenExpiryChanged: changeCount++
    property string psnAccountId: ""
    onPsnAccountIdChanged: changeCount++
    property bool mouseTouchEnabled: false
    onMouseTouchEnabledChanged: changeCount++
    property bool keyboardEnabled: false
    onKeyboardEnabledChanged: changeCount++
    property bool dpadTouchEnabled: false
    onDpadTouchEnabledChanged: changeCount++
    property bool iDROnFECFailureEnabled: false
    onIDROnFECFailureEnabledChanged: changeCount++
    property var dpadTouchIncrement: []
    onDpadTouchIncrementChanged: changeCount++
    property int dpadTouchShortcut1: 0
    onDpadTouchShortcut1Changed: changeCount++
    property int dpadTouchShortcut2: 0
    onDpadTouchShortcut2Changed: changeCount++
    property int dpadTouchShortcut3: 0
    onDpadTouchShortcut3Changed: changeCount++
    property int dpadTouchShortcut4: 0
    onDpadTouchShortcut4Changed: changeCount++
    property bool streamMenuEnabled: true
    onStreamMenuEnabledChanged: changeCount++
    property int streamMenuShortcut1: 9
    onStreamMenuShortcut1Changed: changeCount++
    property int streamMenuShortcut2: 10
    onStreamMenuShortcut2Changed: changeCount++
    property int streamMenuShortcut3: 11
    onStreamMenuShortcut3Changed: changeCount++
    property int streamMenuShortcut4: 12
    onStreamMenuShortcut4Changed: changeCount++
    property int placeboUpscaler: 0
    onPlaceboUpscalerChanged: changeCount++
    property int placeboPlaneUpscaler: 0
    onPlaceboPlaneUpscalerChanged: changeCount++
    property int placeboDownscaler: 0
    onPlaceboDownscalerChanged: changeCount++
    property int placeboPlaneDownscaler: 0
    onPlaceboPlaneDownscalerChanged: changeCount++
    property int placeboFrameMixer: 0
    onPlaceboFrameMixerChanged: changeCount++
    property real placeboAntiringingStrength: 0
    onPlaceboAntiringingStrengthChanged: changeCount++
    property bool placeboDebandEnabled: false
    onPlaceboDebandEnabledChanged: changeCount++
    property int placeboDebandPreset: 0
    onPlaceboDebandPresetChanged: changeCount++
    property int placeboDebandIterations: 0
    onPlaceboDebandIterationsChanged: changeCount++
    property real placeboDebandThreshold: 0
    onPlaceboDebandThresholdChanged: changeCount++
    property real placeboDebandRadius: 0
    onPlaceboDebandRadiusChanged: changeCount++
    property real placeboDebandGrain: 0
    onPlaceboDebandGrainChanged: changeCount++
    property bool placeboSigmoidEnabled: false
    onPlaceboSigmoidEnabledChanged: changeCount++
    property int placeboSigmoidPreset: 0
    onPlaceboSigmoidPresetChanged: changeCount++
    property real placeboSigmoidCenter: 0
    onPlaceboSigmoidCenterChanged: changeCount++
    property real placeboSigmoidSlope: 0
    onPlaceboSigmoidSlopeChanged: changeCount++
    property bool placeboColorAdjustmentEnabled: false
    onPlaceboColorAdjustmentEnabledChanged: changeCount++
    property int placeboColorAdjustmentPreset: 0
    onPlaceboColorAdjustmentPresetChanged: changeCount++
    property real placeboColorAdjustmentBrightness: 0
    onPlaceboColorAdjustmentBrightnessChanged: changeCount++
    property real placeboColorAdjustmentContrast: 0
    onPlaceboColorAdjustmentContrastChanged: changeCount++
    property real placeboColorAdjustmentSaturation: 0
    onPlaceboColorAdjustmentSaturationChanged: changeCount++
    property real placeboColorAdjustmentHue: 0
    onPlaceboColorAdjustmentHueChanged: changeCount++
    property real placeboColorAdjustmentGamma: 0
    onPlaceboColorAdjustmentGammaChanged: changeCount++
    property real placeboColorAdjustmentTemperature: 0
    onPlaceboColorAdjustmentTemperatureChanged: changeCount++
    property bool placeboPeakDetectionEnabled: false
    onPlaceboPeakDetectionEnabledChanged: changeCount++
    property int placeboPeakDetectionPreset: 0
    onPlaceboPeakDetectionPresetChanged: changeCount++
    property real placeboPeakDetectionPeakSmoothingPeriod: 0
    onPlaceboPeakDetectionPeakSmoothingPeriodChanged: changeCount++
    property real placeboPeakDetectionSceneThresholdLow: 0
    onPlaceboPeakDetectionSceneThresholdLowChanged: changeCount++
    property real placeboPeakDetectionSceneThresholdHigh: 0
    onPlaceboPeakDetectionSceneThresholdHighChanged: changeCount++
    property real placeboPeakDetectionPeakPercentile: 0
    onPlaceboPeakDetectionPeakPercentileChanged: changeCount++
    property real placeboPeakDetectionBlackCutoff: 0
    onPlaceboPeakDetectionBlackCutoffChanged: changeCount++
    property bool placeboPeakDetectionAllowDelayedPeak: false
    onPlaceboPeakDetectionAllowDelayedPeakChanged: changeCount++
    property bool placeboColorMappingEnabled: false
    onPlaceboColorMappingEnabledChanged: changeCount++
    property int placeboColorMappingPreset: 0
    onPlaceboColorMappingPresetChanged: changeCount++
    property int placeboGamutMappingFunction: 0
    onPlaceboGamutMappingFunctionChanged: changeCount++
    property real placeboGamutMappingPerceptualDeadzone: 0
    onPlaceboGamutMappingPerceptualDeadzoneChanged: changeCount++
    property real placeboGamutMappingPerceptualStrength: 0
    onPlaceboGamutMappingPerceptualStrengthChanged: changeCount++
    property real placeboGamutMappingColorimetricGamma: 0
    onPlaceboGamutMappingColorimetricGammaChanged: changeCount++
    property real placeboGamutMappingSoftClipKnee: 0
    onPlaceboGamutMappingSoftClipKneeChanged: changeCount++
    property real placeboGamutMappingSoftClipDesat: 0
    onPlaceboGamutMappingSoftClipDesatChanged: changeCount++
    property int placeboGamutMappingLut3dSizeI: 0
    onPlaceboGamutMappingLut3dSizeIChanged: changeCount++
    property int placeboGamutMappingLut3dSizeC: 0
    onPlaceboGamutMappingLut3dSizeCChanged: changeCount++
    property int placeboGamutMappingLut3dSizeH: 0
    onPlaceboGamutMappingLut3dSizeHChanged: changeCount++
    property bool placeboGamutMappingLut3dTricubicEnabled: false
    onPlaceboGamutMappingLut3dTricubicEnabledChanged: changeCount++
    property bool placeboGamutMappingGamutExpansionEnabled: false
    onPlaceboGamutMappingGamutExpansionEnabledChanged: changeCount++
    property int placeboToneMappingFunction: 0
    onPlaceboToneMappingFunctionChanged: changeCount++
    property real placeboToneMappingKneeAdaptation: 0
    onPlaceboToneMappingKneeAdaptationChanged: changeCount++
    property real placeboToneMappingKneeMinimum: 0
    onPlaceboToneMappingKneeMinimumChanged: changeCount++
    property real placeboToneMappingKneeMaximum: 0
    onPlaceboToneMappingKneeMaximumChanged: changeCount++
    property real placeboToneMappingKneeDefault: 0
    onPlaceboToneMappingKneeDefaultChanged: changeCount++
    property real placeboToneMappingKneeOffset: 0
    onPlaceboToneMappingKneeOffsetChanged: changeCount++
    property real placeboToneMappingSlopeTuning: 0
    onPlaceboToneMappingSlopeTuningChanged: changeCount++
    property real placeboToneMappingSlopeOffset: 0
    onPlaceboToneMappingSlopeOffsetChanged: changeCount++
    property real placeboToneMappingSplineContrast: 0
    onPlaceboToneMappingSplineContrastChanged: changeCount++
    property real placeboToneMappingReinhardContrast: 0
    onPlaceboToneMappingReinhardContrastChanged: changeCount++
    property real placeboToneMappingLinearKnee: 0
    onPlaceboToneMappingLinearKneeChanged: changeCount++
    property real placeboToneMappingExposure: 0
    onPlaceboToneMappingExposureChanged: changeCount++
    property bool placeboToneMappingInverseToneMappingEnabled: false
    onPlaceboToneMappingInverseToneMappingEnabledChanged: changeCount++
    property int placeboToneMappingMetadata: 0
    onPlaceboToneMappingMetadataChanged: changeCount++
    property int placeboToneMappingToneLutSize: 0
    onPlaceboToneMappingToneLutSizeChanged: changeCount++
    property real placeboToneMappingContrastRecovery: 0
    onPlaceboToneMappingContrastRecoveryChanged: changeCount++
    property real placeboToneMappingContrastSmoothness: 0
    onPlaceboToneMappingContrastSmoothnessChanged: changeCount++
    function refreshAudioDevices() { record("refreshAudioDevices"); }
    function exportSettings() { record("exportSettings"); }
    function importSettings() { record("importSettings"); }
    function restartApplication() { record("restartApplication"); }
    function clearKeyMapping() { record("clearKeyMapping"); }
    function deleteRegisteredHost(mac) { record("deleteRegisteredHost", [mac]); }
    function changeControllerKey(button, key) { record("changeControllerKey", [button, key]); return "Test key"; }
}
