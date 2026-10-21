# Session notice checks

The production `StreamView` uses one shared component for each notice type.
The supplied state and command record are in `StreamHarness`.
No real console or user settings are used.

## Results

`notices-final.txt` records 40 passing checks on Qt 6.9.3 with the offscreen
software renderer. The checks cover both production renderer host routes.
They cover threshold values below, at, and above the configured limit,
400 ms warning timing, current loss text, warning clearance, and invalid values.
They cover all disabled-media states, blocked content, session replacement,
null sessions, destruction, menu handoff, PIN and connection priority,
disconnect cancellation, guarded actions, input balance, and focus return.
The focus check inspects pixels in the rendered control's owning item.

The layout checks use 900 by 600, 1280 by 720, and 1920 by 1080 logical pixels.
They use dark, bright, and detailed backgrounds, normal text, and long text.
The native notice windows retain the supplied native stream window as their
transient parent. Their bounds follow its movement. Passive windows are
transparent to input and do not accept focus.

The PNG files are Qt component captures. Inline images include a test background.
Separate-window images contain the notice window's content, with transparency.
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
The native-cache build and restoration records are added after its exclusive handoff.
