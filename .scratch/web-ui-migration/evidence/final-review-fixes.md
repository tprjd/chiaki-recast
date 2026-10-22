# Final review fixes

Date: 2026-10-22.
Base: `77ff3338e64b51213f13efafec9bd44cb7b9c78d`.

## Changes

- The library uses the shared focus navigator for visible controls. Card focus
  and card activation still select the console. Connect remains an explicit action.
- The navigator handles shoulder keys in setup panels and dialogs. Text fields
  and select controls keep their arrow keys. Settings keeps its search shortcuts.
- Return activates a focused checkbox through its existing click and change
  handlers. Browser key repeats do not activate it again. Native controller key
  mapping and repeat behavior are unchanged.
- Dialog navigation runs before window handlers. `defaultPrevented` prevents a
  second move. The existing Steam text-field navigation remains available.
- The Add console button can reopen pending registration progress. Other busy
  connection operations still block a new registration.
- Console details shows only actions supported by its management action map.
  Automatic registration remains available from the existing registration routes.
- Required widget profile hints and the request interceptor are available without
  WebEngineQuick. Optional Quick methods retain their guards.

## Bounded checks

The first DOM assertion failed on the original card-only navigation handler.
After the fix, one bounded browser script passed at a 1024 CSS pixel viewport.
It checked explicit Connect activation, one move per shoulder press, console
details labels, pairing controls, native field editing keys, progress reopening,
checkbox change dispatch, browser repeat suppression, and the restored account
route. The fixture used sample data and a local bridge adapter.

The root agent also used CUA on the actual browser preview. Down moved from a
selected card to its Connect button. Right moved to that card's details button.
Return opened details. PageDown moved once from Close to Save console login PIN.
Selection and focus did not start a connection.

`npm run check` reported 0 errors and 0 warnings after the final source change.
The static sample frontend compiled with Vite. `git diff --check` passed.

MSVC preprocessing checked the header and source with WebEngineQuick both absent
and present. Include directives were replaced by markers. Widget declarations,
definitions, the request interceptor, and client hints remained available in both
cases. Quick declarations and definitions appeared only with their macro enabled.
No native source was compiled or linked.

Local check artifacts are in
`C:/Users/david/.codex/tmp/final-review-fixes-preview/`:
`check-review.mjs`, `check-widget-guards.py`, and the compiled `static/` preview.
These sample adapters and assets are outside production entry points and RCC.

## Limits

Browser keys are controller-equivalent input checks. They are not physical
controller, native playback, or release acceptance tests. Native and platform
builds, package checks, hardware checks, performance checks, and HighQuality
qualification remain skipped under the user's waiver. The known HighQuality
failure remains unverified. No saved settings or real console data were changed.
