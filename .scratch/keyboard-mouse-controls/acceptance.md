# Keyboard and mouse acceptance

Recorded: 2026-10-21.
Status: Software implementation and final review fixes are complete.
Selected checks pass. One earlier desktop-check failure has no established cause.
Physical input, calibration, and gameplay acceptance remain open.
This record does not approve release.

## Source, build, and environment

| Item | Recorded value |
| --- | --- |
| Integration branch | `codex/keyboard-mouse-integration` |
| Final integrated source | `ed4d56d767bc426136e2f08ef920bac32fe10658` |
| Initial integrated check | `992cc170e834c6f92132cc72487a071ec7dd4a0d` |
| Build | Release, GCC 16.2.0 UCRT64, Qt 6.11.2, app version 1.10.0 |
| Primary build directory | `C:/Users/david/Projects/chiaki-ng-ui/build/keyboard-mouse-native` |
| Build source directory | `C:/Users/david/Projects/chiaki-ng-ui`, confirmed in `CMakeCache.txt` |
| Features | SDL controller support, Steam shortcuts, and maintained tests enabled |
| Windows | Windows 11 Pro, 64-bit, version 10.0.26100, build 26100 |
| Display adapter | NVIDIA GeForce RTX 5090, driver 32.0.16.1742 |
| Reported desktop mode | 3840 by 2160, not a complete display-arrangement measurement |
| Physical mouse | Generic HID devices reported. Model, DPI, and report rate are unknown |
| Game settings | Not recorded. Neither required game trial ran |

The primary build survives managed worktree cleanup. It uses the installed
UCRT64 runtime. The separate historical cache source tree was not built.
Qt WebEngineQuick is unavailable in this local toolchain.
The build log includes existing dependency, deprecation, and Qt private-header warnings.

The final app-only incremental build passed at `ed4d56d7`.
The [completed build log](C:/Users/david/Projects/chiaki-ng-ui/.scratch/keyboard-mouse-controls/evidence/final-app-build.log)
records `cmake --build build/keyboard-mouse-native --target chiaki --parallel 4`.
No test ran again after the final review-fix evidence was recorded.
The app file is 6,513,914 bytes. Its SHA-256 is
`3171908EE7689CD5979B94FBAA972432E1BF4E95ED51C37F64122D01FB4996E4`.

## Initial integrated check at 992cc170

The incremental build of `chiaki` and `chiaki-stream-input-native-tests` passed.
One native invocation selected `keyboardCapture` and `mouseTriggers`.
Its completed Qt log reports 4 passed, 0 failed, and 0 skipped, including setup and cleanup.
These cases check all keyboard defaults, opposing and diagonal movement,
exclusive touch and motion selection, release and rearm, five mouse buttons,
and simultaneous mouse and keyboard trigger alternatives.

Evidence is in the primary checkout:

- [Incremental app and native-test build](C:/Users/david/Projects/chiaki-ng-ui/.scratch/keyboard-mouse-controls/evidence/final-build.log).
- [Completed native controller-output log](C:/Users/david/Projects/chiaki-ng-ui/.scratch/keyboard-mouse-controls/evidence/final-native-output.log).

The commands ran from the primary checkout. The runtime environment used
`C:/Users/david/.cache/chiaki-ui-native/msys64/ucrt64/bin` and the build's
`third-party/steam-tools` on `PATH`. `QT_PLUGIN_PATH` selected the same UCRT64 Qt plugins.
The native output cases used `QT_QPA_PLATFORM=offscreen` and `QT_QUICK_BACKEND=software`.

```powershell
	cmake --build build/keyboard-mouse-native --target chiaki chiaki-stream-input-native-tests --parallel 4
	./build/keyboard-mouse-native/gui/tests/chiaki-stream-input-native-tests.exe keyboardCapture mouseTriggers -o .scratch/keyboard-mouse-controls/evidence/final-native-output.log,txt
```

PowerShell returned before the GUI-subsystem test process finished.
The completed Qt log, not the immediate shell return, supplies the test result.
The test uses isolated test profiles and intercepts controller feedback.
No normal user-profile app launch or auto-connect ran for this check.

One focused QML check closes the remaining binding layout gate. It uses the
existing SettingsHarness and three representative rows at 900 by 600.
Long action and input labels keep the action, primary, and alternative columns
within bounds. Tab navigation and search keep the focused slot in view.
Search selects the Bindings tab without opening assignment. The target has
active and visible keyboard focus. The completed log reports 3 passed, 0 failed,
and 0 skipped, including setup and cleanup.

