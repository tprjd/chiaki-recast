# 07: Complete keyboard and controller navigation

**What to build:** Predictable navigation across the complete card screen, search, dialogs, and management actions with keyboard and controller input.

**Blocked by:** 04: Find consoles without losing the action target; 06: Manage consoles from their cards.

**Status:** resolved

**Spec stories:** 16, 17, 18, 21.

- [x] Tab and Shift+Tab follow visual order through sidebar, search, toolbar, cards, and discovery. Every interactive control has an accessible name and visible focus.
- [x] Arrow keys move through cards only when the grid has focus. Text fields retain editing keys; Enter in a form or menu cannot also connect a console.
- [x] Escape closes the active popup first, clears a nonempty focused search next, and otherwise retains the main-screen quit confirmation.
- [x] Preserve existing controller shortcuts and selected-console hints with the same eligibility as the matching buttons.
- [x] Restore focus after Settings, PIN entry, confirmation, connection errors, and return from a stream. Use a valid nearby control if the original card disappeared.
- [x] Screen tests cover input dispatch, focus restoration, search/dialog interactions, and empty or all-hidden lists without navigation loops.
- [x] Exercise the complete screen with mouse and keyboard and an available controller. Record any unavailable controller checks explicitly.

**Scope limit:** Earlier tickets must already provide usable focus. This ticket completes cross-feature input behavior, not a new input framework.

**Specification:** [Main console screen: card library](../spec.md).

**Design reference:** B after text cleanup, preserved at `codex/prototype-console-b`, commit `73051300680f89e5c0f6dc3db71c7b581b8176e0`.

## Comments

Verify focus on the rendered controls, not only the `visualFocus` property.
Primary buttons already use an accent background while unfocused. Confirm
that focused controls have a distinct visible indicator. Check the selected
console's controller hints and their eligibility in the completed details
flow. Native dialog return checks are available in
`../native-previews/03-native-checks.json`; physical controller input remains
untested.

## Answer

Merged as 70f22b383ca4639164701af34b00529ab671be60. Grid arrows stay in the grid, text fields keep editing keys, focus scrolls card actions into view, and keyboard/controller hints use current eligibility. Opener restoration has a valid fallback after removal. Details waits until closed before opening PIN or confirmation, which fixes the native Escape-to-Quit race. The production-screen regression fails with early dispatch and passes with deferred dispatch. Rendered focus checks cover the screen and popup controls. The integration checkout passes CTest in 64.22 seconds. Full-screen captures at 1280 by 720 and 800 by 600 were inspected. Full native replay is being recorded separately. Physical controller and real stream-return checks remain untested.