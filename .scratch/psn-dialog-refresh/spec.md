# PSN sign-in and remaining dialogs

Status: ready-for-agent
Date: 2026-10-21
Design: use the new Library, Settings, console setup, connection, and PIN design
Test approach: confirmed by the user on 2026-10-21

## Problem Statement

PSN sign-in and the remaining app dialogs use the old interface. They do not match the screens already refreshed. Instructions do not clearly distinguish PSN sign-in, public Account-ID lookup, and the sign-in redirect URL copied from a browser.

Several dialogs use Yes or No for actions with different effects. No can disconnect a session. Escape can start manual registration or permanently disable a reminder. Operation logs can replace useful progress and result screens. Closing PSN progress looks like cancellation even though the request can continue and save credentials.

Users need consistent screens, clear actions, visible recovery options, and accurate information about what continues after they close a screen.

## Solution

Apply the new design to PSN sign-in, sign-in redirect URL entry, public Account-ID lookup, Steam shortcut creation and results, startup reminders, confirmations, automatic-registration choices, renderer notices, About, and both disconnect confirmations.

Use the existing refreshed controls, spacing, colors, typography, and focus treatment. Use guided forms for tasks and compact dialogs for short decisions or information. Keep keyboard and controller operation and the minimum application width of 900 logical pixels.

Keep the current PSN authentication method, storage model, and platform browser defaults. Improve instructions, validation, progress, results, and recovery. Hide sensitive app-owned values by default. Use explicit action names and Cancel as the initial selection in confirmation dialogs.

Before submission, Cancel leaves a form. After PSN submission, Close dismisses the screen while the request can continue and save credentials. Explain this distinction. Steam writes finish before the result can close. Success requires a final successful result, not an intermediate progress message.

## User Stories

