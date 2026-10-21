# Keyboard and mouse settings and implementation stages

Planning state: Approved by the user on 2026-10-21 and consolidated into [the specification](spec.md). No implementation tickets have been published.
Updated: 2026-10-21.

The accepted behaviour and default bindings are in [the planning notes](planning.md).
This approved design defines the screen, implementation order, and proof required at each stage. The specification is authoritative for implementation.

## Settings screen

Keep the current category sidebar. Rename Keys to Keyboard and mouse rather than add a second overlapping category.
Reuse the existing settings controls, search integration, focus handling, and responsive layout.

At the top, show Enable mouse camera mode, off by default.
The helper text says: "During a stream, press F8 to capture or release your controls."
Changing this preference does not capture the mouse.

Use two local tabs:

| Tab | Controls |
| --- | --- |
| Mouse response | Horizontal sensitivity, vertical sensitivity, game dead-zone compensation, invert vertical camera movement |
| Bindings | Movement, face buttons, shoulders and stick clicks, D-pad, and system buttons |

Keep the mode selector and Reset defaults visible on either tab.
Save valid edits automatically. Show "Saved · Applies the next time you press F8."
Reject invalid numeric edits and retain the last valid setting. Loaded settings must receive the same validation.

When the new mode is off, retain access to the existing keyboard-as-controller, mouse-touchpad, and legacy key-assignment controls.
Their saved values remain separate. New-mode controls remain visible but disabled so the user can inspect the configuration.
The visual proposal shows representative legacy switches only; it does not reproduce the existing legacy binding editor.

## Mouse response

Sensitivity uses an independent multiplier for each axis. It is relative to the mapper's calibrated base response, not degrees of camera rotation.
The draft shows 1.00 times sensitivity for both axes and zero dead-zone compensation.
The draft ranges of 0.1 to 5 times sensitivity and 0 to 30 percent compensation are illustrative, not approved or calibrated limits.
Stage 2 determines useful validated ranges and records the evidence.

Game dead-zone compensation helps small mouse movements cross the game's right-stick dead zone.
It must never create a nonzero stick value while the mouse is idle.
This control is not a promise to remove the game's response curve or maximum turn speed.

Do not add acceleration presets, smoothing controls, game detection, or automatic per-game profiles in this version.
Use the existing application settings/profile conventions; do not add another profile manager.

## Binding editor

Use one row per controller action, with Primary and Alternative input slots.
Two slots per action are the accepted limit and cover the keyboard-plus-mouse trigger bindings.
Empty slots show Add. Each slot can be changed or cleared.
The right stick remains assigned to mouse movement in this mode.

The editor captures a single key or mouse button, not a sequence or a modifier chord.
Ctrl and Shift remain independent game inputs. Operating-system shortcuts remain available.
F8 and F11 are reserved. Escape cancels input assignment. The native implementation must keep a keyboard-accessible way to assign Tab without trapping focus.

If an input is already assigned, show its current action and offer Move binding or Cancel.
Moving it changes the old and new assignments together. Cancelling changes neither.
The accessible name includes the controller action, slot, and current binding.

The inline visual demonstrates key assignment, mouse-button choices, conflicts, clearing, and reset confirmation.
The production editor should use the established native key-assignment dialog and focus-return pattern where practical.
The preview's click choices are a way to inspect mouse bindings without capturing the user's actual side buttons.

## Reset and capture status

Reset defaults asks before replacing this feature's settings. The accepted reset restores the new mode to off, its accepted bindings, and calibrated mouse defaults.
It leaves the legacy keyboard settings, physical-controller settings, and console configuration unchanged.

During a stream with the mode enabled, place the status at the lower edge with the existing stream notices:

- Released: "Keyboard and mouse ready · F8 to capture."
- Captured: "Mouse captured · F8 to release."

Keep it passive: it does not take focus or become a stream input owner.
Place it so it does not cover the existing stream menu controls. Native layout checks must cover both stream presentation routes.

## Stage 1: Play with the accepted controls and release them safely

Prerequisite: Check the current input-ownership implementation against ADR 0001. The design is approved.

Deliver a complete, usable path from the opt-in setting through relative mouse capture and fixed accepted bindings to the existing Remote Play controller state.
Include the capture status and keyboard shortcuts in this stage.

Keep mouse conversion separate from platform input collection so recorded movement and elapsed time can exercise the production conversion.
Use a small mapper with explicit input, elapsed time, and reset operations. Avoid a general input-plugin framework.
Windows capture must provide relative movement without cursor-edge limits and must not depend on render timing.

