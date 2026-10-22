# 14: Configure native color mapping from web setup

**What to build:** Users can open color mapping from renderer settings and configure the existing native tone and color mapping options.

**Blocked by:** 13: Configure display targets and the native renderer.

**Status:** ready-for-agent

**Source:** [Web setup UI specification](../spec.md). Stories 50, 51, 52, 56, 75.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] All existing color mapping controls retain their labels, ranges, choices, defaults, dependencies, and capability conditions.
- [ ] Validated native setters own persistence and renderer notifications. The browser does not implement color or HDR processing.
- [ ] Settings search reaches the linked page and returns to the expected renderer category and focus.
- [ ] The native live color mapping page reads and changes the same settings without a second store.
- [ ] Embedded tests verify representative dependent controls, invalid values, saved native values, and linked search navigation.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.
