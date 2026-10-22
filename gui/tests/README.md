# Console screen tests

## Embedded setup application

The full application build requires Qt 6.9.3 or later with WebEngineWidgets,
WebChannel, and matching GuiPrivate headers. The frontend build uses Node
24.16.0 and npm 11.13.0. Its lock file fixes Svelte 5 and the build tools.
The compiled page and artwork are Qt resources. The installed application
does not need Node or a web server.

`chiaki-embedded-setup-tests` uses the production Svelte page, QWebEngineView,
WebChannel, SetupBridge, and native services. `consoleLibrary` supplies replies
at the native discovery completion boundary and observes the page and native
state. It checks discovery commands, console selection, profile changes,
privacy, navigation, keyboard events, and layout.

Run only the required case:

```text
ctest --test-dir build -R "^chiaki-embedded-setup$" --output-on-failure
ctest --test-dir build -R "^chiaki-embedded-graphics$" --output-on-failure
```

The first case has the `desktop` label. The graphics case has `desktop` and
`gpu` labels. Tests are enabled by default. Use explicit names or exclude
these labels in a headless run. A software or generic test pass does not
establish GPU acceptance.

`graphicsCoexistence` is a separate QtTest case in the same application harness.
It reads `chrome://gpu`, checks imported native OpenGL and Vulkan devices,
renders controlled native frames and the production overlay, recreates native
windows, and checks setup restoration. It requires an unobstructed hardware
desktop. Set `CHIAKI_EMBEDDED_EVIDENCE` to an existing directory to save its
GPU report and images. A protected OS dialog invalidates screen and focus
observations. Controlled frames do not prove live console decode or latency.
The fixture selects the existing native Fast preset through Settings.
Production defaults stay unchanged. The Windows HighQuality processing issue
recorded in ticket 03 evidence remains acceptance work for tickets 04 and 23.

The standalone QML checks below retain coverage for setup flows that have not
yet moved to the web page and for shared native components.

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

The embedded setup tests cover saved console login PIN edits and manual
addresses through compiled Svelte, real WebChannel, and native services.
`manualAddress` checks save and reload. `manualAddressValidation` checks
address validation, current registration association, duplicate activation,
and stale profiles. `consoleManagement` checks PIN validation and leading
zeroes, secret-free replies, focus return, cancelled confirmations, and the
distinction between manual address and registration deletion.
`consoleTargetGuards` checks discovery reorder, ambiguous or removed targets,
changed manual addresses, privacy masking, and narrow or zoomed forms.
`manualHandoff` uses the application controller to connect a registered manual
address, hide and freeze setup, wait for native cleanup, and restore setup.
The handoff case does not prove real console playback, physical focus, or GPU
acceleration. Run embedded application cases serially because Qt services
use a shared application settings identity.

The replaced `ManualHostDialog`, saved `ConsolePinDialog`, and their dedicated
QML fixtures and tests are removed. `ConsolePinContent` remains shared by the
native live PIN route. The legacy library and settings resource chains remain
for the setup flows that later tickets replace.

`StreamRoot.qml` owns the native `LiveConsolePinHost` during playback and connection progress.
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

## Account ID retrieval for registration

`RegistrationAccountHarness.qml` uses production `RegistDialog.qml` and its
default Account ID route through a real `StackView`. `RegistDialog.accountLookup`
replaces only the public lookup service. Its callback receives an HTTP status
and response text. Production keeps the `psn.flipscreen.games` service.

`RegistrationAccount` checks empty input, network and response errors, retry,
the 30-second timeout, privacy, focus, keyboard/controller keys, and both required
window sizes with long text. It checks direct return to the original form,
duplicate and late results, replacement screens, and profile changes.
Public lookup uses no authentication command or credential save.

The sign-in route loads production `PSNTokenDialog.qml` in registration mode.
It retains the credential-save and background-completion explanations. A new
registration screen can show an existing request, but cannot receive its
Account ID. The screen tests supply authentication outcomes. Native persistence
is covered by ticket 01; integrated native acceptance is covered by ticket 08.

