# Stream menu, statistics, and session notices

Status: resolved
Date: 2026-10-21
Design: approved A toolbar from revision 3; approved A statistics and notices from revision 2
Test approach: production QML tests plus native checks, confirmed by the user on 2026-10-21

## Problem Statement

The active-stream interface uses the old presentation. The menu places many controls
and status values in a crowded row. Statistics and notices use large text or panels
that compete with the game. Users need clear controls without covering more of the
stream than necessary.

A full application-style panel covers too much of the game. A very small toolbar
makes controls difficult to identify. The stream interface needs a middle ground:
a transparent toolbar with readable labels, compact statistics, and short notices.

## Solution

Use the approved A designs. Place a descriptive, translucent toolbar at the bottom
center of the stream. Keep Volume and Microphone directly available. Show Picture
mode and Quality preset with their current values. Open their controls in small
pop-ups above the toolbar. Keep Resume and Disconnect as separate named actions.

Show statistics in one small row at the bottom left. Use small top-right notices
for network warnings and disabled audio. Use a short bottom-center notice with
one action for blocked content or disabled video. Keep the game visible around
the overlays. Do not add full-screen dimming or blur.

The overlays do not need to use the main application's panel design. Preserve
stream behavior, settings, keyboard and controller operation, and both renderer
presentations. Keep the 900-logical-pixel minimum application width.

## User Stories

1. As a player, I want the menu to cover a small part of the game, so that I can see the stream while I adjust it.
2. As a player, I want a translucent menu background, so that the game remains visible behind the controls.
3. As a player, I want opaque, readable labels, so that transparency does not make controls difficult to read.
4. As a player, I want larger controls than the compact prototype offered, so that I can identify and select each action.
5. As a player, I want labels separate from current values, so that I understand what each control changes.
6. As a player, I want Resume clearly named, so that I can close the menu without ending the session.
7. As a player, I want Disconnect clearly separated from Resume, so that I do not confuse their effects.
8. As a player, I want the existing disconnect choices retained, so that I can leave the console on or put it into rest mode.
9. As a player, I want Cancel in the disconnect dialog to leave the stream running, so that I can return without a session command.
10. As a player, I want Volume available directly in the toolbar, so that I can adjust sound without opening another panel.
11. As a player, I want a volume percentage, so that I can see the current setting.
12. As a player, I want the existing volume range and saved setting retained, so that the redesign does not change audio behavior.
13. As a player, I want Microphone to show On or Muted, so that I can see its state before I act.
14. As a player, I want the microphone control disabled when no connected session supports it, so that the interface does not offer an invalid action.
15. As a player, I want Picture mode to show the current mode, so that I can check it without opening a pop-up.
16. As a player, I want Fit, Zoom, and Stretch grouped together, so that I can choose how the picture fills the window.
17. As a player, I want a short explanation of picture modes, so that I understand when proportions can change.
18. As a player, I want the zoom adjustment available only in Zoom mode, so that unrelated controls do not take space.
19. As a player, I want the existing zoom range and No black bars option retained, so that my current picture setup still works.
20. As a player, I want Display settings available from the picture pop-up, so that I can reach the existing detailed controls.
21. As a player, I want Quality preset to show its current value, so that I can check rendering quality without opening settings.
22. As a player, I want all five existing quality presets retained, so that the redesign does not remove a rendering option.
23. As a player, I want Renderer settings available for Custom, so that I can edit the existing custom configuration.
24. As a player, I want only one menu pop-up open at a time, so that extra controls do not cover the stream unnecessarily.
25. As a keyboard user, I want Escape to close a pop-up before closing the menu, so that I can return one level at a time.
26. As a keyboard user, I want focus returned to the pop-up opener, so that I can continue navigation without searching for my position.
27. As a controller user, I want the existing menu shortcut and navigation inputs retained, so that I can operate the stream without a mouse.
28. As a keyboard user, I want visible focus on each control, so that I know which action Enter will activate.
29. As a player, I want input returned to the stream after Resume, so that menu input does not remain captured.
30. As a player, I want statistics in a small bottom-left row, so that I can inspect stream health without a large panel.
31. As a player, I want the existing statistics visibility setting retained, so that I control whether the readout appears.
32. As a player, I want bitrate shown in Mbps, so that I can inspect the current data rate.
33. As a player, I want packet loss shown as a percentage, so that I can identify a network problem.
34. As a player, I want dropped and lost frames shown separately, so that I do not confuse the two measurements.
35. As a player, I want average queue depth and pending frame age retained, so that I can inspect frame delivery.
36. As a player, I want statistics based on actual session values, so that the interface does not show sample or invented health data.
37. As a player, I want statistics hidden while the menu is open or closing, so that the controls and readout do not compete.
38. As a player, I want a small network warning with the measured packet loss, so that I understand the warning without a large icon or dialog.
39. As a player, I want the existing network warning threshold retained, so that the visual refresh does not change when it appears.
40. As a player, I want an audio-disabled notice, so that I understand why the session has no sound.
41. As a player, I want separate messages for video disabled and audio and video disabled, so that the notice describes my settings accurately.
42. As a player, I want disabled-media notices to explain that the session is still connected, so that I do not mistake them for a connection failure.
43. As a player, I want the settings location named in disabled-media notices, so that I know where to restore audio or video.
44. As a player, I want blocked Remote Play content explained in a short notice, so that I understand why the screen is unavailable.
45. As a player, I want Go to console home retained for blocked content, so that I can return to supported content.
46. As a player, I want routine notices to leave input with the game, so that a warning does not interrupt play.
47. As a player, I want notice conditions to update from the current session, so that cleared warnings and old-session notices do not remain visible.
48. As a player, I want the same overlay controls in both renderer modes, so that renderer choice does not change how I operate the stream.
49. As a player, I want overlays and pop-ups to follow window movement and resizing, so that controls stay attached to the game.
50. As a player, I want readable controls at the minimum window size and supported display scaling, so that the menu remains usable on my display.
51. As a user of translated text, I want long labels and preset names to remain readable, so that controls do not overlap or disappear.
52. As a streamer, I want any displayed console identity to respect streamer mode, so that the redesign does not reveal private values.
53. As a player, I want connection progress, console login PIN, and disconnect dialogs to keep their current behavior, so that the overlay refresh does not break completed flows.

