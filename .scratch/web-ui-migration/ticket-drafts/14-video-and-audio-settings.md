# 14: Configure stream profiles and audio devices

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can set local and remote PS4 and PS5 video profiles and configure native audio devices and audio options.

**Blocked by:** 13: Change general and stream settings from web setup.

**Status:** needs-info

**Spec stories:** 49, 50, 51, 52, 73.

- [ ] Resolution, frame rate, codec, bitrate, decoder, zero-copy, and other existing video options retain their actual supported choices and conditions.
- [ ] Local and remote PS4 and PS5 values remain independent and persist without overwriting one another.
- [ ] Audio input and output selection, refresh, volume, buffer values, and supported speech or haptic options use the existing native services.
- [ ] Missing devices, device changes, rejected values, and platform-specific options produce correct current UI state.
- [ ] Existing native side effects and next-session or restart-only rules are preserved.
- [ ] Embedded tests verify representative writes for each stream profile, native device refresh, invalid values, and persisted audio settings.