Include all safety behaviour in this first stage: F8 release, focus loss, menus, disconnect, reconnect, held-key suppression, neutral output, and exclusive input selection.
Respect ADR 0001 and existing owner lifecycles. Inspect their current implementation before adding a separate capture controller.
Suppress legacy keyboard/touchpad mapping in the new mode and prevent controller or mouse-touch input from leaking into a captured session.
Disable stream double-click fullscreen behaviour during capture so two trigger clicks cannot change the window mode. F11 remains available.
Use neutral results for opposite movement directions held together. Normalize diagonal movement so WASD does not exceed the stick's intended magnitude.
Trigger bindings initially represent released or fully pressed values. Partial-trigger behaviour is outside this version.

Evidence required:

- Production mapper checks for movement, alternatives, opposite directions, diagonal limits, reset, and bounded stick values.
- Native checks of emitted controller state through the real session boundary, including input-owner overlap and session replacement.
- Real Windows checks of capture, release, side-button suppression, fullscreen transitions, and multiple-display cursor behaviour where available.
- A basic Astro's Playroom session proves movement, jumping, and camera control with the external mapper stopped.

Do not advance on a build result alone. A capture failure or stuck input blocks later stages.

## Stage 2: Tune the mouse during a running stream

Blocked by: Stage 1.

Deliver the Mouse response tab with automatic persistence, validation, and application on the next capture.
Support independent sensitivity, vertical inversion, and game dead-zone compensation.

Determine the base gain, supported sensitivity ranges, dead-zone bounds, and idle-return interval using recorded input and real mouse tests.
Publish these values and the test conditions before completing this stage.
Keep configuration separate from temporary held input. Neither old mouse deltas nor old pressed keys survive a settings change and recapture.

Evidence required:

- Equal timed movement traces produce consistent output across event batching and rendering rates.
- Idle movement yields a centred stick; inversion, saturation, and dead-zone transitions behave correctly.
- Settings survive restart, apply without stream restart, and leave existing input settings unchanged.
- The First Descendant tests cover fine corrections, target tracking, and fast turns. Record mouse DPI, mouse report rate where available, and in-game controller settings.

Do not promise native-PC mouse feel. Document any limit imposed by game controller processing.

## Stage 3: Edit and reset bindings

Blocked by: Stage 1. Can proceed independently of Stage 2 after the settings contract is fixed.

Deliver the Bindings tab, two input slots per action, five supported mouse buttons, duplicate-assignment handling, and a scoped reset.
Every valid edit saves and becomes active on the next capture.
Preserve all legacy settings and do not import or rewrite the external emulator's files.

Evidence required:

- Bind a key and mouse button, restart Recast, and use the saved assignments in the stream.
- Move an occupied binding and verify that only its destination uses it. Cancel a conflict and verify that both original assignments remain unchanged.
- Hold both R2 alternatives and release one; R2 remains held until both are released.
- Reject reserved keys, cancel assignments, and return keyboard focus to the invoking control.
- Reset restores only the new mode's defaults, including turning that mode off.

## Stage 4: Complete native acceptance and release review

Blocked by: Stages 2 and 3.

Verify the complete feature in the real Windows app and both selected games. Review the diff against the accepted scope and repository standards.

Cover menu overlap, reconnect, quit, Alt+Tab, mouse disconnection where practical, fullscreen, display scaling, and both supported stream render paths.
Confirm that existing physical controllers and legacy keyboard mode still work when the new mode is off.
Confirm accessible control names, keyboard navigation, numeric editing, search results, and the existing settings minimum window size.

Publish an acceptance record with the source revision, build, environment, actual game settings, completed checks, and unresolved limits.
Astro acceptance covers basic controls only. Do not mark its full campaign playable from this evidence.
Native capture failures, stuck output, lost legacy settings, or unusable aiming block release.

## Visual-review evidence

The in-conversation proposal is stored in the task visualization directory as keyboard-mouse-settings.html.
The mode is shown enabled for inspection, although its proposed installation default is off.
Interactions change only the preview's memory and do not configure Recast.

Browser checks exercised the Mouse response and Bindings tabs, a conflicting Q assignment, Move binding, and reset confirmation.
The preview rendered in the available desktop browser view. Native Qt layout and gameplay checks remain outstanding.

## Review outcome

The user accepted the two-tab layout, two slots per action, reset turning the mode off, and the four stages with their acceptance gates.
The user requested publication of the specification. Implementation has not started.
