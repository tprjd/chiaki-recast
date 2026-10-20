# Console screen tests

The `chiaki-console-screen-tests` target loads the complete production
`ConsoleLibrary.qml` with supplied host records. It uses the production
controls, Material configuration, console artwork, and C++ SVG image provider.
Tests observe action signals without opening network sessions.

## Build and run on Windows

The standalone target needs Qt Quick, Qt Quick Test, Qt SVG, CMake, Ninja,
and an MSVC x64 compiler. These commands use the Qt 6.9.3 SDK and MSVC build
from the Windows workflow. Run the commands at the repository root.

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

CTest sets `QT_QPA_PLATFORM=offscreen`, `QT_QUICK_BACKEND=software`, and
`QT_QUICK_CONTROLS_CONF` to the production `qtquickcontrols2.conf`.
On Windows, it also sets `QT_QPA_FONTDIR` to the Windows Fonts directory.
The test application uses Segoe UI on Windows.

The executable accepts Qt Test arguments. For a direct run, set the same
environment first. This example runs one rendered appearance check.

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
$env:QT_QUICK_BACKEND = 'software'
$env:QT_QUICK_CONTROLS_CONF = (Resolve-Path gui/src/qml/qtquickcontrols2.conf).Path
$env:QT_QPA_FONTDIR = "$env:WINDIR\Fonts"
& .\build\gui-screen\chiaki-console-screen-tests.exe ConsoleNavigation::test_consoleBodyIsReadableOnDarkCard
```

The normal application configuration includes the screen target when
`CHIAKI_ENABLE_TESTS=ON`. The full application needs the other dependencies in
[the Windows build workflow](../../.github/workflows/build-windows-x86.yml).
After configuration, its build command is `cmake --build build --target chiaki`.
The screen target has no install rule. Test fixtures, captures, and prototype
files are absent from the production QML and icon resource lists.

## Supply screen state

`MainView.qml` binds these properties to the backend in the application.
The tests supply them directly.

| Property | Value |
| --- | --- |
| `hosts` | Complete current backend list, in backend order, including hidden records |
| `profileKey` | Current profile name. A change invalidates saved console targets |
| `streamerMode` | Masks addresses and IDs, including an address used as the name |
| `discoveryEnabled` | Current local-discovery state |
| `psnAvailable` | Whether the PSN refresh command is available |
| `steamAvailable` | Whether the Steam shortcut command is available |
| `applicationVersion` | Version displayed in the sidebar |
| `playStationController` | Selects PlayStation or default controller-button images |
| `searchText` | Editable console search query |

Host records use the existing backend fields: `display`, `name`, `address`,
`mac`, `duid`, `manual`, `discovered`, `registered`, `ps5`, `state`, `app`, and
`titleId`. A record with `display: false` has no card. Missing `ps5` shows a
neutral Console label. Only a discovered `ready` state displays Online.
Registered manual hosts without discovery display Status unknown. Remote PSN hosts
have no local power-state claim.

Search matches all query words against the displayed name and known console
type. It ignores case and extra spaces. Filtering preserves backend order and
source indices. The footer counts visible records, with a filtered count during
search. Streamer mode searches the masked name when the address is the name.

## Observe commands

The screen emits these signals. `MainView.qml` connects them to the existing
backend handlers and dialogs.

| Signal | Effect in the application |
| --- | --- |
| `connectRequested(sourceIndex, nickname)` | Existing Connect, Register, or Wake and connect entry point |
| `settingsRequested()` | Opens Settings |
| `addConsoleRequested()` | Opens the manual-host dialog |
| `refreshPsnRequested()` | Refreshes PSN consoles |
| `steamShortcutRequested()` | Opens the Steam shortcut dialog |
| `discoveryToggled(enabled)` | Changes local discovery |
| `quitRequested()` | Opens the quit confirmation |
| `managementRequested(action, accept, cancel)` | Opens PIN, Delete, or Hide with callbacks for this request |
| `pinRequested(sourceIndex, pin)` | Stores the accepted four-digit PIN |
| `wakeRequested(sourceIndex)` | Sends the explicit manual-host wake request |
| `deleteRequested(sourceIndex)` | Deletes the resolved manual host |
| `hideRequested(mac, nickname)` | Hides the resolved eligible discovered host |

A management dialog calls `accept(pin)` for PIN, `accept()` for Delete or Hide,
or `cancel()` when dismissed. PIN requires exactly four ASCII digits and keeps
leading zeroes. Invalid PIN input leaves the request open. Cancellation,
repeated acceptance, and callbacks from older requests cannot send a command.

`identityOf(host)` returns an opaque profile-scoped identity.
`resolveTarget(identity)` returns the current `host`, `sourceIndex`, and
`nickname`, or `null` for a missing, hidden, or ambiguous target.
`connectToTarget(identity)` resolves the target before it sends the connection
signal. Discovered hosts use the current nickname. Manual and PSN hosts use an
empty nickname.

`eligible(action, host)` controls both details and controller actions.
`requestManagement(action, identity)` rejects invalid or ineligible targets.
Wake sends its command immediately. The other actions request a dialog.
Acceptance resolves the target and checks eligibility again before it sends
the final command. A profile switch invalidates old targets, including after a
switch back to the previous profile.

`selectedIdentity` and `selectedTarget` describe the current selection.
`moveSelection(offset)` moves between visible, resolvable cards.
`rememberFocus()` and `restoreFocus()` preserve the opener across native views.
`restoreCardFocus(identity)` uses the target card, a remaining valid card, or
Add console. Details follow live rename and reorder updates. Target removal,
hiding, ambiguity, and profile changes close details.

## Read the results

The suite covers host states, search, live identity changes, privacy, command
eligibility, management callbacks, popup focus, keyboard input, and controller
key mappings. Layout checks use 1280 by 720, 800 by 600, and 390 by 600 logical
pixels, including optional commands, long names, missing data, and many hosts.
Cards and the discovery footer remain reachable through vertical scrolling.

Rendered checks capture the complete screen and map each control's region.
Child captures can return an origin-cropped image with the offscreen renderer.
Image channels use bytes from 0 to 255. The artwork check requires bright body
pixels across more than 20 percent of its image region. The primary action
check requires blue pixels across more than 65 percent of its inner region.
The guards reject a dark console body and an absent button background.

The screen suite does not verify network discovery, registration acceptance,
PSN authentication, wake, streaming, or physical controller input. Those checks
need the full application and the relevant hardware or account. Native display
scaling also needs a separate full-application check. A passing build alone
does not verify any of these behaviors.

## Settings screen tests

`SettingsHarness.qml` loads the production Settings screen with synthetic
settings and action handlers. The application uses the native defaults for
`backend`, `navigation`, and `openExternal`. The test import path and fake
Chiaki module are available only in the screen-test executable.

Use the harness `settings.changeCount` to observe changed values and
`settings.actions` or `actions` to inspect dispatched commands. Create a
harness with `screenActive: false`, set the fixture values, reset the counters,
and then activate the screen to check construction. Fixture defaults are
synthetic test inputs, not a copy of the native defaults specification.

`SettingsGeneral` checks category selection, General setting writes, all four
shortcut choices, device conditions, privacy, log access, popup Escape handling,
and focus visibility at 1280 and 900 pixels wide.
