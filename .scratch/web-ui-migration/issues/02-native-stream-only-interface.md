# 02: Run native playback with a stream-only interface

**What to build:** A direct stream launch displays native video, overlays, console login PIN prompts, and live settings without loading the application setup UI.

**Blocked by:** 01: Preserve native playback with application-owned services.

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 71, 73, 74, 75, 76, 77, 78, 79, 86, 87.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] The native stream root provides the navigation functions required by the menu, statistics, notices, errors, disconnect dialog, and live settings.
- [ ] Display, renderer, color mapping, and keyboard and mouse settings remain native during playback. Their current native behavior is retained.
- [ ] The console login PIN prompt and renderer fallback dialog work without a setup root.
- [ ] ADR 0001 input ownership remains intact for overlap, duplicate opens, obsolete closes, owner destruction, focus return, and session handoff. Passive notices remain passive.
- [ ] Native video, HDR metadata, audio, microphone input, touch, haptics, and gameplay input retain their existing pipeline.
- [ ] The direct stream command preserves its profile, fullscreen, zoom, stretch, and exit behavior without creating a WebEngine setup page.
- [ ] Existing native stream QML and input tests exercise the extracted production root and its observable behavior.
