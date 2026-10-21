# Built-in keyboard and mouse controls

Status: ready-for-agent
Created: 2026-10-21
Design approval: The user accepted the settings layout, recommendations, and four implementation stages on 2026-10-21.

## Problem Statement

The user wants to play PS5 games through Chiaki Recast with a PC keyboard and mouse, without a physical controller.
The current setup uses a separate launcher, DS4Emulator, and a virtual Windows controller driver.
Recast already maps keyboard input to controller state, but its mouse movement controls touchpad gestures rather than the right stick.

The separate setup adds capture management and configuration work. Running both input paths can also produce duplicate commands.
The user needs one built-in mode with reliable mouse aiming, familiar bindings, and a clear way to return control to Windows.

## Solution

Add an optional Windows keyboard and mouse mode to Recast. Convert PC input directly into the controller state sent through its existing Remote Play session.
WASD controls the left stick. Mouse movement controls the right stick. Keys and five mouse buttons can control standard controller actions.
This mode does not require DS4Emulator, a virtual controller driver, or a physical controller.

The user enables the mode in Settings, then presses F8 during an active stream to enter gameplay capture.
F8 releases capture. Menus, focus loss, and session interruptions also release capture and clear mapped input.
Capture never resumes automatically.

Keep the existing settings sidebar and rename Keys to Keyboard and mouse.
Provide Mouse response and Bindings tabs, automatic saving, two bindings per action, conflict handling, and a scoped reset.
Preserve the existing keyboard and mouse-touchpad settings for use when the new mode is off.

The first version supports ordinary controls in Astro's Playroom and aiming tests in The First Descendant.
It does not claim full Astro's Playroom support or native-PC mouse behaviour in every game.

## User Stories