## Disconnect checks

`DisconnectHarness` loads production `StreamDisconnectDialog` with controlled
session commands. `Disconnect` checks both window routes, initial Cancel focus,
all choices, Escape, close, repeated input, disabled actions, keyboard and
controller key mappings, return focus, and long text at 900 by 600 and
1280 by 720. Both routes use the production `ConfirmDialog` and `SetupButton`.

`StreamDisconnectDialog` uses the native stream input owner lifecycle. The separate dialog uses
the actual native host window for ownership and centering. `Main` binds its
overlay size to its viewport because the render-control Quick window is hidden.

Controlled native checks and replay files are in
`.scratch/psn-dialog-refresh/evidence/ticket06/`. They use real `QmlBackend` and
`StreamSession` objects, isolated settings, and intercepted transport commands.
OpenGL and Vulkan render-target captures verify the current dialog pixels.
Desktop compositor capture was unavailable in that session. Real console power
changes, physical controllers, and broader display scaling need final acceptance.

## Composed stream interface checks

`StreamHarness` loads production `StreamView`, `StreamMenuWindow`,
`StreamMenuContent`, `StreamStatsContent`, `ConnectionView`, and
`StreamDisconnectDialog`. The application uses native defaults for the four
supplied properties on `StreamView`:

| Property | Native default |
| --- | --- |
| `backend` | `Chiaki` |
| `settings` | `backend.settings` |
| `streamWindow` | `backend.window` |
| `navigation` | `root` |

The fixture supplies session, connection-attempt, PIN-request, settings, and
window state. Its `window.menuRequested()` enters the real menu signal handler.
`window.grabs` and `window.releases` record input ownership. `backend.commands`
records session and connection commands. `actions` records linked navigation.
Set `screenActive` to false to destroy the complete stream interface.
The test-only `StreamTestWindow` preserves the native `QWindow` contract and
uppercase `ZoomFactor` property. It is registered only by the screen runner.

`StreamInterface` covers both renderer routes, initial focus, Escape, native
window close, repeat input, destruction, focus return, volume endpoints and an
intermediate value, microphone state, picture modes, zoom endpoints and saved
values, all five presets, linked settings, configured disconnect choices,
disconnect-dialog outcomes, session replacement, and null-session cleanup.
It checks connection, PIN, and disconnect exclusions. The existing statistics
visibility is checked across menu closure.

`StreamMenuContent.qml` owns the shared controls and actions. Both menu hosts
load the same component supplied by `StreamView`. `menuState` in `StreamView`
owns menu closure. `QmlMainWindow.streamInput` owns input capture and return focus.
`StreamMenuWindow` owns its native window
and focus timer. `StreamStatsContent.qml` is the existing shared statistics
presentation. Its inline and separate-window wrappers remain in `StreamView`.
`StreamNotice`, `StreamPassiveNotices`, and `StreamActionNotice` share notice
content between the inline and native window hosts. `StreamView` owns their
conditions, warning timer, and guarded session actions. The native ownership
module owns notice input capture and saved return focus.

`StreamNotices` checks the configured packet-loss threshold and 400 ms warning
pulse, audio and network stacking, blocked content, disabled video, combined
disabled media, and session replacement and teardown. It checks production home
and menu actions, menu and dialog priority, keyboard input, input balance, and
focus return. Layout checks cover 900 by 600, 1280 by 720, and 1920 by 1080 logical
pixels, long text, and dark, bright, and detailed backgrounds. Captures under
`build/notice-*.png` are Qt component images. They do not prove desktop composition.

`StreamHarness.nativeHost` places the supplied stream view and game focus target
inside its visible `StreamTestWindow`. Separate notice windows retain this actual
native parent. The default remains the screen runner's window for existing tests.