1. As a user, I want these screens to use the new app design, so that I can use familiar controls.
2. As a user, I want long tasks to use clear forms and short decisions to use compact dialogs, so that I can identify the next action.
3. As a user, I want all fields and actions reachable at 900 by 600 logical pixels, so that I can use the app in a small window.
4. As a keyboard user, I want visible focus and predictable navigation, so that I can complete each task without a mouse.
5. As a controller user, I want the existing navigation and action inputs retained, so that I can complete each task with a controller.
6. As a user, I want focus to return to the control that opened a dialog, so that I can continue where I left off.
7. As a user, I want long names and translated text to remain readable, so that I do not lose instructions or actions.
8. As a user, I want PSN sign-in distinguished from public Account-ID lookup, so that I can choose the correct task.
9. As a user, I want sign-in to explain that it saves credentials for remote access, so that I understand its effect.
10. As a user, I want public lookup to identify its third-party service, so that I know where my PSN username is sent.
11. As a user, I want public lookup to explain its public-search requirement, so that I can understand a lookup failure.
12. As a user, I want public lookup to return an Account ID without signing me in, so that I can use the existing registration alternative.
13. As a user, I want successful Account-ID retrieval to return to registration, so that I can continue without an extra confirmation.
14. As a Windows user, I want embedded PSN sign-in retained, so that the current default route remains available.
15. As a Linux or macOS user, I want external-browser sign-in retained, so that the current platform route continues to work.
16. As a user, I want a clear Use external browser action, so that I can leave the embedded browser when needed.
17. As a user, I want an external-browser fallback when the embedded browser is unavailable, so that I can still sign in.
18. As a user, I want browser reload and cookie-clearing recovery retained, so that I can recover from an unusable embedded sign-in page.
19. As a user, I want instructions to identify the complete Sign-in redirect URL, so that I do not paste a raw token or an unrelated address.
20. As a user, I want Paste and sign-in-link copy actions, so that I can transfer the required URLs without retyping them.
21. As a user, I want empty and invalid redirect URLs rejected with a useful message, so that I can correct my input before continuing.
22. As a user, I want the sign-in redirect URL hidden by default, so that its authorization code is not exposed on screen.
23. As a user, I want an explicit Show/Hide action, so that I can inspect the pasted value when needed.
24. As a streamer, I want reveal disabled and an already revealed value hidden, so that streamer mode protects app-owned sensitive fields.
25. As a user, I want authorization codes and tokens excluded from diagnostics, so that troubleshooting does not expose credentials.
26. As a user, I want progress to reflect the actual operation, so that the app does not report invented completion.
27. As a user, I want repeated submission prevented while a request is active, so that one action cannot start competing requests.
28. As a user, I want PSN setup success shown only after the operation succeeds, so that the result is reliable.
29. As a user, I want sign-in failures to remain visible, so that I have time to read them.
30. As a user, I want Retry and a route to correct my input, so that I can recover from a sign-in failure.
31. As a user, I want expired credentials explained as a reason to sign in again, so that I understand why the app opened the screen.
32. As a user, I want technical details available but collapsed by default, so that I can inspect a failure without a log replacing the main task.
33. As a user, I want Back or Cancel to leave an unsubmitted form, so that I can stop before starting authentication.
34. As a user, I want Close to explain that submitted PSN sign-in continues, so that I do not mistake dismissal for cancellation.
35. As a user, I want a closed request prevented from changing a newer screen, so that late results do not interrupt my current task.
36. As a user, I want registration to retain its original context through Account-ID retrieval, so that the result fills the intended form.
37. As a user, I want the same result and reminder behavior for embedded and external sign-in, so that browser choice does not change setup completion.
38. As a user, I want Steam shortcut defaults to retain the selected profile, so that the shortcut starts the intended configuration.
39. As a user, I want optional launch options and a custom Steam folder retained, so that existing shortcut setups remain possible.
40. As a user, I want an empty Steam shortcut name prevented, so that I can correct it before creation.
41. As a user, I want the existing shortcut create-or-update behavior retained, so that a visual refresh does not alter my Steam setup policy.
42. As a user, I want Steam progress followed by a clear final result, so that I know whether the operation succeeded.
43. As a user, I want an intermediate successful step followed by failure reported as failure, so that the app does not show a false success.
44. As a user, I want Steam failure details and a retry route, so that I can correct the form or folder and try again.
45. As a user, I want the Steam restart instruction included in the success result, so that I know how to make the shortcut appear.
46. As a user, I want to restart Steam myself, so that shortcut creation does not interrupt another Steam task.
47. As a user, I want the app to avoid offering cancellation for a Steam write it cannot stop, so that each action describes its actual effect.
48. As a user, I want Set up now, Later, and Don't ask again as separate reminder choices, so that I can control future prompts.
49. As a user, I want Escape to mean Later on a reminder, so that dismissal does not permanently disable it.
50. As a user, I want the existing Steam-then-PSN reminder order retained, so that startup setup remains predictable.
51. As a user, I want confirmations to name Delete console, Hide console, Unhide console, or Quit, so that I know what acceptance does.
52. As a user, I want Cancel selected when a confirmation opens, so that an accidental Enter does not perform the requested action.
53. As a user, I want Escape to cancel a confirmation without changes, so that I can safely return to the previous screen.
54. As a user, I want changed or removed console targets checked again at acceptance, so that an old dialog cannot affect the wrong console.
55. As a user, I want Automatic registration, Manual registration, and Cancel as distinct choices, so that cancellation cannot choose a method.
56. As a PS4 user, I want the existing automatic-registration eligibility guidance retained, so that I can select a suitable method.
57. As a user, I want Disconnect and sleep, Disconnect, and Cancel as distinct choices, so that I can control the console power state.
58. As a user, I want Cancel and Escape to leave the stream running, so that opening the disconnect dialog does not end play.
59. As a user, I want the same disconnect behavior in both renderer routes, so that a separate window does not change the available actions.
60. As a user, I want renderer fallback explained with its reason, so that I know why the app switched renderers.
61. As a user, I want About to retain the version, attribution, and license information, so that the redesign preserves that information.
62. As a user, I want simple information dialogs to have one Close action, so that I can dismiss them without an unnecessary decision.
63. As a user, I want completed Library, Settings, connection, and PIN tasks to continue working, so that this refresh does not undo earlier work.

## Implementation Decisions

