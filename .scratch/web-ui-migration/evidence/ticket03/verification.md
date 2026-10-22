# Ticket 03 verification

Date: 2026-10-22. Branch: `codex/web-ui-ticket-03`.

The compiled Svelte library, restricted WebChannel bridge, native discovery
commands, privacy rules, staged package checks, and required Windows GPU
coexistence milestone pass. The final production Fast-preset graphics case
passed **3/0 in 5301 ms**, including real native video and overlays beside
hardware-accelerated Chromium, both native APIs, recreation, forced fallback,
resize/destruction, and restored web discovery. HighQuality/default processing
acceptance remains explicitly open in tickets 04 and 23. Production defaults
are unchanged.

## Source and build identity

The final library source is commit
`0e2702b7ce333a5cb121c077dee17cde99b0f7e1`, tree
`7df3e40c3ec287f8718bb31dd8a04b14c902e825`. It includes integration commit
`037d116f`, ticket 02's production `StreamRoot.qml`, and native graphics helper
commits `e77a7d08` and `2ca47d62`.

[source-and-package.json](source-and-package.json) records SHA256 values for
the built application, embedded harness, and frontend lock file. The staged
application digest is
`1590764248AD82A41E003B7302DD2E238B38287C288C01446D0A87BCFF276664`.
The passing library harness digest is
`29459FCF938A2C1815311D63C3868676160650C3C3D9E2371171E730032840B4`.
The final maintained graphics harness source is `713b5861`; its SHA256 is
`CBDAB5817958174A92D8BB2006CB36A58DBDE970CA5BFADFDF23ADF368AEBED5`.
The JSON records its full commit and tree. Its changes after `0e2702b7` are
test-only observation, preset selection/assertion, and documentation changes.
[final-source-sha256.json](final-source-sha256.json) records the final native,
bridge, setup window, and harness source-file digests.

The Windows build uses Qt 6.9.3 MSVC x64, clang-cl 19.1.7, FFmpeg 7.1,
libplacebo 7.360.1, and Vulkan SDK 1.4.357.0. Exact configure arguments are in
[configure.cmd](configure.cmd); [configure.txt](configure.txt) is its output.
The build cache is external at
`C:/Users/david/.cache/chiaki-msvc-native/ticket03-build`.
The existing Windows `gf-complete` compatibility patch was applied locally
for this compiler. Its source gitlink remains unchanged and the vendor
working change is excluded from the commit.

The GUI now requires Qt 6.9.3 or later, WebEngineWidgets, WebChannel, Widgets,
and matching GuiPrivate headers. Qt 6.9.3 supplies the Chromium 130 baseline
used by the Vite `chrome130` target. Private QRhi compatibility requires checks
with each supported Qt build. Older Qt 6.7 package builders need the toolchain
update assigned to ticket 20.

Frontend pins are Node 24.16.0, npm 11.13.0, Svelte 5.57.2,
`@sveltejs/vite-plugin-svelte` 7.3.1, Vite 8.3.5, svelte-check 4.7.7,
and TypeScript 5.9.3. The exact lock file and engine checks are committed.

The distinct Linux changed-source build also passed for `0e2702b7` with Qt
6.10.2, matching GuiPrivate, GCC 15.2.0, Node 24.16.0, and npm 11.13.0.
It linked the full application and seven test executables. One offscreen
`chiaki --help` exited 0. No test executable or GUI/GPU check ran.
[linux-build.md](linux-build.md) gives the commands and limits; [linux/](linux/)
contains the final logs, archive input manifest, source audit, and artifact
digests. The audit found no change among 9,445 archived source files.
The later Windows capture observer is under `Q_OS_WIN`; its common test-only
change preserves the last QImage and pixel counts on early exit. These changes
do not change the Linux production sources that passed compilation.

## Maintained application harness

`chiaki-embedded-setup-tests` loads the production compiled page in the real
QWebEngineView, Qt WebChannel, SetupBridge, and application-owned native
services. It uses isolated native settings. Discovery replies enter the
production queued `DiscoveryServiceHosts` completion boundary. There is no
fake WebChannel implementation or component-specific production test API.

