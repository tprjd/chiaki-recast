# 19: Configure controllers and their mappings

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can inspect available controllers, change supported controller options, and complete the existing native mapping flow from web setup.

**Blocked by:** 13: Change general and stream settings from web setup.

**Status:** needs-info

**Spec stories:** 50, 51, 52, 59, 63.

- [ ] Controller availability and device type update through native services, including connection and removal during setup.
- [ ] Existing controller options and primary and alternate mappings retain their native values and supported conditions.
- [ ] Native capture, selection, reset, cancel, apply, and mapping conflict behavior are preserved.
- [ ] Mapping input is routed once and does not also activate setup controls or reach a streaming console.
- [ ] Profile changes and device loss invalidate only the relevant pending mapping operation and show a recoverable outcome.
- [ ] Embedded tests verify native mapping results, reset and cancel, device removal, and controller navigation after the mapping flow.
