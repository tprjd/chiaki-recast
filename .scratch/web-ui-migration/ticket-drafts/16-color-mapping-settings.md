# 16: Configure native color mapping from web setup

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can open color mapping from renderer settings and configure the existing native tone and color mapping options.

**Blocked by:** 15: Configure display targets and the native renderer.

**Status:** needs-info

**Spec stories:** 50, 51, 52, 56, 75.

- [ ] All existing color mapping controls retain their labels, ranges, choices, defaults, dependencies, and capability conditions.
- [ ] Validated native setters own persistence and renderer notifications. The browser does not implement color or HDR processing.
- [ ] Settings search reaches the linked page and returns to the expected renderer category and focus.
- [ ] The native live color mapping page reads and changes the same settings without a second store.
- [ ] Embedded tests verify representative dependent controls, invalid values, saved native values, and linked search navigation.