The final staged `consoleLibrary` run passed: **3 passed, 0 failed**, 3083 ms.
See [package-library.txt](package-library.txt). It checks:

- Empty discovery state and queued discovery updates to the visible page.
- Literal untrusted console names and actual packaged PS4/PS5 artwork.
- Native UUID selection, stable keys after discovery reorder, current native
  row resolution, duplicate and missing target rejection, and profile-key
  invalidation.
- Search, reload/resubscribe state, and real discovery button commands that
  change the native discovery setting.
- Qt window key delivery for card navigation, search focus, empty-library
  Discovery focus, and Escape recovery from a search with no results.
- Streamer-mode address masking and native window-title masking; addresses,
  MAC values, tokens, and PIN values are absent from the public snapshot.
- Rejected popup and external main-frame navigation, 200% zoom at 800 by 600,
  and no horizontal clipping at 390 by 720 logical pixels.

The initial compile red is in [red-build.txt](red-build.txt). It failed because
the new production setup header did not yet exist. The privacy regression red
is in [red-private-title.txt](red-private-title.txt); the old native title
disclosed the profile while streamer mode was active. Its green is
[green-private-title.txt](green-private-title.txt).
[red-empty-controller.txt](red-empty-controller.txt) records the old empty-page
focus failure before the final navigation change. The final package run checks
both that fix and Escape recovery. Escape prevents its default action before
the asynchronous Svelte flush, then resolves the newly restored card elements.

[frontend-check.txt](frontend-check.txt) reports zero errors and warnings after
the final navigation change. [navigation-green-build.txt](navigation-green-build.txt)
records the affected application and harness build.

Root visually inspected [library-1280.png](library-1280.png), captured at
1280 by 720 logical pixels and 1600 by 900 device pixels. The cards, long
untrusted literal name, selected state, artwork, search, and discovery controls
fit. Root also inspected [package-library-390.png](package-library-390.png):
the narrow layout reflows, long names wrap, controls remain reachable, and
vertical scrolling is present without horizontal clipping.

The page uses shared CSS values, visible focus, native button/input labels,
reduced-motion rules, and touch-sized discovery controls. Migrated text enters
Qt translation extraction through `QT_TR_NOOP`/`tr`, then crosses the bridge as
display strings. Physical controller, touchscreen, screen reader, translated
locale, and OS display-scale acceptance remain in ticket 23. The native
controller route maps buttons to the same Qt window keys tested here; this is
controlled route evidence, not a physical-controller claim.

## Restricted production surface

Only `SetupBridge` is registered with WebChannel. It publishes display state,
capabilities, string revisions, opaque console keys, selection, and discovery
commands. Native target lookup remains a C++ operation and rereads current
hosts before dispatch. Raw backend/settings objects, addresses, IDs used for
native routing, tokens, PIN values, media, raw game input, and GPU handles do
not cross this bridge.

The setup profile is off the record with memory cache and no persistent
cookies. Its request interceptor permits only the exact packaged setup files,
Qt WebChannel script, and three existing artwork resources. The page applies a
content security policy, rejects other navigation and popups, and has no
clipboard or fullscreen privilege. The production page contains no sample
consoles or runtime web server. Normal `RunMain` selects SetupWindow directly;
there is no QML setup fallback or UI selector. Remaining QML setup components
and wrappers still have callers in flows assigned to later tickets and their
tests. Shared native stream components remain in the production resource file.

## Initial package check

The staged directory is
`C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket03-package`.
It contains the built application and maintained harness, native DLLs,
`QtWebEngineProcess.exe`, Qt plugins and QML modules, WebEngine resource packs,
the `en-US` locale, and relative `qt.conf` paths. The Svelte assets are compiled
into the executables. [deploy.txt](deploy.txt) and
[package-files.txt](package-files.txt) record the deployment.

