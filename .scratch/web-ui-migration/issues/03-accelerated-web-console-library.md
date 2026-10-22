# 03: Show live consoles in the GPU-accelerated web UI

**What to build:** Normal startup opens a packaged Svelte setup window that uses GPU acceleration and displays real discovered consoles through Qt WebChannel.

**Blocked by:** 01: Preserve native playback with application-owned services.

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 1, 2, 5, 6, 7, 8, 9, 11, 35, 36, 62, 63, 64, 65, 66, 67, 68, 69, 70, 90, 91.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] Normal startup loads packaged Svelte 5 and TypeScript output in QWebEngineView. The GUI build includes WebEngineWidgets, WebChannel, and the frontend build dependency.
- [ ] The unconditional startup override that disables WebEngine GPU acceleration is removed from the web setup path. Actual Chromium acceleration is verified in the embedded application.
- [ ] Complete the required graphics check carried from [01](01-application-owned-native-services.md). Use a compatible toolchain to prove accelerated embedded Chromium beside the native renderer. Record actual GPU state and environment limits. Module availability or software rendering cannot establish a pass. Ticket [04](04-local-native-stream-handoff.md) extends this check through native handoff.
- [ ] One restricted production SetupBridge supplies current console state, capabilities, and discovery control through real WebChannel transport. Backend QObjects and secrets are not published.
- [ ] Console keys are native-issued and profile-scoped. Live updates preserve selection, and missing or ambiguous targets are rejected.
- [ ] The selected card library design uses shared CSS values and real artwork. Search, empty states, long names, and discovery changes display correctly.
- [ ] Packaged assets, scripts, styles, and fonts load offline without a runtime Node server. Toolchain versions and the package lock are pinned.
- [ ] The trusted setup page blocks external navigation and unexpected resources, applies its content policy, and treats console names as text. Streamer mode masks private values.
- [ ] Keyboard, native controller navigation, desktop touch, visible focus, labels, reduced motion, scaling, and the specified window sizes work. Translation support is established for migrated text.
- [ ] One embedded application harness exercises the compiled Svelte UI, real QWebEngineView, real WebChannel transport, production bridge, and native services. It observes native discovery updates and a discovery command. Control external results at native asynchronous request boundaries.
- [ ] Perform an initial packaged startup check on the first supported target. Verify offline assets and actual Chromium GPU acceleration. Reuse applicable graphics evidence from the prerequisite check.
- [ ] Normal startup selects the web UI without adding a QML setup fallback. Later web flows extend the same harness instead of adding a mocked bridge or a separate test API per screen.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.
