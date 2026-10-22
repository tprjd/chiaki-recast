# Ticket 02 verification

## Result

`QmlMainWindow` now loads `StreamRoot.qml`. The stream root contains the
existing native `StreamView`, live settings, console login PIN host, error
notice, and renderer fallback dialog. It has no setup view or setup route.
The old `Main.qml` is removed from the source and QML resource list.
Other setup components remain for their later migration tickets and tests.

The root retains display, renderer, color mapping, and keyboard and mouse
settings. Their inline and separate-window adapters remain native. Native
menus, statistics, notices, disconnect handling, progress, and errors still
use their existing production components. Passive notices remain passive.
The renderer fallback dialog now uses ADR 0001 presentation identities.
Closing live settings cannot release input while that dialog remains open.

`showMainView()` calls `QmlMainWindow::dismissStreamView()`. This emits
`streamViewDismissed` as a presentation request. It does not stop a session
or destroy the window. Ticket 04 must wait for native session and frame
cleanup before it destroys the stream window and restores setup.

Native windows read saved stream geometry. They no longer save setup
geometry or restore setup geometry after session completion or PSN failure.
The direct-stream constructor retains its profile, fullscreen, zoom, stretch,
and application-exit paths. The root load does not create a WebEngine page.

The decoder, renderer, frame transport, audio, microphone, HDR metadata,
touch, haptics, gameplay input, and stream-input ownership module have no
pipeline changes in this ticket.

## Test boundary

The user approved the production native window, native services, saved
settings, native input output, and the existing controlled transport boundary.
The checks compile production classes and load the actual Qt QML root.
They use isolated synthetic settings and loopback session objects. Session
events control completion. These checks do not establish live console playback.

The worktree first merged integration `96a34500`. Ticket 01 ownership is
present: application-owned `QmlNativeServices`, shared engine, explicit native
window attachment, and producer cleanup before window teardown.

## Red and green results

| Check | Red evidence | Passing evidence |
| --- | --- | --- |
| A native host shows stream progress without setup | `red-stream-root.txt`: no visible stream progress | `green-stream-root.txt`: 3 passed, 0 failed |
| A prepared stream host uses saved stream size | `red-geometry.txt`: 720x480 instead of 1024x640 | Final root checks below |
| Renderer fallback retains input when live settings close | `red-fallback.txt`: input became unblocked | Native input checks below |

Final focused checks use the same native source as `green-fallback-build.txt`.
The later build updates extend the root check only.

| Evidence | Actual result |
| --- | --- |
| `native-opengl-root-final.txt` | `streamOnlyRoot`: 3 passed, 0 failed, 0 skipped, 887 ms |
| `native-vulkan-root-final.txt` | `streamOnlyRoot`: 3 passed, 0 failed, 0 skipped, 1427 ms |
| `native-opengl-input.txt` | `sessionAndFeedback`, `keyboardMouseSettingsRoute`, `keyboardCapture`: 5 passed, 0 failed, 0 skipped, 1430 ms |
| `native-vulkan-input.txt` | `sessionAndFeedback`, `keyboardMouseSettingsRoute`: 4 passed, 0 failed, 0 skipped, 2371 ms |
| `native-vulkan-root.txt` | `windowRecreation` passed; the separate first root case failed because of the hidden launch described below |

The final root check observes native window exposure and visible connection
progress before a session. It then checks the root return request, a native
console login PIN request, input blocking, settings suppression during PIN,
cancellation, cleanup completion, acknowledgement, and geometry retention.
OpenGL uses the inline PIN host. Vulkan uses the separate native PIN window.

The retained `windowRecreation` case exercises repeated native windows,
backend state, session destruction, reconnect, direct-stream saved geometry,
direct-stream application exit, and profile settings through real native
services. Its passing case in `native-vulkan-root.txt` remains applicable.
The input checks load the extracted production root and exercise display,
renderer, color mapping, keyboard and mouse settings, fallback overlap,
focus return, session handoff, and held-input output release.
Ticket 01's independent ownership checks remain applicable to the unchanged
ownership module, including duplicate opens and obsolete completion.