[package-run.cmd](package-run.cmd) uses only the package and Windows system
directories in PATH. It clears old Qt paths and GPU-disable overrides, points
Qt to the staged dependencies, and finds neither Node nor qmake. See
[package-environment.txt](package-environment.txt). The first stage omitted
`lcms2.dll`; a PE dependency check identified libplacebo's missing dependency.
Adding the existing compatible DLL corrected startup. The Windows workflow
already includes that DLL.

Two entrypoints were checked. The actual staged `chiaki.exe --help` exited
successfully with the clean dependency environment; it exits before creating
Settings. See [package-help.txt](package-help.txt) and
[package-help.cmd](package-help.cmd). The maintained staged harness then opened
the normal setup components with isolated settings and passed `consoleLibrary`.
The actual application's normal `RunMain` entrypoint was not launched against
the user's default profile. This is an initial staged dependency/startup check,
not a clean-machine installer or complete application-entrypoint acceptance.

## Graphics prerequisite and current limit

The final independently selected `graphicsCoexistence` case passes in the
dependency-isolated staged package. See
[graphics-coexistence.txt](graphics-coexistence.txt),
[final-graphics-build.txt](final-graphics-build.txt), and
[package-graphics.cmd](package-graphics.cmd). It uses the existing native Fast
preset through Settings, asserts the actual native preset before each window,
and uses the real production renderer and `StreamRoot.qml`. No temporary
parameter override, old import, direct-image branch, or readback code remains.

| Pass | Native backend | Red frame samples | Blue overlay samples | Capture predicate elapsed |
| --- | --- | ---: | ---: | ---: |
| 0 | OpenGL | 196033 | 1772 | 293 ms |
| 1 | Vulkan | 196026 | 1757 | 230 ms |
| 2 | Vulkan recreation | 196013 | 1758 | 229 ms |
| 3 | Forced Vulkan-to-OpenGL fallback | 148868 | 1260 | 288 ms |

The global Qt Quick API stayed fixed at D3D11. Chromium hardware compositing
passed before native creation. Each imported native device matched libplacebo.
The input desktop was `Default`; actual Win32 foreground matched each native
window in this run. Resize, native destruction, web restoration, and native
discovery commands after each return passed. These capture times are test
observations, not streaming latency or startup-performance measurements.

Both ticket 03 and root visually inspected all four `native-overlay-*.png`
files. They show opaque red fixture video, black letterbox bands, the real
notice text/accent, and the real fallback dialog on pass 3. They contain no
unrelated desktop content and are safe to commit. The restored setup images
contain the synthetic manual host. Image digests are in
[successful-images-sha256.json](successful-images-sha256.json).

The native graphics helper replaced per-window global API changes with a
per-window application-owned QRhi imported from the actual native OpenGL
context or libplacebo Vulkan device/queue. RenderControl uses `fromRhi` and
checks its actual QRhi backend and imported native device on the owner thread.
The QRhi is released before its imported device and render thread.

The helper's GNU Windows Qt 6.11.2 native lifecycle checks passed independently:
OpenGL 3/0 in 1340 ms, Vulkan 3/0 in 3462 ms, and forced Vulkan-to-OpenGL
fallback 3/0 in 1342 ms. See `native-graphics-green-*.txt`.
`native-graphics-msvc-compile.log` records its Qt 6.9.3 MSVC source compile.
That GNU Qt installation has no WebEngine, so those passes establish native
lifecycle/device checks only. They do not pass combined GPU rendering.

[chromium-gpu.txt](chromium-gpu.txt) records real embedded Chromium hardware
compositing, rasterization, WebGL, and WebGL2 on NVIDIA GeForce RTX 5090,
ANGLE D3D11, driver 32.0.16.1742. The process-wide Qt Quick API stayed D3D11
during these observations. The report has `In-process GPU: true` and
`Sandboxed: false`; no full sandbox or release-security certification is
claimed. The final independent graphics case recorded this report again with
the same pre-application WebEngine initializer that production uses and passed
its hardware-compositing assertion. Its exact binary digest is in the JSON.

