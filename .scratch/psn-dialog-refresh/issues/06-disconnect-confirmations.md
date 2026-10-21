# 06: Disconnect confirmations in both renderer routes

Status: claimed
Type: task
Blocked by: None (can start immediately)

**What to build:** Deliver the same explicit disconnect choices in the in-window popup and separate native window, including correct session and console-power commands.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [ ] Use the new design and existing production controls. Verify reachable actions at 900 by 600 and 1280 by 720 logical pixels, long text, keyboard/controller navigation, visible focus, return focus, and applicable privacy rules. Keep test fixtures and synthetic data out of production resources.
- [ ] Use Disconnect and sleep, Disconnect, and Cancel with Cancel initially focused in both presentations.
- [ ] Dispatch stop-with-sleep only for Disconnect and sleep; dispatch stop-without-sleep only for Disconnect. Cancel, Escape, and separate-window close keep the stream running.
- [ ] Preserve saved disconnect preferences, renderer-dependent routing, placement, and input capture/release.
- [ ] Verify production presentation behavior, actual native stop dispatch with controlled session operations, and visible focus/close behavior for both window routes.
- [ ] Keep the active-stream menu and transport behavior outside the change.

## Coverage and dependencies

Primary user stories: 57-59. Shared user stories: 1-7, 63.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.