## Implementation Decisions

- The accepted toolbar is A from the third prototype revision. The accepted statistics and notices are A from the second revision. B and C are rejected alternatives for this implementation.
- Use the latest toolbar's two-level labels: Back to game / Resume, Volume / percentage, Microphone / On or Muted, Picture mode / selected mode, Quality preset / selected preset, and End session / Disconnect. The slash here separates the two visible lines; it is not part of the UI text.
- At standard desktop sizes, the reference toolbar is approximately 820 logical pixels wide and 80 high, with about 18 pixels below it. The control text is approximately 14 pixels, with smaller descriptive labels. Adapt width and wrapping to available space and translated text without returning to a large settings panel. Do not truncate the selected preset into an unidentified value.
- Use the approved dark background at approximately 65% opacity. Apply transparency to the background, not to the whole control tree. Keep text, focus indicators, and controls readable. The opacity selector is a design-comparison tool, not a new saved setting. Do not add blur or full-screen dimming.
- Keep the toolbar at the bottom center. Picture and quality controls open above it in small panels, approximately 320 logical pixels wide. Reuse native controls and existing navigation conventions. Only one of these panels is open at a time. A second activation of its opener closes it.
- Resume closes the menu without ending the stream. The existing stream-menu shortcut still toggles the menu. Escape closes an open picture or quality panel and returns focus to its opener; otherwise it closes the menu. Set initial menu focus to Resume. Preserve controller key mappings and prevent menu actions from also reaching the game.
- Retain the volume backend range of 0 through 128 and its existing persistence. Convert to a percentage only for display. Do not replace the native range with the prototype's simplified 0 through 100 state.
- Bind Microphone to the current session's mute state. Disable it without a connected session. Do not change the stored or protocol meaning of mute.
- Map Fit to the existing normal video mode. Retain Zoom and Stretch as mutually exclusive alternatives. Show the zoom slider only for Zoom. Preserve its range of -1 through 4, step of 0.01, No black bars endpoint, existing value formatting, and saved zoom-factor behavior.
- Preserve Default, High quality, HQ + Spatial, HQ + Advanced spatial, and Custom as the existing preset values. Selecting a preset updates the same runtime and saved settings as today. Do not change rendering algorithms or preset definitions.
- Display settings opens the existing refreshed Display page. Renderer settings is available when Custom is selected and opens the existing renderer settings page. Close the menu through its normal input-release path before navigation. Do not redesign either linked page.
- Disconnect uses the existing session-close route and configured disconnect policy. Where confirmation is required, keep the refreshed dialog, its Cancel-first focus, and its Disconnect, Disconnect and sleep, and Cancel semantics. Do not replace the production dialog with the browser stub.
- Update the production stream view and separate menu window. Share the menu content and action bindings between the two presentations so that controls cannot drift. Preserve the existing renderer-based host selection. Keep one owner for menu and pop-up state, focus restoration, and input capture.
- Replace the fixed old menu geometry with bounds that accommodate the toolbar and an open pop-up. A separate native window must not clip the pop-up, cover the rest of the stream with an opaque rectangle, or intercept input outside the intended interactive region. Update geometry on move, resize, fullscreen changes, and display-scale changes. Preserve host ownership and cleanup.
- Reuse the existing statistics presentation path for inline and separate-window hosts. Position the readout at the bottom left, approximately 18 logical pixels from the edges. Use one compact translucent row with labels Bitrate, Loss, Dropped, Lost, Queue, and Age. Retain complete metric names for accessibility.
- The six values are measured bitrate in Mbps, average packet loss as a percentage, dropped frames, lost frames, average queue depth, and pending frame age in milliseconds. Preserve existing sources, conversions, and finite-value guards. Show valid zero values. Never substitute prototype sample values or expose NaN or Infinity in the UI.
- Keep the existing statistics preference and visibility conditions. Hide the row during menu opening/visibility/closure as required by the current menu state, connection loading or error, and disabled video. Do not let the passive readout take keyboard or pointer input from the stream.
- Replace the large network indicator with A's small top-right warning. Use Unstable connection and the current packet-loss percentage. Preserve the configured packet-loss threshold and existing warning timing. Do not add a new health classifier or notification policy.
- Use a small top-right Audio disabled notice when audio is disabled. Name Settings → General → Audio/Video as the recovery location. If audio and network notices occur together, keep both readable without overlap; use a compact stack in the same corner.
- Use A's short bottom-center notice for blocked content. Preserve the existing blocked-content condition, Go to console home action, and input ownership. The heading is Content blocked by the console. The explanation says that the screen cannot be shown through Remote Play. Do not dim the rest of the stream.
- Use Video disabled or Audio and video disabled according to the existing settings flags. Explain that the session is still connected and identify the existing Audio/Video setting. Provide Open stream menu as in the approved preview. Do not invent a direct restore command. Retain the existing black video area when video is disabled; this is distinct from adding a dimming layer over video.
- Derive notices from current session and settings state. Clear them when their conditions clear or the session ends. Routine audio and network notices are non-modal and do not take focus. Preserve connection, console login PIN, and disconnect-dialog priority, and avoid overlap with the toolbar, pop-ups, and actionable notices.
- Do not expose a console address or name merely to fill the toolbar. If an existing identity is displayed anywhere in this area, apply the existing streamer-mode masking. Use translatable plain text, accessible names, and visible focus. Preserve existing configuration and session API contracts; no schema migration is needed.

