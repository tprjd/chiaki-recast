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
| `managementRequested(action, accept, cancel, consoleName)` | Opens PIN, Delete, or Hide with callbacks for this request; `consoleName()` resolves the current name |
| `pinRequested(sourceIndex, pin)` | Default test observation when no native `savePin` callback is supplied |
| `wakeRequested(sourceIndex)` | Sends the explicit manual-host wake request |
| `deleteRequested(sourceIndex)` | Deletes the resolved manual host |
| `hideRequested(mac, nickname)` | Hides the resolved eligible discovered host |

A management dialog calls `accept(pin)` for PIN, `accept()` for Delete or Hide,
or `cancel()` when dismissed. PIN requires exactly four ASCII digits and keeps
leading zeroes. Invalid PIN input leaves the request open. Cancellation,
repeated acceptance, and callbacks from older requests cannot send a command.

`MainView` supplies `savePin(sourceIndex, pin)` with the native boolean save
outcome. A PIN acceptance returns true only when this callback confirms the
save. `ConsolePinDialog` then shows the saved result. It uses the production
`ConsolePinContent` for validation, masked entry, explicit reveal, privacy,
focus, and the guided workspace. The live PIN route can use this same content.
`ConsolePinHarness` composes the library and saved PIN dialog with a supplied
save outcome. `ConsolePin` tests both the form and its guarded library callbacks,
including target changes and return focus. Test assertions do not print PINs.

`Main.qml` owns one `LiveConsolePinHost` for both PSN and stream routes.
The backend retains `sessionPinRequest` across the route handoff. Its opaque
`id` owns `enterPin(id, pin)`, `cancelSessionPin(id)`, and
`dismissSessionPin(id)`. The request contains no PIN value. Native connection
progress closes an accepted request. Native cleanup completes cancellation.

`LiveConsolePinHarness` loads this production host with supplied request state.
`LiveConsolePin` checks validation, leading zeros, masking, privacy, incorrect
feedback, repeated actions, handoff, replacement, cancellation, return focus,
and compact-window close. The tests cover 900 by 600 and 1280 by 720 layouts.
These offscreen checks do not prove visible native focus or display scaling.

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

## Confirmation and registration method tests

`ConfirmationHarness.qml` loads production `MainView.qml` or `SettingsDialog.qml`
with the production `ConfirmDialog.qml`. Tests supply the backend and navigation.
`showConfirmDialog(title, text, actionText, accept, cancel)` requires an explicit
action label. Cancel and Escape call only `cancel` after the popup closes.

`Confirmations` checks labels, visible initial Cancel focus, keyboard navigation,
disabled actions, repeated input, focus return, and current console targets.
`RegistrationMethod` checks automatic, manual, and cancelled outcomes through
production `RegistrationMethodDialog.qml`. Both suites check long text at
900 by 600 and 1280 by 720. Backend commands are controlled test observations.
Real registration and native input capture require separate acceptance checks.

## Startup reminder tests

`RemindersHarness.qml` loads production `MainView.qml`, `RemindDialog.qml`, and
the Steam setup screen through a real `StackView`. `Reminders` checks all three
choices, Escape, the controller triangle/Y dismissal key, initial Later focus,
focus return, duplicate requests, startup eligibility, and Steam-then-PSN order.
The long-text checks use 900 by 600 and 1280 by 720 logical pixels.

The navigation contract remains `showRemindDialog(title, text, remotePlay, callback)`.
Set up now starts that callback after the popup closes. Later leaves the preference
unchanged. Don't ask again disables the selected reminder. Setup completion uses
the existing preference and credential-presence contract. The Steam route tests
use controlled final outcomes and check that success alone disables its reminder.
These tests do not create Steam files or authenticate a PSN account.

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

## Local connection tests

`ConnectionHarness.qml` loads the production `ConnectionView.qml` with a
supplied backend and navigation object. `StreamView.qml` uses this same
component for local connection progress. The renderer is outside this test.

`ConnectionLocal` checks connecting, video waiting, persistent failure,
cleanup-gated retry, cancellation, disabled actions, privacy, view removal,
and action focus at 900 by 600 and 1280 by 720 logical pixels. Native events
control readiness and cleanup. Presentation timers do not complete either task.

