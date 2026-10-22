# 19: Ship the Windows web and native application

**What to build:** A supported Windows machine without a Qt SDK or Node installation can launch accelerated web setup and stream in the native window from an installed package.

**Blocked by:** 04: Connect locally from web setup to native playback.

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 89, 90, 91, 94.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] The Windows package includes widget libraries, WebChannel resources, WebEngine helper files, locales, native dependencies, and required runtime libraries.
- [ ] Setup assets load offline, the Chromium sandbox remains enabled, and release remote debugging is disabled.
- [ ] Actual WebEngine GPU acceleration is recorded on the target. Both supported native renderer paths and direct streaming are exercised.
- [ ] Native video, overlays, keyboard and mouse input, controller input, microphone behavior, window handoff, and setup restoration pass package smoke checks.
- [ ] Each published Windows architecture uses a compatible toolchain and matching WebEngine modules. Unsupported combinations are reported explicitly without a QML setup fallback.
- [ ] Missing packaged assets or helper files fail the release check. Reuse passing package evidence until relevant code, dependencies, package contents, or environment change.

## Comments

- Claimed after ticket 04. Prepare the matching MSVC and WebEngine package path and update its actual published callers. Keep platform and hardware acceptance explicit and reuse source-equivalent evidence.
