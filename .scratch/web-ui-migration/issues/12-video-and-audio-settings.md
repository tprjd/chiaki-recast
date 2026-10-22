# 12: Configure stream profiles and audio devices

**What to build:** Users can set local and remote PS4 and PS5 video profiles and configure native audio devices and audio options.

**Blocked by:** 11: Open settings and change general session options.

**Status:** ready-for-agent

**Source:** [Web setup UI specification](../spec.md). Stories 49, 50, 51, 52, 73.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] Resolution, frame rate, codec, bitrate, decoder, zero-copy, and other existing video options retain their actual supported choices and conditions.
- [ ] Local and remote PS4 and PS5 values remain independent and persist without overwriting one another.
- [ ] Audio input and output selection, refresh, volume, buffer values, and supported speech or haptic options use the existing native services.
- [ ] Missing devices, device changes, rejected values, and platform-specific options produce correct current UI state.
- [ ] Existing native side effects and next-session or restart-only rules are preserved.
- [ ] Embedded tests verify representative writes for each stream profile, native device refresh, invalid values, and persisted audio settings.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.
