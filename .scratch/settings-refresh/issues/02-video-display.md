# 02: Edit Video and linked Display settings

Status: resolved

**What to build:** Configure decoding, stream-window behavior, and rendering from
the Video category, then open and edit the linked Display page.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [x] Video includes native decoder choices, zero-copy, all window modes, conditional custom dimensions, fullscreen behavior, cursor behavior, vertical sync, every render preset, queue depth, and renderer backend.
- [x] Display includes all six controls, complete option lists, and numeric peak and contrast values under their native conditions.
- [x] Values use the existing defaults, limits, validation, conversions, and update timing. Invalid dimensions follow the existing handling.
- [x] Intentional backend and vertical-sync changes retain the correct restart requests. Page creation and navigation cause neither writes nor restarts.
- [x] Custom Renderer opens its existing working page until ticket 10 replaces that page's layout.
- [x] Display returns to the correct caller and restores focus. Its existing stream-time entry points and window behavior continue to work.
- [x] Production-QML tests verify writes, conditions, option coverage, numeric boundaries, restart dispatch, and child-page navigation.
- [x] Build and demonstrate both native pages with long option labels and a narrow window.



## Answer

Implemented in 6cd325c and 681de2a, merged as fee3915. Video and Display tests pass, including complete choices, validation, restarts, numeric boundaries, profile changes, and real StackView return focus. The native Windows build passes. Native navigation and focus checks pass at 1280 by 720 and 900 by 600 without changing stored values. Display opens and returns through the application. Both native pages were inspected at 900 pixels. Evidence: ../native-previews/02-video-dpr1-checks.json. Actual stream-time entry with console hardware remains untested.


Focused-value follow-up: cea7d73 (merged ec5d19b) fixes shared focus scrolling and keeps Stream bitrate descriptions with their sliders. Nine main Settings suites pass 205 checks. Native settings-values-fixed-dpr1-checks.json passes both supported sizes, including Display numeric modes and persistence. Root inspected Video and Stream 900 by 600 captures.