1. As a player, I want keyboard and mouse controls built into Recast, so that I do not need a separate mapper or launcher.
2. As a player, I want controller commands sent through Remote Play, so that I can play the supported actions without a physical controller.
3. As an existing user, I want the new mode to be off initially, so that an update does not change my current controls.
4. As a player, I want F8 to start gameplay capture during a stream, so that I choose when my input controls the game.
5. As a player, I want F8 to release gameplay capture, so that I can immediately use my desktop.
6. As a player, I want a visible capture status, so that I know where my keyboard and mouse input will go.
7. As a player, I want capture status to remain passive, so that it does not steal focus or block game controls.
8. As a player, I want menus and dialogs to release capture, so that I can interact with Recast without moving or firing in the game.
9. As a player, I want switching apps to release capture, so that typing in another app cannot control the game.
10. As a player, I want a new or reconnected session to wait for F8, so that it does not capture my mouse unexpectedly.
11. As a player, I want held inputs and pending mouse movement cleared across capture transitions, so that the game cannot receive stale commands.
12. As a player, I want only the selected input mode to control the captured session, so that another controller does not add unintended input.
13. As a player, I want normal physical-controller use restored after release, so that I can use it when I choose, subject to open menus and dialogs.
14. As a player, I want WASD movement and familiar keyboard bindings, so that I can use the controls from my current setup.
15. As a player, I want opposing movement keys to cancel and diagonal movement to stay within the stick limit, so that movement remains predictable.
16. As a player, I want mouse movement to control the camera without screen-edge limits, so that I can keep turning.
17. As a player, I want the camera stick to return to centre when the mouse stops, so that the camera does not drift.
18. As a player, I want mouse response to remain consistent across rendering rates, so that stream performance does not change my sensitivity.
19. As a player, I want direct mouse response without added acceleration controls, so that the first version remains predictable and easy to tune.
20. As a player, I want separate horizontal and vertical sensitivity, so that I can adjust each camera axis.
21. As a player, I want vertical inversion, so that the camera follows my preferred direction.
22. As a player, I want game dead-zone compensation, so that small mouse movements can overcome the game's stick dead zone.
23. As a player, I want compensation to produce no output while the mouse is idle, so that it cannot create camera drift.
24. As a player, I want left click for R2 and right click for L2, so that I can use familiar firing and aiming controls.
25. As a player, I want the existing X and Z trigger keys retained as alternatives, so that I can use either input.
26. As a player, I want a trigger held by two inputs to remain active until both are released, so that one release does not cancel the other input.
27. As a player, I want to assign middle click and two side buttons, so that I can use the buttons available on my mouse.
28. As a player, I want primary and alternative bindings for each supported action, so that I can use two convenient inputs for the same action.
29. As a player, I want each physical input to have one action, so that one press cannot activate unrelated actions by accident.
30. As a player, I want a conflicting assignment to show its existing action, so that I can decide whether to move it or cancel.
31. As a player, I want to clear a binding, so that unused actions do not occupy inputs.
32. As a player, I want F8 and F11 protected from reassignment, so that capture release and fullscreen remain available.
33. As a player, I want Ctrl combinations to reach the game during capture, so that Circle plus L1 cannot close Recast through Ctrl+Q.
34. As a player, I want double-click firing to remain gameplay input, so that it cannot toggle fullscreen.
35. As a player, I want normal app shortcuts restored after release, so that I can use Recast normally outside gameplay capture.
36. As a player, I want separate Mouse response and Bindings tabs, so that I can tune aiming without scanning every binding.
37. As a player, I want settings search to find these controls, so that I can reach them from the existing settings navigation.
38. As a keyboard user, I want accessible labels, predictable focus, and a way to cancel assignment, so that I can configure controls without a mouse.
39. As a player, I want valid edits saved automatically and applied on the next capture, so that I can tune controls without restarting the stream.
40. As a player, I want invalid values rejected without losing the last valid setting, so that configuration errors cannot break the controls.
41. As an existing user, I want my old input settings and external mapper files preserved, so that I can return to my previous setup.
42. As a player, I want Reset defaults to ask before replacing my settings, so that I do not lose custom bindings by accident.
43. As a player, I want reset to affect only this feature and turn its mode off, so that other Recast settings remain intact.
44. As a player, I want basic controls verified in Astro's Playroom, so that I know movement, jumping, and camera control work together.
45. As a player, I want aiming verified in The First Descendant with recorded sensitivity settings, so that tuning comparisons are meaningful.
46. As a maintainer, I want checks at the existing session boundary and native input interface, so that tests verify the commands and capture behaviour users receive.
47. As a player, I want accurate limits and acceptance evidence, so that basic input support is not presented as full-game compatibility or a guarantee about account risk.

## Implementation Decisions

### Integration and input ownership

- Target Windows for the first implementation. Keep other supported platform builds working and do not expose a nonfunctional Windows-only mode on them.
- Use the existing Remote Play feedback path. Extend input handling around the stream window, stream session, settings bridge, and QML settings controls.
- Keep conversion in a small mapper that accepts input events, elapsed time, and reset operations. Separate it from platform input collection without adding a general input-plugin framework.
- Use relative mouse movement without cursor-edge limits. Select the capture mechanism within the existing Qt, SDL, and Windows integration after checking current support.
- Reuse the established settings components, stream notices, key-assignment presentation, and focus-return patterns where practical.
- Follow ADR 0001, Give stream input an owner lifecycle. Inspect its current implementation before integration. Do not create competing owner records, counters, or independent cleanup rules.
- Preserve input-owner overlap, presentation identity, late-callback rejection, focus return, and session handoff behaviour. Gameplay capture cannot override a blocking stream input owner.
- Capture status is passive and does not become a stream input owner.

### Mode and capture contract

| State | Keyboard and mouse behaviour | Other controller input |
| --- | --- | --- |
| New mode off | Use existing input settings | Existing behaviour |
| New mode on, capture released | No game input from either new or legacy keyboard/mouse mapping | Normal use, subject to existing stream input owners |
| New mode on, capture active | Only the new mapped input controls the game | Suppressed for the captured session |

