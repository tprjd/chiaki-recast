# 21: Ship the macOS web and native application

**What to build:** Supported Intel and Apple silicon Macs can launch accelerated web setup and native playback from the signed application package.

**Blocked by:** 04: Connect locally from web setup to native playback.

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 89, 90, 91, 94.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] The macOS bundle includes widget libraries, native dependencies, WebEngine helpers, resources, locales, and packaged frontend assets.
- [ ] Helper signing, required entitlements, and the existing notarization process are correct for the package.
- [ ] Actual Chromium GPU acceleration is recorded on supported target hardware and checked alongside the native renderer.
- [ ] Native playback, overlays, display scaling, controller and keyboard input, window activation, microphone permission, and setup restoration work.
- [ ] Normal and direct stream launches work without a Qt SDK, Node installation, development server, or external frontend assets.
- [ ] Package checks cover both published architectures or identify an unsupported combination explicitly, with no QML setup fallback.

## Comments

- Claimed after ticket 04. Prepare deployment, matching helper entitlements, signing, and package checks for both maintained macOS jobs. Source inspection found ad-hoc signing and no existing notarization workflow; establish the release path instead of claiming that one exists. No Mac SDK or hardware is available on this Windows host, so native build, signing, notarization, and runtime results remain unverified.
