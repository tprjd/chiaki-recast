# Native stream interface acceptance

Date: 2026-10-21. [Ticket 05](../../issues/05-native-acceptance.md).
Final production change: `62f9676e`; integration and review merge: `faadb689b`.
All final native and normal-build source hashes are recorded below.

## Scope and results

The user limited the remaining tests to **900x600 at one scale** and asked to
avoid repeated tests. The specification records that decision. The final native
runs use 900x600 and measured DPR 1 on both renderers. Earlier results remain
labelled with their actual source. No full suite or larger matrix was repeated.

| Check | Result and source | Evidence |
| --- | --- | --- |
| Integrated production QML | Pass: 1,018 passed, 0 failed, 2 native-platform skips; `fafc1398`, Qt 6.9.3 MSVC offscreen | `production-qml-full.txt` |
| Affected composed QML after native input fix | Pass: Interface 37, Toolbar 36, Statistics 23, Notices 46; 2 known native-platform skips; `7d6d2563` | `final-Stream*.txt` |
| Final OpenGL / separate windows | Pass: 270 checks, DPR 1; final production at `faadb689b` | `final-opengl900.json`, `final-opengl900-source.json` |
| Final Vulkan / inline overlays | Pass: 251 checks, DPR 1; same final production | `final-vulkan900.json`, `final-vulkan900-source.json` |
| Settings focus, both routes and all notice conditions | Pass: 8/0 including setup/cleanup; `1246ab6d`, Qt 6.9.3 | `review-settings-green.txt` |
| Visible flyout focus pixels, both routes | Pass: 4/0; `1246ab6d`, Qt 6.9.3 | `review-focus-pixels.txt` |
| Existing linked settings actions | Pass: 6/0; `1246ab6d`, Qt 6.9.3 | `review-linked-actions.txt` |
| Settings-to-disconnect priority and focused flyout Ctrl+O | Pass: 5/0; `62f9676e`, Qt 6.9.3 | `review-settings-priority-green.txt` |
| Qt 6.7 production QML compatibility | Pass: final separate 900 layout, both focus captures, all six settings rows; `1246ab6d`. Its one failed Ctrl+O case is corrected and passes with both new priority cases at `62f9676e` (5/0) | `stream-qt67-review-final.txt`, `stream-qt67-shortcut-priority-final.txt`, matching source JSON |
| Earlier native 900/1280/1920 layouts, movement, resize, fullscreen | Pass on earlier `7d6d2563`: OpenGL 515 checks and Vulkan 479 checks, DPR 1. These precede the final Tool flyout host | `opengl100.json`, `vulkan100.json`, their source records and captures |
| Final normal application and resources | Pass: 342 source inputs and 77 generated resource inputs match; QA/test/prototype resources and symbols absent | `normal-checks.json`, `final-normal-build.txt`, `final-normal-help.txt` |
| Original native cache | Pass: original source hashes, options and output restored; both original targets rebuilt; separate final normal output preserved | `cache-restoration.json` |

The two offscreen skips require native window activation. The final Windows runs
cover passive overlay focus and real pointer routing. Qt 6.7 evidence establishes
production QML loading/rendering and the affected behavior, not a full native
application build on that Qt version. The Qt 6.7 source records include matching
before/after source hashes. `captures/qt67` contains the rendered flyouts.

## Native boundary and behavior

The native runs use real Main, QmlBackend, QmlMainWindow, StreamSession and the
production overlays. GCC/UCRT, Qt 6.11.2 and libplacebo 7.360.1 build this saved QA
copy. Actual runtime renderer identity is asserted: 1 is OpenGL with separate
windows; 0 is Vulkan with inline overlays. A requested renderer alone is not proof.

Session events, metrics and backgrounds are controlled. Start, Stop, GoToBed,
GoHome, ToggleMute and SetLoginPIN are intercepted before transport or microphone
capture. Discovery, wake traffic, account refresh and session logging are disabled.
Every process uses fresh isolated INIs. The `*-settings.json` records verify that
registered/manual consoles, controller maps, discovery, auto-connect, reminders
and disconnect policy match the run baseline. No real console or user settings
are used.

Final native checks establish initial Resume focus, actual foreground HWND,
controller-mapped keys through QmlMainWindow, both flyouts, Zoom, Custom, the
longest preset, explicit focus/selection, popup-first Escape and balanced input.
Real Windows mouse input switches and repeats openers, drags sliders past their
bounds, and reaches the stream outside panels. Immediate Escape after that outside
click closes the popup and restores its opener. The tests also establish native
input ownership across menu-open and blocked-to-blocked session replacement.

Statistics and simultaneous audio/network notices remain passive. Menu open/close
suppresses/restores them. Blocked and disabled-media notices have the correct
messages and controlled actions. Real Main Display/Renderer settings retain input
and native focus after menu closure, suppress actionable notices, then restore the
correct notice/input on close. Real Disconnect retains Cancel-first modal priority
and Escape sends no stop/sleep command. Teardown removes overlays and native grabs.
Connection, PIN and configured disconnect policy retain their earlier composed and
native feature-level evidence; unchanged suites were not repeated.

