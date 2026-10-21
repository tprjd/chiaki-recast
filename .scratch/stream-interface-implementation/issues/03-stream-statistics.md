# 03: Build the approved A statistics overlay

Status: resolved
Type: task
Blocked by: 01

**What to build:** Show the player's actual stream statistics in A's compact,
translucent bottom-left row, with correct values and visibility, without taking
input from the game. Deliver the same readout in both renderer modes.

Parent and specification: [implementation map](../map.md).

## Acceptance criteria

- [x] Match the approved A readout: one small row about 18 logical pixels from the bottom and left edges, with the approved dark translucent background and opaque readable text. Do not add a large panel, graph, or history.
- [x] Display Bitrate, Loss, Dropped, Lost, Queue, and Age in that order, with complete metric names available for accessibility.
- [x] Bind measured bitrate in Mbps, average packet loss as a percentage, dropped frames, lost frames, average queue depth, and pending frame age in milliseconds to their existing production sources and conversions.
- [x] Values update as session/window state changes. Preserve finite-value safeguards and show valid zero values. No NaN, Infinity, stale-session value, or prototype sample value appears.
- [x] Keep the current statistics preference and visibility rules. The readout is hidden while the menu is open or closing, during connection loading/error, and when video is disabled. It returns only when the existing conditions permit it.
- [x] Inline and separate-window hosts render the same readout with correct placement after movement, resizing, and fullscreen changes. Session teardown leaves no stale overlay window.
- [x] The statistics presentation is passive: it does not capture keyboard focus or consume pointer/controller input intended for the stream.
- [x] Production QML tests use the composed boundary from 01 and cover all six values, units, zero and large values, finite-value fallbacks where supported, updates, preference changes, visibility transitions, and session replacement/destruction.
- [x] Check 900×600, 1280×720, and 1920×1080 with long translated labels and large values. Keep values readable and within the stream bounds. Verify contrast on dark, bright, and detailed backgrounds.
- [x] Build the affected native target and record feature verification. Do not add metrics, new settings, configurable positions, or new backend contracts. Combined checks with the redesigned menu and notices belong to 05.

## Comments

Ticket 01 is the only blocker. Use the existing menu visibility contract rather
than depending on ticket 02's appearance or pop-up implementation.

Claimed after ticket 01 merged into the integration branch in `fe4850b8`.

## Answer

Implemented the approved compact A statistics row in both production hosts.
All six metrics use their existing sources and units. Finite fallbacks preserve
valid zero values. Values update directly without rebuilding the metric row.
Long labels can shorten while values and accessible names remain complete.

Statistics hide during menu opening and closure, loading, failure, disabled
video, and session removal. The separate window uses its actual native owner
and rejects input and activation. Both hosts keep the approved margins.
The native session-change handler clears prior-session window statistics.

The composed production QML checks pass. Windows focus checks and real native
Vulkan and OpenGL checks pass. Native captures retain all required sizes at
measured DPR 1.25. The normal native targets built, and the shared cache was
restored and rebuilt. The hidden menu's two shortcuts now disable on closure,
so Escape reaches the separate disconnect dialog.

[Verification and replay evidence](../evidence/ticket03/README.md) records the
TDD failures, final results, source hashes, captures, and native build limits.
Ticket 05 owns the final combined desktop composition and broader native input
and display-scale checks. No settings, metrics, backend APIs, or package outputs
were added.
