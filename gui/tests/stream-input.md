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
paths. The test settings use a temporary INI directory.

## Check native keys separately

The default desktop run skips `spontaneousKeys`. This check injects a complete
key press through Win32 `SendInput` and observes production console feedback.
It requires an interactive Windows desktop that permits injected input.

```powershell
$env:CHIAKI_INPUT_TEST_KEYS = '1'
./build/input-native/gui/tests/chiaki-stream-input-native-tests.exe spontaneousKeys
Remove-Item Env:CHIAKI_INPUT_TEST_KEYS
```

A successful `SendInput` return alone does not prove key delivery. The initial
attempt on this desktop did not observe mapped console feedback. Keep this
result separate from constructed QKeyEvent checks and from the deterministic
native test results. Physical key delivery still needs acceptance evidence.

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
