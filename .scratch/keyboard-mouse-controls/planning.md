# Keyboard and mouse controls

Planning state: Decisions accepted and consolidated into [the implementation specification](spec.md). Retained as design history.
Updated: 2026-10-21.

## Goal

Play PS5 games through Recast with a PC keyboard and mouse, without a physical controller.
The user initially specified single-player use and no multiplayer use.
The selected test games are Astro's Playroom and The First Descendant.
The First Descendant is an online co-op game with solo-play options. Its selection does not establish an offline-only test scope.

## Accepted decisions

The user accepted these decisions across the planning rounds.

- Build keyboard and mouse mapping directly into Recast, for Windows first.
- Send controller input through the existing Remote Play session.
- Do not require DS4Emulator or a virtual Windows controller driver for this mode.
- Allow gameplay capture only while the stream window is active.
- Use F8 to enter or leave gameplay capture.
- Release the mouse and clear held game inputs when a menu opens or the user switches apps.
- Require F8 to resume gameplay capture after an interruption.
- Map WASD to the left stick and mouse movement to the right stick.
- Map mouse buttons to triggers and provide editable keys for standard controller buttons, including PS and touchpad click.
- Limit first-version Astro's Playroom acceptance to normal movement, jumping, camera control, and standard buttons. Full-game coverage is a later stage.
- Defer motion, touchpad gestures, partial-trigger controls, microphone interaction checks, and full Astro's Playroom coverage from the first version.
- Start with one editable control profile.
- Provide separate horizontal and vertical mouse sensitivity, invert Y, and stick dead-zone adjustment.
- Provide a dedicated Keyboard and mouse section in Settings for bindings and sensitivity.
- During gameplay capture, show a small status message: "Mouse captured · F8 to release."
- During gameplay capture, use only the selected keyboard and mouse input mode for console control. Ignore other controller input.
- F8 releases gameplay capture and restores normal controller use.
- Use Astro's Playroom for initial movement and camera checks, and The First Descendant for aiming and controller sensitivity checks.
- Preserve the standard keyboard bindings listed below.
- Add left click for R2 and right click for L2. Retain Z and X as alternative trigger bindings.
- Start with direct mouse response and no added acceleration or optional smoothing controls. Return the mapped stick to centre when mouse movement stops.
- Support left, right, middle, and two side mouse buttons as binding inputs. Leave middle and side buttons unassigned by default.
- Defer mouse-wheel mapping from the first version.
- During gameplay capture, consume Ctrl combinations as gameplay input instead of invoking Recast's Ctrl shortcuts.
- Reserve F8 for capture release and F11 for fullscreen. Restore normal app shortcuts after gameplay capture ends.
- Make the new mode opt-in in Settings. Preserve existing keyboard bindings and mouse-touchpad preferences for the existing mode.
- In the new mode, suppress the old keyboard and mouse-touchpad mapping, including while gameplay capture is released. Physical controllers can operate normally after release, subject to existing stream input owners.
- Save valid edits automatically and apply them when gameplay capture next starts, without restarting the stream.
- Reset only this feature's settings, with confirmation before replacing custom settings.
- Give each physical key or mouse button one action. For conflicts, show the existing action and offer Move binding or Cancel.
- Allow alternative inputs for the same action. Reject binding attempts for F8 and F11.
- Continue planning before implementation.

## Accepted default bindings

The user accepted the existing standard bindings from PS5Controls CONTROLS.txt, with mouse-button trigger bindings added.

| PC input | Controller input |
| --- | --- |
| WASD | Left stick |
| Mouse movement | Right stick |
| Space | Cross |
| Ctrl | Circle |
| R | Square |
| F | Triangle |
| Q and E | L1 and R1 |
| Z and X | L2 and R2 |
| Right click and left click | L2 and R2 |
| Shift and C | L3 and R3 |
| Arrow keys | D-pad |
| Enter | Options |
| Backspace | Share or Create |
| Tab | Touchpad click |
| F1 | PS |

The old setup also has touchpad swipe and shake bindings. These are outside the agreed first-version scope.

## Astro's Playroom coverage