Earlier native capture attempts are retained as **inconclusive observations**:
`red-gpu-coexistence.txt`, `red-desktop-capture.txt`, `red-native-only.txt`,
`native-matched-api.txt`, and `observation-native-only-last.txt`.
Windows Security/PickerHost owned the real foreground while Qt reported native
activation. Protected desktop content and focus therefore invalidate those
screen results. The external `ticket03-native-final-0.png` is a flat capture from that period, not a
passing native frame or proof of a renderer defect. A direct window capture
also has Qt's documented layered-window capture limit; the maintained check
now captures the desktop region. Temporary GL readback instrumentation was
removed because its initial read framebuffer was not bound to the output.
No production render-state fix was made from these inconclusive results.

All failed native screen images remain outside Git. The old flat capture is
`ticket03-native-final-0.png` (SHA256
`6CBE8CE4F1272E74DF3ACB48446368982DAC9E3B173CFEB39E93E55D2332631C`).
Two stale native captures are `ticket03-native-overlay-stale-1.png` (SHA256
`E163D2389D2DDC784489370E62918BF365FB832B16A5291B1F65E9A68278BEA9`)
and `ticket03-native-overlay-stale-2.png` (SHA256
`560B0159C565CF309D6BE9D60D420799F07F22199F43A3D4BA57EA7D5EF20939`).
These files are in the external temporary directory listed above. They are
not passing evidence. Repository PNGs contain synthetic setup and native
graphics fixtures only.

After Windows Security closed, one independent run stopped on an over-strict
foreground ownership check. Qt reported the native window exposed and active,
but Windows retained Vivaldi as foreground. See
[observation-foreground-policy.txt](observation-foreground-policy.txt).
That result does not establish a rendering failure. The GPU observer was
corrected to use a test-only nonactivating topmost window, matching physical
client size, four interior WindowFromPoint/GetAncestor checks, and absence of
a visible PickerHost window. Foreground ownership remains a diagnostic and
ticket 04/23 acceptance item; no synthetic input or thread attachment is used.

The first corrected observation had a 5-second pixel deadline. Qt reported
that its predicate transiently matched after 9.9 seconds, but the macro exited
before the image could be saved. See
[observation-first-frame-deadline.txt](observation-first-frame-deadline.txt).
That predicate result does not prove native render readiness: changing desktop
content behind a transparent native area can contain similar colors.

The earlier HighQuality `graphicsCoexistence` run used the same exact pixel
predicate with a 15-second deadline and failure-safe final capture/counts. It passed all
unobscured-window checks and recorded a matching actual OpenGL/NVIDIA QRhi
import, but failed native output on the first OpenGL pass. Its final image had
76 blue samples and 149 red samples, below the required 500 and 5000.
The last capture completed at 55,704 ms; the whole test took 56,864 ms.
Qt's timeout diagnostic retry and the synchronous screen capture contribute
to this elapsed time. This is not a streaming latency or startup-performance
measurement. See [observation-highquality-native-output.txt](observation-highquality-native-output.txt),
[graphics-observation-build.txt](graphics-observation-build.txt), and
[package-graphics.cmd](package-graphics.cmd).

The captured native area showed underlying desktop content and black bands,
without the required video frame or blue overlay. The PNG is retained outside
the repository because it contains the user's background browser content:
`C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket03-native-output-failed.png`.
Its digest is in the JSON. This remains an unresolved native output/capture
failure; it is not attributed to the imported QRhi without a controlled
diagnosis. No production rendering change was made from these observations.

This earlier HighQuality run stopped at the first OpenGL pass. The final Fast
run above completes the required milestone sequence. It does not certify
HighQuality, other presets, live decode, or other platforms.

## Bounded native GL diagnosis

