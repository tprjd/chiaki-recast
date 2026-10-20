# Console setup: guided workspace

Status: resolved
Date: 2026-10-20
Selected design: A, Guided workspace
Test approach: confirmed by the user on 2026-10-20

## Problem Statement

The console library and Settings have a new layout. Console registration and
manual host entry still use the earlier forms. Their spacing, help, and actions
do not match the rest of the application.

Registration currently shows a log dialog. The form does not distinguish a
successful registration from a failed registration when the operation ends.
Users need a clear result and a way to correct their details after a failure.
Manual host entry also needs a clear explanation of the difference between
an address and a console registration.

The user selected prototype A for both screens. The selected scope includes
registration progress, success, and failure, plus manual entry results.

## Solution

Use a guided workspace for both native screens. Put the form in a bounded
panel on the left. Put task instructions in a separate column on the right.
Match the colors, typography, spacing, controls, and navigation of the completed
console library and Settings redesign.

Keep registration and manual host entry as separate screens. Registration
pairs a console with a PSN account. Manual host entry saves an address and
uses an existing registration, or defers registration until the first connection.

Replace the registration form panel with progress or result content for the
current attempt. Keep the task context visible. Show real backend outcomes,
retain useful diagnostics, and let the user correct details after failure.
Manual host entry shows a result after the local save without a simulated
network operation.

Support a minimum application width of 900 logical pixels. Verify the layout
at 900 by 600 and 1280 by 720 logical pixels. Vertical scrolling is acceptable
when all content cannot fit. Every control and action must remain reachable.

## User Stories

1. As a user, I want console setup to match the library and Settings, so that the application feels consistent.
2. As a user, I want registration and manual host entry to remain separate, so that I understand which task I am doing.
3. As a user, I want instructions beside the form, so that I can complete setup without leaving the screen.
4. As a user, I want registration to open from its existing entry points, so that familiar setup routes still work.
5. As a user, I want the selected console address and type to appear when available, so that I do not enter known details again.
6. As a PS5 user, I want a PS5 registration option and matching instructions, so that I can find the Remote Play PIN.
7. As a PS4 user, I want all existing firmware choices, so that I can register a supported older console.
8. As a user, I want to enter an IP address or hostname, so that I can reach my console through its existing address.
9. As a user, I want local broadcast registration to remain available, so that the existing local setup route still works.
10. As a user, I want the account field appropriate to my console firmware, so that I enter the correct PSN identity.
11. As a user, I want an available saved PSN Account ID to fill the form, so that I do not have to retrieve it again.
12. As a user, I want to use the existing PSN sign-in and public lookup routes, so that I can obtain an Account ID.
13. As a user, I want my other entries to remain after account lookup, so that I can continue registration without starting again.
14. As a user, I want the eight-digit Remote Play PIN explained, so that I know which code to enter.
15. As a user, I want the optional four-digit console PIN explained separately, so that I do not confuse the two PINs.
16. As a user, I want incomplete or invalid fields to prevent submission, so that I can correct them before a request starts.
17. As a user, I want leading zeroes in PIN input to remain intact, so that the entered code is not altered by the form.
18. As a user, I want a visible registration progress state, so that I know the application is waiting for the console.
19. As a user, I want repeated activation during progress to have no additional effect, so that one action starts one request.
20. As a user, I want a clear success result, so that I know the console registration completed.
21. As a user, I want a clear failure result based on real information, so that I do not follow an incorrect diagnosis.
22. As a user, I want diagnostic logs available on demand, so that I can inspect a problem without reading logs during normal setup.
23. As a user, I want to return from failure to my entered details, so that I can correct the problem and retry.
24. As a user, I want a retry to use the same intended console, so that it does not create an unrelated manual entry.
25. As a user, I want closing progress to describe its actual effect, so that I do not mistake dismissal for cancellation.
26. As a user, I want an old registration callback to leave a newer screen alone, so that an earlier attempt cannot change my current task.
27. As a user, I want to add a manual address from the library, so that I can reach a console that discovery does not show.
28. As a user, I want to select an existing registered console by a clear identity, so that I associate the address with the right console.
29. As a user without a registration, I want to register on first connection, so that I can save the address before pairing.
30. As a user, I want an empty registered-console list to remain usable, so that first-time setup has no dead end.
31. As a user, I want a changed console list to preserve or invalidate my selection correctly, so that the address is not linked to another console.
32. As a user, I want confirmation after a manual address is added, so that I know the local save completed.
33. As a user, I want an invalid manual selection to keep my entered address, so that I can correct the selection without retyping it.
34. As a user, I want adding an address to leave the console disconnected, so that saving details does not start a session.
35. As a user, I want Back and Cancel to return to the appropriate page, so that nested setup routes remain predictable.
36. As a user, I want to return to the console library after success, so that I can choose when to connect.
37. As a keyboard user, I want visible focus and a complete focus order, so that I can use every field and action without a mouse.
38. As a controller user, I want the existing navigation and activation conventions, so that I can complete setup with a controller.
39. As a user with a small window or a scaled display, I want readable labels and visible focused controls, so that setup remains usable.
40. As a user of translated text, I want labels and help to wrap, so that longer text does not hide controls or actions.
41. As a streamer, I want private values masked throughout setup and results, so that the redesigned screens respect streamer mode.
42. As a user, I want opening, editing, and leaving a form to avoid saving changes, so that only explicit submission changes my console records.