- The mode preference and gameplay capture are separate. Enabling the preference never captures input by itself.
- Enter capture only on a fresh F8 press when a usable stream is active, its window has focus, and no stream input owner blocks console input.
- F8 toggles capture once per physical press. Ignore key autorepeat for capture transitions and gameplay button presses.
- Release capture on F8, focus loss, a menu or dialog taking input ownership, disconnect, session replacement, stream exit, or app exit.
- New streams and reconnects always start released. Closing a menu or returning focus does not resume capture.
- Reset pending mouse movement and all mapped button, trigger, and stick state on release. Restore cursor control and send neutral mapped input when the connection is available.
- Do not promise delivery of a final neutral packet after a connection is already lost. The next session starts with fresh state.
- Inputs held across transitions must be released before they can create a fresh gameplay press. Discard stale callbacks and events from previous sessions or capture periods.
- Mouse repositioning, resizing, and fullscreen transitions must not produce artificial camera movement. If a transition loses focus, apply the release rule.
- Mode and settings-profile changes must not retain old held input. Changes made through Settings occur with gameplay capture released and take effect before the next capture.
- Suppress legacy keyboard and mouse-touchpad mapping whenever the new mode is selected, including during released periods.
- Suppress all other controller-state contributions during capture, including buttons, sticks, touches, and motion data. Release capture before restoring normal input-source selection.
- Ignoring an external controller does not stop another process from capturing the PC mouse. Stop the external mapper's capture before gameplay acceptance. Do not automatically kill, uninstall, or reconfigure that software.

### Mapping and default bindings

Use the following primary and alternative bindings. Empty alternatives are unassigned.

| Controller action | Primary | Alternative |
| --- | --- | --- |
| Left stick up | W | |
| Left stick down | S | |
| Left stick left | A | |
| Left stick right | D | |
| Cross | Space | |
| Circle | Ctrl | |
| Square | R | |
| Triangle | F | |
| L1 | Q | |
| R1 | E | |
| L2 | Right click | Z |
| R2 | Left click | X |
| L3 | Shift | |
| R3 | C | |
| D-pad up | Up arrow | |
| D-pad down | Down arrow | |
| D-pad left | Left arrow | |
| D-pad right | Right arrow | |
| Options | Enter | |
| Create | Backspace | |
| Touchpad click | Tab | |
| PS | F1 | |

- The right stick is assigned to mouse movement. It is not an editable keyboard binding in this mode.
- Support left, right, middle, and two side mouse buttons. Middle and side buttons start unassigned. Mouse wheel input has no mapping in this version.
- A held keyboard direction supplies full directional input. Opposite directions on one axis cancel. Normalize diagonal movement to the intended stick magnitude.
- Track held physical inputs. An action remains active while either assigned input is held. Releasing one alternative cannot cancel the other.
- Triggers have released and fully pressed values. Partial trigger pressure is outside this version.
- During capture, treat Ctrl and Shift as independent game inputs. Consume Recast's Ctrl shortcuts before they can close the window, open its menu, toggle its microphone, or change its video mode.
- Preserve operating-system shortcuts. F8 controls capture and F11 controls fullscreen. Neither is assignable to a game action.
- Disable stream double-click fullscreen behaviour during capture. Restore existing app shortcut behaviour when capture ends.

### Mouse response and calibration