Temporary native diagnostics read the actual default back buffer immediately
before swap. They bind `GL_READ_FRAMEBUFFER=0` and `GL_BACK` explicitly, then
restore the prior read framebuffer and read buffer. They do not send graphics
data through JavaScript or alter texture configuration. Each diagnostic run
ends with an explicit diagnostic-only failure before screen polling; none is
a graphics acceptance pass. Exact patches, commands, logs, and available
binary/source digests are retained in this directory and
[diagnostic-sources.json](diagnostic-sources.json). The instrumentation was
removed from maintained source before the final passing graphics run.

The valid baseline has the correct native context and onscreen surface,
double buffering, 1200 by 800 physical size, all color write masks enabled,
and zero errors before reading, after reading, and after state restoration.
The center is opaque black, while the known production overlay sample
animates blue with alpha 255. Existing read FBO 2 is restored after each
default-buffer sample. See [gl-back-baseline.txt](gl-back-baseline.txt).

The controlled red frame is present in the native frame mix with three
mapped planes. The existing loading item and its enabled native signal
connections exist before `presentFrame`; `hasVideoChanged` fires once. After
one second, `sessionLoading`, `legacySessionLoading`, and `managedConnection`
are false, and the black loading cover is invisible with opacity zero.
This excludes the proposed fixture order/loading-cover cause. See
[gl-back-fixture-baseline.txt](gl-back-fixture-baseline.txt).

A temporary `QQuickOpenGLUtils::resetOpenGLState` comparison after Quick's
`endFrame` did not restore the red video. Pre-reset blending was sometimes
enabled with `GL_CONSTANT_COLOR`/`GL_ONE_MINUS_SRC_COLOR`; stencil and scissor
were disabled and color masks enabled. Resetting this state changed the
fading cover's output alpha but left center RGB black. The negative comparison
ended that hypothesis; the reset call was removed. See
[gl-state-comparison.txt](gl-state-comparison.txt). No production reset fix is
claimed.

The next diagnostic reads the existing native textures with
`pl_opengl_unwrap` and `glGetTextureSubImage`, without creating a host-readable
texture variant. It saves and restores the pixel-pack buffer and pack
alignment, row length, image height, and all skip values. Direct-state access
does not change texture bindings. At 760 ms, actual uploaded Y/U/V center
bytes are `51`/`5a`/`f0` (81/90/240), each in an R8 2D texture. The existing
RGBA8 Quick texture's center is `00000000`, so it is transparent at the video
center. All texture reads and state restoration report zero GL errors.
The default back buffer remains `000000ff` at center, while the real overlay
sample is `0086ccff`. See [gl-texture-diagnostic.txt](gl-texture-diagnostic.txt).
The mapped crop is 0,0 to 320,180; plane sizes are 320 by 180, 160 by 90,
and 160 by 90, with Y/U/V channel mapping and limited-range BT.601 metadata.
The remaining issue is in native composition/output, after upload and outside
the loading cover. Source review continues before another runtime check.

The two independent source reviewers then selected a one-frame direct-image
comparison. One stable render used `pl_render_image` with the same renderer,
mapped frame, target, crop, HighQuality parameters, and overlay. It returned
true, but the immediate default back buffer center was `00000000` and the
overlay remained blue. Input texture bytes were unchanged. This negative
comparison excludes the extra mix/cache path as a sufficient cause. The
temporary direct-image branch was removed. See
[gl-direct-image.txt](gl-direct-image.txt).

A bounded old-import control selected global OpenGL before initialization,
created no WebEngine page, and used `fromOpenGLContext` for the same native GL
fixture. It also produced black video and a blue overlay, with correct texture
bytes and zero GL errors. Its actual target crop was 0,62.5 to 1200,737.5;
target and mapped input bit encoding were both 8/8/0. This shows that the new
per-window QRhi import and WebEngine coexistence are not sufficient causes of
the black buffer. It is not a full pre-migration baseline: native services and
the stream root are current. This temporary partial import reversion crashed
during teardown after the intentional diagnostic failure (exit -1073741819).
The valid buffer observations preceded teardown; no lifecycle pass is claimed.
The old-import control was removed. See [gl-old-control.txt](gl-old-control.txt).

