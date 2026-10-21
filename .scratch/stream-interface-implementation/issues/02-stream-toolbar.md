# 02: Build the approved A toolbar

Status: claimed
Type: task
Blocked by: 01

**What to build:** Let the player adjust the stream through the approved descriptive
A toolbar, with live audio controls, picture and quality pop-ups, existing settings
links, Resume, and Disconnect. The complete interaction must work in both renderer
presentations with correct focus and input handling.

Parent and specification: [implementation map](../map.md).

## Acceptance criteria

- [ ] Match the approved third-revision A toolbar: bottom center, approximately 820 by 80 logical pixels at desktop sizes, with readable control text and separate descriptive labels. Adapt to available space without returning to a large settings panel.
- [ ] Show Back to game / Resume, Volume / percentage, Microphone / On or Muted, Picture mode / current mode, Quality preset / current preset, and End session / Disconnect as two-level labels. No slash is rendered between the lines.
- [ ] Use the approved dark background at approximately 65% opacity with opaque text, controls, and visible focus. Do not add blur, full-screen dimming, or a saved opacity setting.
- [ ] Volume uses the real range 0–128 and existing persistence; percentage is presentation only. Test both endpoints and an intermediate value through production bindings.
- [ ] Microphone reflects and changes the session mute state, and is disabled without a connected session. Tests cover On, Muted, and session loss.
- [ ] Picture mode offers Fit, Zoom, and Stretch using the existing native modes. Zoom alone shows its adjustment. Preserve range -1–4, step 0.01, No black bars, current formatting, and saved zoom behavior.
- [ ] Quality offers all five existing presets and updates the same runtime and saved settings. The selected value remains identifiable, including HQ + Advanced spatial. Renderer settings appears for Custom only.
- [ ] Display settings and Renderer settings open the existing refreshed pages. The normal menu-close path releases input before navigation. Neither linked page is redesigned.
- [ ] Picture and quality open in small panels above the toolbar, approximately 320 logical pixels wide. Only one is open at a time; activating its opener again closes it. Escape closes the pop-up and focuses its opener before a later Escape closes the menu.
- [ ] Initial menu focus is Resume. Resume and the menu shortcut preserve session state. Keyboard and existing controller key mappings operate all controls, and actions do not also reach the game.
- [ ] Disconnect follows the existing configured session-close policy. Where confirmation is needed, keep the existing dialog, Cancel-first focus, both disconnect choices, and cancellation semantics. Repeated input does not duplicate a command.
- [ ] Both renderer hosts use the shared content. Host geometry includes an open pop-up without clipping, an opaque full-window rectangle, or unwanted input interception. Move, resize, fullscreen changes, and teardown retain correct ownership and focus.
- [ ] Connection progress, console login PIN, and disconnect dialogs retain priority. Opening or closing the menu during session changes cannot leave input captured or a separate window visible.
- [ ] Production QML tests cover all controls, persisted values, navigation, command effects, Escape order, repeat input, disabled state, and focus restoration through the composed boundary from 01.
- [ ] Check 900×600, 1280×720, and 1920×1080 layouts with long translated labels, the longest preset, Zoom, and Custom. Capture production overlays on dark, bright, and detailed backgrounds. Retain accessible names and streamer-mode masking wherever identity is shown.
- [ ] Build the affected native target and record feature checks. Keep prototype controls and sample data out of production. Full combined native acceptance remains in 05.

## Comments

Ticket 01 is the only blocker. Statistics and notices keep their existing behavior
until their own tickets land; this menu must not require their redesigned versions.

Claimed after ticket 01 merged into the integration branch in `fe4850b8`.
