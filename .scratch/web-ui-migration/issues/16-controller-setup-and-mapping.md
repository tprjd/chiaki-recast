# 16: Configure controllers and their mappings

**What to build:** Users can inspect available controllers, change supported controller options, and complete the existing native mapping flow from web setup.

**Blocked by:** 11: Open settings and change general session options.

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 50, 51, 52, 59, 63.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] Controller availability and device type update through native services, including connection and removal during setup.
- [ ] Existing controller options and primary and alternate mappings retain their native values and supported conditions.
- [ ] Native capture, selection, reset, cancel, apply, and mapping conflict behavior are preserved.
- [ ] Mapping input is routed once and does not also activate setup controls or reach a streaming console.
- [ ] Profile changes and device loss invalidate only the relevant pending mapping operation and show a recoverable outcome.
- [ ] Embedded tests verify native mapping results, reset and cancel, device removal, and controller navigation after the mapping flow.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Comments

- Claimed after general settings. Prepare the isolated worktree while pairing, PSN connection, and color controls are implemented. Reuse the native controller service and mapping flow; preserve stream input ownership.