Team Asobi describes motion-sensor, touchpad, microphone, and trigger use in [its official game overview](https://blog.playstation.com/2020/11/11/unleash-the-power-of-the-dualsense-wireless-controller-with-astros-playroom/).
Its Monkey Suit example uses motion and requires lighter trigger presses on weak handholds.
The accepted basic controls cannot be presented as full-game support.

Recast's controller state has touch coordinates, analog trigger values, gyro, acceleration, and orientation fields.
Those fields make further input mapping possible. They do not establish that simulated motion or partial-trigger controls work well in the game.
Microphone input uses a separate audio path. Full-game coverage needs a check of the relevant interactions through Remote Play.

## Approved settings and implementation stages

The user accepted the detailed layout and stage breakdown in [Settings screen and implementation stages](settings-and-stages.md).
The [specification](spec.md) is the authoritative source for implementation scope, including two bindings per action and reset turning the mode off.

## Capture lifecycle derived from accepted decisions

- New streams, reconnects, and session replacements start with gameplay capture released. A fresh F8 press is required to enter capture.
- On release, clear pending mouse movement, centre mapped sticks, clear mapped buttons and triggers, and release the cursor. Send neutral mapped input if the connection is available. A lost connection cannot guarantee delivery of a final packet.
- Do not replay input from an old session or from a dialog into a new capture period.
- Input held across capture transitions must be released before it can produce a fresh gameplay press. Repeated F8 key events must not toggle capture repeatedly.
- Window resizing and fullscreen transitions must not generate camera movement from cursor repositioning. If a transition loses focus, follow the existing release rule.
- The existing stream input ownership rules still block console input for menus and dialogs.

These rules extend the accepted manual capture and input-release decisions. They do not add automatic resume behaviour.

## Proposed acceptance checks

- Hold a movement key and a trigger, then release capture or switch apps. Mapped output becomes neutral and the desktop cursor is available.
- Reconnect while inputs are held. The new session remains neutral until a fresh capture and fresh gameplay inputs.
- Hold both alternative inputs for one trigger. Releasing either input keeps the trigger active until the other input is released.
- Press Ctrl with Q, S, Z, M, or O during capture. Game bindings work without invoking app shortcuts. After release, the app shortcuts work normally.
- Move the mouse to every edge of the display. Relative camera control continues without cursor-edge limits.
- Stop the mouse. The mapped stick returns to centre without drift. Determine and record the acceptable reset interval during calibration.
- Compare equal recorded mouse movement over equal elapsed time with different event batching and render timing. Mapping must not depend on rendering frames.
- Check settings persistence, reset scope, binding conflicts, and the preservation of existing input settings.
- In Astro's Playroom, check ordinary movement, jumping, camera control, and menu actions. Do not claim full-game acceptance.
- In The First Descendant, check small aim corrections, target tracking, fast turns, and movement while aiming and firing. Record game controller settings and mouse DPI for each comparison.

Releasing one alternative binding must not release a controller button that another input still holds.
Exact mouse timing and dead-zone compensation require calibration.
Ignoring external controller state cannot stop another process from capturing the PC mouse. Resolve the external launcher's capture before gameplay tests.

## Verified code facts

- `StreamSession::HandleKeyboardEvent` already converts keys into controller buttons, triggers, and stick values.
- `StreamSession::HandleMouseMoveEvent` currently uses mouse movement for touchpad gestures.
- `StreamSession::SendFeedbackState` combines controller, keyboard, and touch state.
- `chiaki_session_set_controller_state` passes that state to the Remote Play feedback sender.
- The feedback sender copies stick values into the state sent to the console.
- ADR 0001 requires one shared stream input ownership lifecycle. This feature must respect its blocking and cleanup rules.
- `QmlMainWindow::handleShortcut` currently handles Ctrl+Q as window close, Ctrl+O as menu, Ctrl+M as microphone mute, and Ctrl+S or Ctrl+Z as video modes. These conflict with ordinary gameplay combinations when Ctrl maps to Circle.
- The existing Keys settings page already has a Keyboard and mouse group. Its controls write settings when toggled. Reuse the surrounding settings components and automatic-save pattern in the proposed layout.
- Existing stream test harnesses cover menu and dialog input blocking and session replacement. Controlled native checks use real backend and session objects with intercepted transport commands. Native mouse capture and gameplay still need real Windows and PS5 checks.

These facts support the proposed integration. They do not prove mouse aiming quality or acceptance by every game.

## Account and detection uncertainty

Reviewed Sony sources on 2026-10-21:

- [PlayStation Terms of Service, version 12, April 2026](https://www.playstation.com/en-gb/legal/terms-of-service/), sections 24.6.3 and 22.3.
- [PlayStation suspension guidance](https://www.playstation.com/en-gb/support/account/suspensions/).

Sony's published terms restrict unauthorised hardware and software. Sony can suspend accounts for breaches of its terms.
The reviewed sources do not approve Recast or establish whether Sony detects this particular input mapping.
They do not supply an enforcement probability for single-player use.

A virtual DS4 exists on the PC. With either proposed input route, Recast sends Remote Play controller state to the console.
This is a code-based reason not to treat a virtual DS4 driver as proof of authorisation or protection from detection.
It is not proof that the two routes produce identical traffic or that Sony cannot distinguish input patterns.

Single-player use avoids the intended use of this feature against other players. It does not establish that the session is offline or exempt from account terms.
Do not claim that the feature is undetectable or free of account risk.

Nexon describes The First Descendant as an online game with co-op missions, and describes dungeons that can be played solo.
See [Dev Note vol.7](https://tfd.nexon.com/en/news/2489141) and [Community Q&A](https://tfd.nexon.com/en/news/2675068).
Selecting solo content in this game does not make it an offline test or establish publisher approval of input conversion.

## Implementation calibration and verification work

- Mouse timing, dead-zone compensation, calibration, and acceptance thresholds. Direct response without added acceleration is accepted.
- Native implementation and visual verification of the accepted settings layout and capture status placement.
- Handling the external mapper before gameplay tests. The accepted mode preserves existing settings and suppresses old input mapping while selected.
- Execute the accepted checks for aiming, input release, and timing consistency.
- Implement the accepted stages and tests while preserving existing settings.

## Evidence limits

No feature implementation, native build, or PS5 gameplay test has been performed for this plan.
The existing handoff reports a slow Astro camera with both external mouse mapping and the built-in keyboard right-stick mapping.
Treat that report as historical evidence, not as a fresh test or a promise about other games.