`StreamStatistics` checks the six live values, units, finite-value fallbacks,
valid zeros, large values, long labels, preference and visibility changes,
session replacement, teardown, passive input, and both production hosts.
Set `StreamHarness.nativeHost` to true for the separate route. This places the
actual `StreamView` and game input control in the visible `StreamTestWindow`.

Pixel checks use the offscreen software renderer at 900 by 600, 1280 by 720,
and 1920 by 1080. They cover dark, bright, and detailed backgrounds. The
offscreen platform activates passive native windows, so separate-window focus
is checked with the Windows platform instead. Native evidence and replay files
are in `.scratch/stream-interface-implementation/evidence/ticket03/`.

The baseline replay files are in
`.scratch/stream-interface-implementation/evidence/ticket01/`.
They include real Vulkan and OpenGL menu-to-disconnect checks with isolated
settings and intercepted transport. These checks do not prove desktop
composition, live media, physical controllers, or other display scales.

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

## About and renderer information

`InformationDialogs` opens About through production Settings and loads the production
`RendererFallbackDialog`. The suite checks attribution, license text, a supplied
application version, the logo, the actual supplied fallback reason, one Close action,
visible focus, modal input, repeat opening, disabled opener fallback, keyboard
scrolling, and keyboard/controller dismissal. Layout checks use 900 by 600 and
1280 by 720 with long translated text and titles. Captures are written under
`build/information-*.png`.

The screen runner supplies the application name Chiaki and synthetic version
`0.0.0-screen-test`. These values do not change the normal application metadata.
The tests do not exercise native input capture, the Main non-Library focus fallback,
display scaling, a real renderer failure, or a physical controller. Ticket 08 covers
native acceptance.

The maintained `chiaki-native-services-tests` check loads the production
`StreamRoot.qml` through `QmlMainWindow`. It checks stream-only progress,
saved stream geometry, PIN cancellation and acknowledgement, the return request,
window recreation, and direct-stream exit. `sessionAndFeedback` and
`keyboardMouseSettingsRoute` retain production root settings and input checks.
The former also checks renderer-fallback input ownership while live settings close.
Use a direct process launch for normal saved-size window checks. On Windows,
`Start-Process -WindowStyle Hidden` can suppress the native first normal show,
although Qt reports the window as visible.

## Web setup and native stream handoff

`chiaki-embedded-setup-tests localHandoff` starts a connection through the
compiled Svelte page, WebChannel, and production bridge. The application
controller prepares the native renderer before session creation. Setup hides
after the native window presents its first frame. The check covers page
freezing, cancellation, cleanup, and setup restoration.

`decodedSessionCleanup` supplies native session events and encoded H.264
samples at the existing transport and decoder boundaries. It uses the real
decoder, frame worker, native renderer, QML PIN form, menu, and disconnect
dialog. It tracks decoded AVBuffer lifetimes through normal OpenGL disconnect
and Vulkan controller destruction with queued GUI delivery. The same native
backend and QML engine serve both connections.

`suspendClosesSignIn` checks the account and handoff merge boundary. It closes
the native sign-in page during a pending exchange, freezes setup, completes
the exchange through the controlled native HTTP boundary, and resumes with
fresh account status. Closing the page does not cancel the native operation.

The fixture explicitly clears the saved console login PIN. A zeroed
`ChiakiRegisteredHost` otherwise creates the saved string `0`, which selects
the existing automatic PIN submission route. The frame fixture is generated
with FFmpeg 7.1:

```text
ffmpeg -f lavfi -i color=c=red:s=320x180:r=60 -frames:v 1 -c:v libx264 -preset ultrafast -tune zerolatency -f h264 red.h264
```

On Windows, `nativeInitializationFailure` uses the offscreen platform's real
OpenGL context creation failure. Its test process disables Chromium GPU use
and crash-report UI. It checks that setup remains visible and presents the
native error once. These process options do not change production startup.

The native tests use the existing Fast preset. They do not certify other
presets, real-console playback, hardware decoding, physical input, or screen
output. Actual foreground checks require the active unlocked input desktop.
