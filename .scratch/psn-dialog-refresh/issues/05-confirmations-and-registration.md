# 05: Explicit confirmations and registration choice

Status: resolved
Type: task
Blocked by: None (can start immediately)

**What to build:** Deliver clear confirmation actions for Quit and console management, plus a three-way automatic/manual/cancel registration choice.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [x] Use the new design and existing production controls. Verify reachable actions at 900 by 600 and 1280 by 720 logical pixels, long text, keyboard/controller navigation, visible focus, return focus, and applicable privacy rules. Keep test fixtures and synthetic data out of production resources.
- [x] Use explicit action labels for Quit, Delete console, Hide console, and Unhide console. Update every affected caller rather than leaving a mixed callback contract.
- [x] Initially focus Cancel. Enter activates only the enabled focused action. Escape cancels without changes or a fallback action.
- [x] Preserve current console eligibility, target resolution, profile invalidation, and single-acceptance guards.
- [x] Offer Automatic registration, Manual registration, and Cancel. Only the selected method starts; Escape starts neither. Retain PS4 eligibility guidance.
- [x] Restore focus without activating controls behind the dialog. Verify guarded management commands, changed targets, repeated input, and all three registration outcomes through production UI tests.

## Coverage and dependencies

Primary user stories: 51-56. Shared user stories: 1-7, 63.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.


## Answer

Implemented explicit Quit, Delete console, Hide console, and Unhide console actions.
All callers use `showConfirmDialog(title, text, actionText, accept, cancel)`.
The compact dialog uses production SetupButton controls, wrapped plain text,
and a scroll area that keeps the actions visible. Cancel receives visible initial
focus. Escape cancels. Completion runs once after the popup closes, then restores
focus. The existing console target, eligibility, and profile guards remain intact.

Automatic registration and Manual registration are separate actions in production
`RegistrationMethodDialog.qml`. Cancel and Escape start neither method. The PS4
main-console guidance and native automatic-registration context remain intact.
The production resource list contains the new dialog and excludes test fixtures.

Evidence is in `../evidence/ticket05/`:

- `confirm-red.txt` records the missing Cancel action before implementation.
- `focus-red.txt` and `focus-green.txt` show the initial visible-focus defect and fix.
- `confirm-final.txt`: 36 checks passed, no failures.
- `method-final.txt`: 8 checks passed, no failures.
- `settings-consoles-final.txt`: 20 checks passed, no failures.
- `console-management-final.txt`: 44 checks passed, no failures.
- `settings-search-final.txt`: 3 checks passed, no failures.
- `registration-method-900.png`: inspected offscreen rendering at 900 by 600.

The tests load production screens and controls. They cover current target resolution,
removal, ambiguous targets, profile changes, repeated input, disabled actions,
focus return, all registration choices, and long text at both specified sizes.
The screen target built in this ticket's managed worktree with Qt 6.9.3 and MSVC.
Commands follow `gui/tests/README.md`; focused suites use `-input` and named test
functions. `git diff --check` passed.

A broad pre-fix run was stopped at SettingsSearch to reserve full regression work
for ticket 08. It recorded the registration Cancel failure before the visible-focus
fix. The final focused suites pass with an explicit visible-focus assertion.

Not checked here: a normal application build, visible native input capture,
display scaling, physical controllers, or real automatic/manual registration.
Ticket 08 owns final native and integrated acceptance. Controlled backend calls
are not evidence of a real console operation. The shared native build was not used.
