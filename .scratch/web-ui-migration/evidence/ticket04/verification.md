# Ticket 04 verification

Date: 2026-10-22. Branch: `codex/web-ui-ticket-04`.

The local handoff and native lifetime milestone passes. The production page
starts one native connection from the current opaque console key. The native
window presents progress before setup hides. The backend still owns connection
state. Native cleanup completes before queued window destruction and setup
restoration. The shared backend and QML engine survive repeated windows.

This is not full hardware acceptance. Live-console playback, actual foreground
focus, hardware decoding, the native video-disabled route, other platforms,
and full HighQuality/default-preset output remain open in ticket 23.

## Source and artifacts

The final code is `effd472a54ff5bd5fe677b86803361c20dacda1e`, tree
`d3ed83043981e4b5535be67024b0f906e848815d`. It contains integration tip
`2abe34f588a9c91d7e39cbb1b991559e69bc1666`, including tickets 06, 11, and 12.
The initial isolated worktree was clean at claim commit
`b90c7f27d4f8c36c5272502fe764058fc9664fcf`. The original dirty user checkout
was not used or modified.

[identity.json](identity.json) records the final code tree, changed source
SHA256 values, lock file, application, harnesses, and built assets.
[tested-affected-artifacts.json](tested-affected-artifacts.json) records the
actual binaries used by the final affected runtime group at source `7cd7ec8e`.
The embedded harness digest for that group is
`B6FF2881FCF94A35BDFE7C4165486EB698C80CCB450FBB4EF2A02068D41769C7`.
The direct native harness digest is
`5411687F82CF603A80EFA803ED603B539D8A2235EB3E1081B6D543F77DB6050E`.

The earlier initialization and decoded-cleanup harness executables were
overwritten before their binary digests were recorded. Their logs remain
valid behavioral evidence, but no exact binary digest is claimed for those
runs. `55140aa9` records their implementation source; two unused includes were
removed before that commit. [source-equivalence.json](source-equivalence.json)
records 29 passing source comparisons. The native controller, renderer,
session, and relevant cleanup methods match that source. Both initialization
and decoded-cleanup test bodies are unchanged. The affected runtime group
matches the final native and sign-in lifetime source, and its test bodies are
unchanged after the ticket 12 merge.

The last merge adds media settings fields and controls. Its three shared
bridge conflicts preserve both field sets, the media setter, and the hidden
page emission guard. The final application and both harnesses build in
[final-build.txt](final-build.txt). The final frontend has zero errors and
warnings in [final-frontend-check.txt](final-frontend-check.txt). No valid
runtime check was repeated only to obtain a digest or after an unrelated merge.

[configure.cmd](configure.cmd) and [affected-build.cmd](affected-build.cmd)
use the separate `ticket04-build` cache. The toolchain is Qt 6.9.3 MSVC x64,
clang-cl 19.1.7, FFmpeg 7.1, libplacebo 7.360.1, and Vulkan SDK 1.4.357.0.
Pinned submodules were initialized. The existing Windows gf-complete patch
was applied locally for MSVC; the vendor gitlink and patch are not committed.

## Focused behavior checks

| Case | Result | Evidence and actual observation |
| --- | --- | --- |
| Local handoff freeze/return | 3 passed, 0 failed, 1861 ms | [green-freeze.txt](green-freeze.txt): real compiled page and bridge create the native window/session; setup freezes; dismissal alone does not destroy an active session; cancel cleans up and restores the active page. |
| Real native initialization failure | 3 passed, 0 failed, 827 ms | [green-initialization.txt](green-initialization.txt): Windows offscreen QPA cannot create the native OpenGL context; setup stays visible, no window/session remains, and the retained error dismisses once and stays dismissed after reload. |
| Decoded session cleanup | 3 passed, 0 failed, 3744 ms | [decoded-cleanup-final.txt](decoded-cleanup-final.txt): native PIN/retry, menu input ownership, disconnect confirmation, fresh discovery state, repeated native windows, real decoder and frame worker, all tracked buffers released. |
| Suspend closes native sign-in | 3 passed, 0 failed, 772 ms | [green-suspend-sign-in.txt](green-suspend-sign-in.txt): real page opens native sign-in; pending native exchange survives page close and setup freeze; completion persists native credentials; resume reads success without replay or token disclosure. |
| Direct native caller | 3 passed, 0 failed, 1646 ms | [green-direct-native.txt](green-direct-native.txt): native window recreation, shared settings/backend, direct-stream creation and exit, and fallback destructor cleanup. No setup page is constructed. |

