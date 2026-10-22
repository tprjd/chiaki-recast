# 20: Ship the Linux web and native application

**What to build:** Supported Linux users can run accelerated web setup and native playback from the maintained AppImage and Flatpak packages.

**Blocked by:** 04: Connect locally from web setup to native playback.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 89, 90, 91, 94.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] AppImage and Flatpak contain matching WebEngine widget and channel support, helper processes, resources, locales, and all packaged frontend assets.
- [ ] Helper paths, permissions, file dialogs, and Chromium sandbox operation work in the actual packages without a development environment.
- [ ] Actual Chromium GPU acceleration is recorded. Supported X11, Wayland, and gamescope window handoff and focus behavior are checked.
- [ ] Native OpenGL and Vulkan playback, supported HDR behavior, input, audio, microphone operation, and direct streaming pass package smoke checks.
- [ ] Architecture and runtime combinations are reported with tested module availability. Unsupported combinations do not gain a second setup UI.
- [ ] Automated package checks detect missing assets and helper files. Rerun an affected check when a later change can alter package behavior.

## Comments

- Claimed after ticket 04. Prepare current compatible AppImage and Flatpak build inputs and verify package completeness in the available WSL environment. WSL has software graphics; Linux hardware, focus, and GPU acceptance remains unverified because only a Windows PC is available.
- Deployment and dependency preparation is saved in [the waiting checkpoint](../evidence/ticket20/checkpoint.md). The matching AppImage SDK builder and Flatpak dependency build passed. Final application builds wait for the approved ticket 22 recovery and ticket 19 common package preflight and notice merges. The ticket remains claimed.
- The user then skipped the remaining build tests and performance tests to prioritize prototype fidelity. This supersedes the checkpoint's waiting state and resume plan.

## Answer

Resolved for the Linux packaging source milestone after merging integration `1d769ee9ec3bfd267137d8ca738255feed9c7f90` in `4c1e7a08`. No conflicts occurred. Windows and macOS workflow job changes are preserved.

The maintained AppImage path uses a complete matching Qt 6.9.3 SDK and WebEngine extension, with exact build-only Node and npm pins. All three Flatpak manifests use KDE and WebEngine BaseApp 6.11, the selected checkout, and shared locked offline npm inputs. CMake remains the only frontend build and RCC path. Native dependencies and QML deployment remain. The shared frontend notice and `--check-package` preflight from ticket 19 are included.

Package checks require the helper, widget and channel libraries, WebEngine resources and locales, native QML plugins, embedded frontend, and notices. They reject missing helpers, helper permission errors, packaged Node, and AppImage Qt dependencies from the host. Obsolete ARM AppImage and Qt 5 paths stop explicitly, and release workflows no longer expect those assets. ARM Flatpak is configured but untested.

Reuse the completed matching AppImage builder, Flatpak dependency build, four package rejection fixtures, and source syntax checks in [the checkpoint](../evidence/ticket20/checkpoint.md). These are preparation results, not application or package passes.

Final Linux application builds, AppImage and Flatpak creation, package startup, sandbox, file dialogs, handoff, GPU, playback, device, and performance checks were not run. The remaining validation is skipped at the user's request. Existing hardware limits also remain. The unchecked acceptance items above do not become passes. [Final evidence](../evidence/ticket20/verification.md) records this boundary. No Linux release acceptance is claimed.
