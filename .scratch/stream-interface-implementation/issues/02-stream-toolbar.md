# 02: Build the approved A toolbar

Status: resolved
Type: task
Blocked by: 01

**What to build:** Let the player adjust the stream through the approved descriptive
A toolbar, with live audio controls, picture and quality pop-ups, existing settings
links, Resume, and Disconnect. The complete interaction must work in both renderer
presentations with correct focus and input handling.

Parent and specification: [implementation map](../map.md).

## Acceptance criteria

- [x] Match the approved third-revision A toolbar: bottom center, approximately 820 by 80 logical pixels at desktop sizes, with readable control text and separate descriptive labels. Adapt to available space without returning to a large settings panel.
- [x] Show Back to game / Resume, Volume / percentage, Microphone / On or Muted, Picture mode / current mode, Quality preset / current preset, and End session / Disconnect as two-level labels. No slash is rendered between the lines.
- [x] Use the approved dark background at approximately 65% opacity with opaque text, controls, and visible focus. Do not add blur, full-screen dimming, or a saved opacity setting.
- [x] Volume uses the real range 0–128 and existing persistence; percentage is presentation only. Test both endpoints and an intermediate value through production bindings.
- [x] Microphone reflects and changes the session mute state, and is disabled without a connected session. Tests cover On, Muted, and session loss.
- [x] Picture mode offers Fit, Zoom, and Stretch using the existing native modes. Zoom alone shows its adjustment. Preserve range -1–4, step 0.01, No black bars, current formatting, and saved zoom behavior.
- [x] Quality offers all five existing presets and updates the same runtime and saved settings. The selected value remains identifiable, including HQ + Advanced spatial. Renderer settings appears for Custom only.
- [x] Display settings and Renderer settings open the existing refreshed pages. The normal menu-close path releases input before navigation. Neither linked page is redesigned.
- [x] Picture and quality open in small panels above the toolbar, approximately 320 logical pixels wide. Only one is open at a time; activating its opener again closes it. Escape closes the pop-up and focuses its opener before a later Escape closes the menu.
- [x] Initial menu focus is Resume. Resume and the menu shortcut preserve session state. Keyboard and existing controller key mappings operate all controls, and actions do not also reach the game.
- [x] Disconnect follows the existing configured session-close policy. Where confirmation is needed, keep the existing dialog, Cancel-first focus, both disconnect choices, and cancellation semantics. Repeated input does not duplicate a command.
- [x] Both renderer hosts use the shared content. Host geometry includes an open pop-up without clipping, an opaque full-window rectangle, or unwanted input interception. Move, resize, fullscreen changes, and teardown retain correct ownership and focus.
- [x] Connection progress, console login PIN, and disconnect dialogs retain priority. Opening or closing the menu during session changes cannot leave input captured or a separate window visible.
- [x] Production QML tests cover all controls, persisted values, navigation, command effects, Escape order, repeat input, disabled state, and focus restoration through the composed boundary from 01.
- [x] Check 900×600, 1280×720, and 1920×1080 layouts with long translated labels, the longest preset, Zoom, and Custom. Capture production overlays on dark, bright, and detailed backgrounds. Retain accessible names and streamer-mode masking wherever identity is shown.
- [x] Build the affected native target and record feature checks. Keep prototype controls and sample data out of production. Full combined native acceptance remains in 05.

## Comments

Ticket 01 is the only blocker. Statistics and notices keep their existing behavior
until their own tickets land; this menu must not require their redesigned versions.

Claimed after ticket 01 merged into the integration branch in `fe4850b8`.

## Answer

Implemented the approved A toolbar in shared production QML. Both renderer
presentations use live volume and microphone bindings, grouped picture controls,
all five quality presets, settings links, Resume, and the configured disconnect
route. The native toolbar stays compact. Native popup windows follow it above
the stream. One menu state owns flyouts and session input.

The native tests found and fixed the controller-key route to separate windows,
duplicate Escape handling, hidden-menu shortcut capture, and native stream
activation after closure. Composed tests also found and fixed connection-state
priority and duplicate settings navigation. Long-label captures found and fixed
a Volume label overlap. Initial Resume focus now has a tested visible outline.

Verified 36 toolbar checks, 37 baseline interface checks, all related suites,
and 18 layout cases through both hosts. Extended translations pass 18 further
layout cases. Native OpenGL passes 386 checks and Vulkan passes 360 at DPR 1.25.
Real Windows hit testing and SendInput verify flyout opener clicks. Native
transport commands and settings remain isolated.

A normal production build passed after merging integration `a91d1d65`.
All 338 staged production files matched. QA hooks, fixture resources, and
prototype resources were absent. The cache was restored and its original targets
rebuilt. The final one-line focus-outline style change has native QML pixel
checks; ticket 05 includes it in the final integrated native build.

Evidence and replay: [ticket 02 record](../evidence/ticket02/README.md).
The record separates component captures and controlled input results from
remaining combined desktop, fullscreen, scale, and live-console acceptance in 05.