Long test translations, the longest preset, Zoom, Custom, large metrics and notice
combinations fit at the final 900x600 size. Geometry checks and retained desktop
crops show no clipping or overlap. Earlier dark/bright/detailed captures remain
in `captures/opengl100` and `captures/vulkan100`; final bright and detailed states
are in `captures/final-opengl900` and `captures/final-vulkan900`. Visual inspection
confirms translucent backplates, opaque readable text, visible focus, and no large
opaque host or full-screen dimming layer.

## Regressions found

- `native-red.json`: native session replacement reset capture while QML still
  owned input. Preserve capture for non-null old and new sessions; retain teardown
  and PIN/PSN behavior. Final checks include replacement, closure and input return.
- `native-pointer-red.json`: Qt Popup.Window grabbed the toolbar owner's mouse,
  so an outside click never reached the actual stream. Shared Popup.Item content
  now renders in a tight owned Tool window on the separate route. This also avoids
  the Qt 6.8-only popupType property, which failed to load on supported Qt 6.7.
- `native-focus-red.json`: after an outside click, a Tool window can report active
  while Qt's focusWindow still points at the stream. Keys now wait for the actual
  focusWindowChanged event, then restore the retained focused control. Cloned
  press/release events retain order and cancel on hide/session change. The final
  physical-pointer run includes immediate Escape, including reentrant popup hide.
  See [source diagnosis](../../review/native-focus.md).
- `review-settings-red.txt`: settings lost focus to menu cleanup/action notices.
  Navigation now waits for normal menu closure, and settings owns the notice/input
  boundary. `review-settings-priority-red.txt` separately proves the pending-settings
  versus Disconnect race; Disconnect cancels that pending action.
- Qt 6.7 exposed a flyout Ctrl+O shortcut scope difference. The native flyout owns
  its shortcut while the toolbar handler is disabled. Final Qt 6.7 verification
  passes. Flyout focus and selection also use explicit production styling.

Intermediate failing logs are retained as red evidence, not counted as passes.
`stream-qt67-observed-focus.txt` records a test-observer timing correction: wait
for actual focus before sending Return, and assert the Picture remains visible
before capture. A blank host capture is not accepted as rendering evidence.

## Capture method and identity

`*-desktop.png` uses FFmpeg ddagrab (DXGI Desktop Duplication). Each capture checks
the visible/exposed target, actual foreground HWND and Win32 client bounds. The
full frame must contain exact pixels of the current green or magenta application
marker. It is cropped to the real app client, then the full frame is deleted.
Thus retained images show actual desktop composition and contain no unrelated
full-desktop pixels. Each record includes capture method, bounds, DPR and hashes.

`*-target.png` instead uses OpenGL framebuffer readback, Vulkan texture readback,
or QQuickWindow.grabWindow as identified in JSON. Those are component evidence.
They are not desktop composition. Native DPR 1.25 QtTest grabImage was rejected
because it cropped incorrectly. Repeated controlled metric updates keep the normal
network-warning pulse visible without changing its production timer or threshold.

QT_SCALE_FACTOR=0.8 on the unchanged 125% Windows monitor yields measured DPR 1.
The final normal build compares every tracked build input with staged production,
both source QRC files, actual generated RCC inputs, normal link inputs and binary
markers. Exact executable, Qt DLL, helper and source/capture hashes are in the
source records and `normal-checks.json`. The preserved normal executable is:
`C:/Users/david/.cache/chiaki-ui-native/stream-acceptance-normal-bin/chiaki.exe`.

## Replay and restoration

Obtain exclusive access to the native cache and desktop. Add new source files to
Git before staging. Use ticket01 `stage-native.py` with a fresh
`stream-interface-prep*` name, then this directory's `prepare-native.py` with a
fresh nested name. Configure/build chiaki into `stream-acceptance-qa-bin`.
Run `run-native.ps1` with a fresh run name, renderer and Scale 0.8. The helper's
final size is 900x600. `run-matrix.ps1` now contains only the user-limited two-renderer
runs. `record-native.py RUN LABEL PREPARATION` saves evidence and checks the INIs.

Restore the nested preparation with PSN ticket06 `restore-native.py`, refresh
production sources, then build chiaki and chiaki-console-screen-tests into
`stream-acceptance-normal-bin`. Run `check-normal.py` and normal chiaki --help.
Restore the outer preparation with ticket01 `stage-native.py --restore`, configure
its saved original output and rebuild both original targets. Run
`check-restoration.py OUTER_PREPARATION`. The current outer preparation is
`stream-interface-prep05-acceptance01`; final nested preparation is
`stream-acceptance-native-prep-final01`. Restoration keeps the final normal output
separate from the original `console-setup-final-bin` output.

## Unverified and excluded

There is one physical monitor. Cross-monitor/live OS DPI transitions are unavailable.
An earlier 150% run failed an observer bounds check; it is not a pass. No 200% result
is claimed. Further scale/resolution testing was removed by the user's later scope
decision. Final Tool-window fullscreen/resize coverage is not inferred from the
earlier host's checks; the final run covers its actual movement and 900x600 bounds.

Live console media, actual network degradation, console power effects and physical
controller hardware remain unverified. Mapped Qt keys and controlled command counts
do not establish those effects. No release package, push, publication or pull request
was created. The throwaway prototype was not merged.
