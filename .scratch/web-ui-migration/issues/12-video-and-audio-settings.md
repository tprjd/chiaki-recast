# 12: Configure stream profiles and audio devices

**What to build:** Users can set local and remote PS4 and PS5 video profiles and configure native audio devices and audio options.

**Blocked by:** 11: Open settings and change general session options.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 49, 50, 51, 52, 73.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] Resolution, frame rate, codec, bitrate, decoder, zero-copy, and other existing video options retain their actual supported choices and conditions.
- [x] Local and remote PS4 and PS5 values remain independent and persist without overwriting one another.
- [x] Audio input and output selection, refresh, volume, buffer values, and supported speech or haptic options use the existing native services.
- [x] Missing devices, device changes, rejected values, and platform-specific options produce correct current UI state.
- [x] Existing native side effects and next-session or restart-only rules are preserved.
- [x] Embedded tests verify representative writes for each stream profile, native device refresh, invalid values, and persisted audio settings.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.


## Answer

Implemented Video decoding and stream-window controls, all four independent
PS4/PS5 local/remote stream profiles, Audio/Wifi, native audio refresh and
conditional speech settings. Ticket13 retains display targets, render
backend/preset/custom renderer settings and queue depth.

All writes use explicit validated native setters and profile generations.
Saved missing devices remain visible. Software decoder selection retains the
native empty-string conversion. Numeric drafts reset when the profile changes
and retain their original generation for delayed submission. Resolution changes
reset only that stream profile's bitrate. Native notifications and the existing
live-volume update connection remain in use.

Focused real WebEngine/WebChannel tests passed for independent writes,
validation, stale profiles, native device enumeration through SDL's controlled
dummy driver, reload, separate-process persistence, search conditions and
responsive layout. The test reports, exact source/artifact identity and limits
are in [ticket12 evidence](../evidence/ticket12/verification.md).

SettingsDialog.qml still has callers for unmigrated groups and fixtures.
Its shared resource remains until those callers migrate; final cleanup must
remove it. No new setup-only callback adapter was added.

Physical audio/hotplug, HDR/decoder hardware, haptics, live playback, audible
volume, Linux/macOS execution, GPU/focus and package acceptance remain unverified.

Integration merge `fd41b8d2` contains worker commit `cca37231` without conflicts.
The merged tree matches the worker tree. Its source matches tested source
`3f19a3c6`; production source matches `b3dbb435`. All 16 recorded source,
artifact, and vendor-patch hashes match. The merger also compared all four
capture hashes and inspected the images. The narrow and zoomed Audio/Wifi
captures show the top of the scrollable page. The recorded layout assertions
check horizontal bounds.

Ticket 06 account controls and ticket 11 general controls remain. Shared fields
retain numeric and string choices, the profile-generation key reset, and the
generation captured when a numeric draft starts. The recorded expanded 3/0,
state/restart/general 5/0, child 3/0, and SDL dummy device/layout 4/0 results
were reused without builds or test reruns. Source-only whitespace checks pass.
Physical audio, hotplug, GPU, live playback, and platform acceptance remain open.
