# 22: Ship the Windows web and native application

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** A supported Windows machine without a Qt SDK or Node installation can launch accelerated web setup and stream in the native window from an installed package.

**Blocked by:** 04: Connect locally from web setup to native playback.

**Status:** needs-info

**Spec stories:** 89, 90, 91, 94.

- [ ] The Windows package includes widget libraries, WebChannel resources, WebEngine helper files, locales, native dependencies, and required runtime libraries.
- [ ] Setup assets load offline, the Chromium sandbox remains enabled, and release remote debugging is disabled.
- [ ] Actual WebEngine GPU acceleration is recorded on the target. Both supported native renderer paths and direct streaming are exercised.
- [ ] Native video, overlays, keyboard and mouse input, controller input, microphone behavior, window handoff, and setup restoration pass package smoke checks.
- [ ] Each published Windows architecture uses a compatible toolchain and matching WebEngine modules. Unsupported combinations are reported explicitly without a QML setup fallback.
- [ ] Package and deployment checks can be repeated as later web features land. Missing packaged assets or helper files fail the release check.