- Convert mouse movement and elapsed time into bounded right-stick values. Do not couple the conversion clock to rendering frames.
- Provide independent horizontal and vertical sensitivity multipliers and vertical inversion.
- Provide game dead-zone compensation for small movements. This is output compensation for the game's stick dead zone, not a filter that discards small mouse input.
- Idle mouse input must yield an exactly centred mapped stick. Compensation must not cause idle drift.
- Add no acceleration presets or optional smoothing controls. Any timing needed to handle discrete mouse events must preserve the accepted direct response.
- Validate settings at entry and load boundaries. Reject non-finite, out-of-range, or otherwise invalid values. Retain the last valid value or use a validated default on invalid stored data.
- Stage 2 determines base gain, valid sensitivity ranges, valid compensation bounds, idle-return timing, and comparison tolerances using recorded input and real mouse tests.
- Record those numeric decisions and their evidence before Stage 2 is complete. Calibration is required implementation work, not an optional later enhancement.
- The visual's sensitivity values and ranges are examples. They are not approved numeric defaults or acceptance thresholds.
- The game still applies its own controller sensitivity, response curve, and maximum turn speed. Do not promise that mouse conversion removes those limits.

### Settings screen and persistence

- Keep the category sidebar. Rename Keys to Keyboard and mouse, with no duplicate category.
- Place Enable mouse camera mode at the top. Default it to off. Use the helper text: "During a stream, press F8 to capture or release your controls."
- Provide Mouse response and Bindings tabs. Keep the mode selector and Reset defaults available on either tab.
- Mouse response contains Horizontal sensitivity, Vertical sensitivity, Game dead-zone compensation, and Invert vertical camera movement.
- Use the approved slider and numeric-entry layout. Pair numeric controls with clear units and accessible labels.
- Bindings groups the actions into Movement, Buttons, Shoulders and sticks, D-pad, and System.
- Show Primary and Alternative slots for each action. Two slots per action are the accepted limit for this version. Each can be changed or cleared.
- Save valid edits automatically. Apply the saved configuration when capture next starts, without requiring a stream restart.
- Use the save message: "Saved · Applies the next time you press F8."
- Preserve existing keyboard-as-controller, mouse-touchpad, and key-assignment settings. Keep their controls accessible when the new mode is off.
- Keep new-mode controls visible but disabled when the mode is off, so users can inspect the saved configuration.
- Store the new mode separately from legacy input preferences using the existing application settings/profile conventions. Do not add per-game profiles or another profile manager.
- Missing new settings use mode-off and validated feature defaults. Do not overwrite legacy input values or import external emulator configuration.
- Integrate all new controls and bindings into settings search, keyboard navigation, accessibility names, and the existing responsive layout.

### Binding assignment and reset

- Assign one key or mouse button per slot. Do not support sequences, macros, or modifier chords.
- Each physical input can map to only one action. Multiple distinct inputs can map to the same action through its two slots.
- When an input is occupied, show its existing action and offer Move binding or Cancel. Move updates the old and new assignments together. Cancel changes neither.
- Avoid duplicate copies of the same physical input within one action's two slots.
- Reject F8 and F11 assignments with an explanation of their reserved purpose.
- Escape cancels assignment. Provide a keyboard-accessible way to assign Tab while preserving focus navigation. Return focus to the invoking slot when assignment ends.
- Accessible binding names include the action, slot, and current input or unassigned state.
- Reset defaults requires confirmation. It restores the feature's default bindings and calibrated mouse settings and turns the new mode off.
- Reset leaves legacy keyboard settings, physical-controller settings, console configuration, and external emulator files unchanged.

### Stream status

- When the new mode is enabled and capture is available, show "Keyboard and mouse ready · F8 to capture."
- During capture, show "Mouse captured · F8 to release."
- Place the indicator at the lower edge with the existing stream notices. Keep it clear of stream menu controls and support both presentation routes.
- Do not display an actionable capture-ready prompt while a blocking owner prevents capture. Restore the ready state only when input can be accepted.

### Implementation stages

