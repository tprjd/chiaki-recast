# 04: Startup reminder choices

Status: resolved
Type: task
Blocked by: None (can start immediately)

**What to build:** Deliver redesigned Steam and PSN reminders with distinct setup-now, temporary dismissal, and permanent refusal actions.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [x] Use the new design and existing production controls. Verify reachable actions at 900 by 600 and 1280 by 720 logical pixels, long text, keyboard/controller navigation, visible focus, return focus, and applicable privacy rules. Keep test fixtures and synthetic data out of production resources.
- [x] Show Set up now, Later, and Don't ask again. Connect each action to its current setup route or reminder preference.
- [x] Later and Escape leave the reminder preference unchanged. Only Don't ask again disables that reminder.
- [x] Preserve the existing Steam-then-PSN order and eligibility rules. Restore focus and prevent duplicate or stacked prompts.
- [x] Verify action dispatch, setting writes, Escape, focus, reminder chaining, and successful-setup eligibility using the existing setup outcome contract.

## Coverage and dependencies

Primary user stories: 48-50. Shared user stories: 1-7, 63.

This ticket does not depend on 01 or 03: the existing setup routes and reminder preferences already provide its contract. Cross-flow verification remains in 08.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.


## Answer

Delivered Set up now, Later, and Don't ask again through the existing compact
confirmation presentation and production controls. Later receives visible initial
focus. Escape and the existing controller triangle/Y key mean Later. Only the
explicit Don't ask again action changes the selected reminder preference.
Successful setup retains its existing completion preference contract.

The four-argument `showRemindDialog` contract remains intact. Setup starts after
the popup closes. Repeated prompt requests cannot replace the current callback
or save a popup control as the library return target. Steam stays first when
eligible. A dismissed Steam reminder offers an eligible PSN reminder after closure.
Startup auto-connect, direct-stream, preference, platform-command, and complete-PSN
credential checks remain intact. Re-entry does not repeat the startup prompts.

The PSN reminder states that sign-in saves account credentials. Reminder text
contains no account values. Long instructions scroll while all three actions
remain reachable. The shared compact popup uses a dark modal overlay.
The Steam close path retains its captured navigation/settings and once guards;
this ticket changes only its reminder wording.

Evidence is in `../evidence/ticket04/`:

- `reminder-red.txt` and `reminder-green.txt` record the first production-dialog test.
- `controller-red.txt` and `controller-green.txt` prove retention of the Later key.
- `reminder-final.txt`: 28 checks passed, no failures.
- `confirmations-regression.txt`: 36 checks passed, no failures.
- `registration-regression.txt`: 8 checks passed, no failures.
- `steam-reminder-regression.txt`: 6 checks passed, no failures.
- `steam-reminder-900.png` and `psn-reminder-900.png`: inspected production routes
  at 900 by 600, with visible Later focus and all actions reachable.

Tests use production MainView, RemindDialog, SteamShortcutDialog, and StackView.
They check the actual Steam form close path after cancellation, final success,
and final failure, plus preference changes only after final success.
Backend results and PSN setup navigation are controlled test inputs.
Long-text tests also cover 1280 by 720. Fixtures are absent from production QML
resources. The local screen target built with Qt 6.9.3 and MSVC.
`git diff --check` passed. Commands follow `gui/tests/README.md` with `-input`
for the focused suites and named Steam regression functions.

Not checked here: a complete application build, real PSN authentication,
actual Steam writes, physical controllers, native input capture, or display scaling.
Ticket 08 owns final native and integrated acceptance. No shared native cache
or authentication state implementation was changed by this ticket.
