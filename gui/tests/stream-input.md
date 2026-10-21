# Check stream input ownership

`QmlMainWindow.streamInput` exposes `present`, `dismiss`, and `settled`.
Callers keep the returned string for that presentation. A close callback must
keep its original string. `blocked` reports the current native capture state.
Session capture reset does not change a continuing presentation's string.

## Build the native checks

Use a GNU toolchain, Qt 6 with GuiPrivate, Qt Test, FFmpeg, libplacebo,
SDL2, Opus, OpenSSL, json-c, and miniupnpc. Initialize the pinned desktop
submodules before configuration. The full build uses the existing project
dependency checks. The native executable links the same `chiaki-gui` objects
as the application. GNU linker wrapping intercepts controller feedback at
`chiaki_session_set_controller_state`. It does not replace ownership or input
release logic. The test creates real sessions without starting transport.

From a toolchain shell, run these commands. Set `nativePrefix` to the dependency
prefix. On Windows, add its `bin` directory to `PATH`.

```powershell
cmake -S . -B build/input-native -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$nativePrefix" -DCHIAKI_ENABLE_CLI=OFF -DCHIAKI_ENABLE_STEAMDECK_NATIVE=OFF -DCHIAKI_ENABLE_TESTS=ON
cmake --build build/input-native --target chiaki chiaki-stream-input-tests chiaki-stream-input-native-tests chiaki-console-screen-tests --parallel 4
```

On Windows, also add `build/input-native/third-party/steam-tools` to `PATH`.
Set `QT_PLUGIN_PATH` to the toolchain's Qt plugin directory.

```powershell
ctest --test-dir build/input-native -R '^chiaki-stream-input$' --output-on-failure
$env:QT_QPA_PLATFORM = 'windows'
ctest --test-dir build/input-native -L desktop --output-on-failure
```

The first test runs offscreen. It checks overlap, duplicate opens, obsolete
callbacks, destruction, reentrant callbacks, and session capture rules.
The desktop tests require an interactive desktop and OpenGL or Vulkan support.
They check real backend session changes, settings transitions, cursor state,
blocked feedback, and resumption after held buttons release in both feedback
paths. Each native test uses a separate test profile and clears that profile on exit.

Run each native window test in a separate process. Qt renderer setup can block
when the same process creates a second complete stream window. The CTest entries
select one window test per process.

The `chiaki-gameplay-keyboard-output` entry checks saved mode settings, every
default keyboard action, opposing and diagonal movement, triggers, held-key
quarantine, and exclusive output through both session feedback paths.
`chiaki-gameplay-keyboard-desktop` supplies timestamped events through Qt's
platform input interface. It checks production window routing, F8 repeat,
Ctrl shortcuts, overlapping owners, obsolete key events, session replacement,
and obsolete session notifications. It does not inject operating-system keys.

## Check native keys separately

The default desktop run skips `spontaneousKeys`. This check injects a complete
key press through Win32 `SendInput` and observes production console feedback.
It requires an interactive Windows desktop that permits injected input.

```powershell
$env:CHIAKI_INPUT_TEST_KEYS = '1'
./build/input-native/gui/tests/chiaki-stream-input-native-tests.exe spontaneousKeys
Remove-Item Env:CHIAKI_INPUT_TEST_KEYS
```

A successful `SendInput` return alone does not prove key delivery. This test
requires the stream window to be the Windows foreground window before it sends
keys. A foreground check failure is an environment failure, not input evidence.
The keyboard case uses a fresh window. Settings-window activation from the
ownership case must not run at the same time as keyboard injection.

On 2026-10-21, an injected-key run passed F8, autorepeat, mapped Space, Ctrl+Q,
Ctrl+O, menu release, and disconnect release. Later attempts could not obtain
Windows foreground focus, so the complete final injected-key case remains
unverified. Physical F8, F11, Alt+Tab, and live-console checks remain open.

## Run the screen checks

The existing standalone screen configuration remains available. It now compiles
the production ownership module. `StreamInputFixture` observes lifecycle calls
for the existing input-balance assertions. It delegates behavior to that module.