GUI and native tests ran only in root-assigned serial slots. The affected
sign-in and initialization checks used test-only offscreen QPA and Chromium
`--disable-gpu`. They establish behavior, not acceleration. Decoded cleanup
and the direct caller used native Windows rendering. No foreground or screen
output claim is made while the active input desktop is protected.

The maintained commands select only their named cases. See `freeze-green.cmd`,
`init-green.cmd`, `decoded-final.cmd`, `suspend-green.cmd`, and
`direct-native.cmd`. The final direct native target required `#undef main`
because SDL redirects `main` on Windows. The embedded harness already uses
that convention.

## Decoder and teardown boundary

`decodedSessionCleanup` uses a controlled loopback transport, the production
C event dispatch method, real native PIN controls, and the existing FFmpeg
sample callback. It decodes the maintained `gui/tests/fixtures/red.h264` at
16 ms cadence. The native decoder generates advancing synthetic packet PTS
with its production frame duration. It uses the existing Fast preset through
native Settings and asserts the actual native API. No production preset or
GPU-disable flag is changed.

The OpenGL pass submits 16 timed samples and releases all 51 tracked AVBuffer
allocations after normal disconnect confirmation. The Vulkan pass submits
17 timed samples and releases all 54 allocations after controller destruction.
An additional sample is queued before each teardown. GUI delivery stays
queued while the production worker pulls the decoded sample. This covers the
controller route and the active-window fallback destructor route. Both return
with no native window/session and no outstanding tracked decoded buffer.

`WaitForStop` reuses the native producer join and is idempotent. Cleanup first
stops the session and pending PSN start work, then joins the producer, drains
the frame worker, destroys queued frame-delivery callbacks, and deletes the
decoder/session. Window destruction retains the QML root's backend/window
references during root callbacks, then detaches and cleans the session before
swapchain and GPU release. Native initialization failures retain their error;
they do not throw across the render thread or call the removed fatal paths.

The fixture clears the saved console PIN because a zeroed registered-host
structure otherwise produces the saved string `0` and selects automatic PIN
submission. No production PIN change was needed. After disconnect, native
discovery restarts and can remove the controlled synthetic host. The test
checks that fresh state, then supplies the next controlled discovery result.
The small handoff check now uses the same fresh-discovery assertion instead
of assuming the old Connect action stays enabled.

## Red checks and observation corrections

The missing Connect action failed before implementation in `red-handoff.txt`.
The hidden page remained Active before freezing was added (`red-freeze.txt`).
The real offscreen OpenGL failure aborted the old fatal path
(`red-offscreen-initialization.txt`, exit -1073740791); the green process exits
normally. Process-local abort UI suppression prevents an expected red abort
from leaving a crash dialog and does not change global OS settings.
`red-suspend-sign-in.txt` fails because the native sign-in window survives
suspend; the close-before-hide hook makes the same case pass.

The earlier OpenGL version-99 experiment did not force initialization failure:
Qt obtained a valid context. It is not failure evidence. Decoder intermediate
failures came from the saved-PIN fixture and an incorrect assumption about
fresh discovery. The final maintained case fixes those observations and
passes. These logs are retained so the failed assertions are not presented as
product regressions or hidden from review.

## Reused graphics evidence and remaining acceptance

[Ticket 03 verification](../ticket03/verification.md) remains the applicable
Windows hardware Chromium ANGLE D3D11 plus native Fast-preset OpenGL/Vulkan,
recreation, forced fallback, resize/destruction, and setup-return evidence
(3/0, 5301 ms, source `713b5861`). Root inspected its four opaque captures.
The graphics import and package/library evidence were not changed or repeated.

The [minimal native HighQuality control](../native-highquality-control/pure-libplacebo-mix-control/verification.md)
reproduces wrong pixels in pure `pl_render_image_mix` without Qt Quick, QRhi,
WebEngine, or WebChannel. Its paired compute-disabled call passes. This
localizes a native dependency/control failure independently of this handoff;
it is not a complete pre-migration application or live-console baseline.
Production defaults and the user's selected preset are preserved.

Ticket 23 still needs a real console, actual foreground/focus and physical
input, hardware decoding, audio/video-disabled operation, console sleep and
held-input release under live input, live settings effects, default and other
presets, matched performance measurements, and supported-platform/package
acceptance. These checks do not certify those conditions.
