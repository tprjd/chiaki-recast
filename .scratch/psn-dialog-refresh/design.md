# PSN sign-in and remaining dialogs

Date: 2026-10-21
All twelve interview decisions are confirmed. The user requested the specification and confirmed its testing scope on 2026-10-21. See [the specification](spec.md). Implementation has not started.

## Confirmed decisions

- Scope covers PSN sign-in, redirect URL entry, and the remaining app dialogs listed below.
- Use the new design from the completed Library, Settings, console setup, connection, and PIN refresh work. Replace the old PSN and dialog presentation. Preserve keyboard and controller support and the 900-logical-pixel minimum application width.
- Improve wording, step order, validation messages, and recovery actions. Keep the current PSN authentication method and stored data. Any change to authentication requires a separate decision.
- Include Steam shortcut creation and results, reminders, Quit/Delete/Hide/Unhide confirmations, automatic-registration prompts, renderer notices, About, and both disconnect confirmations. Keep the refreshed Settings prompts and existing file pickers. The active-stream menu is outside this scope.
- Label the pasted browser value "Sign-in redirect URL" and explain where to copy the complete URL after sign-in. The field does not accept a raw token.
- Keep the current browser defaults: embedded sign-in on Windows and an external browser on Linux and macOS. Provide an explicit "Use external browser" action and fallback when the embedded browser is unavailable.
- Show PSN setup progress, a success result, or a persistent error with Retry. Collapse technical details by default. Successful Account-ID lookup returns directly to registration. Explain why expired credentials require sign-in again.
- Hide the sign-in redirect URL by default. Provide Paste and Show/Hide controls. Streamer mode prevents reveal. Diagnostic details exclude authorization codes and tokens.
- Keep public Account-ID lookup separate from PSN sign-in. Explain that lookup uses a third-party service and does not sign the user in. Explain that PSN sign-in also saves credentials for remote access.
- Use explicit confirmation actions and initial focus on Cancel. Automatic registration offers Automatic registration, Manual registration, and Cancel. Disconnect offers Disconnect and sleep, Disconnect, and Cancel. Escape cancels without starting another action. Both disconnect presentations behave the same way.
- Keep reminder choices for Set up now, Later, and Don't ask again. Escape means Later. Keep the existing reminder order.
- Keep Steam form defaults and optional fields. Show progress and a terminal result with optional details. Include the restart instruction in the success result without restarting Steam. Keep renderer notices and About as compact dialogs with their existing information.
- Before submission, Back or Cancel leaves the form. After a PSN request starts, use Close with an explanation that sign-in can finish and save credentials. Prevent duplicate requests and late UI updates. Steam writes finish before the result can close; do not offer a false Cancel action.

## Specification handoff

- The to-spec request advances the confirmed design into a specification. No design questions remain open.
- Test scope is confirmed: production QML screens with supplied backend state, focused native checks with isolated settings, and visible native checks for focus, both disconnect presentations, and supported layouts. Real service and hardware results remain separate from controlled checks.

## Decision tree

- New visual design from the completed refresh work: confirmed.
- Flow improvements within the existing authentication and storage model: confirmed.
  - Redirect URL terminology and browser defaults: confirmed.
  - PSN results, retry, privacy, and public lookup distinction: confirmed.
  - Cancellation and closure during an operation: confirmed.
- Complete dialog scope: confirmed.
  - Confirmation actions and initial focus: confirmed.
  - Reminder choices and Escape behavior: confirmed.
  - Steam form and result behavior: confirmed.
  - About and renderer notices: confirmed.
- Shared understanding: carried into the specification at the user's request.
- Testing scope: confirmed.

## Source findings

- `gui/src/qml/ConfirmDialog.qml` accepts on Enter and rejects on Escape. `gui/src/qml/Main.qml` uses rejection to start manual registration.
- `gui/src/qml/RemindDialog.qml` saves a permanent refusal on rejection, including Escape. Later preserves the reminder setting.
- `gui/src/qml/StreamView.qml` has in-window and separate-window disconnect confirmations. Both initially focus Sleep. No disconnects without sleep.
- `gui/src/qml/SteamShortcutDialog.qml` treats intermediate successful callbacks as success. The revised result must use terminal success.
- `gui/src/qmlbackend.cpp` implements Steam shortcut writes synchronously and PSN setup asynchronously. Neither interface provides operation cancellation.
- `gui/src/psnaccountid.cpp` saves credentials during sign-in. Closing the PSN screen does not cancel those writes.
