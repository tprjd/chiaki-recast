# 02: Edit Video and linked Display settings

Publication: draft; awaiting breakdown approval

**What to build:** Configure decoding, stream-window behavior, and rendering from
the Video category, then open and edit the linked Display page.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [ ] Video includes native decoder choices, zero-copy, all window modes, conditional custom dimensions, fullscreen behavior, cursor behavior, vertical sync, every render preset, queue depth, and renderer backend.
- [ ] Display includes all six controls, complete option lists, and numeric peak and contrast values under their native conditions.
- [ ] Values use the existing defaults, limits, validation, conversions, and update timing. Invalid dimensions follow the existing handling.
- [ ] Intentional backend and vertical-sync changes retain the correct restart requests. Page creation and navigation cause neither writes nor restarts.
- [ ] Custom Renderer opens its existing working page until ticket 10 replaces that page's layout.
- [ ] Display returns to the correct caller and restores focus. Its existing stream-time entry points and window behavior continue to work.
- [ ] Production-QML tests verify writes, conditions, option coverage, numeric boundaries, restart dispatch, and child-page navigation.
- [ ] Build and demonstrate both native pages with long option labels and a narrow window.

