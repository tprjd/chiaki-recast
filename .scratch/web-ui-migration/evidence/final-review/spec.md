# Final SPEC review

Reviewed `d0e2f052cb076b7fda85c51f60dab1cc428951fb...77ff3338e64b51213f13efafec9bd44cb7b9c78d`, read-only.

## Missing or partial spec behavior

- **P1: Controller users cannot reach console actions.** `gui/web/src/App.svelte:98-105` confines D-pad and R1 navigation to card selectors. At line 200, card activation only selects. Cross sends Return (`gui/src/qmlcontroller.cpp:18`), so it cannot activate the separate Connect/Register/details buttons. The previous `ConsoleLibrary.qml:189` connected the selected console. Pairing and account panels also bypass this navigator without a replacement. This violates “Preserve keyboard, controller, and desktop touch operation” and user story 63, “the existing setup navigation and actions.” Keep intentional card selection; extend focus navigation to visible per-card actions and setup controls. Reuse `moveFocus` where suitable.

## Unasked behavior

No independent unasked behavior confirmed.

## Implemented but incorrect

- **P2: Registration progress cannot reopen while busy.** `gui/web/src/App.svelte:177` disables “View registration progress” when `connectionBusy` is true. `gui/src/setupbridge.cpp:799` includes `registrationBusy` in that value. Start manual registration, close its screen or reload, then try to reopen progress: the button stays disabled until completion. This violates “Reload or restore a page during a pending operation without losing state” and user story 25, “registration progress and results from the native backend.” Permit reopening the current registration while retaining the block on starting another operation. The existing scenario at `gui/tests/embeddedsetup.cpp:3807` expects this route.

- **P2: Console details exposes a blank automatic-registration action.** `gui/src/setupbridge.cpp:280` publishes `autoRegister` for an unregistered discovered console with a DUID. `gui/web/src/ConsoleManagement.svelte:131` renders every action, but its label map at line 18 and `manageConsole` have no such action. Opening details exposes a blank button followed by a blank confirmation that fails. This violates “Preserve all existing setup ... account flows” and the requirement for labels. Filter this action from management controls or route it to the existing automatic-registration command with a label.

## Evidence limits

Source inspection only; no tests, builds, native GUI, package, hardware, or performance checks run. The user's waived checks remain unverified. No new native media or lifetime defect was confirmed in the inspected paths. This bounded review is not full platform qualification.
