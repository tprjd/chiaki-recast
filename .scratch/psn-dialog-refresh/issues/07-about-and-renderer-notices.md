# 07: About and renderer notices

Status: claimed
Type: task
Blocked by: None (can start immediately)

**What to build:** Deliver compact information dialogs in the new design with complete content, one Close action, and reliable return focus.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [ ] Use the new design and existing production controls. Verify reachable actions at 900 by 600 and 1280 by 720 logical pixels, long text, keyboard/controller navigation, visible focus, return focus, and applicable privacy rules. Keep test fixtures and synthetic data out of production resources.
- [ ] Retain About's logo, application name/version, original Chiaki attribution, and license information. Permit scrolling for long content.
- [ ] Retain the renderer fallback reason and make clear that fallback already occurred. Do not add a renderer-selection action.
- [ ] Close through the button and expected keyboard/controller inputs, restore valid focus, and prevent activation behind the dialog.
- [ ] Verify production dialogs at supported sizes with long text. Leave refreshed Settings input-capture prompts unchanged.

## Coverage and dependencies

Primary user stories: 60-62. Shared user stories: 1-7, 63.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.

