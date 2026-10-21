# Registration Account ID verification

Base: `d0ea2f19`. Branch: `codex/psn-registration`.
Standalone Qt 6.9.3 and MSVC 2022 screen target. The shared native cache was not changed.

The tests load production `RegistDialog.qml`, its default `StackView` route,
`PSNLoginDialog.qml`, and `PSNTokenDialog.qml`. They supply only the public lookup
service and authentication backend. The production controls and response parser run.

## Results

- [Final Account ID checks](account-final.txt) cover both routes, errors, retry,
  a real 30-second timeout, late and repeated results, profile changes, destroyed
  callers, a pending request in a replacement registration, and focus during replacement.
- [PSN setup](regression-PsnSetup.txt): 10 passed.
- [Registration form](regression-RegistrationForm.txt): 22 passed.
- [Registration results](regression-RegistrationResults.txt): 19 passed.
- [Settings remote](regression-SettingsRemote.txt): 31 passed.
- [Capture checks](capture-results.txt): 4 passed. All six PNG files were inspected.

Red results precede each implementation slice: [public explanation](public-red.txt),
[lookup recovery](lookup-red.txt), [sign-in return](sign-in-red.txt),
[profile and timeout](ownership-timeout-red.txt), [long text](layout-red.txt),
[replacement focus](replaced-sign-in-red.txt), and [profile-change closure](profile-close-red.txt).
`layout-red.txt` omits repeated Windows scrollbar warnings.
The corresponding green files and final results retain the successful checks.

The sign-in return test exposed a binding-order error in the shared completion
handler. It now reads the supplied attempt ID and phase directly. Registration
can observe an existing request without taking ownership of its completion.

## Captures

| Screen | 900 by 600 | 1280 by 720 |
| --- | --- | --- |
| Public lookup | [Capture](lookup-900x600.png) | [Capture](lookup-1280x720.png) |
| Lookup recovery | [Capture](lookup-error-900x600.png) | [Capture](lookup-error-1280x720.png) |
| Registration sign-in | [Capture](sign-in-900x600.png) | [Capture](sign-in-1280x720.png) |

The public form explains the service, public-search requirement, and absence of
credential saves. Focus and retry remain visible. Sign-in retains the credential
storage explanation and a masked redirect field. Long-text tests check scrolling
and reachable actions after the layout updates.

## Reproduce

Use the standalone build and environment commands in `gui/tests/README.md`.
Run `chiaki-console-screen-tests.exe -input gui/tests/qml/tst_RegistrationAccount.qml`.
Run each `tst_PsnSetup.qml`, `tst_RegistrationForm.qml`,
`tst_RegistrationResults.qml`, and `tst_SettingsRemote.qml` with the same `-input` option.
Run `-input .scratch/psn-dialog-refresh/evidence/ticket02/capture.qml` to save the captures.

Production `gui/src/qml/qml.qrc` includes all three changed screens and excludes
the fixtures and evidence. These checks use supplied service outcomes. Native
persistence remains covered by ticket 01. Ticket 08 owns the final native build,
visible native focus, and display scaling. Real PSN sign-in, live public lookup,
and physical controllers were not tested.
