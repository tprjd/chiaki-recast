# Final review fixes

Both final reports reviewed integration commit `992cc170e834c6f92132cc72487a071ec7dd4a0d`.
The reports are `C:/Users/david/AppData/Local/Temp/chiaki-kbm-standards-review.md`
and `C:/Users/david/AppData/Local/Temp/chiaki-kbm-spec-review.md`.

`StreamInputOwnership` now receives the notice action target and owns deferred
activation. A repeated notice presentation can move its target between hosts
without replacing its identity or saved return focus. The QML focus timer is
removed. The native generation and session epoch checks remain in place.

Shift and Ctrl retain separate scan-code identities. Capture startup quarantines
each held side until its release. The Windows adapter collects modifier edges
because Qt can omit the second same-modifier press. Duplicate native and Qt
events are idempotent. The adapter uses the capture, foreground, and time guards.
[Qt's Windows mapper](https://github.com/qt/qtbase/blob/6.11/src/plugins/platforms/windows/qwindowskeymapper.cpp)
uses `0xE000` for extended scan codes, matching `MAPVK_VK_TO_VSC_EX` for right Ctrl.

Named action IDs replace unnamed positions in defaults, mapped output, labels,
and group boundaries. Size checks derive from the binding arrays. Persisted
action values and their order remain unchanged.

## Focused checks

The existing GNU UCRT64 build was reused. No submodule was initialized.
Only the affected targets and cases ran. All logs below are in
`C:/Users/david/AppData/Local/Temp/`.

| Check | Result | Exact log |
| --- | --- | --- |
| `noticeTargetHandoff`, before the fix | Failed at the second host focus | `chiaki-review-notice-red.txt` |
| `noticeTargetHandoff`, after the fix | Passed | `chiaki-review-notice-green.txt` |
| Windows `StreamNotices` blocked clearance, disabled-video priority, initial media, and renderer handoff | All 10 case rows passed | `chiaki-review-notice-qml.txt` |
| `physicalModifiers`, before the fix | Both Shift and Ctrl rows failed after the first release | `chiaki-review-modifiers-red.txt` |
| Final `physicalModifiers`, `keyboardCapture`, `editableBindings`, and `bindingValidation` | All five case rows passed | `chiaki-review-session-final.txt` |
| Final `platformKeyboard` | Added native-only second-modifier and obsolete-message assertions passed. The later existing Space-output assertion failed at line 226. | `chiaki-review-native-modifiers-final.txt` |
| Final native target build | Passed | `chiaki-review-build-final.txt` |
| One bounded `platformKeyboard` diagnostic run | Complete case passed with synchronous Space output | `chiaki-review-space-diagnostic.txt` |

The owner check used the offscreen software backend. Session checks used the
offscreen platform. The notice checks and `platformKeyboard` used Windows.
The native executable ran through `Start-Process -WindowStyle Hidden -Wait`.

The bounded follow-up added temporary diagnostics around the existing Space
press and ran only `platformKeyboard` once. Before the press, capture, active
window, Qt focus, and Windows foreground were all true. Input ownership was
unblocked and the button state was zero. Immediately after the synchronous
press, the same states remained true and the button state was Cross, value 1.
The complete case passed with exit code zero. The prior failure's cause was
not established. The temporary diagnostics were removed. No production change
or retry loop was needed.

No other suite repeated. Physical mouse calibration, PS5 output, and game
acceptance remain open. These fixes do not establish those results.
