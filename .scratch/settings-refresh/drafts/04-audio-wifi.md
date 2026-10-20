# 04: Configure audio, microphone, and connection feedback

Publication: draft; awaiting breakdown approval

**What to build:** Use the complete Audio/Wifi page to configure session audio,
supported speech processing, and connection feedback.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [ ] Include audio input/output, buffer size, volume, microphone startup, speech processing, noise and echo suppression, Wi-Fi threshold, packet-loss maximum, frame recovery, and stream statistics.
- [ ] Populate devices from the native source and handle empty or changed device lists without writing to an unintended device.
- [ ] Preserve build and dependent-setting conditions. Unsupported speech processing remains unavailable.
- [ ] Display units and defaults correctly; retain numeric bounds, precision, steps, conversions, and update timing.
- [ ] Production-QML tests verify every setting destination, conversions, boundaries, feature conditions, and device-list updates.
- [ ] Build and demonstrate the native page at normal and narrow sizes, including the supported speech-processing states.

