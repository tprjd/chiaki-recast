# 24: Ship the macOS web and native application

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Supported Intel and Apple silicon Macs can launch accelerated web setup and native playback from the signed application package.

**Blocked by:** 04: Connect locally from web setup to native playback.

**Status:** needs-info

**Spec stories:** 89, 90, 91, 94.

- [ ] The macOS bundle includes widget libraries, native dependencies, WebEngine helpers, resources, locales, and packaged frontend assets.
- [ ] Helper signing, required entitlements, and the existing notarization process are correct for the package.
- [ ] Actual Chromium GPU acceleration is recorded on supported target hardware and checked alongside the native renderer.
- [ ] Native playback, overlays, display scaling, controller and keyboard input, window activation, microphone permission, and setup restoration work.
- [ ] Normal and direct stream launches work without a Qt SDK, Node installation, development server, or external frontend assets.
- [ ] Package checks cover both published architectures or identify an unsupported combination explicitly, with no QML setup fallback.
