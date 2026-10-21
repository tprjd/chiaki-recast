# Connection progress and console login PIN: guided workspace

Status: resolved
Date: 2026-10-20
Selected design: A, Guided workspace, for all screens and states
Test approach: confirmed by the user on 2026-10-20
Completion: [Integrated connection and PIN acceptance](acceptance.md)

## Problem Statement

Connection progress, auto-connect waiting, and console login PIN entry still use the earlier interface. They do not match the completed console library, Settings, or console setup screens. Progress often appears as a spinner on a black background. Errors disappear after a short delay, which gives users little time to read them or recover.

The app has separate saved-PIN and live-PIN flows. Some labels confuse the four-digit console login PIN with the eight-digit Remote Play pairing code. Live PIN prompts appear through different connection routes and can use a separate native window. Their validation, cancellation, privacy, and focus behavior must agree.

The user selected prototype A for local and PSN connection progress, auto-connect and console wake-up, cancellation, errors, and both PIN flows. The selection includes all displayed progress and result states.

## Solution

Apply A's guided workspace throughout this group. Use a bounded status or form panel on the left and persistent, task-specific help on the right. Reuse the visual language and controls of console setup. Keep the selected console and connection route visible when that information is available.

Show progress from actual backend events. Show persistent errors with useful details, a safe retry action, and a route back to the console library. Keep cancellation visible until the active operation has stopped. Continue into the stream as soon as the existing readiness conditions are met, without an extra success confirmation.

Use the term console login PIN for the four-digit sign-in value. Keep saving a PIN separate from submitting one to the current session. Mask PIN entry by default, allow explicit reveal outside streamer mode, and show incorrect-PIN feedback when the console rejects a PIN.

Support a minimum application width of 900 logical pixels. Verify 900 × 600 and 1280 × 720. Vertical scrolling is acceptable when necessary; all fields, help, and actions must remain reachable. Use A's content and controls in the separate PIN window, adapted to its compact window size.

## User Stories

1. As a user, I want connection screens to match console setup, so that I can use familiar controls.
2. As a user, I want the selected console identified during connection, so that I can confirm the target.
3. As a user, I want local and PSN routes identified, so that I understand how the app is connecting.
4. As a user, I want guidance beside the current task, so that I can resolve common problems without leaving the screen.
5. As a user, I want local connection progress, so that I know the app has started my request.
6. As a user, I want video waiting distinguished from session connection, so that I understand why a connected session has no picture yet.
7. As a user with video disabled, I want connection to finish without waiting for video, so that my existing session mode still works.
8. As a user, I want the stream to start automatically when ready, so that a result screen does not delay play.
9. As a remote user, I want internet waiting shown, so that I know why PSN connection has not advanced.
10. As a remote user, I want console linking and data connection shown as separate stages, so that progress remains understandable.
11. As a user, I want progress to reflect actual events, so that percentages or countdowns do not mislead me.
12. As a user of automatic registration, I want its progress and result preserved, so that the existing PSN registration route still works.
13. As a user with auto-connect enabled, I want to see which console the app is waiting for, so that startup does not appear frozen.
14. As a user connecting to a sleeping console, I want wake-up waiting shown, so that I know the console needs time to become available.
15. As a user, I want a PSN fallback identified when it actually starts, so that the route change is clear.
16. As a user, I want a clear wake timeout, so that I can check the console and its network settings.
17. As a user, I want a visible Cancel action, so that I do not need to remember a shortcut.
18. As a keyboard or controller user, I want the existing cancel inputs to remain available, so that I can stop without a mouse.
19. As a user, I want a reason when cancellation is temporarily unavailable, so that I understand the disabled action.
20. As a user, I want cancellation progress to reflect the operation stopping, so that the app does not report a false result.
21. As a user, I want repeated Cancel activation to have no additional effect, so that one attempt stops once.
22. As a user, I want a cancelled result and a route to the library, so that I can select another console.
23. As a user, I want connection errors to remain visible, so that I have time to read them.
24. As a user, I want relevant diagnostic details available, so that I can understand or report a failure.
25. As a user, I want to retry the same intended console after a failure, so that I do not need to start from the library each time.
26. As a user, I want retry to wait for the earlier attempt to stop, so that two attempts cannot compete.
27. As a user, I want removed consoles and changed profiles to invalidate old actions, so that I do not connect to or change the wrong console.
28. As a user, I want late results from an old attempt ignored, so that they cannot replace the current screen or session.
29. As a user, I want the console login PIN explained separately from the pairing code, so that I enter the correct value.
30. As a user, I want to save a four-digit console login PIN from the existing library action, so that the app can sign in automatically later.
31. As a user, I want a confirmed saved-PIN result, so that I know the value was accepted for the intended console.
32. As a user, I want saving a PIN to leave connection state unchanged, so that saving does not start Remote Play.
33. As a user, I want incomplete or invalid PINs to prevent submission, so that I can correct them first.
34. As a user, I want leading zeros preserved, so that the app sends the PIN exactly as entered.
35. As a user, I want PIN entry masked by default with a Show action, so that I can choose when it is visible.
36. As a streamer, I want private console values and PINs hidden, so that the connection screens do not expose them.
37. As a user, I want a live PIN prompt when the console needs sign-in, so that I can continue the session.
38. As a user, I want an incorrect-PIN message and a new entry attempt, so that I can recover from a rejected or outdated saved PIN.
39. As a user, I want live PIN submission to apply only to the current session, so that it does not silently overwrite my saved PIN.
40. As a user, I want Cancel, Escape, and window close to cancel a live PIN session consistently, so that a hidden prompt cannot leave the connection waiting.
41. As an OpenGL user, I want the separate PIN window to have the same content and behavior, so that renderer choice does not change the task.
42. As a user, I want focus placed in the PIN field and restored after the task, so that keyboard navigation remains predictable.
43. As a user, I want long names, translated text, and larger display scales to remain readable, so that actions do not disappear or overlap.
44. As a user, I want active-stream controls to continue working, so that a connection-screen change does not disrupt play.
45. As a user, I want PINs excluded from logs and diagnostics, so that routine troubleshooting does not expose them.

