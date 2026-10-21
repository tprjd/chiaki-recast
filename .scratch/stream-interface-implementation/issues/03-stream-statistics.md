# 03: Build the approved A statistics overlay

Status: claimed
Type: task
Blocked by: 01

**What to build:** Show the player's actual stream statistics in A's compact,
translucent bottom-left row, with correct values and visibility, without taking
input from the game. Deliver the same readout in both renderer modes.

Parent and specification: [implementation map](../map.md).

## Acceptance criteria

- [ ] Match the approved A readout: one small row about 18 logical pixels from the bottom and left edges, with the approved dark translucent background and opaque readable text. Do not add a large panel, graph, or history.
- [ ] Display Bitrate, Loss, Dropped, Lost, Queue, and Age in that order, with complete metric names available for accessibility.
- [ ] Bind measured bitrate in Mbps, average packet loss as a percentage, dropped frames, lost frames, average queue depth, and pending frame age in milliseconds to their existing production sources and conversions.
- [ ] Values update as session/window state changes. Preserve finite-value safeguards and show valid zero values. No NaN, Infinity, stale-session value, or prototype sample value appears.
- [ ] Keep the current statistics preference and visibility rules. The readout is hidden while the menu is open or closing, during connection loading/error, and when video is disabled. It returns only when the existing conditions permit it.
- [ ] Inline and separate-window hosts render the same readout with correct placement after movement, resizing, and fullscreen changes. Session teardown leaves no stale overlay window.
- [ ] The statistics presentation is passive: it does not capture keyboard focus or consume pointer/controller input intended for the stream.
- [ ] Production QML tests use the composed boundary from 01 and cover all six values, units, zero and large values, finite-value fallbacks where supported, updates, preference changes, visibility transitions, and session replacement/destruction.
- [ ] Check 900×600, 1280×720, and 1920×1080 with long translated labels and large values. Keep values readable and within the stream bounds. Verify contrast on dark, bright, and detailed backgrounds.
- [ ] Build the affected native target and record feature verification. Do not add metrics, new settings, configurable positions, or new backend contracts. Combined checks with the redesigned menu and notices belong to 05.

## Comments

Ticket 01 is the only blocker. Use the existing menu visibility contract rather
than depending on ticket 02's appearance or pop-up implementation.

Claimed after ticket 01 merged into the integration branch in `fe4850b8`.
