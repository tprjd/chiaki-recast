# 04: Startup reminder choices

Status: claimed
Type: task
Blocked by: None (can start immediately)

**What to build:** Deliver redesigned Steam and PSN reminders with distinct setup-now, temporary dismissal, and permanent refusal actions.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [ ] Use the new design and existing production controls. Verify reachable actions at 900 by 600 and 1280 by 720 logical pixels, long text, keyboard/controller navigation, visible focus, return focus, and applicable privacy rules. Keep test fixtures and synthetic data out of production resources.
- [ ] Show Set up now, Later, and Don't ask again. Connect each action to its current setup route or reminder preference.
- [ ] Later and Escape leave the reminder preference unchanged. Only Don't ask again disables that reminder.
- [ ] Preserve the existing Steam-then-PSN order and eligibility rules. Restore focus and prevent duplicate or stacked prompts.
- [ ] Verify action dispatch, setting writes, Escape, focus, reminder chaining, and successful-setup eligibility using the existing setup outcome contract.

## Coverage and dependencies

Primary user stories: 48-50. Shared user stories: 1-7, 63.

This ticket does not depend on 01 or 03: the existing setup routes and reminder preferences already provide its contract. Cross-flow verification remains in 08.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.