The backend exposes `connectionAttempt` and three commands that accept its
opaque string `id`: `cancelConnection`, `retryConnection`, and
`dismissConnection`. Each command returns whether it accepted the action.
The native backend resolves the target again before retry. A changed profile
invalidates the target even after a switch back to the previous profile.

The native contract checks and source manifests are in
`.scratch/connection-flow-refresh/evidence/ticket01-*`. These checks use real
session objects with isolated settings and controlled transport calls.
They cover event ownership, actual retry and stop dispatch, and direct entry
that has no library row. A hidden native window cannot verify visible input
capture, display scaling, or physical controllers. Ticket 06 covers visible
native acceptance. Real transport checks need a console.

## Auto-connect and console wake-up tests

`ConnectionWake` loads production `AutoConnectView.qml` through
`ConnectionHarness.qml`. This view uses `ConnectionView.qml` for the selected
console, actual wait state, persistent timeout, safe retry, and cancellation.
`context` distinguishes startup auto-connect from a manual wake request.
`waitingForWake` retains wait-screen ownership until the connection starts.
The same attempt ID continues through local or PSN handoff.

These tests check repeated input, cleanup before retry, closed views, invalid
targets, streamer mode, and action focus at 900 by 600 and 1280 by 720.
The real `StackView` check verifies that an old view cannot dismiss a new
attempt or take focus during its exit transition.
The fake backend supplies native results. It does not simulate discovery,
wake packets, or a session. Isolated native checks cover those handlers with
transport disabled. Real console wake-up still needs a console.

## PSN setup tests

`PsnSetupHarness.qml` loads production `PSNTokenDialog.qml`. It supplies the
backend, navigation, and browser creation boundary. `PsnSetup` checks redirect
privacy, explicit copy and paste, input rejection, browser fallback and recovery,
actual progress states, persistent failure, retry, closure, and both supported
window sizes. The supplied browser does not contact Sony.

The native backend owns `psnAuthAttempt` across screen closure. `startPsnAuth`
returns its opaque ID and rejects a second active request. Profile changes
invalidate the request before a result can save in another profile. Credentials
save only after both the token and Account ID responses pass validation.

The controlled native checks and captures are in
`.scratch/psn-dialog-refresh/evidence/ticket01-*`. They use isolated INI files
and controlled network replies. They check actual native persistence and visible
production screens at 900 by 600 and 1280 by 720, with measured DPR 1.25.
The native Qt build lacks QtWebEngine, so its browser check proves external
fallback. The embedded route uses the supplied browser in screen tests.
Real PSN sign-in and physical controller input remain separate checks.

## Steam shortcut checks

`SteamShortcut` loads the production `SteamShortcutDialog.qml` with a supplied
backend and navigation object. The checks use the production controls. They cover
profile defaults, name validation, custom options and folders, persistent failure,
retry, terminal success, privacy, keyboard input, controller key mappings, and
return focus. Layout checks use 900 by 600 and 1280 by 720 logical pixels.
`SteamShortcutHarness` composes the production Library and Steam form.

The separate `gui/tests/steam` CMake target builds the exact Steam command from
`qmlbackend.cpp` with the real pinned SteamTools dependency and artwork. The test
supplies only the executable path boundary. It creates a temporary Steam directory
and checks the written shortcut records, matching-entry updates, artwork, and
controller configuration. Windows file locks test backup and write failures.
The checks do not access the user's Steam installation.

Build it with the same Qt and compiler environment as the screen target:

```powershell
& $cmake -S gui/tests/steam -B build/steam-native -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$qtPrefix"
& $cmake --build build/steam-native
$env:PATH = (Resolve-Path build/steam-native/steam-tools).Path + ';' + $env:PATH
$env:QT_QPA_PLATFORM = 'offscreen'
& .\build\steam-native\chiaki-steam-shortcut-tests.exe
```

`cmake/SteamTools.cmake` applies `cmake/steam-tools.patch` to a build copy of the
pinned dependency. Both the normal application and the native check use this
copy. The source submodule stays unchanged. The patch fixes custom folder checks,
new shortcut files, Windows backup names, artwork error reporting, and failed
backup or write results. Reconfiguration copies the original source before the
patch is applied again.

These controlled checks do not verify physical controllers, native display
scaling, or shortcut visibility after the user restarts Steam.
