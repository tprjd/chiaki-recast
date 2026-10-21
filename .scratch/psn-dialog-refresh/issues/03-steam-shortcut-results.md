# 03: Steam shortcut creation and results

Status: claimed
Type: task
Blocked by: None (can start immediately)

**What to build:** Deliver the refreshed Steam form through actual create-or-update behavior, accurate final results, retry, and the manual restart instruction.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [ ] Use the new design and existing production controls. Verify reachable actions at 900 by 600 and 1280 by 720 logical pixels, long text, keyboard/controller navigation, visible focus, return focus, and applicable privacy rules. Keep test fixtures and synthetic data out of production resources.
- [ ] Preserve profile-based defaults, optional launch options, the optional Steam folder, and existing file pickers. Reject an empty trimmed name.
- [ ] Preserve native matching, shortcut updates, artwork, and controller-configuration effects.
- [ ] Show operation progress and a terminal result. Do not offer cancellation for synchronous writes or claim rollback after failure.
- [ ] Treat only terminal success as success. An intermediate successful step followed by failure cannot disable the reminder or show the restart instruction.
- [ ] Retain editable input and useful details on failure. Clear old outcome/details for retry and prevent duplicate creation while active.
- [ ] Include the manual Steam restart instruction in success and preserve the eligible next reminder without restarting Steam or stacking dialogs.
- [ ] Verify production QML results and actual native writes in a temporary Steam directory, including matching-entry updates and failure after partial progress.

## Coverage and dependencies

Primary user stories: 38-47. Shared user stories: 1-7, 63.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.

