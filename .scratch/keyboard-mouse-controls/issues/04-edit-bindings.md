# 04: Edit keyboard and mouse bindings

**What to build:** Assign or clear primary and alternative inputs for each supported controller action. Save changes automatically and use them in the next gameplay capture, with clear conflict handling and full keyboard access.

**Blocked by:** [02: Play with mouse camera control and trigger buttons](02-mouse-camera-and-triggers.md).

Status: ready-for-agent

Parent: [Built-in keyboard and mouse controls](../spec.md)

This ticket implements the binding editor portion of Stage 3. It can proceed independently of ticket 03 using the settings contract from ticket 01.

- [ ] Provide the Bindings tab with Movement, Buttons, Shoulders and sticks, D-pad, and System groups. Show Primary and Alternative slots for all 22 editable actions.
- [x] Initialize the accepted default bindings. Keep the right stick assigned to mouse movement and outside the editable action list.
- [x] Permit one key or one of the five mouse buttons per slot. Allow either slot to be cleared. Do not add wheel bindings, chords, sequences, or macros.
- [x] Enforce one action per physical input and prevent duplicate copies within an action. For conflicts, identify the current action and offer Move binding or Cancel.
- [x] Move the old and new assignments together. Cancellation leaves both unchanged. Validate loaded assignments as well as edits so invalid saved data cannot bypass these rules.
- [x] Reject F8 and F11 with an explanation of their reserved purpose. Escape cancels assignment. Provide a keyboard-accessible way to assign Tab without trapping normal focus navigation.
- [x] Return focus to the invoking slot after assignment or cancellation. Accessible names identify the action, slot, and assigned input or unassigned state.
- [ ] Save valid edits automatically and show the specified save message. Apply them on recapture without restarting the stream or replaying old held input.
- [ ] Keep the mode selector available, and display new-mode bindings disabled when the mode is off. Preserve access to the existing legacy assignment editor. Ticket 05 adds reset.
- [x] Verify edited bindings through actual session controller output. Cover all supported mouse buttons, simultaneous alternatives, release of one alternative, conflicts, cancellation, clearing, reserved keys, and persistence.
- [ ] Check both tabs coexist using established settings components. Verify settings search, keyboard navigation, long labels, and the existing minimum settings window size.


## Implementation evidence, 2026-10-21

The binding pane, settings model, validation, and mapper integration are
implemented. Ticket 03 owns the shared settings page and will mount this pane.
Shared tab, search, saved-message, disabled-state, and minimum-size checks remain
open until that integration. Ticket 05 owns reset.

- `KeyboardMouseBindingsPane.qml` accepts a settings object and emits `saved()`.
  It has 22 action rows, two slots, an assignment dialog, conflict movement,
  clearing, reserved-key feedback, explicit Tab assignment, and focus return.
- The settings bridge saves valid changes in one feature settings value. It
  rejects duplicate, reserved, unsupported, fractional, and malformed stored
  inputs. Invalid storage falls back to feature defaults. Legacy input values
  remain separate.
- Capture start loads the binding snapshot and samples every assigned key for
  held-input quarantine. Windows normalization keeps shifted punctuation and
  Backtab on the same input identity across modifier changes.
- Alt and Windows keys cannot be assigned. They remain Windows shortcut keys.
  The dialog explains this limit. Ctrl and Shift remain independent game inputs.
- A fresh UCRT64 Release build compiled the production GUI library and native
  tests. `editableBindings` and `bindingValidation` passed offscreen. The writer
  is destroyed before the persistence reader starts. These tests inspect actual
  stream-session controller output, including all five mouse buttons, held
  alternatives, settings snapshots, and custom held-key quarantine.
- `bindingEditor` passed through the production main-window event route on
  Windows 11 and Qt 6.11.2 with OpenGL, an active session, and a blocking input
  owner. It checks F8/F11 feedback, unchanged
  fullscreen state during assignment, Escape, conflicts, movement, clearing,
  explicit Tab assignment, middle click, focus return, settings replacement,
  and rejection of a late Move callback after cancellation.
- Existing `keyboardCapture` and `mouseTriggers` passed after the mapper lookup
  changed. No unrelated suite ran.

No physical input, live PS5, Astro's Playroom, or The First Descendant acceptance
ran for this ticket. These results do not prove native foreground capture or
full-game compatibility. The shared-page integration remains a separate check.
