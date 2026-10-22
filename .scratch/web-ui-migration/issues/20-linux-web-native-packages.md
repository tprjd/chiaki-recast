# 20: Ship the Linux web and native application

**What to build:** Supported Linux users can run accelerated web setup and native playback from the maintained AppImage and Flatpak packages.

**Blocked by:** 04: Connect locally from web setup to native playback.

**Status:** ready-for-agent

**Source:** [Web setup UI specification](../spec.md). Stories 89, 90, 91, 94.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] AppImage and Flatpak contain matching WebEngine widget and channel support, helper processes, resources, locales, and all packaged frontend assets.
- [ ] Helper paths, permissions, file dialogs, and Chromium sandbox operation work in the actual packages without a development environment.
- [ ] Actual Chromium GPU acceleration is recorded. Supported X11, Wayland, and gamescope window handoff and focus behavior are checked.
- [ ] Native OpenGL and Vulkan playback, supported HDR behavior, input, audio, microphone operation, and direct streaming pass package smoke checks.
- [ ] Architecture and runtime combinations are reported with tested module availability. Unsupported combinations do not gain a second setup UI.
- [ ] Automated package checks detect missing assets and helper files. Rerun an affected check when a later change can alter package behavior.