## Implementation Decisions

### Layout and module boundaries

- Use A for both screens. Retain the left form panel and right instruction
  column. Put field labels above controls, with help below them. Separate
  console details, account identity, and PIN entry within the registration form.
- Keep the application context and a clear Back route. Use the existing
  navigation ownership and focus restoration patterns. Do not mount a second
  independent application sidebar or duplicate navigation state.
- The registration panel changes between form, progress, success, and failure.
  Manual entry changes between form, saved result, and recoverable failure.
  Keep the same screen identity through these changes.
- Modify the existing registration and manual host screens, their route
  connections where necessary, and their tests. Reuse the existing controls
  and visual conventions. Share only the layout or result elements that both
  screens actually need.
- Use the existing screen-level backend and navigation injection pattern from
  Settings for testing. Production defaults use the real application objects.
  Test fixtures and prototype controls remain outside production resources.
- The native implementation is the authority for capabilities, validation,
  values, and side effects. The prototype is the visual reference. Rewrite
  the selected design in QML rather than embedding the browser prototype.

### Registration fields and account routes

- Preserve the host and PS5 route inputs. Settings registration continues to
  supply the broadcast address and default console type. Discovery and
  connection routes retain their selected console context.
- Preserve all four console choices: PS5, PS4 firmware before 7.0, PS4 firmware
  from 7.0 to before 8.0, and PS4 firmware 8.0 or later. Preserve their native
  registration targets. Show Online ID only for the oldest PS4 choice.
- For other console choices, use the PSN Account ID in Base64 format. Prefill
  an available saved Account ID when the form is first initialized. Returning
  from a linked screen must not overwrite an intentional edit with a stale value.
- Keep the existing PSN sign-in and public lookup entry points and their
  availability rules. Use their returned Account ID. Preserve the other
  form values, restore useful focus, and keep unsuccessful lookup separate
  from registration failure.
- Trim the host and PSN identity at submission. Preserve hostname support and
  the existing broadcast comparison. Do not restrict the address field to IPv4.
- Require a nonempty address, the applicable PSN identity, an eight-digit
  Remote Play PIN, and either an empty console PIN or four digits. Keep PINs
  as text until the native boundary. Preserve native Account ID validation.
- Show validation feedback near the relevant field. A synchronous native
  rejection leaves the form available and exposes the available error message.
  It must not leave the screen showing progress for a request that did not start.
- Use console-specific help to distinguish the temporary pairing code from
  the optional console login PIN. Do not change how the native backend stores
  or uses those values.

### Registration progress, results, and request lifetime

- Enter progress only for an accepted registration request. Disable repeat
  submission while that attempt is active. Browsing the form and opening
  linked account screens must not start registration.
- Consume the existing callback contract: nonterminal callbacks append log
  text, and terminal callbacks distinguish success from failure. The callback
  currently supplies no progress percentage and no structured failure reason.
- Use indeterminate progress. Do not reproduce the prototype's fixed progress
  fraction or its sample milestones as facts. Show only states that the native
  operation actually reports.
- On success, show the completed registration and the relevant console details.
  The result must agree with the registered-console update and any manual-host
  association. Move completion notification after those updates if necessary.
  Returning to the library must show the updated console without starting a stream.
- On failure, use a neutral registration-failed message with available details.
  The prototype's expired-PIN explanation is a sample, not a diagnosis rule.
  Use specific error text only when the backend provides that information.
- Put diagnostics behind an expandable log section. Support scrolling, keyboard
  focus, and long lines. Do not infer status by parsing translated log messages.
- Back to details preserves the address, console choice, identity, and PIN
  entries. A retry starts a new attempt and a new log. Preserve the original
  target and manual-host association across a failed attempt and retry.
- The backend currently has no registration-cancel command exposed to QML.
  Use Close progress for dismissal and explain that the operation can continue.
  Do not label dismissal Cancel registration or report a canceled outcome.
  Dismissal returns through the existing caller route.
- Scope UI callbacks to the attempt that created them. After a screen closes,
  callbacks must not access destroyed controls, navigate, or change a newer
  screen. An old terminal callback must not replace the result of a retry.
