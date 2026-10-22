# 15: Configure display targets and the native renderer

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can configure display targets, renderer selection, presets, and custom native renderer options from web setup.

**Blocked by:** 13: Change general and stream settings from web setup.

**Status:** needs-info

**Spec stories:** 50, 51, 52, 56, 75.

- [ ] The web UI retains existing display controls, renderer choices, presets, and custom renderer fields with their actual ranges and conditions.
- [ ] Values are converted and validated by the existing native settings logic and persist in the existing store.
- [ ] Renderer changes retain the existing restart behavior and actual runtime renderer reporting. A saved choice does not falsely report a live switch.
- [ ] Renderer settings import, export, and documentation entry points remain native actions or connect to the existing native services.
- [ ] Live stream display and renderer settings stay native and use the same saved values and notifications.
- [ ] Embedded tests verify native writes, rejected values, preset changes, restart requirements, and consistency with native live settings.