## Testing Decisions

Acceptance scope update, 2026-10-21: the user limited the remaining checks to
900×600 logical pixels at 100% scale and requested that passing checks not be
repeated. Keep the results already collected. Re-run only checks affected by a
fix or needed to confirm a failure. This update overrides the resolution and
scale matrix below for the remaining acceptance work. Both renderer routes,
the normal production build, and final review remain required.

- The user confirmed production QML tests with supplied session state plus native checks for both renderer modes, transparency, window placement, keyboard/controller key handling, and focus. Live console commands and physical-controller tests remain separate.
- Prefer one test boundary at the composed production stream interface. Supply backend, settings, window, and navigation state at that boundary and observe resulting UI and commands. Where the stream view currently binds directly to the singleton, add the smallest top-level injection point needed, with the native backend as its production default. Do not add independent test APIs to every control.
- Load actual production menu, statistics, and notice components, including their real inline and separate-window wrappers. A fixture may provide state and record commands; it must not copy the production layout or implement a second menu state machine. Use the existing Qt Quick screen runner.
- Follow the existing Disconnect, LiveConsolePin, and Connection harness patterns. These already compose production QML with controlled backend state, observe actions, and check focus and both window routes. Reuse this prior art rather than creating a separate browser-based acceptance system.
- Good tests assert externally visible behavior: displayed values, enabled actions, focus movement, visible geometry, and the exact command caused by user input. Avoid assertions about private helper names, component counts, or the implementation's internal structure.
- Exercise volume at both endpoints and an intermediate value, microphone on/off and disconnected state, every picture mode, zoom endpoints and persistence, every quality preset, Custom-only renderer access, and both linked settings actions. Observe real production bindings, not only a test signal disconnected from the runtime handler.
- Verify Resume, menu shortcut toggling, one-open-pop-up behavior, Escape order, return focus, initial Resume focus, keyboard traversal, controller key mappings, repeated input, and disabled controls. Confirm that menu closure releases input once and leaves no stale separate window.
- Retain the existing disconnect tests and add coverage of the new toolbar entry point through the configured session-close policy. Check Cancel, Escape, both disconnect results, and duplicate input. Keep the console login PIN and connection-flow exclusion rules covered at the composed interface.
- Supply all six metrics, including zero values and invalid/missing inputs where supported. Check labels, units, conversions, live updates, and statistics visibility during menu closure, loading, failure, and disabled video. Check that the statistics row does not consume stream input.
- Exercise network warnings below, at, and above the existing threshold and when the condition clears. Cover blocked content and all three disabled-media states, notice combinations, session replacement, and session destruction. Observe Go to console home through a controlled command boundary. Confirm that no notice falsely changes connection status.
- Check layouts at 900×600, 1280×720, and 1920×1080 logical pixels. Include long translated labels, the longest preset name, large metric values, zoom and Custom controls, and notice combinations. Check bounds, legibility, and focus visibility without enforcing an exact browser screenshot match.
- Render the production overlays against dark, bright, and detailed sample backgrounds. Verify that only the backplates are translucent and that text and focus remain readable. Keep the accepted prototype as a visual reference, not as proof that native rendering works.
- Run focused native checks using isolated settings and controlled session state for the inline and separate-window renderer routes. Check window move/resize, fullscreen transitions, supported display scaling including 100%, 150%, and 200% where available, pop-up placement, transparency, activation, input capture, and return focus. Exercise the real application handlers while intercepting transport commands.
- Record renderer/backend availability and capture limits. A render-target capture proves component pixels, not desktop composition. If desktop capture returns unrelated pixels, discard it and use a reliable alternative or report that composition remains unverified. Do not label an unavailable renderer or unobserved behavior as passed.
- Run a normal production build after the checks. Confirm that prototype files, fixtures, controlled backend hooks, and comparison controls are absent from the shipped resources. Run the related existing connection, PIN, disconnect, and linked-settings checks for regressions.
- Report controlled automated results separately from live console behavior. Real console power changes, live media, physical controllers, and real network degradation are not established by the agreed test fixtures or synthetic key input.

