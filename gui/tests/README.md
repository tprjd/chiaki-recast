# Console screen test commands

The test target uses the production `ConsoleLibrary.qml`, controls, console
artwork, and SVG image provider. Sample hosts exist only in `qml/`. Tests observe
action signals without opening a network session.

The normal application build includes this target when `CHIAKI_ENABLE_TESTS`
is enabled. A separate build needs only Qt Quick, Qt Quick Test, and Qt SVG.
No test target or sample host is part of application packaging.

## Windows Qt 6.9.3 commands

These PowerShell commands use the MSVC compiler and the Qt version from the
Windows build workflow. The Qt installation contains `qtbase`, `qtdeclarative`,
`qtsvg`, and `qttools`. The commands start at the repository root.

```powershell
$qtPrefix = 'C:\Users\david\.cache\chiaki-native-qt\6.9.3\msvc2022_64'
$vsRoot = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools'
Import-Module "$vsRoot\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath $vsRoot -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64'
$cmake = "$vsRoot\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$ctest = "$vsRoot\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe"
$env:PATH = "$qtPrefix\bin;" + $env:PATH
& $cmake -S gui/tests -B build/gui-screen -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$qtPrefix"
& $cmake --build build/gui-screen
& $ctest --test-dir build/gui-screen --output-on-failure
```

The registered test sets `QT_QPA_PLATFORM=offscreen` and
`QT_QUICK_BACKEND=software`. `QT_QUICK_CONTROLS_CONF` points to the production
`gui/src/qml/qtquickcontrols2.conf`, including its Material theme and font size.
On Windows, `QT_QPA_FONTDIR` points to the Windows Fonts directory. The test
application selects Segoe UI to match the normal Windows font. These settings
prevent missing glyphs and geometry checks with the wrong control style.
The executable also accepts Qt Test arguments.

The full application needs the additional dependencies and configuration in
`.github/workflows/build-windows-x86.yml`. After that configuration, the build
and launch commands are:

```powershell
& $cmake --build build --config Release --target chiaki
& .\build\gui\chiaki.exe
```

## Initial native behavior

Before this change, the main screen used a full-width `ListView`. A row click
or Return called `Chiaki.connectToHost(index, name)` for a discovered console.
Other entries used `Chiaki.connectToHost(index)`. The backend handled
registration, standby wake-and-connect, local sessions, and PSN sessions.

The baseline replaces the artwork area of a registered, ready row with the
production card. Its Connect action uses that same handler. The existing
toolbar, details, PIN, wake, hide, delete, discovery, and controller commands
remain in the screen until the complete card library replaces the list.

## Console library API

`ConsoleLibrary` displays each supported host case. A record with
`display: false` has no card. Unknown power states have no online light.
Missing console types show the neutral Console label.

The component accepts these properties:

- `hosts` contains the card candidates in backend order.
- `sourceHosts` contains the complete current backend list. It defaults to
  `hosts`. A displayed subset must keep this property bound to the full list.
- `sourceIndices` is an optional complete index map for `sourceHosts`.
  An empty map uses each entry's index in `sourceHosts`. An incomplete or
  invalid map cancels the action.
- `streamerMode` masks an address used as the console name.
- `profileKey` identifies the active profile. The native view binds it to
  `Chiaki.settings.currentProfile`.

`identityOf(host)` returns an opaque identity. It uses the MAC for discovered
hosts, the address and available registered MAC for manual hosts, and the DUID
for remote PSN hosts.
The identity also includes the current profile generation. A profile change
invalidates saved identities, including after a switch back to the old profile.
Missing identity fields return an empty identity.

`resolveTarget(identity)` returns the current `host`, `sourceIndex`, and
`nickname`, or `null` if the target is missing, hidden, or ambiguous.
`connectToTarget(identity)` resolves the target and emits
`connectRequested(sourceIndex, nickname)` once. It returns `false` when the
action cannot proceed. Card buttons and the native row activation use this
method. Delayed details and confirmation actions must resolve a saved identity
again when the user accepts.

The backend remains the authority for registration and connections.
Unregistered hosts use the same connection signal as registered hosts.
Discovered standby hosts include the current nickname for wake-and-connect.
Manual and remote PSN hosts pass an empty nickname.

The PSN part of `displayServerAt()` now reads the same list as `hosts()`.
This keeps its indices correct when the backend suppresses a discovered PSN
duplicate, Main PS4 Console, or a console in the wake delay. Both PSN automatic
connection callers use that list and retain their remote-only condition.

## Baseline results, 2026-10-20

MSVC 19.44 and Qt 6.9.3 built the C++ screen test target. CTest discovered and
passed `chiaki-console-screen`. Three behavior checks passed, plus the Qt Test
setup and cleanup checks:

- A ready console displays its name and Online status. A mouse click emits
  one action for source index 7 with the expected nickname.
- Return emits one action from a Connect button with visible keyboard focus.
- Streamer mode masks an address used as a name in visible, accessible, and
  tooltip text.

The first test failed before `ConsoleLibrary.qml` existed, then passed after
the production component was added. Test runs use the real QML screen and the
existing C++ SVG provider.

The full application build and launch are not yet verified at this baseline.
A real registered local-console connection is untested. This test harness has
no hardware or account access and cannot prove discovery or streaming works.

## Host behavior results, 2026-10-20

MSVC 19.44 and Qt 6.9.3 built the updated screen test target. CTest passed
`chiaki-console-screen`. The executable reported 37 passed checks, including
setup and cleanup, with no failures or skipped checks.

New checks cover the specified host states and primary actions, automatic
registration, hidden records, duplicate suppression, and neutral missing types.
Saved target checks cover rename, reorder, removal, reused positions or manual
addresses, ambiguous
identities, invalid source maps, and profile changes. Actual card activation
also resolves discovered, manual, and PSN targets against a separate current
source list. A long-name registration card keeps its primary action inside
the card bounds with the production Material configuration.

The standby, host status, hidden-host, identity, profile, neutral-type, and
action-bounds tests failed before their respective changes. An offscreen
render confirmed readable Windows text and the long-name registration layout.

The full application build and launch are pending in the integration build.
The screen tests observe the existing connection entry point without opening
network sessions. Real registration, standby wake-and-connect, manual
connection, and PSN connection remain untested without console or account
access. The existing PIN dialog still needs the delayed-target checks in
the details ticket.