- Capture the initiating console context for the native request rather than
  resolving a mutable global target at completion. Retain the current operation
  lifetime, but prevent overlapping manual registration attempts from sharing
  mutable target state. Use a small request guard if needed, not a new job system.
- Keep profile ownership explicit. A stale form must not submit against another
  profile. Completion must not write into a different profile after dismissal.
  If the initiating profile is no longer active and the existing persistence
  interface cannot safely save to it, reject that completion's settings update
  and report the condition rather than silently changing the active profile.

### Manual host entry and results

- Retain an address field and a registered-console selector. Identify existing
  registrations clearly, including enough information to distinguish equal names.
  Always offer Register on first connection.
- With existing registrations, require a deliberate selection. With no registered
  consoles, the deferred-registration choice remains available and can be selected
  by default, as in the current screen.
- Associate a selected registration with its stable identity. Resolve the current
  native index immediately before submission. If the registration disappears or
  becomes ambiguous, retain the address and show a recoverable selection error.
  Reordering must not select a different console. A profile change invalidates
  the previous selection.
- Represent deferred registration explicitly in the UI. Preserve the backend's
  existing unregistered-host behavior at the call boundary. A stale existing
  selection must not fall through to that behavior.
- Save only on Add console. Disable repeat submission while dispatching the
  local action. Preserve current address and registration storage semantics.
  No schema migration, address reachability check, wake request, or stream start
  is part of this operation.
- Show the saved result after the local backend action completes under the
  existing persistence contract. If registration is deferred, state that pairing
  starts on the first connection. Provide Back to consoles.
- Manual entry has no asynchronous network progress. Do not add an artificial
  wait, percentage, or registration-stage list to its local save.
- Show failure for actual rejected input, an invalidated selection, or a failure
  exposed by the backend. Preserve entered details for correction. Do not invent
  a disk-write result: the current action returns no durable-storage status.
  A small return-status change is permitted only if the implementation needs
  it to report a real failure, with all callers updated.

### Navigation, privacy, and accessibility

- Before submission, Cancel and Back leave without saving. Escape follows the
  same route after any active child popup handles it. Progress dismissal follows
  the request-lifetime rules above. Result actions do not resubmit the form.
- Restore focus to a valid opener when returning to Settings, the library, or
  the registration form. If that opener no longer exists, focus a valid fallback.
- Support mouse, Tab, Shift+Tab, existing directional controller navigation,
  activation, and Back conventions. Keep text-editing keys within their fields.
  Bring the focused control fully into view when the page scrolls.
- Give fields, buttons, and state changes accessible names. Keep a visible focus
  indicator and a useful focus target after each panel change. Do not rely only
  on color to communicate progress, failure, or disabled submission.
- At both target window sizes, keep the form and help readable, and every action
  reachable. Allow long labels, console names, and help text to wrap. Check
  display scaling and expanded translated text without reducing support below
  the agreed 900-pixel minimum width.
- Respect the existing streamer-mode setting in fields, selector labels, help,
  summaries, and results. Mask private addresses and account identifiers, and
  keep console names consistent with the library's privacy behavior. Do not echo
  PINs in result summaries. Hide or sanitize diagnostic content in streamer mode
  so that expanding logs does not bypass masking.
- Localize new user-facing text with the existing translation mechanism. Keep
  the browser-only state selector, sample values, reset controls, and layout
  switcher out of the native application.

## Testing Decisions

The user confirmed this approach on 2026-10-20: test the production QML screens
with a fake backend in the existing Qt Quick runner, then check the native app
with isolated settings. Real PSN authentication and console pairing are separate
hardware checks, not prerequisites for accepting this UI change.

- Use one primary automated seam at the complete screen boundary. Load the
  actual registration and manual entry QML with the production controls. Supply
  controlled settings, backend outcomes, and navigation handlers. Observe
  visible state, focus, dispatched arguments, and action counts.
- Follow the Settings harness and console screen tests. They already support
  production QML, fake collaborators, signal observation, rendering checks,
  privacy cases, stable identities, and invalidation after profile changes.
- Test behavior through user actions. A good test checks what the user sees
  and what command the backend receives. Avoid tests tied to private helper
  functions, object structure, arbitrary delays, or copies of the implementation.
- Registration coverage includes all four console targets, host and broadcast
  dispatch, stored Account ID, Online ID visibility, both lookup routes, focus
  restoration, whitespace, PIN lengths, leading zeroes, optional PINs, native
  rejection, and no writes before explicit submission.
- Drive nonterminal logs and both terminal outcomes through the fake backend.
  Check progress, success, generic failure, expanded diagnostics, preserved
  input, retry, duplicate activation, dismissal, late callbacks, and callbacks
  from an earlier attempt. Require real outcomes to control the displayed result.
