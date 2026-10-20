# 04: Configure audio, microphone, and connection feedback

Status: resolved

**What to build:** Use the complete Audio/Wifi page to configure session audio,
supported speech processing, and connection feedback.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [x] Include audio input/output, buffer size, volume, microphone startup, speech processing, noise and echo suppression, Wi-Fi threshold, packet-loss maximum, frame recovery, and stream statistics.
- [x] Populate devices from the native source and handle empty or changed device lists without writing to an unintended device.
- [x] Preserve build and dependent-setting conditions. Unsupported speech processing remains unavailable.
- [x] Display units and defaults correctly; retain numeric bounds, precision, steps, conversions, and update timing.
- [x] Production-QML tests verify every setting destination, conversions, boundaries, feature conditions, and device-list updates.
- [x] Build and demonstrate the native page at normal and narrow sizes, including the supported speech-processing states.



## Answer

Implemented in 6b98b17, merged as 5dfcf8. All 32 Audio/Wifi screen checks pass, including native destinations, conversions, optional speech controls and device-list changes. Native navigation, focus bounds and no-write checks pass at 1280 by 720 and 900 by 600. The 900-pixel native layout was inspected. Evidence: ../native-previews/03-06-input-dpr1-checks.json.
