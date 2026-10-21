# Approved A stream toolbar

The production toolbar is shared by the inline renderer and the separate native
window. It uses the approved descriptive labels, a 65% dark backplate, named
Resume and Disconnect actions, and picture and quality flyouts. Volume, mute,
zoom, preset persistence, settings navigation, and disconnect policy retain their
existing backend contracts.

The separate toolbar window follows the real native stream window. Each flyout
uses its own Qt popup window. The native host remains the size of the toolbar.
The shared menu state owns flyout selection and input capture. Native controller
keys now reach an owned active window or its visible popup before the hidden
render-control window. Closed popups cannot receive this route.

## Verified results

- `screen-summary.json` records 230 passing checks across ten production QML
  suites. The toolbar has 36 passing checks and StreamInterface has 37.
  Statistics has one documented offscreen-only skip. The related Disconnect,
  LiveConsolePin, ConnectionLocal, ConnectionPsn, ConnectionWake, SettingsDisplay,
  and SettingsRenderer suites pass.
- `layout-long.txt` records 20 passing checks with a test-only QTranslator that
  extends each toolbar label. Both hosts cover 900 by 600, 1280 by 720, and
  1920 by 1080 against dark, bright, and detailed backgrounds. The checks include
  Zoom, Custom, the longest preset, wrapped labels, control bounds, and measured
  global flyout bounds. The normal toolbar is 820 by approximately 80 logical
  pixels and is 18 pixels above the stream bottom.
- `captures/` contains production component captures. Separate-window images
  retain their component alpha. They do not establish desktop composition.
- `native-opengl.json` records 386 passing native checks.
  `native-vulkan.json` records 360. The actual renderer selects each host route.
  Measured DPR is 1.25. The checks use real Main, QmlBackend, QmlMainWindow,
  StreamSession, and production QML, with Start, Stop, and GoToBed intercepted
  before transport and with settings below a fresh isolated INI directory.
- Native controller-mapped Qt keys enter through QmlMainWindow, the target used
  by QmlController. Checks cover Resume focus, toolbar navigation, both flyouts,
  preset selection, Escape order, opener focus, Menu closure, native stream
  activation, and balanced session input. Both logical native sizes, 900 by 600
  and 1280 by 720, exercise Cancel, Escape, native close, Disconnect, and
  Disconnect and sleep. Repeated close does not duplicate input acquisition.
- Windows WindowFromPoint checks select the actual toolbar owner. SendInput
  pointer events switch Quality to Picture and close Picture on second opener
  activation. Separate-window checks verify the native transient-parent chain,
  compact bounds, popup placement, and actual stream-window move and resize.
- `isolated-settings-*.json` confirm that registered consoles, manual consoles,
  controller mappings, disconnect policy, discovery, and reminder preferences
  stay unchanged. Test preset writes remain in isolated settings.

## TDD record

`red-resume.txt`, `red-picture.txt`, `red-quality.txt`, and `red-priority.txt`
record failing slices before their implementation. The priority slice found that
an open flyout remained over a connection transition. The menu now closes when
a managed connection leaves ready state.

`native-red.json` fails at the first controller-mapped key sent through the real
main window. The final native results establish the corrected window route.
The native runs also found duplicate Escape processing. The window Cancel
shortcut is disabled while a flyout owns Escape. Hidden menu shortcuts are
inactive while the disconnect dialog is open.

`red-initial-focus.txt` fails an actual blue-outline pixel check with the former
visualFocus binding. `initial-focus.txt` and the final toolbar suite pass after
the border uses activeFocus. The two `resume-focus-*.png` captures show this
initial focus. This is the only production change after the normal build below.
The native input checks establish focus ownership, not this final outline.

## Build and source records

`native-source.json` records the feature source at `99b8427c`, the staged QA
source hashes, helper hash, and native executable hash. The final helper is
produced by `prepare-native.py`. `native-build-final.txt` records its build.
The native run names are `stream-toolbar-native-pointer01` and
`stream-toolbar-native-pointer-vulkan01`.

The normal production target was built after integration merge `4dd8813c`.
`normal-checks.json` verifies all 338 production input files against the
worktree, executable and Qt DLL hashes, and the absence of QA symbols, fixtures,
and prototype resources. `normal-help.txt` records a successful `--help` run.
`final-source.json` records the final source, including the focus-outline fix
at `6ab85a7d`. Ticket 05 owns the final integrated native build of that style
change.

`cache-restoration.json` verifies all 338 original source hashes. The original
CMake options and runtime output were restored. `restored-build.txt` records
successful rebuilds of the original chiaki and screen-test targets. The cache
was released to the coordinator before documentation completion.

## Replay

Use this worktree's screen runner as described in `gui/tests/README.md`.
`run-screens.ps1` runs the ten recorded suites. Set
`CHIAKI_SCREEN_LONG_TRANSLATIONS=1` and run only
`StreamToolbar::test_layout` for the extended-label cases. The test-only
translator is in the screen runner and is absent from the production target.
Captures are written below this worktree's `build/` directory.

Native cache access requires exclusive ownership from the coordinator. Follow
[ticket 01's staging and restoration procedure](../ticket01/README.md), with
`stage-native.py`, `prepare-native.py`, and `run-native.ps1` from this directory.
Use fresh preparation and run names. Configure the QA runtime output as
`stream-toolbar-qa-bin`. Restore the nested native preparation before refreshing
production inputs. Configure the normal runtime output as
`stream-toolbar-normal-bin`, run `check-normal.py`, then restore the outer
preparation and rebuild the original targets. Do not run these steps while
another ticket owns the cache.

## Remaining acceptance

Ticket 05 checks the combined statistics, notices, and toolbar on the desktop.
Fullscreen, 100%, 150%, and 200% display scaling, final desktop blending, empty
region hit testing, and combined notice overlap remain there. The captures here
are component readbacks. They do not prove desktop composition. Live console
media, physical controllers, console power changes, and real network loss are
not established by the controlled checks.