`chiaki`, `chiaki-native-services-tests`, and
`chiaki-stream-input-native-tests` build successfully. Build records are
`green-build.txt`, `green-geometry-build.txt`, `green-fallback-build.txt`, and
`pin-check-build.txt`. `git diff --check` passes. Existing FFmpeg deprecation
warnings and QML Shortcut warnings remain. No complete test suite or stress
loop ran.

## Failed harness observations

The first saved-size native root attempts used
`Start-Process -WindowStyle Hidden`. Qt reported the window as visible and
active, but `IsWindowVisible` returned 0. `root-native-placement.txt` records
that observation. `green-geometry.txt`, `native-vulkan-root-focused.txt`,
`root-placement.txt`, and the first case of `native-vulkan-root.txt` are failed
harness runs. They are not passing evidence. A direct process launch removed
the hidden startup flag and exposed the saved-size window without a production
code change. Temporary diagnostic code was removed.

The initial PIN extension expected adjustable geometry with the default window
mode. That expectation was wrong: the default mode uses the video profile
size. The maintained check explicitly selects `AdjustableResolution` before
it checks saved stream geometry. The final OpenGL and Vulkan records include
that correction.

## Commands and environment

The build uses the GNU/UCRT Qt 6.11.2 native prefix from ticket 01. This prefix
has no WebEngine. It is used only for this native slice. Commands run from
this ticket worktree, without changing the primary or integration checkouts.

```powershell
$nativePrefix = 'C:/Users/david/.cache/chiaki-ui-native/msys64/ucrt64'
$env:PATH = "$nativePrefix/bin;$env:PATH"
git submodule update --init third-party/nanopb third-party/jerasure third-party/gf-complete third-party/curl third-party/cpp-steam-tools test/munit
& "$nativePrefix/bin/cmake.exe" -S . -B build/native-stream -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$nativePrefix" "-DCMAKE_C_COMPILER=$nativePrefix/bin/cc.exe" "-DCMAKE_CXX_COMPILER=$nativePrefix/bin/c++.exe" -DCHIAKI_ENABLE_CLI=OFF -DCHIAKI_ENABLE_STEAMDECK_NATIVE=OFF -DCHIAKI_USE_SYSTEM_CURL=OFF -DCHIAKI_ENABLE_FFMPEG_DECODER=ON -DCHIAKI_ENABLE_TESTS=ON
& "$nativePrefix/bin/cmake.exe" --build build/native-stream --target chiaki-native-services-tests chiaki-stream-input-native-tests chiaki -j 6
$env:PATH = "$((Resolve-Path build/native-stream/third-party/steam-tools).Path);$env:PATH"
```

The final root calls use `subprocess.run` so that Windows does not receive a
hidden first-show flag. Set `CHIAKI_INPUT_TEST_RENDERER=vulkan` for Vulkan.
Remove that variable for OpenGL. The representative root call is:

```python
import subprocess
from pathlib import Path
exe = str(Path('build/native-stream/gui/tests/chiaki-native-services-tests.exe').resolve())
output = str(Path('.scratch/web-ui-migration/evidence/ticket02/native-vulkan-root-final.txt').resolve())
result = subprocess.run([exe, 'streamOnlyRoot', '-o', output + ',txt'])
assert result.returncode == 0
```

The input calls use the executable
`build/native-stream/gui/tests/chiaki-stream-input-native-tests.exe` with the
scenario arguments in the table and `-o <absolute-evidence-path>,txt`.
Those maximized initial windows were exposed despite the hidden startup flag.
Their actual passing output remains valid.

## Limits

No physical console, live decoded frames, physical controller, microphone,
HDR output, external latency measurement, suspend/resume, or release package
was checked. No accelerated WebEngine/native coexistence or matched live
performance comparison ran in this ticket. Native Vulkan format logs do not
prove HDR output. Tickets 03, 04, and 23 retain those acceptance checks.