## Implementation Decisions

- Use A for every agreed screen and state. The primary status or form panel, guidance column, typography, dark colors, focus treatment, and actions must match the selected prototype. Adapt the separate PIN window to its smaller space without introducing a different design.
- Reuse ConsoleButton, SetupButton, existing dialog and navigation conventions, console identity resolution, privacy rules, and translation support. Add shared presentation only where these screens need the same content. Do not create a general workflow framework.
- Update the connection presentation owned by AutoConnectView, PsnView, and the loading/error portion of StreamView. Update ConsolePinDialog and both live PIN presentations. Update Main routing only where needed to retain the active attempt, present errors, and manage transitions.
- Keep the active stream, stream menu, stop confirmation, renderer selection, and existing session settings behavior intact. A connection is ready when the existing video readiness condition is met, or when the session connects with video disabled. A completed PSN data connection alone is not video readiness.
- Retain the existing backend selection and wake policies, including local discovery and eligible PSN fallback. Do not alter transport timeouts or authentication policy to imitate prototype timing. Auto-connect at startup and an explicit wake-and-connect action can use the same wait presentation without being described as the same user action.
- Identify a target with the existing stable, profile-scoped console identity where available. Preserve the actual connection context for entry routes that do not originate from a library row. Never retry using a stale row index. Resolve and validate the intended target again before retry or saving a PIN.
- Bind progress to existing backend states and session events. Add the smallest missing notification needed to expose a true state; do not infer completed work from elapsed UI time. No percentage bar, fabricated step, or countdown is required.

| Flow | Required states and transition behavior |
| --- | --- |
| Local connection | Connecting, waiting for video, ready, failure, cancelling, cancelled. Ready continues to the stream automatically. |
| PSN connection | Waiting for internet, initiating connection, linking console, automatic registration when applicable, registration completed, data connection start, data connection completed/video waiting, ready, start failure, general failure, console unreachable, cancelling, cancelled. Registration is a conditional branch, not a mandatory step in every connection. |
| Auto-connect and wake | Waiting for the selected console, waiting for a requested wake-up, actual PSN fallback, console found/connection handoff, timeout, cancelling, cancelled. Finding a console continues through the applicable connection route. |
| Saved console login PIN | Editable form and confirmed saved result. Invalid input cannot submit. An invalidated target or rejected save remains actionable and cannot show success. |
| Live console login PIN | Initial entry, incorrect PIN, submitting/waiting, accepted/connection handoff, cancelling, cancelled. A repeated native request updates the current prompt rather than opening another one. |

