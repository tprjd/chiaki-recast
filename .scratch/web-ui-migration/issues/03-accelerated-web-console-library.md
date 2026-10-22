# 03: Show live consoles in the GPU-accelerated web UI

**What to build:** Normal startup opens a packaged Svelte setup window that uses GPU acceleration and displays real discovered consoles through Qt WebChannel.

**Blocked by:** 01: Preserve native playback with application-owned services.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 1, 2, 5, 6, 7, 8, 9, 11, 35, 36, 62, 63, 64, 65, 66, 67, 68, 69, 70, 90, 91.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] Normal startup loads packaged Svelte 5 and TypeScript output in QWebEngineView. The GUI build includes WebEngineWidgets, WebChannel, and the frontend build dependency.
- [x] The unconditional startup override that disables WebEngine GPU acceleration is removed from the web setup path. Actual Chromium acceleration is verified in the embedded application.
- [x] Complete the required graphics check carried from [01](01-application-owned-native-services.md). Use a compatible toolchain to prove accelerated embedded Chromium beside the native renderer. Record actual GPU state and environment limits. Module availability or software rendering cannot establish a pass. Ticket [04](04-local-native-stream-handoff.md) extends this check through native handoff.
- [x] One restricted production SetupBridge supplies current console state, capabilities, and discovery control through real WebChannel transport. Backend QObjects and secrets are not published.
- [x] Console keys are native-issued and profile-scoped. Live updates preserve selection, and missing or ambiguous targets are rejected.
- [x] The selected card library design uses shared CSS values and real artwork. Search, empty states, long names, and discovery changes display correctly.
- [x] Packaged assets, scripts, styles, and fonts load offline without a runtime Node server. Toolchain versions and the package lock are pinned.
- [x] The trusted setup page blocks external navigation and unexpected resources, applies its content policy, and treats console names as text. Streamer mode masks private values.
- [x] Keyboard, native controller navigation, desktop touch, visible focus, labels, reduced motion, scaling, and the specified window sizes work. Translation support is established for migrated text.
- [x] One embedded application harness exercises the compiled Svelte UI, real QWebEngineView, real WebChannel transport, production bridge, and native services. It observes native discovery updates and a discovery command. Control external results at native asynchronous request boundaries.
- [x] Perform an initial packaged startup check on the first supported target. Verify offline assets and actual Chromium GPU acceleration. Reuse applicable graphics evidence from the prerequisite check.
- [x] Normal startup selects the web UI without adding a QML setup fallback. Later web flows extend the same harness instead of adding a mocked bridge or a separate test API per screen.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Answer

Normal setup now selects the static Svelte 5/TypeScript library in a restricted
QWebEngineView. The sole production bridge publishes current display state,
opaque profile-scoped console keys, selection, and discovery commands. Native
services retain routing, credentials, media, raw game input, and GPU resources.
The GUI requires Qt 6.9.3 or later and pinned frontend build tools.

[Verification](../evidence/ticket03/verification.md) records the staged real
application harness library pass (3/0), Windows hardware Chromium/native
coexistence pass (3/0, 5301 ms), frontend check, initial package startup, Linux
production build, source/binary identities, and inspected images. The native
graphics sequence uses the existing Fast preset and covers OpenGL, Vulkan,
recreation, forced fallback, resize/destruction, and restored web discovery.
Production defaults are unchanged.

This resolves the console-library and graphics foundation milestone. The
HighQuality native processing failure remains explicit acceptance work in
[04](04-local-native-stream-handoff.md) and
[23](23-complete-web-only-acceptance.md). It is not hidden by a lowered default.
Physical controller/touch/screen-reader checks, translated locales, display
scales, other platform GPUs, live media/HDR, and matched latency/performance
remain in final hardware acceptance. The actual staged application's help
entrypoint and isolated harness setup were checked; normal application startup
against the user's real default profile was not exercised. Remaining QML setup
components still have later-flow/test callers; the production native root is
stream-only, with no QML setup fallback.

Integration merge `fd207526` contains worker commit `ec92fb73` without source
changes or conflicts. The merged tree matches the worker tree. Recorded
source, package executable, frontend lock, and image digests match. The
production source matches library build `0e2702b7`; the final graphics source
matches `713b5861`. The recorded passing checks were reused without builds or
test reruns. Raw logs retain their original whitespace; the source-only
whitespace check passes.