- Manual coverage includes an existing registration, an empty list, deferred
  registration, empty and whitespace-only addresses, equal names, reordering,
  removal, ambiguity, profile changes, repeat activation, save confirmation,
  failure recovery, and no connection command on Add console.
- Check both screens and their result states at 900 by 600 and 1280 by 720.
  Verify focus visibility, scroll reachability, long text, streamer mode, and
  keyboard and controller key mappings. Inspect native captures for visual
  agreement with A. Browser layout checks are design evidence, not native proof.
- The fake backend cannot prove native persistence or callback ordering. For
  any changed C++ behavior, add a narrow native contract check with temporary
  settings and controlled completion events. Verify target ownership, storage
  before success, and rejection without unintended writes. Keep this additional
  seam limited to native behavior that the screen harness cannot exercise.
- Run the affected screen tests during implementation, then the existing screen
  suite and normal application build. Review the final diff against this spec
  and the project rules. Do not treat a successful build as a UI acceptance check.
- Use isolated native settings and synthetic console records for application
  checks. Disable automatic connection and discovery in that test environment.
  Do not alter the user's profiles or registered consoles to obtain evidence.
- Native checks cover the library and Settings entry points, account-screen
  return, form navigation, progress and results driven by controlled events,
  both target sizes, and supported display-scale cases. Physical controller
  hardware, real authentication, and real pairing remain explicitly unverified
  unless separately exercised.

## Out of Scope

- Redesign of PSN sign-in, token entry, public lookup, or automatic PSN registration.
  This change preserves their existing routes and returned data.
- Stream view, connection progress outside manual registration, auto-connect,
  stream menus, overlays, and the separate console PIN prompt.
- A merged registration wizard, new account-management features, network
  diagnostics, or automatic address discovery.
- A new cancellation protocol, a general background-job framework, or a broad
  registration-backend refactor. Only changes needed for correct results and
  request ownership belong in this feature.
- A storage-format change, a storage-engine redesign, or a new guarantee of
  durable disk writes beyond the current settings contract.
- Layouts below 900 logical pixels wide, browser prototype controls in the
  application, and the unselected B and C designs.
- A redesign of the completed library or Settings pages, except for their
  necessary route and focus connections to these two screens.

## Further Notes

- The user approved separate screens and progress/result scope, selected A,
  and then requested this specification. No further design interview is needed.
- Prototype source: [console setup preview](../../gui/prototype/console-setup/index.html).
  The review URL is [layout A](http://127.0.0.1:8765/prototype/console-setup/?variant=A).
  Switch between Registration and Manual host to inspect both selected screens.
- Captured primary source: branch `prototype/console-setup-guided-workspace`,
  commit `d3b02f91db69c6b6dca2c89a69036da5edc74b4d`.
  It retains all three compared layouts in the three console-setup prototype
  files. Links to the earlier library and Settings prototypes use the existing
  local prototype workspace. The chosen layout and acceptance contract are
  recorded in this spec.
- Prototype checks: [layout results](../console-setup-prototype/layout-checks.json)
  and [selected-layout capture](../console-setup-prototype/guided-workspace.png).
  The browser checks covered 42 layout, screen, and state combinations at the
  two target sizes, plus selected input and navigation behavior.
- The prototype contains sample milestones, sample error explanations, and
  simulated account actions. Those are not native backend capabilities.
- Source review used revision `e0e33b830d0f5f7f8eb5101657b16845e1f4d9ba`
  and the existing local console-library changes. The native behavior references
  are the registration and manual-host dialogs, the QML backend registration
  callbacks, Settings persistence, and the main navigation routes.
- The existing console-library work and Settings redesign must remain intact.
  The working checkout contains unrelated tracked and untracked changes.
  Implementation must inspect and preserve them rather than stage or reset
  the entire checkout.
- The normal local Windows build requires its launcher to supply runtime paths.
  Use [Launch-chiaki.cmd](C:/Users/david/.cache/chiaki-ui-native/settings-bin/Launch-chiaki.cmd)
  for a manual launch. Process liveness alone does not confirm that the app
  window opened.
- Implementation and acceptance were completed after this specification was approved.
  The Answer below records the result.

## Answer

Implemented on `codex/console-setup-integration`. All four tickets are resolved.
Both separate guided screens include the approved progress and result states.
The [final review](review.md) has no open findings. The shared-button cleanup
preserves the existing screen behavior and removes the one P3 duplication.

The [native acceptance](native-acceptance.md) and
[cleanup verification](review-cleanup-verification.md) distinguish the full
suite and native checks from the focused checks and final build after cleanup.
The original checkout and its recorded files remain unchanged. Real PSN
authentication, console pairing, and physical controller input remain unverified.
