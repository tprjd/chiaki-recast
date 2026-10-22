# 18: Tune mouse response and reset keyboard and mouse controls

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can adjust mouse response using the existing native calibration and reset the keyboard and mouse feature to its defined defaults.

**Blocked by:** 17: Edit keyboard and mouse mappings from web setup.

**Status:** needs-info

**Spec stories:** 50, 51, 52, 57, 58, 79.

- [ ] Mouse response fields retain their native units, ranges, curve behavior, platform conditions, and calibration rules.
- [ ] Native settings remain authoritative for validation, persistence, and response computation. The web UI changes no streaming input algorithm.
- [ ] The reset action restores all settings and bindings in its existing feature scope, including the values introduced by the binding editor.
- [ ] Reset confirmation, cancellation, profile handling, focus, and live native settings notifications retain their behavior.
- [ ] Embedded tests verify representative numeric limits, persistence, complete reset results, cancellation, and consistency with native live settings.
- [ ] Existing native mouse response, gameplay capture, and reset regression checks still pass.