- Reuse the new visual language and production controls from the completed refresh work. Do not reproduce the old PSN and Material dialog layouts. Forms may use the existing guided workspace; brief confirmations and information remain compact. Preserve translation support, readable wrapping, scrolling, focus indication, keyboard input, and controller mappings.
- Modify PSNLoginDialog, PSNTokenDialog, SteamShortcutDialog, RemindDialog, ConfirmDialog, the renderer information dialog, About, and both StreamView disconnect presentations. Update their Main, MainView, Settings, and registration callers where routing, action labels, focus, or callback contracts require it. Keep the existing guarded console-management callbacks.
- Use the current backend, navigation, and external-action injection pattern where these screens need test inputs. Prefer existing shared controls and presentations. Add shared content only when it prevents real duplication, such as the two disconnect presentations. Do not add a general dialog or workflow framework.
- Support a minimum application width of 900 logical pixels. Validate full-window content at 900 by 600 and 1280 by 720. Compact separate windows adapt to their own size. Every field, explanatory message, and action remains reachable with long text and larger display scales.

### PSN routes and entry

- Retain the entry routes from registration, Settings, startup reminders, and credential expiry. Registration keeps its caller context and receives its result only while that context remains valid. Distinguish initial PSN setup from expired credentials in the title and explanation.
- Preserve embedded-browser default behavior on Windows and external-browser default behavior on Linux and macOS. Keep an explicit Use external browser action and automatic fallback when the embedded browser cannot be created. Preserve reload, cookie clearing, and browser cleanup behavior. Reuse the current PSN authorization URLs and authentication process.
- Label the input Sign-in redirect URL. Explain that the user must copy the complete final browser URL after sign-in. Retain explicit link-copy and redirect-paste actions. Do not silently read the clipboard or submit pasted text.
- Reject empty input and invalid redirect input with actionable feedback. Keep validation at the native authentication boundary; presentation validation must not replace it. Retain the expected redirect destination and required authorization code. A rejected URL remains editable. An expired or used code must have a route to fresh sign-in instead of an endless retry of the same code.
- Keep public Account-ID lookup as a separate route. Explain the third-party lookup service and public-search requirement. It retrieves the Account ID without storing PSN authentication credentials. Network failures, malformed responses, and missing Account IDs must produce a recoverable result rather than leave submission disabled indefinitely.
- PSN sign-in continues to save authentication credentials and the Account ID through the current storage model. Explain that effect even when sign-in was opened to obtain an Account ID for registration. Do not introduce an account-management flow or credential-storage migration.

### PSN state, privacy, and closure

| Situation | Required behavior |
| --- | --- |
| Before submission | Show the form or browser. Back or Cancel leaves without starting a request. |
| Invalid input | Explain the problem beside the task and permit correction. Do not show success. |
| Request active | Show actual progress, prevent duplicate submission, and collapse technical details by default. |
| Setup succeeds | Show a clear success result and a completion action. Update setup reminder state consistently for both browser routes. |
| Account-ID retrieval succeeds | Return directly to the valid registration caller and fill the Account ID. |
| Request fails | Keep the error visible with useful recovery actions and optional sanitized details. |
| Credentials expired | Explain why sign-in is needed again and use the same authentication flow. |
| Submitted PSN screen closes | Explain that sign-in can continue and save credentials. Dismiss the screen without claiming cancellation. |
| Late result arrives | Allow the agreed request completion but do not navigate, focus, or update a replacement screen through a stale callback. |

- Only final operation results establish success or failure. Reset per-attempt presentation and details when a new attempt starts. Ignore repeated final results and prevent simultaneous submissions, including after a progress screen closes and the user returns through another entry route.
- Keep asynchronous results associated with their originating request and profile context. Closing the screen allows the original request to finish; it does not authorize applying its result to a different profile or a replacement registration form. Reuse the existing context and generation patterns for invalidation. Do not implement cancellation or rollback as part of this refresh.
- After submission, the dismiss action is Close, not Cancel. Escape or equivalent back input must not imply that the backend request was cancelled. Screen dismissal is not a promise that authentication survives application exit.
- Hide the redirect URL by default. Provide Paste and Show/Hide controls. Streamer mode prevents reveal and immediately hides an already revealed value. Retain existing privacy handling for usernames, Account IDs, console identity, and user paths where applicable.
- Exclude authorization codes and tokens from app diagnostics, copied details, logs introduced or exposed by this flow, and retained test evidence. Sanitize error text that can contain sensitive URLs before display. The embedded Sony page and external browser are outside the app-owned field-masking contract.
- Correct the existing expiry-route argument mismatch and embedded/external reminder-completion inconsistency as part of the flow update. Preserve the existing credential-expiry policy.

