# Ticket 01 verification

## Scope and ownership

`QmlNativeServices` owns one `QmlBackend` and one native `QQmlEngine`.
Its member order destroys the engine before the backend. Each native window
uses these existing objects. Each window creates one QML root and supplies
its actual host as `nativeWindow`. The engine retains its SVG provider and
retargets its incubation controller for each native window.

The backend now supports explicit window attachment and detachment. The
backend keeps settings, discovery, controllers, and asynchronous work while
no stream window exists. Playback requires a prepared native renderer.
Registration-only sessions can run without a native stream window.
Application activity controls controller polling. Controller navigation uses
the active setup or dialog window while the native window is absent or hidden.

Session cleanup stops producers, waits for frame and PSN work, and destroys
the session before the window releases GPU resources. Queued frame callbacks
own their AVFrame with a move-only unique_ptr. Dropped callbacks release the
frame. This adds no frame clone or per-frame owner allocation. Cleanup clears
the window's session reference before it destroys the session. Render-control
invalidation completes on its render thread before that thread stops.

`main.cpp`, both native window constructors, renderer access through
`getBackend()`, profile changes, and maintained native input adapters use the
new ownership. The production decoder, renderer, native input, audio, HDR,
and overlay code paths remain native. No UI selector or fallback was added.

## Confirmed test boundary

The user approved native services, observable native window and saved settings
state, existing native input/output, and later real embedded web acceptance.
This ticket uses production native classes and the actual Qt QML engine and
OpenGL or Vulkan renderer. Native session events control completion. Loopback
session objects do not prove live console transport or playback performance.

## Red and green results

The maintained check is `gui/tests/nativeservices.cpp`. Its sequence tests
window recreation through one native service owner.

| Change | Red evidence | Green evidence |
| --- | --- | --- |
| Application-owned services and window recreation | `red-build.txt`: required services API absent | `green-recreation.txt`: 3 passed, 0 failed |
| Session destruction before native window teardown | `red-cleanup.txt`: backend retained a session | `green-cleanup.txt`: 3 passed, 0 failed |
| Playback without an attached renderer fails safely | `red-no-window.txt`: backend created a session | `green-no-window.txt`: 3 passed, 0 failed |
| Direct streaming keeps saved stream geometry | `red-direct.txt`: 3072x1708 instead of 1024x640 | `green-direct.txt`: 3 passed, 0 failed |

The final extended sequence also checks current settings through two window
lifetimes, actual session completion, reconnect, active-session teardown,
separate setup and stream geometry, and direct-stream application exit.
It switches profiles with no window, changes native audio volume, and reads
that saved value through a new native Settings object.

`green-profile-and-direct.txt` reports 3 passed, 0 failed, 0 skipped, 3618 ms.
It uses the actual Vulkan device. Earlier OpenGL service evidence remains
applicable to its tested ownership paths. The final profile and direct-exit
extensions ran with Vulkan.

`native-opengl-input.txt` reports 4 passed, 0 failed, 0 skipped, 1264 ms.
It runs existing `sessionAndFeedback` and `keyboardCapture` checks.
`native-vulkan-input.txt` reports 3 passed, 0 failed, 0 skipped, 1411 ms.
It runs existing `sessionAndFeedback` with Vulkan. These checks cover native
owner overlap, stale completion, focus return, input blocking, session handoff,
and held-key output release. They compile the production input sources with
the maintained GNU transport adapter.

The production `chiaki`, new native-services check, and GNU native-input check
all build. `owned-frame-build.txt` records the final frame ownership change.
`green-direct-build.txt` records the final production window change.
`profile-check-build.txt` records the final check extension. Build warnings
include existing FFmpeg Vulkan-field deprecations. Existing QML Shortcut
warnings remain in runtime output.

The first repeated-show attempt used resize on a maximized window. That
expectation was invalid. `window-recreation-result.txt` records that failure.
The check now restores normal state before resize. `green-build.txt` records
two compile repairs before the first successful service run. These files are
history, not passing evidence.

