# Compact stream statistics

Ticket 03 implements approved A from `compact-v2/statistics-a.png`.
The row uses the six existing production metrics. Its dark backplate has
65 percent alpha. Labels and values remain opaque. Long translated labels
can shorten with an ellipsis. Metric values and accessible names stay complete.

## Results

| Check | Result | Evidence |
| --- | --- | --- |
| Composed production QML | 23 passed, 1 platform skip | `screen-statistics.txt` |
| Separate and inline Windows keyboard focus | 4 passed | `native-passive.txt` |
| Existing composed menu and disconnect cases | 37 passed | `screen-interface.txt` |
| Real Vulkan session and statistics | 38 passed | `native-vulkan.json` |
| Real OpenGL session and statistics | 51 passed | `native-opengl.json` |
| Normal `chiaki` and screen target | Built | `normal-build.txt`, `normal-final-build.txt` |
| Normal executable and resources | No QA symbols or test resources, `--help` passed | `normal-checks.json`, `normal-help.txt` |
| Shared cache restoration | 337 original source hashes restored, original targets rebuilt | `cache-restoration.json`, `restored-build.txt` |

The QML cases load actual `StreamView`, its inline and separate statistics
hosts, and `StreamStatsContent`. They check all six values and units, live
updates, zero, NaN and both infinities where the native property type permits
them, preference changes, menu closure, connection loading and failure,
disabled video, session replacement, and destruction.

The layout cases use 900 by 600, 1280 by 720, and 1920 by 1080 logical pixels.
They supply long translated labels, 10-digit frame counters, and large metric
values. All six values stay complete and inside the row. Both hosts keep
18-pixel bottom and left margins. `component-long-labels-*.png` retain these
views. `component-bright.png` and `component-detailed.png` retain background
checks. The image tests read actual rendered backplate and text pixels.

`red-live.txt`, `red-finite.txt`, and `red-visibility.txt` record failing TDD
slices. `native-red-reset.json` records the native session-reset failure before
the fix. The real backend now clears the dropped-frame counter, pending drops,
average queue depth, and pending frame age when its session changes.

The visible native test host exposed a hidden-menu shortcut bug. Its Escape
shortcut consumed Escape in the separate disconnect dialog after menu closure.
`StreamMenuWindow` now enables its shortcuts only while the menu is open.
`shortcut-trial.txt` records the passing eight disconnect-entry cases.

## Native environment and source

Native checks use GCC 16.2.0, Qt 6.11.2, and libplacebo 7.360.1.
The actual renderer is Vulkan for inline and OpenGL for separate statistics.
Both runs measure DPR 1.25. The 900, 1280, and 1920 captures have physical sizes
1125 by 750, 1600 by 900, and 2400 by 1350.

The native checks use real `QmlBackend`, `QmlMainWindow`, `StreamSession`, and
production QML. The established test preparation isolates INI settings and
disables discovery, account refresh, wake, logging, and session transport.
Two staged helpers supply video readiness and nonzero window metrics.
They are absent from the normal source and executable.

Native checks cover actual session metric reset, replacement, preference,
teardown, three sizes, native movement, fullscreen entry and exit, transient
ownership, session input ownership, and Qt passive-window flags. OpenGL also
checks Windows `WS_EX_TRANSPARENT` and `WS_EX_NOACTIVATE` styles.

`native-source.json` records exact source hashes for the native QA runs.
Those runs precede the final QML change that updates value labels without
recreating the metric delegates. The final QML cases verify that change.
The normal native build includes that value-binding change. It precedes only
the two shortcut `enabled` lines and final whitespace cleanup in statistics
QML. `final-source.json` lists those exact production deltas and final hashes.
The composed QML cases verify the shortcut change. Ticket 05 rebuilds and
checks the final combined interface.

`native-vulkan-*.png` use Qt Quick target GPU readback.
`native-opengl-*.png` use the separate `QQuickWindow.grabWindow` target.
These are component captures. They do not establish final desktop composition.
The offscreen platform activates passive windows, so its separate focus case
is skipped. The same case passes with the Windows platform. Windows QML pixel
checks are skipped because that capture path clips items outside its origin.
The offscreen renderer and native renderer targets supply the retained pixels.

## Replay

Run `run-screens.ps1` from this directory after the standalone screen build
described in `gui/tests/README.md`. Use `-Platform windows -Cases
StreamStatistics::test_passive` for native focus. Use `-Suite StreamInterface`
for the existing menu and disconnect cases.

Use the shared native cache only with exclusive ownership from the parent task.
Track new inputs before staging. Stage with ticket 01's `stage-native.py` and
a fresh `stream-interface-prep-*` name. Run this directory's `prepare-native.py`
with a fresh QA preparation name. Build `chiaki` into
`stream-statistics-qa-bin`. Run `run-native.ps1` with a fresh run name and
`-Renderer vulkan` or `-Renderer opengl`.

Restore the nested QA preparation with PSN ticket 06's `restore-native.py`.
Refresh the staged production inputs. Build normal `chiaki` and
`chiaki-console-screen-tests`. Restore the outer staging preparation, then
configure and build the original output directory and targets.

Live console media, physical controller input, real network loss, desktop
composition, and unobserved display scales remain for the applicable native
acceptance checks. No package or remote push was made.
