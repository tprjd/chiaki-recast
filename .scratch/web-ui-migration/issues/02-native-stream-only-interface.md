# 02: Run native playback with a stream-only interface

**What to build:** A direct stream launch displays native video, overlays, console login PIN prompts, and live settings without loading the application setup UI.

**Blocked by:** 01: Preserve native playback with application-owned services.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 71, 73, 74, 75, 76, 77, 78, 79, 86, 87.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] The native stream root provides the navigation functions required by the menu, statistics, notices, errors, disconnect dialog, and live settings.
- [x] Display, renderer, color mapping, and keyboard and mouse settings remain native during playback. Their current native behavior is retained.
- [x] The console login PIN prompt and renderer fallback dialog work without a setup root.
- [x] ADR 0001 input ownership remains intact for overlap, duplicate opens, obsolete closes, owner destruction, focus return, and session handoff. Passive notices remain passive.
- [x] Native video, HDR metadata, audio, microphone input, touch, haptics, and gameplay input retain their existing pipeline.
- [x] The direct stream command preserves its profile, fullscreen, zoom, stretch, and exit behavior without creating a WebEngine setup page.
- [x] Existing native stream QML and input tests exercise the extracted production root and its observable behavior.

## Answer

The native stream root is extracted and its production window checks pass.
`StreamRoot.qml` retains native progress, overlays, PIN, errors, disconnect,
and live settings. `Main.qml` is removed. Native hosts use saved stream
geometry and do not write or restore setup geometry.

`streamViewDismissed` reports a presentation request. Ticket 04 must wait for
native session and frame cleanup before it destroys the stream window or
restores setup. The existing direct-stream exit path remains in native code.

[Verification](../evidence/ticket02/verification.md) records the red and green
checks, actual OpenGL and Vulkan results, retained ticket 01 evidence, and
the Windows hidden-startup test correction. The pipeline checkbox records
unchanged production paths. It does not claim physical media or input acceptance.
Live console, native/browser GPU coexistence, HDR, latency, performance, and
package acceptance remain with tickets 03, 04, and 23.