- All preview result states have a native meaning. Ready, signed-in, and console-found states may be brief during automatic handoff; they must not hold up the stream. A saved-PIN result has Done. Automatic-registration completion and cancelled results have Back to consoles. No prototype switcher or Next state control ships in the app.
- Errors in this connection scope remain visible until the user chooses retry or returns to the library. Remove automatic dismissal for these errors. Preserve useful error titles and details, including start failures currently sent through the root error presentation. Unrelated root errors keep their existing behavior.
- Retry starts one fresh attempt for the same valid target and route intent after the previous attempt has ended. Disable retry while cleanup is pending. If the target or profile is no longer valid, explain that the console must be selected again and provide Back to consoles. Registration retry must preserve its registration-only intent.
- Cancellation dispatches the applicable existing native stop operation once. Preserve genuine backend restrictions, including temporarily non-cancellable PSN stages, and explain them beside the disabled action. Show cancelling until stop completion or the existing equivalent cleanup condition is observed. Do not report cancelled merely because a presentation timer elapsed.
- Cancellation must not send a sleep command to the console or clear the saved auto-connect preference. Suppress later connection or wake events from the cancelled attempt. Scope error, PIN, completion, and cancellation events to the active session/attempt so that a late event cannot affect a replacement attempt, a closed view, or another profile.
- Preserve the existing navigation and controller cancel routes where applicable. Escape and the Cancel button use the same cancellation decision. Existing right-click and Ctrl+Q behavior must not bypass a disabled cancellation state. Return/Enter activates only the enabled, focused action.
- Saved PIN entry accepts exactly four ASCII digits as text. Keep leading zeros. Reuse the existing guarded management callbacks and resolve the current target at acceptance. Show success only after a confirmed save. If the callback currently provides no outcome, add the smallest outcome needed to distinguish success from an invalidated target. Cancel and repeated acceptance cannot save.
- Live PIN entry uses the same four-digit validation. Forward the existing incorrect-PIN flag from the native session to the QML prompt. An incorrect saved PIN must lead to a visible correction prompt. Clear rejected/transient input for a new prompt, retain retry context, and prevent duplicate submissions while waiting for a response.
- Live submission does not save the PIN. A saved PIN can still be tried automatically through the existing native behavior. Session progress or a confirmed native result controls successful continuation; elapsed time does not prove PIN acceptance.
- Mask PIN entry by default. Show/Hide is available outside streamer mode. Streamer mode always masks PINs and hides private console names, addresses, and identifiers according to the existing library rules, including details text. Entering streamer mode immediately hides an already revealed PIN.
- Remove the live-PIN value from the current log output. PINs must not appear in new logs, error details, test reports, URLs, or screenshots. Keep the existing saved-PIN storage model; this work does not introduce a credential-storage migration.
- Use the same live-PIN content in the in-window presentation and the existing separate OpenGL window. Preserve native input capture and release. Cancel, Escape, and window close must terminate the pending session consistently. Clear transient PIN text on completion, cancellation, or replacement; close obsolete prompts when the session disappears.
- Prevent background commands from operating through an active PIN prompt. Reuse the established handling for the native render-control overlay and separate-window geometry. Keep the separate window positioned relative to its host after moves and display-scale changes.
- Restore focus to a valid opener or library control after leaving the task. During progress, do not focus unavailable actions. A pending live PIN starts with the PIN field focused. Avoid duplicate input capture when a request repeats or moves between PSN and stream presentations.
- Preserve plain text treatment for user and backend values. Wrap long names, labels, messages, and button text. Make busy, success, and error states distinguishable without color alone. Keep keyboard and controller focus visible.
- No settings schema change, transport redesign, or new external dependency is planned. Only the backend/QML contract changes required for accurate state, safe retry, PIN feedback, and save outcomes are in scope.

## Testing Decisions

