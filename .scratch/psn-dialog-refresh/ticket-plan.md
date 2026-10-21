# Approved PSN sign-in and dialogs ticket plan

Date: 2026-10-21
State: approved by the user; all eight tickets published

This plan records the approved breakdown. All eight implementation tickets are published and unclaimed. See [the implementation map](map.md) for ticket links and current status. The parent specification remains unchanged.

Each feature ticket delivers usable behavior through its callers, production presentation, native handlers where needed, and focused tests. Reuse the existing new design and test boundaries. Each ticket includes privacy, translation support, reachable controls at the supported sizes, keyboard/controller navigation, and focus restoration where applicable. No separate prefactor is needed before these slices.

## 01: PSN setup and credential renewal

Blocked by: None (can start immediately).

Deliver PSN remote-access setup and expired-credential renewal through the current embedded and external browser routes, including input, actual operation progress, results, retry, and screen closure.

Acceptance outline:

- Use the new design, preserve platform browser defaults, retain external-browser fallback and browser recovery actions, and preserve Settings, reminder, and expiry entry routes.
- Explain the complete Sign-in redirect URL and saved credentials. Provide explicit copy/paste actions, masked input, reveal controls, and streamer-mode protection.
- Reject invalid redirect input at the native boundary and provide a route to fresh sign-in for expired or used codes.
- Show progress, terminal success, or persistent failure with retry and collapsed sanitized details. Correct expiry routing and embedded/external reminder-completion parity.
- Before submission, Cancel exits. After submission, Close explains that sign-in can finish and save credentials. Prevent duplicate requests across closure and re-entry.
- Associate requests with their original profile and presentation. Late results cannot save into another profile or change a replacement screen. Preserve compatibility for existing registration callers while the next ticket refreshes that route.
- Verify production QML behavior and actual native persistence with isolated settings and controlled responses. Cover failure, replacement, duplicate actions, late results, and sensitive-value exclusion.

Primary user stories: 9, 14-35, 37. Shared user stories: 1-7, 63.

## 02: Account-ID retrieval for registration

Blocked by: 01 (PSN setup and credential renewal).

Deliver the refreshed registration routes for PSN sign-in and public Account-ID lookup, returning a valid result directly to the intended registration form.

Acceptance outline:

- Reuse the completed PSN request and browser behavior from 01 for the registration sign-in route, including privacy, closure, recovery, and credential-storage explanation.
- Keep public lookup distinct from sign-in. Explain the third-party service and public-search requirement. Do not save PSN credentials from public lookup.
- Handle empty input, network failure, malformed responses, missing Account IDs, and retry without leaving the form permanently disabled.
- Return success directly to the valid registration caller. Preserve its original console/profile context; closed or replaced callers cannot receive a stale result.
- Verify both paths through the production registration screen and supplied service outcomes. Check that lookup does not write authentication credentials.

Primary user stories: 8-13, 36. Shared user stories: 1-7, 14-35, 63.

## 03: Steam shortcut creation and results

Blocked by: None (can start immediately).

Deliver the refreshed Steam form through actual create-or-update behavior, accurate final results, retry, and the manual restart instruction.

Acceptance outline:

- Preserve profile-based defaults, optional launch options, the optional Steam folder, and existing file pickers. Reject an empty trimmed name.
- Preserve native matching, shortcut updates, artwork, and controller-configuration effects.
- Show operation progress and a terminal result. Do not offer cancellation for synchronous writes or claim rollback after failure.
- Treat only terminal success as success. An intermediate successful step followed by failure cannot disable the reminder or show the restart instruction.
- Retain editable input and useful details on failure. Clear old outcome/details for retry and prevent duplicate creation while active.
- Include the manual Steam restart instruction in success and preserve the eligible next reminder without restarting Steam or stacking dialogs.
- Verify production QML results and actual native writes in a temporary Steam directory, including matching-entry updates and failure after partial progress.

Primary user stories: 38-47. Shared user stories: 1-7, 63.

## 04: Startup reminder choices

Blocked by: None (can start immediately).

Deliver redesigned Steam and PSN reminders with distinct setup-now, temporary dismissal, and permanent refusal actions.

Acceptance outline:

- Show Set up now, Later, and Don't ask again. Connect each action to its current setup route or reminder preference.
- Later and Escape leave the reminder preference unchanged. Only Don't ask again disables that reminder.
- Preserve the existing Steam-then-PSN order and eligibility rules. Restore focus and prevent duplicate or stacked prompts.
- Verify action dispatch, setting writes, Escape, focus, reminder chaining, and successful-setup eligibility using the existing setup outcome contract.