One comparison then changed only the temporary render parameter pointer to
`pl_render_fast_params`. It restored the expected opaque native red video:
default back buffer center `fe0000ff` and overlay `0086ccff` at 754 ms, with the
same Y/U/V bytes, transparent Quick center, and zero GL errors. This used the
production per-window QRhi and accelerated embedded WebEngine under global
D3D11. It localizes the black output to processing enabled by HighQuality.
It is GPU-buffer evidence only and remains an intentional diagnostic-only
failure, not the required full graphics acceptance. No production preset
change is made. See [gl-fast-comparison.txt](gl-fast-comparison.txt) and
[diagnostic-comparisons.json](diagnostic-comparisons.json).

All temporary parameter overrides, old imports, direct-image branches, and
native readback instrumentation were then removed. The maintained graphics
case selects Fast through the existing Settings API, sets it on the current
native settings before each window, and asserts the resulting native preset.
Production defaults remain unchanged.

The first full production-Fast attempt (`ee16038e`) returned an all-black
screen capture: red 0, blue 0; last capture elapsed 50,110 ms; test total
51,309 ms. Foreground HWND was zero. A read-only native desktop query then
found thread desktop `Default` and active input desktop `Screen-saver`, with
successful read access. See [input-desktop-state.json](input-desktop-state.json).
This invalidates the screen result; it is not a native-rendering failure or
pass. The capture remains external as `ticket03-fast-desktop-capture-failed.png`,
SHA256 `4100BDB04F2C9E01137F73D2CBC6E77689CCAF755E194591CBCB51B30A8993CD`.
The corrected observer checks the active input desktop and available
foreground window before expensive work and again before native capture.
It was not rerun while the desktop was unavailable.

After the user unlocked the desktop, the read-only check confirmed active
input desktop `Default` and a nonzero foreground HWND. See
[input-desktop-ready.json](input-desktop-ready.json). The single final
maintained graphics run then passed as recorded above. No library, package,
or Linux check was repeated after the observation-only changes.

The [read-only dependency audit](graphics-build-provenance.md) found no
compiler, public-header, import-library, staged-DLL, FFmpeg, or packing mismatch.
The prefix, build, and package libplacebo DLL digests agree. No probe was live
during the audit, so it does not establish actual loaded module paths.
No dependency rebuild was made. HighQuality remains an explicit unresolved
native processing acceptance item in tickets 04 and 23 and the migration map.
Fast coexistence is not permission to lower the user's preset or certify all
processing paths.
The proposed native-only `no_compute` experiment was deferred when the user
unlocked the desktop; it was not implemented or run.

The maintained focused commands are:

```text
ctest --test-dir build -R "^chiaki-embedded-setup$" --output-on-failure
ctest --test-dir build -R "^chiaki-embedded-graphics$" --output-on-failure
```

The setup case is labeled `desktop`; the graphics case is labeled
`desktop;gpu`. Generic, headless, or software test passes do not establish
GPU acceptance. Existing ticket 01/02 native passes are retained; unchanged
baseline and input suites were not repeated. Future GNU input checks use the
prepared Linux Qt environment because Windows GNU Qt has no WebEngine.

Native initialization still has fatal error paths, including QRhi creation
failure. Ticket 04 must assess graceful initialization failure and setup
restoration at its real native handoff boundary. This change does not add a
web exception handler around `qFatal`.

Linux hardware and macOS hardware coexistence remain platform acceptance
work. WSL llvmpipe cannot establish a GPU pass. Live console discovery/decode,
HDR on a supported display, audio/microphone, physical devices, frame pacing,
and end-to-end input latency remain ticket 23 acceptance limits. Controlled
native frames do not prove live network playback.

## Log format

Source-only `git diff --check -- gui` passes. Raw compiler, deploy, and Qt logs
retain their original whitespace. A whitespace warning from a captured raw log
must not be reported as a source check failure. Intermediate build logs are
retained for source identity and diagnosis; the final changed-source package
run above is the library result to use.