- The user confirmed this approach: reuse the existing Qt Quick runner to load production connection and PIN views with a fake backend, then check the native application with isolated settings. Real PSN, console wake-up, and streaming remain separate hardware checks.
- Prefer one screen-level test boundary: supply backend/session state and navigation commands to the production views, and observe rendered content, focus, and dispatched actions. Follow the existing registration and manual-host harness pattern. Do not reproduce the screen logic in a test-only implementation.
- Test the production connection presentation within its real route composition where practical. Keep the real shared controls and native defaults. If StreamView's renderer dependencies prevent direct screen loading, use a narrow production connection/PIN component through the same host contract and cover its StreamView wiring in native acceptance. Do not fake the entire streaming renderer to test a form.
- Good tests assert externally visible behavior and effects: the displayed state, available action, destination, exact command count, intended console, saved value, and focus. They do not assert incidental object structure, private helper names, animation frame timing, or the number of shared components.
- Cover every state in the implementation table, long labels and error details, missing console metadata, privacy changes, and video-disabled sessions. Verify that PSN data completion does not falsely end video waiting.
- Drive cancellation before and after allowable states, repeated activation, timeout during cancellation, late success/error/PIN events, view removal, profile changes, target reorder/removal, and retry before old cleanup completes. Assert one command per accepted action and no command to a stale target or session.
- Cover saved PIN entry, leading zeros, non-digit and short input, repeated save, invalid target, cancelled dialog, and save outcome. Verify a known synthetic PIN reaches only the intended isolated record. Live PIN submission must not alter saved PINs.
- Cover live initial request, automatic saved-PIN rejection, incorrect input response, repeated requests, submit waiting, session removal, Escape, Cancel, and separate-window close. Verify consistent stop behavior and input/focus release.
- Cover masked input, explicit reveal, streamer mode entered while revealed, hidden identity/details, and absence of PIN values from affected logs and diagnostics. Use synthetic values only.
- Add focused native contract checks where QML fakes cannot prove correctness: incorrect-PIN propagation, attempt/session ownership, actual retry/cancellation dispatch, and saved-PIN persistence. Prefer existing native handlers with isolated test state over a new broad testing layer.
- Native acceptance uses the normal production QML and controls at 900 × 600 and 1280 × 720 logical pixels. Check measured DPR 1, 1.5, and 2, expanded labels, keyboard navigation, mouse actions, cancellation, errors, and return focus. Include the separate OpenGL PIN window and in-window PIN route. Check that controls and popups remain reachable and correctly placed.
- Use isolated settings, synthetic consoles, disabled discovery, and controlled backend events for native checks. Verify saved state after process exit where persistence is relevant. Do not use the user's real account or saved console records for automated tests.
- Build the normal application after checks. Confirm that test fixtures, browser prototypes, captures, and QA helpers are absent from production resources and the shipped executable. Run relevant existing console, setup, and Settings regressions because shared controls and routing are affected.
- Report native acceptance separately from Qt Quick results. A successful build, browser prototype, or fake-backend test does not prove real wake-up, PSN authentication, PIN acceptance by hardware, live video, or physical controller behavior.

## Out of Scope

- Redesigning the active stream, stream menu, stream statistics, disconnect/stop confirmation, or unrelated in-session overlays.
- Redesigning PSN sign-in, token entry, credential expiry, or account-management screens. Existing routes remain usable.
- Changing console setup registration/manual-host forms, except preserving their existing navigation and shared-control behavior.
- New connection protocols, automatic retry loops, timeout policy, renderer selection, or streaming-quality settings.
- PIN removal, existing-PIN reveal from storage, new PIN storage formats, or automatic saving of a live-entered PIN.
- Steam, reminder, About, and unrelated confirmation dialogs.
- Mobile/Android UI, widths below 900 logical pixels, and new dependencies.
- A requirement to reproduce prototype controls or simulated transitions in production.
- Treating hardware-dependent checks as passed without the required console, account, and physical controller.

## Further Notes

- Design approval: the user selected A for all these screens and states on 2026-10-20. No additional design interview is required for this spec.
- Test approval: the user answered "Yes, use this approach" to the production-QML fake-backend and isolated native acceptance proposal on 2026-10-20.
- The selected prototype is preserved on branch `codex/prototype-connection-flows`. Its implementation commit is `7a0111b6`. The prototype source is `gui/prototype/connection/`, with A selected by the `variant=A` parameter. The running preview is [layout A](http://127.0.0.1:8766/prototype/connection/?variant=A&screen=local&state=connecting). The branch is the durable source; the localhost server is temporary.
- The prototype contains 33 selectable states across the five flows. It intentionally does not connect to a console or validate native timing. It also labels persistent errors/retry, masked PIN defaults, and incorrect-PIN feedback as behavior proposals. The selected design incorporates these proposals subject to the native correctness rules above.
- Production implementation starts from the completed console-setup integration baseline, `fd21bfafc5720ac41b9f6b5f72169cb670f8fb27`, on `codex/console-setup-integration`. Keep prototype alternatives off the production branch.
- Existing screen-test guidance is in `gui/tests/README.md`. Prior art includes the registration and manual-host screen harnesses and the completed console-setup acceptance records under `.scratch/console-setup-refresh/`.
- This publication creates a specification, not implementation tickets or completed production changes.
