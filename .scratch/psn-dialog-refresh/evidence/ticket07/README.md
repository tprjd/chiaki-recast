# Ticket 07 evidence

Source: `6e89fe18`, with integration `d0ea2f19` merged at `4a79cd92`.
`source-sha256.txt` identifies the tested source files. The standalone screen target
built with Qt 6.9.3 and MSVC 19.44 in this worktree's `build/gui-screen` directory.
The shared native build cache was not used.

## Results

- `about-red.txt`: the old About screen fails the explicit Close action test.
- `keyboard-scroll-red.txt`: long information initially could not scroll by keyboard.
- `information-final.txt`: 23 passed, no failures. This includes 16 size/input rows,
  original attribution and license content, nonempty supplied version, logo, actual
  fallback reason, initial visible Close focus, modality, repeat opening, disabled
  opener fallback, keyboard scrolling, and Return/Escape/controller-Yes/button close.
- `information-after-merge.txt`: 5 passed, no failures after the integration merge.
- The six `regression-*.txt` files record 141 passed checks and no failures. They
  cover confirmations, registration choices, reminders, General, Keys, and controller
  mapping. These ran before the test runner metadata changed to the synthetic version.

All eight PNGs were inspected. The normal and long-text captures cover About and
renderer notices at 900 by 600 and 1280 by 720. Normal About fits completely at
1280 by 720 and scrolls at 900 by 600. Long content scrolls with Close fixed below it.
The captures use synthetic metadata and supplied device reasons. No credentials,
account information, or private paths are retained.

## Reproduce

Use the Qt/MSVC environment in `gui/tests/README.md`, then run:

```powershell
cmake --build build/gui-screen
$env:QT_QPA_PLATFORM = 'offscreen'
$env:QT_QUICK_BACKEND = 'software'
$env:QT_QUICK_CONTROLS_CONF = (Resolve-Path gui/src/qml/qtquickcontrols2.conf).Path
$env:QT_QPA_FONTDIR = "$env:WINDIR\Fonts"
& ./build/gui-screen/chiaki-console-screen-tests.exe -input gui/tests/qml/tst_InformationDialogs.qml -o build/information.txt,txt
```

## Limits

These are production QML tests with supplied state and navigation. They do not
prove native input capture/release, visible native focus, display scaling, actual
renderer failure, or physical controller operation. In particular, the bounded
non-Library fallback in production `Main.restoreMainFocus()` needs a native check
with a removed or disabled opener in ticket 08. The injected navigation test proves
the dialog invokes the fallback, not that Main's native focus chain is correct.
Normal application resource/build acceptance also remains in ticket 08. The new
renderer component is listed in production `qml.qrc`; fixtures are absent.
Refreshed Settings input-capture prompts were not changed.