## Out of Scope

- B or C menu layouts, statistics variants, or notice variants.
- The first large-panel design and the second revision's undersized A toolbar.
- Prototype layout switching, preview tabs, sample-state inspection, host simulation, background selection, and an end-user opacity setting.
- New statistics, graphs, histories, configurable overlay positions, or new network-warning thresholds.
- New audio, video, renderer, streaming protocol, or persistence behavior.
- Redesign of Display settings, renderer settings, the disconnect dialog, Library, Settings, PSN setup, connection progress, or console login PIN flows.
- A real service or console command during automated acceptance, changes to the user's settings for testing, and an assertion that synthetic key input proves physical-controller behavior.
- Standalone release packaging, publishing, pushing, and pull request creation unless requested separately.

## Further Notes

The user approved the final preview with "looks good" and requested this specification.
All visual choices are settled. The earlier request to match the main application's
design was superseded by the explicit request for independent, transparent overlays.

The primary design source is branch `codex/stream-interface-prototype`, revision
`56869701`. It contains the approved A toolbar and preserves A for statistics and
notices. The previous compact revision is `4ef1fdcb`. The first panel study is
historical and is not the implementation reference.

Source and evidence pointers:

- Prototype: `gui/prototype/stream/`.
- Approved toolbar capture: `.scratch/stream-interface-prototype/evidence/descriptive-v3/menu-a.png`.
- Approved statistics and notices: `.scratch/stream-interface-prototype/evidence/compact-v2/`.
- Design record and implementation pointer: `.scratch/stream-interface-refresh/issues/01-stream-interface.md`.
- Native source context: `gui/src/qml/StreamView.qml` and `gui/src/qml/StreamMenuWindow.qml`.
- Existing test workflow: `gui/tests/README.md`.

The 675 browser assertions recorded with the prototype establish its sample behavior
and layout only. They do not satisfy the native acceptance requirements above.
Implementation must use production QML and preserve native behavior. Keep the
throwaway prototype on its reference branch instead of merging it into production.

## Answer

Implemented on `codex/stream-interface-integration`. The five tickets deliver the
approved descriptive A toolbar, compact statistics, and translucent notices in
both renderer routes. The final production changes are recorded in `62f9676e`.
The prototype remains on its reference branch.

Final native acceptance passed 270 OpenGL checks and 251 Vulkan checks at
900x600 logical pixels and measured DPR 1, as requested in the scope update.
Earlier passing tests were retained; only affected or failed checks were repeated.
The normal production build passed and excludes test hooks and prototype resources.
The original build cache was restored. Standards and specification reviews have
no remaining findings.

See the [completed ticket map](../stream-interface-implementation/map.md),
[acceptance record](../stream-interface-implementation/evidence/ticket05/acceptance.md),
[standards review](../stream-interface-implementation/review/standards-final.md),
and [specification review](../stream-interface-implementation/review/spec-final.md).

Live media, real network degradation, console power effects, physical controllers,
multiple displays, and live display-scale changes remain unverified. Final Tool
flyout resize/fullscreen behavior is not inferred from earlier host results.
No release package, push, or pull request was created.
