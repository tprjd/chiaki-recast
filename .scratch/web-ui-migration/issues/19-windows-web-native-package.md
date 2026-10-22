# 19: Ship the Windows web and native application

**What to build:** A supported Windows machine without a Qt SDK or Node installation can launch accelerated web setup and stream in the native window from an installed package.

**Blocked by:** 04: Connect locally from web setup to native playback.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 89, 90, 91, 94.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] The Windows package includes widget libraries, WebChannel resources, WebEngine helper files, locales, native dependencies, and required runtime libraries.
- [x] Setup assets load offline, the Chromium sandbox remains enabled, and release remote debugging is disabled.
- [ ] Actual WebEngine GPU acceleration is recorded on the target. Both supported native renderer paths and direct streaming are exercised.
- [ ] Native video, overlays, keyboard and mouse input, controller input, microphone behavior, window handoff, and setup restoration pass package smoke checks.
- [x] Each published Windows architecture uses a compatible toolchain and matching WebEngine modules. Unsupported combinations are reported explicitly without a QML setup fallback.
- [x] Missing packaged assets or helper files fail the release check. Reuse passing package evidence until relevant code, dependencies, package contents, or environment change.

## Comments

- Claimed after ticket 04. Prepare the matching MSVC and WebEngine package path and update its actual published callers. Keep platform and hardware acceptance explicit and reuse source-equivalent evidence.

## Answer

Windows x64 now uses one compatible MSVC/WebEngine deployment path across release, weekly, PR, and manual builds. The portable ZIP includes native and Qt dependencies, all Chromium locales, offline RCC setup, and dependency notices. Unsupported MinGW and ARM64 routes report the limit without a QML fallback.

The actual staged normal application, restricted renderer token, hardware Chromium acceleration, native Fast OpenGL/Vulkan rendering, startup handoff, reload, and setup restoration passed. Every ZIP file hash matches the prelaunch stage. The full native settings state was restored. See [verification](../evidence/ticket19/verification.md) for source/artifact hashes and exact limits.

This resolves the Windows package milestone. The unchecked combined criteria retain live/direct CLI, physical input, microphone, focus, and remaining hardware acceptance in [23](23-complete-web-only-acceptance.md). Default HighQuality processing remains an independent native failure. Local installer execution is unverified because automatic approval review blocked the compiler tool installation. The reviewed CI installer path remains available; no push or publish ran.

The user then requested that remaining build/test checks and performance work be skipped so work can move to prototype fidelity. Completed package evidence is retained; no further verification ran after that instruction.
