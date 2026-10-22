# 13: Change general and stream settings from web setup

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can open the settings sidebar, search implemented settings, and persist general and stream behavior changes through native services.

**Blocked by:** 03: Show live consoles in the GPU-accelerated web UI.

**Status:** needs-info

**Spec stories:** 48, 50, 51, 52.

- [ ] The selected settings sidebar and shared field components display native values, labels, units, defaults, and capability conditions.
- [ ] General and stream settings retain their current validated ranges, enums, native side effects, and restart requirements.
- [ ] Search routes to the implemented category or control and preserves appropriate focus. Later settings groups can join the same search behavior.
- [ ] Native state remains authoritative. Rejected writes do not appear saved, and changes survive page reload and application restart.
- [ ] Platform-specific controls and privacy masking retain their existing conditions.
- [ ] Embedded tests change settings through the real bridge, inspect native persisted values, and verify invalid input and reload behavior.