| Stage | Dependency | Complete result |
| --- | --- | --- |
| 1. Play with accepted controls and release safely | Check current input ownership against ADR 0001 | Opt-in setting, fixed default controls, relative mouse capture, status, exclusive input selection, shortcut handling, and all release/reconnect safeguards work through the real session |
| 2. Tune the mouse during a running stream | Stage 1 | Mouse response settings persist, validate, apply on recapture, and have documented calibration evidence |
| 3. Edit and reset bindings | Stage 1 and the shared settings contract | Two binding slots, five mouse buttons, conflicts, reserved-key checks, persistence, and scoped reset work through the stream |
| 4. Native acceptance and review | Stages 2 and 3 | Windows behaviour, selected gameplay, legacy input regressions, layout, and review have recorded evidence |

Stages 2 and 3 can proceed independently after their common interfaces are settled.
Capture failures and stuck input block completion of Stage 1. Do not defer them to Stage 4.
Each stage includes its relevant tests. Stage 4 verifies the integrated result rather than supplying all testing at the end.

## Testing Decisions

### Primary test boundary

Use the existing stream-session controller-output boundary as the main integration seam.
Exercise production input handling and session objects, then inspect the controller state supplied to the Remote Play feedback path.
Intercept transport in the controlled native test build rather than connect to a PS5 for every automated check.

This follows the accepted stage plan and existing native acceptance work that uses real backend and session objects with intercepted transport commands.
Reuse the stream input-owner tests and QML stream harnesses for menu overlap, focus return, session replacement, and presentation lifecycle checks.
Honor ADR 0001's maintained native-test approach. Do not copy its historical source-rewriting test techniques.

Only add a smaller mapper test seam for precise timed input traces that cannot be usefully checked through the main boundary.
Use the same production conversion and reset logic. Avoid separate fake implementations of the mapper or a new mock layer for every component.

### Automated behaviour checks

Good tests assert externally visible behaviour: emitted controller state, persisted settings, focus, capture availability, and visible feedback.
Do not assert internal helper call order or duplicate implementation formulas in the tests.

- Verify every default binding and supported mouse button, including unassigned middle and side buttons.
- Check opposite directions, normalized diagonals, valid stick ranges, triggers, and multiple held alternatives.
- Hold movement and a trigger, then release capture, open a menu, lose focus, or end a session. Verify neutral mapped state and no replay.
- Test duplicate and overlapping owner presentations, obsolete close callbacks, destruction, focus return, and session handoff with the new mode enabled.
- Reconnect while keys are held. Verify that capture remains released and held keys require fresh presses after rearming.
- Verify F8 autorepeat cannot toggle repeatedly. Verify F11 remains available and two trigger clicks cannot toggle fullscreen.
- Verify Ctrl+Q, Ctrl+S, Ctrl+Z, Ctrl+M, and Ctrl+O produce game actions without app shortcuts during capture. Verify normal shortcuts after release.
- Test other controller sources during capture and after release, including touch and motion contributions. Existing owners must still block console input.
- Replay equal timed movement traces with different event batching and rendering schedules. Compare mapped output within the recorded calibration tolerance.
- Check idle centring, inversion, compensation transitions, saturation, invalid values, and stale mouse deltas after reset or settings changes.
- Verify mode-off defaults, persistence across restart, application on recapture, separate legacy values, and valid handling of malformed saved settings.
- Move a conflicting binding, cancel a conflict, clear a slot, reject reserved keys, and verify accessible focus return.
- Reset with and without confirmation. Verify that only feature settings change and the mode turns off after confirmed reset.
- Check settings search, both tabs, numeric entry, disabled states, long labels, keyboard navigation, and the existing minimum settings window size.

### Native Windows acceptance

Automated state checks cannot prove OS capture or actual cursor behaviour.
Test the built app with real keyboard and mouse input:

- Continuous relative movement at screen edges and across the available display arrangement.
- F8 release, Alt+Tab, menus, dialogs, quit, disconnect, reconnect, and focus return.
- No lost held-input cleanup, artificial movement during fullscreen/resize, or side-button navigation while captured.
- Mouse removal where practical, display scaling, and both supported stream rendering paths.
- Existing physical-controller, keyboard, and mouse-touchpad behaviour with the new mode off.
- Both stream presentation routes, readable capture status, and no overlap with menu controls.

