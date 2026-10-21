# 04: Edit keyboard and mouse bindings

**What to build:** Assign or clear primary and alternative inputs for each supported controller action. Save changes automatically and use them in the next gameplay capture, with clear conflict handling and full keyboard access.

**Blocked by:** [02: Play with mouse camera control and trigger buttons](02-mouse-camera-and-triggers.md).

Status: ready-for-agent

Parent: [Built-in keyboard and mouse controls](../spec.md)

This ticket implements the binding editor portion of Stage 3. It can proceed independently of ticket 03 using the settings contract from ticket 01.

- [ ] Provide the Bindings tab with Movement, Buttons, Shoulders and sticks, D-pad, and System groups. Show Primary and Alternative slots for all 22 editable actions.
- [ ] Initialize the accepted default bindings. Keep the right stick assigned to mouse movement and outside the editable action list.
- [ ] Permit one key or one of the five mouse buttons per slot. Allow either slot to be cleared. Do not add wheel bindings, chords, sequences, or macros.
- [ ] Enforce one action per physical input and prevent duplicate copies within an action. For conflicts, identify the current action and offer Move binding or Cancel.
- [ ] Move the old and new assignments together. Cancellation leaves both unchanged. Validate loaded assignments as well as edits so invalid saved data cannot bypass these rules.
- [ ] Reject F8 and F11 with an explanation of their reserved purpose. Escape cancels assignment. Provide a keyboard-accessible way to assign Tab without trapping normal focus navigation.
- [ ] Return focus to the invoking slot after assignment or cancellation. Accessible names identify the action, slot, and assigned input or unassigned state.
- [ ] Save valid edits automatically and show the specified save message. Apply them on recapture without restarting the stream or replaying old held input.
- [ ] Keep the mode selector available, and display new-mode bindings disabled when the mode is off. Preserve access to the existing legacy assignment editor. Ticket 05 adds reset.
- [ ] Verify edited bindings through actual session controller output. Cover all supported mouse buttons, simultaneous alternatives, release of one alternative, conflicts, cancellation, clearing, reserved keys, and persistence.
- [ ] Check both tabs coexist using established settings components. Verify settings search, keyboard navigation, long labels, and the existing minimum settings window size.