### Steam shortcut creation

- Preserve the default shortcut name, active-profile launch options, optional custom launch options, and optional custom Steam folder. Keep existing file pickers. Require a nonempty trimmed name and preserve the existing native create-or-update behavior, artwork, and controller-configuration effects.
- Present the form, operation state, and terminal result as one coherent task. Do not offer Cancel for the synchronous write operation. Do not invent cancellability, rollback, percentages, or intermediate completion.
- Treat only the terminal successful callback as success. Intermediate successful messages must not dismiss the reminder or trigger the restart instruction if the final result fails. Clear per-attempt details and outcome before retry.
- Keep failures visible with useful details and a route to edit or retry. Do not claim that a failed operation rolled back earlier writes. Preserve native validation and error handling.
- Include the manual Steam restart instruction in the success result. Do not restart Steam. Preserve the existing completion route and any eligible next reminder without stacking duplicate dialogs.

### Reminders and confirmations

- Reminders offer Set up now, Later, and Don't ask again. Set up now opens the relevant setup task. Later dismisses without changing the reminder preference. Don't ask again disables that reminder through its existing preference. Escape and equivalent dismissal mean Later. Preserve the current reminder order and eligibility rules.
- Replace ambiguous Yes and No labels with the actual action. Confirmation dialogs initially focus Cancel. Enter activates only the enabled focused action. Escape cancels without accepting or selecting an alternative operation.
- Preserve existing console target resolution, eligibility checks, profile invalidation, and single-acceptance guards. A stale confirmation cannot affect a replacement console or dispatch twice.
- Automatic-registration choice offers Automatic registration, Manual registration, and Cancel. Only an explicit method action starts that method. Cancel and Escape start neither. Preserve the existing PS4 eligibility explanation and registration behavior.
- Both disconnect presentations offer Disconnect and sleep, Disconnect, and Cancel. Cancel initially has focus. Disconnect and sleep sends the existing stop-with-sleep operation. Disconnect sends stop without sleep. Cancel, Escape, and separate-window close keep the stream running. Neither route changes the saved disconnect preference.
- Retain the existing renderer-dependent choice between in-window and separate-window disconnect presentation. Preserve native input capture and release, placement, and focus restoration. The active-stream menu and stream transport remain unchanged.

### Information dialogs

- Keep renderer fallback notices informational. Preserve the fallback reason and one Close action. Do not imply that the notice asks the user to choose a renderer.
- Preserve About's logo, application name, version, original Chiaki attribution, and license information. Permit scrolling when needed. Provide one Close action and restore focus to the opener.
- Dismissal must not activate controls behind a dialog. If the original opener no longer exists, return focus to a valid control in the current view.

## Testing Decisions

