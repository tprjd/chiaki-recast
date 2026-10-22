# 15: Configure keyboard and mouse bindings, response, and reset

**What to build:** Users can capture physical input natively, edit keyboard and mouse bindings, tune the existing mouse response settings, and reset the complete feature to its defined defaults.

**Blocked by:** 11: Open settings and change general session options.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 50, 51, 52, 57, 58, 78, 79.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] All existing binding actions and slots appear with their current native values and supported conditions.
- [x] Native capture supplies input identities and display names. DOM key names do not replace platform input normalization.
- [x] Conflict detection, move or replace actions, clearing, cancellation, and validation retain their existing behavior.
- [x] Capture does not send game input to a console or consume unrelated text entry. Held inputs and focus are released through native rules.
- [x] Changes persist in the current profile and remain visible in the native live settings page.
- [x] Mouse response fields retain their native units, ranges, curve behavior, platform conditions, and calibration rules.
- [x] Native settings remain authoritative for validation, persistence, and response computation. The web UI changes no streaming input algorithm.
- [x] The reset action restores all settings and bindings in its existing feature scope, including both the binding and response tabs.
- [x] Reset confirmation, cancellation, profile handling, focus, and live native settings notifications retain their behavior.
- [x] Existing native mouse response, gameplay capture, and reset regression checks still pass.
- [x] Embedded tests verify capture completion and cancellation, conflicts, invalid input, profile changes, native saved bindings, numeric limits, response persistence, complete reset results, reset cancellation, and consistency with native live settings.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Answer

Implemented the web Keyboard and mouse category with native response values,
22 actions and both slots, and the existing legacy keyboard controls. Physical
input capture stays in a native Qt dialog. The existing native helpers retain
normalization, physical identity, validation, names, conflicts, move, clear,
reserved keys, and explicit Tab assignment.

Native form identities and profile generations reject stale commands. Form,
profile, page, window, and suspend changes cancel capture. Restoration renews
the form and preserves its tab. Web reset confirmation starts on Cancel and
retains its original form and profile. Both tabs restore the complete native
feature defaults without changing legacy preferences or unrelated settings.
The stream input algorithms, calibration values, ownership module, and shared
native live settings panes remain unchanged.

Focused compiled application checks pass capture, response/reset, guards,
legacy persistence and notifications, search, narrow/200-percent layouts,
and affected renderer settings after the integration merge. The final shared
settings group passes 5/0. A current-source GNU numeric response check passes
10/0. The full application and embedded test executable build. The frontend
check reports zero errors and warnings. Earlier valid passes are reused.

[Verification](../evidence/ticket15/verification.md) records source and binary
identities, commands, exact outcomes, source comparisons, images, historical
native evidence, and remaining limits. No physical calibration, live console,
Raw Input, GPU, or new platform desktop acceptance is claimed. Native defaults
remain provisional. Shared QML resources still have live stream callers.