Use the screen runner's existing `-input` and test function filters to run the
affected StreamInterface, StreamNotices, LiveConsolePin, and Disconnect cases.
Use the environment in `gui/tests/CMakeLists.txt` for offscreen runs. The full
screen suite remains registered as `chiaki-console-screen`.

These checks do not prove live console input, console sleep, physical controller
input, desktop composition, or display scaling. The complete prerequisite
acceptance matrix remains in `.scratch/stream-input-ownership/spec.md`.

## Mouse capture interface

`QmlMainWindow::startGameplayCapture(timestamp)` starts foreground mouse Raw
Input on its Qt window. Capture requires both Qt activation and the actual
Windows foreground HWND. The adapter saves the previous mouse registration and
cursor clip. Capture uses no background input flag and keeps Qt mouse button
messages. Release restores the previous registration unless a newer owner has
replaced it. An unrestricted cursor restores with `ClipCursor(nullptr)` so a
display change cannot retain old desktop bounds. Capture setup failure restores
partial native state and leaves gameplay capture released.

`releaseGameplayCapture()` releases the adapter and clears the session mapper.
The existing owner, focus, session, and exit paths call this method. Window
position changes update confinement in physical screen coordinates. Absolute
device motion and old queued Raw Input packets do not enter the mapper. Qt
cursor position events never supply camera movement.

`KeyboardMouseMapper` has these input methods:

| Method | Input |
| --- | --- |
| `reset(held)` | Clear mapped state and ignore held inputs until release. |
| `key(code, down)` | Qt key value and press state. |
| `mouseButton(button, down)` | Left, right, middle, back, or forward button. |
| `mouseMove(dx, dy, timestampUs)` | Relative counts and monotonic microseconds since capture start. |
| `advanceMouse(timestampUs)` | Update the right stick. Return whether its output changed. |
| `state()` | Return the mapped controller state. |

Held mouse inputs use negative Qt button values. Keyboard inputs use positive
Qt key values. Both use the same held-input set so either trigger alternative
can keep the action pressed.

`StreamSession::HandleRelativeMouseMove(dx, dy)` supplies the capture-relative
clock. A precise 4 ms timer advances the mapper and sends changed output through
the existing controller feedback path. Capture release stops this timer.

The provisional response uses a trailing 16 ms count window and 1,000 stick
units per count in that window. This is a gain of 16 stick units per count per
second. Output has a radial limit of 32,767. Idle output becomes exactly zero
when the last count leaves the window, normally within 20 ms with the 4 ms
timer. Scheduler delays can increase that time. Ticket 03 owns calibration.

## Check mouse output and native capture

The `chiaki-gameplay-mouse-motion` entry checks the production mapper with timed
125 Hz and 1,000 Hz traces. Each 32-count trace has the same total unsaturated
response. It also checks idle centre, saturation, and reset. The
`chiaki-gameplay-mouse-triggers` entry checks emitted controller state for both
trigger alternatives, unassigned buttons, release, and held-button rearming.

The `chiaki-gameplay-mouse-desktop` entry reads actual Win32 registration and
cursor bounds. It checks resize, prior registration restoration, newer-owner
protection, double-click firing, stale Qt events, owner release, focus release,
and the cursor shape after capture ends. This test and `platformKeyboard` use
a test subclass that supplies controlled foreground eligibility. All capture,
release, event routing, registration, and confinement code remains production
code. These checks do not prove actual Windows foreground eligibility.

An optional foreground test injects relative movement through Win32 `SendInput`.
Run it in a separate process:

```powershell
$env:CHIAKI_INPUT_TEST_MOUSE = '1'
./build/input-native/gui/tests/chiaki-stream-input-native-tests.exe spontaneousMouse -o mouse-injected.log,txt
Remove-Item Env:CHIAKI_INPUT_TEST_MOUSE
```

On 2026-10-21, this optional test failed its foreground-window precondition
before injection. It provides no Raw Input delivery evidence. Physical mouse
movement, display changes, capture API failure injection, and live console
acceptance remain unverified.
