# 01: Enable gameplay capture with default keyboard controls

**What to build:** Enable the new mode in Settings, then use F8 to direct the default keyboard controls to an active Remote Play session. Show where input goes. Release gameplay input safely when the user returns to Recast or Windows.

**Blocked by:** [Stream input ownership implementation](../../stream-input-ownership/spec.md), including the shared owner lifecycle and maintained native tests required by ADR 0001. Verify that this prerequisite has landed before starting integration.

Status: ready-for-agent

Parent: [Built-in keyboard and mouse controls](../spec.md)

This ticket is the first part of Stage 1. Ticket 02 completes the mouse path. Use the parent specification for the complete binding table and mode contract.

- [ ] Rename the existing Keys category to Keyboard and mouse. Add Enable mouse camera mode with the specified F8 helper text. Save the preference separately from legacy settings and default it to off.
- [ ] Establish the shared feature settings contract using the existing settings and profile conventions. Preserve legacy values and their controls. Do not introduce a per-game profile manager.
- [ ] Permit capture only on a fresh F8 press with an active, usable, focused stream and no blocking stream input owner. Ignore autorepeat. Enabling the setting does not start capture.
- [ ] Send every default keyboard action through the production Remote Play controller-output path. Opposite movement keys cancel, diagonals stay within the stick limit, and triggers use released or full values.
- [ ] Suppress legacy keyboard and mouse-touchpad mapping whenever the new mode is selected. During capture, suppress every other controller contribution, including touch and motion. After release, restore normal controller selection subject to input owners.
- [ ] Release capture on F8, menu or dialog ownership, focus loss, disconnect, session replacement, stream exit, and app exit. Clear held mapped input and pending movement. Send neutral mapped state when the connection is available.
- [ ] Start new and reconnected sessions released. Do not recapture on menu close or focus return. Held inputs must be released before a fresh press can activate them. Reject obsolete events and callbacks.
- [ ] Preserve the shared ownership rules for overlap, presentation identity, destruction, focus return, and session handoff. Do not add another ownership counter or independent cleanup policy.
- [ ] During capture, treat Ctrl and Shift as game inputs and suppress Recast's Ctrl shortcuts. Preserve operating-system shortcuts and F11. Restore normal app shortcuts after release.
- [ ] Show the specified passive ready and captured notices in both stream presentations. Hide the capture-ready prompt while an owner blocks capture. Keep notices clear of menu controls.
- [ ] Make the setting and notices accessible and searchable where applicable. Keep other supported platform builds working without exposing an unusable Windows-only mode.
- [ ] Test production input and session objects at the controller-output boundary. Cover all keyboard defaults, transitions with held buttons, owner overlap, stale callbacks, reconnect, exclusive input, shortcuts, and settings persistence.
- [ ] Verify F8, menus, Alt+Tab, and session transitions with real Windows keyboard input. Record the build and results. Capture failures or stuck input prevent completion.