Stop the external mapper's capture before these tests. Record unavailable hardware or checks as limits rather than claiming they passed.

### Gameplay acceptance

| Game | Required checks | Claim limit |
| --- | --- | --- |
| Astro's Playroom | Ordinary movement, jumping, camera control, standard buttons, and menu actions | Basic control coverage only; not full campaign completion |
| The First Descendant | Fine aim corrections, target tracking, fast turns, aiming while moving, and firing while moving | Tested settings and scenarios only; not universal mouse-feel equivalence |

Record source revision, build, Windows environment, mouse DPI, mouse report rate where available, game controller settings, and scenario results.
Separate mapper failures from game-imposed turn-speed limits through repeatable input comparisons.
Do not change test settings between comparisons without recording the change.

Before release, review the change against this spec and repository standards. Publish completed checks and unresolved limits in an acceptance record.
Native capture failures, stuck output, lost legacy settings, or unusable aiming block release.
Compilation and preview interactions alone do not satisfy these gates.

## Out of Scope

- Linux, macOS, or Android implementation of the new mode in this version.
- A virtual Windows controller device, DS4Emulator bundling, driver installation, or an input mapper for other applications.
- Background gameplay capture, automatic recapture, or sending keyboard and mouse game input while capture is released.
- Mouse-wheel mapping, horizontal scrolling, or support beyond the five specified mouse buttons.
- More than two bindings per action, modifier-chord bindings, macros, timed sequences, or automated gameplay.
- Per-game profile management, automatic game detection, acceleration presets, or optional smoothing controls.
- Motion simulation, touchpad gestures, partial-trigger pressure controls, microphone interaction work, or full Astro's Playroom coverage.
- Physical haptic feedback or adaptive-trigger force on ordinary keyboard and mouse hardware.
- Automatic installation, removal, process termination, or configuration changes to the existing external mapper setup.
- Claims of native-PC mouse behaviour, universal game compatibility, Sony approval, undetectability, or zero account risk.

## Further Notes

The user approved the settings layout, two input slots per action, reset turning the mode off, and the four-stage order.
The test boundaries and stage acceptance requirements were part of that approved proposal.
No additional design interview is needed to publish this specification.

The existing ADR on stream input ownership remains a constraint. The current checkout must be checked against it before integration because related work can advance independently.

Calibration values are intentionally measured implementation outputs. The agent must record validated values before completing Stage 2 rather than copy sample values from the mockup.

The user initially described single-player use, then selected The First Descendant as an aiming test.
Nexon describes that game as online co-op with solo-play options. Solo content does not establish an offline-only test scope.
See [Nexon's developer note](https://tfd.nexon.com/en/news/2489141) and [community Q&A](https://tfd.nexon.com/en/news/2675068).

Astro's Playroom uses motion, touchpad input, microphone interaction, and partial trigger presses.
These requirements explain its limited acceptance scope. See [Team Asobi's game overview](https://blog.playstation.com/2020/11/11/unleash-the-power-of-the-dualsense-wireless-controller-with-astros-playroom/).

Sony's published terms restrict unauthorised hardware and software and allow account action for breaches.
The reviewed sources do not establish approval, detection behaviour, or enforcement probability for this feature.
A virtual DS4 on the PC is not evidence of Sony authorisation or protection from detection.
See [PlayStation terms](https://www.playstation.com/en-gb/legal/terms-of-service/) and [suspension guidance](https://www.playstation.com/en-gb/support/account/suspensions/), reviewed during planning on 2026-10-21.

Design history is retained in [planning notes](planning.md) and [the approved screen and stages](settings-and-stages.md).
The interactive visual is design evidence, not production code. Its numeric ranges are illustrative, and its legacy section is incomplete by design.

This specification is ready for implementation planning. No implementation tickets, native feature changes, or gameplay acceptance results are created by publishing it.