The check is `gui/tests/qml/tst_KeyboardMouseBindingsLayout.qml` on the acceptance
branch based on the recorded integrated revision. Its source imports the
production shared page and binding pane. The saved image was inspected.
Action labels wrap. Long input labels use the existing button's ellipsis.

- [Binding layout and focus log, native Windows with OpenGL](C:/Users/david/Projects/chiaki-ng-ui/.scratch/keyboard-mouse-controls/evidence/bindings-minimum-native-opengl.log).
- [Bindings at 900 by 600](C:/Users/david/Projects/chiaki-ng-ui/.scratch/keyboard-mouse-controls/evidence/keyboard-mouse-bindings-minimum.png).

The native Windows image shows the blue focus background at the Forward slot.
Windows scales the logical 900 by 600 page to 1125 by 750 image pixels at 125%.
The offscreen software-renderer image misplaced Material button backgrounds at
the upper-left corner, although item coordinates and focus properties were correct.
That image is a test-environment limit, not accepted visual evidence.
The native Windows OpenGL run resolves the visual check. Its completed log
includes the temporary coordinate probe. The probe was then removed from the test.
The earlier offscreen geometry log remains in `bindings-minimum-layout.log`.
The artifact is retained in `bindings-offscreen-software-artifact.png`.

## Final review and affected checks

Both reports reviewed `992cc170`. Production fix `aaedee02` addresses all three
code items. `ed4d56d7` adds the diagnostic evidence record without production changes.
The full [fix record](review-fixes.md) explains the changes and exact cases.
No broad suite ran again.

### Standards review

The [standards report](C:/Users/david/Projects/chiaki-ng-ui/.scratch/keyboard-mouse-controls/evidence/standards-review.md)
identified duplicated notice-focus rules and unnamed controller-action positions.
The ownership module now owns deferred notice focus and target handoff.
The QML focus timer is removed. Named action IDs replace array-position meanings
without changing persisted action values or order.

The notice handoff case passes. The four affected StreamNotices scenarios pass
all ten case rows across both presentation routes. Session and binding checks
pass after the action IDs change. Logs are `chiaki-review-notice-green.txt`,
`chiaki-review-notice-qml.txt`, and `chiaki-review-session-final.txt` in the primary evidence directory.

### Spec review

The [spec report](C:/Users/david/Projects/chiaki-ng-ui/.scratch/keyboard-mouse-controls/evidence/spec-review.md)
identified premature release when both physical Shift keys or both Ctrl keys are held.
The fix retains separate scan-code identities and captures native modifier edges
that Qt can omit. Capture startup quarantines each held side until release.
The final session checks pass both modifier rows, keyboard defaults, editable
bindings, and binding validation. The log reports 7 Qt cases passed, including setup and cleanup.

The first post-fix `platformKeyboard` run passed the added modifier and obsolete
message assertions, then failed the existing Space-output assertion.
One bounded diagnostic run of the complete case passed with exit code zero.
Capture, activation, Qt focus, and Windows foreground were true before and after
Space. Ownership was unblocked. Synchronous output changed from zero to Cross, value 1.
The earlier failure's cause remains unproved. Focus loss is not an established explanation.
Temporary diagnostics were removed. No production change or retry loop followed.

Both completed logs are preserved as `chiaki-review-native-modifiers-final.txt`
and `chiaki-review-space-diagnostic.txt`. The passing diagnostic does not erase
the earlier failure. Real desktop input still requires acceptance.

## Earlier checks retained without redundant reruns

These checks ran during their implementation stages. They are earlier-revision
evidence, not a claim that every suite ran again at the integrated revision.
The logs are preserved in the
[primary evidence directory](C:/Users/david/Projects/chiaki-ng-ui/.scratch/keyboard-mouse-controls/evidence).