- The user confirmed this scope: production QML screen tests with supplied backend state; focused native checks with isolated settings for PSN saves, late results, and Steam writes; visible native checks for focus, both disconnect dialogs, and layout at 900 by 600 and 1280 by 720.
- Use the production-screen boundary as the main test seam. Drive visible controls and supplied request outcomes, then observe visible results, focus, navigation, dispatched commands, and setting writes. Test behavior rather than private property names, component structure, or source-text patterns.
- Reuse the existing Qt Quick screen-test target, production resources, controls, and test-only backend import setup. RegistrationHarness, SettingsHarness, ConnectionHarness, and LiveConsolePinHarness provide prior art. RegistrationResults already covers progress, final results, retained inputs, late callbacks, and return navigation. ConsoleManagement covers guarded target acceptance. LiveConsolePin covers two presentation routes, privacy, focus, and compact-window closure.
- Extend that same screen-boundary approach for PSN and the remaining dialogs. Use the smallest top-level backend, navigation, browser-launch, or lookup substitution needed. Do not create separate mocking layers for each control or test copies of the production screens. Where the existing seam cannot observe persistence, use focused native contract checks instead of asserting only that a fake recorded a call.
- Cover every entry route, browser fallback selection, the sign-in redirect URL label, explicit copy and paste actions, invalid input, missing authorization code, public lookup failure, malformed lookup responses, successful registration return, setup success, expiry messaging, and parity between embedded and external completion.
- Check hidden-by-default input, explicit reveal, streamer-mode transitions, and sanitized details. Use synthetic credentials and assert their absence from outputs without printing their values on test failure.
- Exercise final failure after intermediate success, repeated submission, repeated final events, close during PSN work, reopening while work is active, late callbacks after dismissal or destruction, retry, replacement views, changed profiles, and invalid registration contexts. Verify permitted background completion and the absence of navigation or writes into the wrong context.
- Exercise Steam name validation, existing defaults, optional inputs, final success, partial progress followed by failure, retry, reminder changes only after success, manual restart wording, and the absence of a false Cancel action. Use a temporary Steam directory and synthetic shortcut records for native writes. Verify actual written results and the existing matching-entry update behavior without modifying the user's Steam installation.
- Check all reminder choices and Escape separately. Verify the correct preference changes, prompt order, and focus restoration. Check explicit confirmation labels, Cancel as initial focus, Enter on the focused control, Escape, stale targets, and duplicate activation.
- Test automatic registration, manual registration, and cancellation as three separate outcomes. Test disconnect with sleep, disconnect without sleep, and cancellation in both presentations. Observe the native command effect through isolated session controls; do not treat a fake callback as proof of a real console sleeping.
- Verify About content, renderer fallback reasons, close behavior, long text, translated-length stress text, and reachable actions. Test keyboard navigation and existing controller-key mappings. Physical-controller testing remains a separate hardware check.
- Build the normal application and verify that production resources include the changed screens and exclude test fixtures. Run the focused screen suites and relevant existing Library, Settings, registration, connection, and PIN regression checks.
- Perform visible native checks of real focus, input capture and release, window closure, both renderer-dependent disconnect routes, supported sizes, and display scaling. Offscreen QML results do not prove native window behavior. Use the previous connection acceptance workflow as prior art for isolated native settings, controlled events, source/build records, and retained captures.
- Keep automated checks isolated from the user's settings, account, Steam installation, and real console. Report controlled PSN responses, intercepted session commands, and temporary Steam writes as controlled checks. Report real PSN sign-in, actual Steam visibility after restart, live console disconnect/sleep, and physical controller results separately, including any checks not performed.

## Out of Scope

- A new visual direction, or reworking the completed Library, Settings, console setup, connection-progress, and PIN screens beyond necessary integration fixes.
- The active-stream menu, unrelated stream overlays, transport, video decoding, controller calibration, and changes to saved disconnect behavior.
- Redesigning the already refreshed Settings input-capture prompts or replacing the existing file pickers.
- New authentication providers, a new PSN protocol, account switching, automatic callback capture, raw-token entry, credential-storage migration, and a new cancellation or rollback mechanism for authentication.
- Replacing the public lookup service or changing its privacy requirements.
- Changing Steam matching rules, adding a Steam restart command, or adding a general transaction/rollback system for Steam writes.
- A generic dialog framework, new dependencies for presentation helpers, unrelated cleanup, or a new cross-application notification system for background sign-in completion.
- Claiming real external-service or hardware acceptance from simulated events.

## Further Notes

The twelve design decisions came from the grill-with-docs session. The user then requested this specification and confirmed the testing scope. The new design means the completed refresh work, not the legacy screens being replaced.

The sign-in redirect URL term is recorded in the root glossary. It is distinct from a raw token, a console login PIN, and a Remote Play pairing code.

Closing submitted PSN sign-in can still result in saved credentials. This behavior is explicitly accepted and must remain visible in the UI wording. Steam failure can occur after some writes; do not describe failure as rollback.

This specification is stored in the project's local Markdown issue tracker. It does not create implementation tickets, authorize a push, or claim implementation or runtime verification. No new architecture decision record is needed for this work because the design keeps the current authentication, storage, and presentation patterns.
