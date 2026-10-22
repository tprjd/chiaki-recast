# 11: Open settings and change general session options

**What to build:** Users can open the settings sidebar, search implemented controls, and save general and session-behavior settings through native services.

**Blocked by:** 03: Show live consoles in the GPU-accelerated web UI.

**Status:** ready-for-agent

**Source:** [Web setup UI specification](../spec.md). Stories 48, 50, 51, 52.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] The selected settings sidebar and shared field components display native values, labels, units, defaults, and capability conditions.
- [ ] General and session-behavior settings retain their validated ranges, enums, native side effects, and restart requirements. This ticket owns the shared settings shell, search, and general session controls. Video profiles, audio, display, renderer, color mapping, input, profiles, and file actions belong to their respective tickets.
- [ ] Search routes to the implemented category or control and preserves appropriate focus. Later settings groups can join the same search behavior.
- [ ] Native state remains authoritative. Rejected writes do not appear saved, and changes survive page reload and application restart.
- [ ] Platform-specific controls and privacy masking retain their existing conditions.
- [ ] Embedded tests change settings through the real bridge, inspect native persisted values, and verify invalid input and reload behavior.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.
