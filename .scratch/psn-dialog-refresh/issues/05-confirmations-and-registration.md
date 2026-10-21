# 05: Explicit confirmations and registration choice

Status: ready-for-agent
Type: task
Blocked by: None (can start immediately)

**What to build:** Deliver clear confirmation actions for Quit and console management, plus a three-way automatic/manual/cancel registration choice.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [ ] Use the new design and existing production controls. Verify reachable actions at 900 by 600 and 1280 by 720 logical pixels, long text, keyboard/controller navigation, visible focus, return focus, and applicable privacy rules. Keep test fixtures and synthetic data out of production resources.
- [ ] Use explicit action labels for Quit, Delete console, Hide console, and Unhide console. Update every affected caller rather than leaving a mixed callback contract.
- [ ] Initially focus Cancel. Enter activates only the enabled focused action. Escape cancels without changes or a fallback action.
- [ ] Preserve current console eligibility, target resolution, profile invalidation, and single-acceptance guards.
- [ ] Offer Automatic registration, Manual registration, and Cancel. Only the selected method starts; Escape starts neither. Retain PS4 eligibility guidance.
- [ ] Restore focus without activating controls behind the dialog. Verify guarded management commands, changed targets, repeated input, and all three registration outcomes through production UI tests.

## Coverage and dependencies

Primary user stories: 51-56. Shared user stories: 1-7, 63.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.

