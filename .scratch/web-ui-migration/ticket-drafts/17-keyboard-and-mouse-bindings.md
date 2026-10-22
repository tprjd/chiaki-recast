# 17: Edit keyboard and mouse mappings from web setup

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can enable keyboard and mouse options, capture physical input natively, and edit bindings with the current conflict behavior.

**Blocked by:** 13: Change general and stream settings from web setup.

**Status:** needs-info

**Spec stories:** 50, 51, 52, 57, 78, 79.

- [ ] All existing binding actions and slots appear with their current native values and supported conditions.
- [ ] Native capture supplies input identities and display names. DOM key names do not replace platform input normalization.
- [ ] Conflict detection, move or replace actions, clearing, cancellation, and validation retain their existing behavior.
- [ ] Capture does not send game input to a console or consume unrelated text entry. Held inputs and focus are released through native rules.
- [ ] Changes persist in the current profile and remain visible in the native live settings page.
- [ ] Embedded tests cover capture completion and cancellation, conflicts, invalid input, profile changes, and native saved bindings. Existing native input regression tests remain.
