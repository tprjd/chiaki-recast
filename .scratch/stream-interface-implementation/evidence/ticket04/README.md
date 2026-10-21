# Session notice checks

The production `StreamView` uses one shared component for each notice type.
The supplied state and command record are in `StreamHarness`.
No real console or user settings are used.

## Results

`notices-qt611.txt` records 46 passing checks and one skipped check on Qt 6.11.2.
It uses the screen target from the normal native build, with the offscreen
software renderer. The skipped check concerns passive native window activation.
`notices-windows-final.txt` records 47 passing checks on Qt 6.9.3 with the Windows
platform. `notices-qt611-windows.txt` records nine passing checks on Qt 6.11.2 with
the Windows platform. This last run covers passive input, blocked-content keys,
renderer-host changes, and the native close route.

The checks cover both production renderer host routes. They cover threshold
values below, at, and above the configured limit, 400 ms warning timing, current
loss text, warning clearance, and invalid values. They cover all disabled-media
states, blocked content, session replacement, null sessions, destruction, menu
handoff, PIN and connection priority, disconnect cancellation, guarded actions,
input balance, and focus return. The offscreen focus check inspects actual pixels
in the rendered control's owning item. The Windows passive check observes that
the game retains focus and receives Space while both routine notices are visible.

The eight `notices-regression-*.txt` files contain 171 passing checks for the menu,
connection, PIN, disconnect, and linked settings screens. The integrated menu and
statistics logs contain 37 passing menu checks and 23 passing statistics checks.
One offscreen statistics activation check is skipped as specified by that suite.

The layout checks use 900 by 600, 1280 by 720, and 1920 by 1080 logical pixels.
They use dark, bright, and detailed backgrounds, normal text, and long text.
The native notice windows retain the supplied native stream window as their
transient parent. Their bounds follow its movement. Passive windows are
transparent to input and do not accept focus.

The PNG files are Qt component captures. Inline images include a test background.
Separate-window images contain the notice window's content, with transparency.
The offscreen captures use DPR 1. The Windows layout logs record the actual screen
DPR of 1.25. QtTest crops native-window captures in logical pixels at that scale,
so those captures are rejected. `notices-windows-capture-observer-red.txt` records
the pixel assertion that exposed this observer defect. Windows runs check behavior
and logical bounds. They do not provide focus-pixel or desktop-composition proof.
These images do not establish desktop composition, a native renderer backend,
physical controller behavior, fullscreen, or display-scale changes.
Ticket 05 owns the combined native checks.

`notices-*-red.txt` retain failing tests before each behavior change.
The long-action-text check first rejected the unwrapped button label.
The session replacement check first found the previous warning's fade.
The menu handoff check first found a focus restore that took focus from a notice.

## Replay

Use the screen runner from `gui/tests/README.md`. At the repository root, run:

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
$env:QT_QUICK_BACKEND = 'software'
$env:QT_QUICK_CONTROLS_CONF = (Resolve-Path gui/src/qml/qtquickcontrols2.conf).Path
$env:QT_QPA_FONTDIR = "$env:WINDIR/Fonts"
& .\build\gui-screen\chiaki-console-screen-tests.exe -input gui/tests/qml/tst_StreamNotices.qml -o build/notices-final.txt,txt
```

Create `build/` before the run. The layout tests write `build/notice-*.png`.
## Normal native build

`normal-build.txt` records a successful Release build of `chiaki` and
`chiaki-console-screen-tests` with GCC/UCRT, Qt 6.11.2, and the populated native
cache. `normal-checks.json` records source revision `91264688` and integration
base `a91d1d65`. All 341 staged inputs match the worktree. The generated resources
contain all three new notice components. They exclude test fixtures and prototype
assets. The normal target excludes the screen runner and native QA symbols.
The normal executable exits successfully with `--help` before settings creation.
`normal-source.json` records the built source hashes.

The normal output remains at
`C:/Users/david/.cache/chiaki-ui-native/stream-notices-normal-bin`.
`normal-checks.json` records the executable and Steam DLL hashes.

The existing ticket 01 `stage-native.py` stages and restores the build inputs.
This run uses preparation `stream-interface-prep04-notices`. Both normal targets
build with runtime output `stream-notices-normal-bin`. After restoration, both
original targets rebuild with runtime output `console-setup-final-bin`.
`cache-restoration.json` verifies all 341 saved path hashes after that rebuild.
It also verifies all 36 saved build options and the original output directory.
The native cache is released to the parent task.

No real backend session, console command, network service, user settings write,
physical controller, release package, or push is used. The controlled actions run
through production QML bindings against the supplied command boundary. Ticket 05
covers the combined actual-renderer application and desktop composition.