| Check | Recorded result | Preserved evidence |
| --- | --- | --- |
| Ownership and session output, OpenGL and Vulkan | Passed | `ownership-native.log`, `keyboard-native.log` |
| Mouse conversion, idle centre, saturation, reset, and trigger alternatives | Passed | `mouse-motion.log`, `mouse-triggers.log` |
| Production Win32 registration, confinement, restore, resize, and controlled window routing | Passed with controlled foreground eligibility | `mouse-desktop.log` |
| Numeric response, stored-value validation, and 12 timed-trace comparisons | 5 Qt cases passed | `response-numeric.log` |
| Response and binding snapshots on recapture | 5 Qt cases passed | `response-snapshots.log` |
| Actual Main live settings route, both presentation routes, owner release, focus return, and no recapture | Passed in OpenGL and Vulkan | `response-route-opengl.log`, `response-route-vulkan.log` |
| Binding defaults, persistence, validation, conflict handling, reserved keys, and assignment focus | Passed | `bindings-defaults.log`, `bindings-persistence.log`, `bindings-validation.log`, `bindings-ui.log` |
| Reset through production settings, window, and session output | Passed | `reset-native-green.log` |
| Reset confirmation, cancellation, replacement, and search | 9 Qt cases passed | `reset-KeyboardMouseReset.log` |
| Final reset-row layout, numeric entry, invalid edit recovery, tab search, and saved message at 900 by 600 | Passed, saved image inspected | `reset-MouseResponseSettings.log`, `mouse-response-minimum.png` |
| Existing keyboard settings and legacy long labels | 43 Qt cases passed | `settings-legacy-keys.log` |
| Settings search | 27 Qt cases passed | `settings-search.log` |
| Stream interface and toolbar layout | 39 and 20 Qt cases passed | `stream-interface.log`, `stream-toolbar-layout.log` |

Qt totals include setup and cleanup. The final reset search also passed separately
in `reset-search-final.log`. The reset-row image replaces the earlier response image.
The [mouse response evidence](mouse-response-evidence.md) records the source
revisions, provisional numeric values, trace inputs, and comparison limits.

The saved [900 by 600 settings image](C:/Users/david/Projects/chiaki-ng-ui/.scratch/keyboard-mouse-controls/evidence/mouse-response-minimum.png)
shows the mode and reset row, both tabs, and the response controls within the scroll area.
The separate binding image and focused check above cover the new pane's long labels.
Existing long-label tests cover legacy controls.

## Open acceptance gates

| Gate | Result or limit |
| --- | --- |
| Real Windows foreground input | Not accepted. `mouse-injected.log` failed its foreground-window precondition before injection. No Raw Input delivery result exists from that attempt |
| Physical mouse capture | Not run. Screen edges, display arrangement and scaling, hardware removal, fullscreen, and resize need physical input checks |
| Intermittent desktop check | One post-fix Space assertion failed. One bounded diagnostic passed. The initial failure has no established cause |
| Held physical inputs through transitions | Not run. F8, Alt+Tab, menus, dialogs, quit, disconnect, reconnect, and session replacement still need real-input acceptance |
| Legacy hardware behaviour | Stored legacy preferences and controlled paths passed. Physical-controller and mouse-touchpad use remain unverified |
| Mouse calibration | Incomplete. Defaults and bounds are provisional. Synthetic timing checks do not establish usable game aiming |
| Astro's Playroom | Not run. Basic movement, jumping, camera, buttons, and menu checks remain open |
| The First Descendant | Not run. Fine corrections, tracking, fast turns, aiming while moving, and firing while moving remain open |
| Final review | Both reviews complete. Three code items addressed by `aaedee02`; affected check results are recorded above |

Before gameplay trials, stop the external mapper's capture without changing its
files or installation. Record mouse DPI, report rate where available, game
controller settings, source revision, build, and each scenario result.
Do not change comparison settings without recording the change.
No full-game compatibility or native-PC mouse-feel result is established.

## Use the local build

1. Start [Launch-chiaki.cmd](C:/Users/david/Projects/chiaki-ng-ui/build/keyboard-mouse-native/Launch-chiaki.cmd).
2. Open **Settings > Keyboard and mouse**.
3. Enable **Enable mouse camera mode**.
4. Connect to the console and press **F8** to capture controls.
5. Press **F8** again to release controls.

The stream menu opens the same Keyboard and mouse settings during a session.
Saved edits apply on the next capture. Capture does not resume when settings close.
The launcher sets the installed runtime and plugin paths before it starts the app.
Its paths were checked. The normal app was not launched for acceptance.

The remaining use limits are provisional mouse defaults, unverified physical
input and game acceptance, and the unexplained intermittent desktop-check failure.
Tickets 01, 02, 03, 05, and 06 remain open for their acceptance gates.
Ticket 04 has complete software checks and is resolved.
