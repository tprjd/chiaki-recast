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

- `hosts` contains the complete current backend list in backend order.
  Hidden records remain in this input, so their source indices stay correct.
  The interim `sourceHosts` and `sourceIndices` options are removed.
- `streamerMode` masks addresses and console IDs in details and an address
  used as the console name. Accessible text and tooltips use the same values.
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

## Full screen API and focus

`ConsoleLibrary` is now the complete production main screen. The test boundary
remains this screen. `MainView.qml` binds backend state and handles its signals.
The original startup reminders and StackView navigation remain in MainView.

Additional state properties are `discoveryEnabled`, `psnAvailable`,
`steamAvailable`, `applicationVersion`, and `playStationController`.
The screen emits these commands:

- `settingsRequested()` opens the existing Settings screen.
- `addConsoleRequested()` opens the existing manual-host dialog.
- `refreshPsnRequested()` uses the existing PSN refresh handler.
- `steamShortcutRequested()` opens the existing Steam shortcut dialog.
- `discoveryToggled(enabled)` requests the new discovery state.
- `quitRequested()` opens the existing quit confirmation.
- `managementRequested(action, identity)` requests `pin`, `wake`, `delete`, or
  `hide` for an opaque identity.

`eligible(action, host)` applies the same rules to menu and controller actions.
MainView resolves management identities again before backend calls. Delete and
Hide also resolve after confirmation. PIN submit still uses the existing dialog
and requires the delayed identity check in ticket 06.

`selectedIdentity` is the selected console. `selectedTarget` resolves its
current host, source index, and nickname. `moveSelection(offset)` moves through
visible, resolvable cards and terminates with an empty list.
`restoreCardFocus(identity)` focuses that console's primary action, a remaining
resolvable card, or Add console.

`ConsoleCard.qml` owns only the card appearance and its focusable controls.
It accepts `host`, `streamerMode`, `selected`, and `targetEnabled`.
It emits `activated()`, `actionsRequested(opener)`, and `selectionRequested()`.
`focusPrimary()` puts visible keyboard focus on its primary action.
The screen resolves the host identity when these signals arrive.

The card's ellipsis opens `ConsoleDetails.qml` without a connection request.
The popup shows available address, ID, registration, route, app, and title ID
fields. ID uses the MAC when available and otherwise the PSN DUID. Missing
fields have no row. Long details scroll while Close remains visible.
Host names, details fields, and card tooltips use plain text.

Details resolve their saved identity against the current full host list.
Rename and reorder update the popup. Removal, hidden entries, duplicate
identities, and profile changes close it. Escape closes details and returns
keyboard focus to the opener, a valid card, or Add console.

Manage opens the temporary action menu for eligible consoles. Escape closes
the menu first and returns focus to Manage. A management action closes both
popups before its command runs. Ticket 06 replaces this temporary management
route and completes delayed PIN target checks.

Search and counts are in `ConsoleLibrary.qml`, including `libraryToolbar`
and the footer after the card grid. Details use
`ConsoleCard.actionsRequested(opener)`. Both features resolve identities
against the full `hosts` list.

## Card library results, 2026-10-20

The standalone C++ target built with MSVC 19.44 and Qt 6.9.3. The complete screen
suite passed. Checks include command dispatch, conditional PSN and Steam
controls, discovery, menu actions, controller commands, empty-list navigation,
focus restoration, and action bounds at 1280 by 720 and 800 by 600 logical pixels.
The former index-map checks now supply complete backend host fixtures.

Offscreen captures at both sizes confirmed the sidebar, dark cards, white
console artwork, blue primary actions, and narrow sidebar controls. The console
SVG body fill changed to white to keep it visible on the dark artwork panel.
The card uses a local plain background because Material elevation effects do
not render with Qt Quick's software renderer. Pixel checks cover both defects.

The captures use sample hosts outside application resources. The preserved
prototype branch and approved screenshot remain the design reference.
The full changed application build and launch still require integration
verification. Console hardware, PSN accounts, physical controllers, and Windows
100%, 150%, and 200% display scaling remain untested in this ticket.

## Console details results, 2026-10-20

The MSVC and Qt 6.9.3 screen target passed all 77 checks, with no failures,
warnings, or skipped checks. CTest also passed `chiaki-console-screen`.
The details checks use the complete production screen and sample hosts.
They cover available and missing fields, live reorder and rename, stale
targets, profile changes, privacy, mouse and keyboard input, focus recovery,
and long details at 800 by 600 logical pixels.

The details control failed before the popup existed. Target-loss checks
failed before popup invalidation. Long text and repeated keyboard opening
also failed before their fixes. Offscreen captures at 1280 by 720 and
800 by 600 confirmed the popup layout and the visible Close control.

The screen test observes commands without a network session. Full-application
verification, console hardware, PSN accounts, physical controllers, and
Windows display scaling remain outside this ticket's checks. The existing
management menu remains available through Manage until ticket 06 replaces it.

After the search integration, all 106 screen checks and CTest passed. The added
plain-text regression first detected a rendered hyperlink in a console name.
It now checks literal names, app text, and tooltips through their rendered
link behavior and accessible text.

## Search and live selection API

`searchText` is the search field's text. `visibleHosts` excludes records with
`display: false`. `displayedHosts` applies search to that visible list without
changing backend order or source indices.

Search matches every space-separated word against the displayed name and the
known console type. It ignores case and extra spaces. Known types include
PS4, PS5, PlayStation 4, and PlayStation 5. A missing type has no type keyword.
Streamer mode searches the masked name when an address is the name.

The footer shows the visible total and the filtered count during search.
The empty screen offers Add console, Enable discovery, or Clear search for
its current state. Clear search also returns focus to the search field.

Selection retains its identity after reorder or rename when the console still
matches. A removed, filtered, or ambiguous target selects the first remaining
resolvable card, or clears selection. Profile changes invalidate old identities.
Normal text editing stays active during host updates, with the same cursor
position and focus. Enter does not connect a console from search. Escape clears
nonempty search first, then uses the existing Quit request for empty search.
Tab and Shift+Tab use normal focus navigation.

The grid derives card width from the viewport column count. One or two results
keep their normal card width, left alignment, and 18-pixel gap.

## Search results, 2026-10-20

The MSVC 19.44 and Qt 6.9.3 screen target built successfully. CTest passed.
The complete suite reported 89 passed checks with no failures or skipped checks.
The new search checks use `qml/tst_ConsoleSearch.qml` and the production screen.

The name and type filters, counts, empty-state actions, selection after filter,
editing keys, Escape, Tab navigation, and partial-row geometry failed before
their corresponding changes. Checks also cover live reorder and removal,
profile changes, all-hidden lists, missing or ambiguous identities, streamer
mode, empty navigation, and narrow empty-state bounds.

Offscreen renders at 1280 by 720 and 800 by 600 confirmed the filtered card
width, search field, count labels, and no-match action. Full application launch,
hardware connections, physical controllers, and Windows display scaling remain
for integration verification.
