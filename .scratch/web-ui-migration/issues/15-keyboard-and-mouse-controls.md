# 15: Configure keyboard and mouse bindings, response, and reset

**What to build:** Users can capture physical input natively, edit keyboard and mouse bindings, tune the existing mouse response settings, and reset the complete feature to its defined defaults.

**Blocked by:** 11: Open settings and change general session options.

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 50, 51, 52, 57, 58, 78, 79.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] All existing binding actions and slots appear with their current native values and supported conditions.
- [ ] Native capture supplies input identities and display names. DOM key names do not replace platform input normalization.
- [ ] Conflict detection, move or replace actions, clearing, cancellation, and validation retain their existing behavior.
- [ ] Capture does not send game input to a console or consume unrelated text entry. Held inputs and focus are released through native rules.
- [ ] Changes persist in the current profile and remain visible in the native live settings page.
- [ ] Mouse response fields retain their native units, ranges, curve behavior, platform conditions, and calibration rules.
- [ ] Native settings remain authoritative for validation, persistence, and response computation. The web UI changes no streaming input algorithm.
- [ ] The reset action restores all settings and bindings in its existing feature scope, including both the binding and response tabs.
- [ ] Reset confirmation, cancellation, profile handling, focus, and live native settings notifications retain their behavior.
- [ ] Existing native mouse response, gameplay capture, and reset regression checks still pass.
- [ ] Embedded tests verify capture completion and cancellation, conflicts, invalid input, profile changes, native saved bindings, numeric limits, response persistence, complete reset results, reset cancellation, and consistency with native live settings.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.