Primary user stories: 48-50. Shared user stories: 1-7, 63.

This ticket does not depend on 01 or 03: the existing setup routes and reminder preferences already provide its contract. Cross-flow verification remains in 08.

## 05: Explicit confirmations and registration choice

Blocked by: None (can start immediately).

Deliver clear confirmation actions for Quit and console management, plus a three-way automatic/manual/cancel registration choice.

Acceptance outline:

- Use explicit action labels for Quit, Delete console, Hide console, and Unhide console. Update every affected caller rather than leaving a mixed callback contract.
- Initially focus Cancel. Enter activates only the enabled focused action. Escape cancels without changes or a fallback action.
- Preserve current console eligibility, target resolution, profile invalidation, and single-acceptance guards.
- Offer Automatic registration, Manual registration, and Cancel. Only the selected method starts; Escape starts neither. Retain PS4 eligibility guidance.
- Restore focus without activating controls behind the dialog. Verify guarded management commands, changed targets, repeated input, and all three registration outcomes through production UI tests.

Primary user stories: 51-56. Shared user stories: 1-7, 63.

## 06: Disconnect confirmations in both renderer routes

Blocked by: None (can start immediately).

Deliver the same explicit disconnect choices in the in-window popup and separate native window, including correct session and console-power commands.

Acceptance outline:

- Use Disconnect and sleep, Disconnect, and Cancel with Cancel initially focused in both presentations.
- Dispatch stop-with-sleep only for Disconnect and sleep; dispatch stop-without-sleep only for Disconnect. Cancel, Escape, and separate-window close keep the stream running.
- Preserve saved disconnect preferences, renderer-dependent routing, placement, and input capture/release.
- Verify production presentation behavior, actual native stop dispatch with controlled session operations, and visible focus/close behavior for both window routes.
- Keep the active-stream menu and transport behavior outside the change.

Primary user stories: 57-59. Shared user stories: 1-7, 63.

## 07: About and renderer notices

Blocked by: None (can start immediately).

Deliver compact information dialogs in the new design with complete content, one Close action, and reliable return focus.

Acceptance outline:

- Retain About's logo, application name/version, original Chiaki attribution, and license information. Permit scrolling for long content.
- Retain the renderer fallback reason and make clear that fallback already occurred. Do not add a renderer-selection action.
- Close through the button and expected keyboard/controller inputs, restore valid focus, and prevent activation behind the dialog.
- Verify production dialogs at supported sizes with long text. Leave refreshed Settings input-capture prompts unchanged.

Primary user stories: 60-62. Shared user stories: 1-7, 63.

## 08: Integrated native acceptance

Blocked by: 02, 03, 04, 05, 06, 07.

Deliver a verified normal application build in which all refreshed routes work together, with reproducible acceptance evidence and explicit external-service limits.

Acceptance outline:

- Build the normal application with the production resources and without test fixtures. Integrate the completed slices and fix cross-flow regressions within the specification.
- Run the focused feature tests and relevant Library, Settings, registration, connection, and PIN regressions. Feature tests are prerequisites from their own tickets, not work deferred to this ticket.
- Exercise Settings/startup/expiry sign-in, both browser selections, registration return, sign-in closure and re-entry, reminder sequencing after Steam/PSN results, and console-management return routes together.
- Complete visible native checks at 900 by 600 and 1280 by 720, compact-window sizes, and display scaling. Check actual focus, input capture/release, closure, and both renderer-dependent disconnect routes.
- Use isolated settings, synthetic credentials, temporary Steam data, and controlled session/service responses. Verify the tested source and build and retain sanitized evidence.
- Separate controlled outcomes from real PSN sign-in, real Steam visibility after restart, console sleep/disconnect, and physical-controller results. Clearly record external or hardware checks not performed.

User stories: integration coverage for 1-63, with emphasis on 1-7 and 63.

Ticket 01 is a transitive blocker through 02. No redundant direct edge is needed.

## Dependency review

- Initial eligible tickets: 01, 03, 04, 05, 06, 07.
- After 01: 02 becomes eligible.
- After 02-07: 08 becomes eligible.
- Shared-file overlap requires coordination but does not create a functional dependency by itself. A shared native build cache must have one writer at a time.
- Every numbered user story has a primary or shared owner. Common accessibility, privacy, and layout requirements belong to each affected feature ticket.
- All eight tickets are published as separate numbered files with ready-for-agent status and these blocking edges. The parent specification is unchanged.
