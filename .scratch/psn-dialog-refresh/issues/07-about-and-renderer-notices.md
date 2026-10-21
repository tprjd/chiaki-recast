# 07: About and renderer notices

Status: resolved
Type: task
Blocked by: None (can start immediately)

**What to build:** Deliver compact information dialogs in the new design with complete content, one Close action, and reliable return focus.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [x] Use the new design and existing production controls. Verify reachable actions at 900 by 600 and 1280 by 720 logical pixels, long text, keyboard/controller navigation, visible focus, return focus, and applicable privacy rules. Keep test fixtures and synthetic data out of production resources.
- [x] Retain About's logo, application name/version, original Chiaki attribution, and license information. Permit scrolling for long content.
- [x] Retain the renderer fallback reason and make clear that fallback already occurred. Do not add a renderer-selection action.
- [x] Close through the button and expected keyboard/controller inputs, restore valid focus, and prevent activation behind the dialog.
- [x] Verify production dialogs at supported sizes with long text. Leave refreshed Settings input-capture prompts unchanged.

## Coverage and dependencies

Primary user stories: 60-62. Shared user stories: 1-7, 63.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.


## Answer

About and renderer notices use the refreshed production `ConfirmDialog` with one
Close action. About retains the original logo, application name/version, Street Pea
and Florian Markl attribution, Chiaki 2.1.1 origin, and full existing license text.
The renderer component preserves the actual fallback reason and states that the
app already switched to OpenGL. It has no renderer-selection action.

Empty primary actions are hidden. The shared dialog supports the About logo and
rich text while renderer text remains plain text. Long information scrolls through
keyboard arrows/Page Up/Page Down, with visible scroll focus and fixed Close.
About resets its return item on each open. Main preserves Library focus restoration
and uses a bounded focus-chain search for another current view when no opener exists.
The three-action confirmation defaults and input-capture prompts are preserved.

[Evidence](../evidence/ticket07/README.md) records meaningful red/green checks,
23 passing focused checks, 141 passing regression checks, five post-merge checks,
source hashes, and eight inspected captures at the required sizes.

Visible native input/focus/scaling checks, actual renderer failure, normal application
acceptance, and the production Main non-Library fallback check remain in ticket 08.
No real account, console, Steam installation, or shared native cache was used.
