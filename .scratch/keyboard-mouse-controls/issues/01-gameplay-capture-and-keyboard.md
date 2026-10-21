# 01: Enable gameplay capture with default keyboard controls

**What to build:** Enable the new mode in Settings, then use F8 to direct the default keyboard controls to an active Remote Play session. Show where input goes. Release gameplay input safely when the user returns to Recast or Windows.

**Blocked by:** [Stream input ownership implementation](../../stream-input-ownership/spec.md), including the shared owner lifecycle and maintained native tests required by ADR 0001. Verify that this prerequisite has landed before starting integration.

Status: claimed

Parent: [Built-in keyboard and mouse controls](../spec.md)

This ticket is the first part of Stage 1. Ticket 02 completes the mouse path. Use the parent specification for the complete binding table and mode contract.

- [x] Rename the existing Keys category to Keyboard and mouse. Add Enable mouse camera mode with the specified F8 helper text. Save the preference separately from legacy settings and default it to off.
- [x] Establish the shared feature settings contract using the existing settings and profile conventions. Preserve legacy values and their controls. Do not introduce a per-game profile manager.
- [x] Permit capture only on a fresh F8 press with an active, usable, focused stream and no blocking stream input owner. Ignore autorepeat. Enabling the setting does not start capture.
- [x] Send every default keyboard action through the production Remote Play controller-output path. Opposite movement keys cancel, diagonals stay within the stick limit, and triggers use released or full values.
- [x] Suppress legacy keyboard and mouse-touchpad mapping whenever the new mode is selected. During capture, suppress every other controller contribution, including touch and motion. After release, restore normal controller selection subject to input owners.
- [x] Release capture on F8, menu or dialog ownership, focus loss, disconnect, session replacement, stream exit, and app exit. Clear held mapped input and pending movement. Send neutral mapped state when the connection is available.
- [x] Start new and reconnected sessions released. Do not recapture on menu close or focus return. Held inputs must be released before a fresh press can activate them. Reject obsolete events and callbacks.
- [x] Preserve the shared ownership rules for overlap, presentation identity, destruction, focus return, and session handoff. Do not add another ownership counter or independent cleanup policy.
- [x] During capture, treat Ctrl and Shift as game inputs and suppress Recast's Ctrl shortcuts. Preserve operating-system shortcuts and F11. Restore normal app shortcuts after release.
- [x] Show the specified passive ready and captured notices in both stream presentations. Hide the capture-ready prompt while an owner blocks capture. Keep notices clear of menu controls.
- [x] Make the setting and notices accessible and searchable where applicable. Keep other supported platform builds working without exposing an unusable Windows-only mode.
- [x] Test production input and session objects at the controller-output boundary. Cover all keyboard defaults, transitions with held buttons, owner overlap, stale callbacks, reconnect, exclusive input, shortcuts, and settings persistence.
- [ ] Verify F8, menus, Alt+Tab, and session transitions with real Windows keyboard input. Record the build and results. Capture failures or stuck input prevent completion.

## Answer

Implemented on 2026-10-21 after merging ownership commit `bc918cf7`.
The ticket remains claimed because physical-input and live-console acceptance are open.
Ownership implementation is present. Its separate hardware acceptance is also still open.

The feature uses `settings/keyboard_mouse/enabled` in the existing application profile.
Missing or invalid mode values select off. Legacy preferences are preserved.
`QmlSettings.mouseCameraEnabled` and `mouseCameraSupported` expose the mode.
`QmlMainWindow.gameplayCaptured` and `gameplayCaptureAvailable` expose passive status.

The production mapper tracks held keys and preheld-key quarantine.
Both session feedback paths select only mapped input during capture.
The window uses the existing ownership module, rejects obsolete timestamped events,
and consumes Qt ShortcutOverride before app shortcuts can use captured Ctrl combinations.
No mouse collection or mouse tuning is included in this ticket. Ticket 02 adds that path.

Validation used a fresh Release build under `build/keyboard-mouse-native`, with
UCRT64 GCC 16.2, Qt 6.11.2, and production GUI/session objects.
No historical source rewriting or old build-cache source tree was used.

- `chiaki.exe` builds successfully.
- Four selected CTest entries pass: ownership desktop, ownership Vulkan,
  gameplay keyboard output, and gameplay keyboard desktop.
- The session output test covers all keyboard defaults, direction cancellation,
  diagonal limits, full triggers, reset, held-key quarantine, and exclusive touch/motion output.
- The platform input test covers F8 repeat, released-mode suppression, Ctrl+Q/O,
  overlapping owners, stale events, session replacement, and obsolete session notifications.
- SettingsKeys passed 43 checks. The changed switch/navigation checks passed again after the final focus edit.
- StreamNotices passed 48 checks, with one existing environment skip.
- SettingsSearch passed its other 25 checks. Its two category-inventory rows initially
  used the old Keys label. Both pass after updating the expected category label.
- An actual Win32 SendInput run passed F8, autorepeat, Space, Ctrl+Q/O, menu release,
  and disconnect release. Final reruns could not obtain Windows foreground focus.
  The final expanded injected-key case is therefore not accepted as complete.

Remaining evidence: physical F8/F11, menus, Alt+Tab, real session transitions,
live PS5 controls, and checks on other supported platforms.
Native mouse capture remains ticket 02 work. The full Stage 1 result is not complete.