## Commands

All commands ran in this ticket worktree. The source started at `547c608`.
The following PowerShell setup selects the installed native dependencies.

```powershell
$nativePrefix = 'C:/Users/david/.cache/chiaki-ui-native/msys64/ucrt64'
$env:PATH = "$nativePrefix/bin;$env:PATH"
git submodule update --init third-party/nanopb third-party/jerasure third-party/gf-complete third-party/curl third-party/cpp-steam-tools test/munit
& "$nativePrefix/bin/cmake.exe" -S . -B build/native-services -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$nativePrefix" "-DCMAKE_C_COMPILER=$nativePrefix/bin/cc.exe" "-DCMAKE_CXX_COMPILER=$nativePrefix/bin/c++.exe" -DCHIAKI_ENABLE_CLI=OFF -DCHIAKI_ENABLE_STEAMDECK_NATIVE=OFF -DCHIAKI_USE_SYSTEM_CURL=OFF -DCHIAKI_ENABLE_FFMPEG_DECODER=ON -DCHIAKI_ENABLE_TESTS=ON
& "$nativePrefix/bin/cmake.exe" --build build/native-services --target chiaki-native-services-tests chiaki chiaki-stream-input-native-tests -j 6
$env:PATH = "$((Resolve-Path build/native-services/third-party/steam-tools).Path);$env:PATH"
```

Configure returned 0. All final build commands returned 0. The red build
returned 1 because `qmlnativeservices.h` did not exist. Each red behavioral
run returned 1. Each recorded green behavioral run returned 0.

Runtime calls use Start-Process with Wait because these Windows executables
can return control to PowerShell before they finish. The representative final
service call was:

```powershell
$env:CHIAKI_INPUT_TEST_RENDERER = 'vulkan'
$p = Start-Process -FilePath (Resolve-Path build/native-services/gui/tests/chiaki-native-services-tests.exe) -ArgumentList '-o',"$((Resolve-Path .scratch/web-ui-migration/evidence/ticket01).Path)/green-profile-and-direct.txt,txt" -WindowStyle Hidden -PassThru -Wait
$p.ExitCode
```

The OpenGL input call ran without `CHIAKI_INPUT_TEST_RENDERER`. The Vulkan
input call set it to `vulkan`. Their exact argument lists were:

```powershell
'sessionAndFeedback','keyboardCapture','-o',"$((Resolve-Path .scratch/web-ui-migration/evidence/ticket01).Path)/native-opengl-input.txt,txt"
'sessionAndFeedback','-o',"$((Resolve-Path .scratch/web-ui-migration/evidence/ticket01).Path)/native-vulkan-input.txt,txt"
```

The executable for these calls was
`build/native-services/gui/tests/chiaki-stream-input-native-tests.exe`.
The initial CTest attempt returned 8 after the maximized-window expectation
failed. Its output was empty because of the Windows runner. Subsequent calls
used QtTest output files to preserve actual results. No complete suite was run.
`git diff --check` returned 0.

## Hardware and package limits

The pre-edit module check and baseline conditions are in `baseline.md`.
The GNU Qt prefix has no WebEngine module. This ticket did not run accelerated
WebEngine beside native rendering. A separate compatible toolchain remains
required. The parent task prepares that prerequisite. The root task's WSL
WebEngine environment uses llvmpipe, which cannot pass GPU acceleration.

No physical console, live decoded frames, microphone, physical controller,
HDR display output, external latency measurement, suspend/resume, or release
package was checked. Native Vulkan capability logs list HDR formats. Those
logs do not prove HDR output. The tests use isolated settings and loopback
session transport. Keep the live baseline and predeclared tolerances pending
for ticket 23. Do not treat this ticket's code checks as live acceptance.

Ticket 02 owns native stream-root extraction. Ticket 03 owns the embedded
Svelte, WebEngine, WebChannel, and GPU startup changes. Ticket 04 owns the
setup-to-stream controller. The original main checkout remains unchanged.
