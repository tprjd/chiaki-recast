# 09: Register a console automatically through PSN

**What to build:** Users can choose the existing automatic PSN registration method and receive its native result while remaining in web setup.

**Blocked by:** 08: Stream from a registered console through PSN.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 16, 21, 24, 25, 26, 27, 32, 46.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] A complete automatic PSN registration action resolves the intended console and current registration context. It works independently of the manual pairing form; the combined two-method chooser is checked at final acceptance.
- [x] Extend the shared native context so manual pairing accepts the same current discovered local console with a DUID that automatic registration accepts. Keep remote-only PSN rows rejected by manual pairing. Check both methods for that same local target, including changed or duplicate discovery rows, changed addresses, and changed profiles. This carries the unchecked discovered-DUID case from ticket 07.
- [x] Automatic registration uses the native registration-only connection path and keeps setup visible.
- [x] Any native renderer resources needed by the internal session are prepared without presenting a playback handoff.
- [x] Progress, cancellation limits, success, failure, diagnostics, and retry use the existing native attempt state.
- [x] The correct registration is saved once. Late callbacks, changed profiles, and removed targets cannot save an unrelated registration.
- [x] Session, worker, and hidden native resources are cleaned up before another request or playback attempt starts.
- [x] Embedded tests distinguish registration-only work from playback and verify saved native state and cleanup.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Comments

- Claimed after native PSN streaming. Reuse native registration-only attempts, profile/target guards, and hidden native resource preparation. Manual pairing is finishing separately; retain its form and completion interface.


## Answer

Automatic registration is available as an independent library action. It resolves
an opaque current target and uses the native registration-only attempt, native
completion, save-once guard, progress, retry, and cancellation limits. A real
hidden renderer is prepared while setup stays visible. Native worker, session,
and hidden resources are released before retry or playback. Stream geometry is
not changed by registration-only work.

The shared manual context now accepts the same discovered local console with a
DUID. The focused real-page case verifies changed addresses, duplicate targets,
native manual completion, remote-only rejection, and old-profile rejection.
Manual and automatic operations cannot overlap. Changed DUIDs, removed targets,
wrong returned MACs/types, old profiles, duplicate completion, and the PS4
nickname condition cannot save an unrelated registration.

All ten automatic data rows and affected manual form/progress cases pass across
the two final groups. Full application/harness builds and Svelte checks pass.
The latest controller integration is merged. A bounded source audit preserves
passing evidence without repeating runtime groups. See
[verification](../evidence/ticket09/verification.md) for logs, identities, the
red/green sequence, corrected fixture failures, and limits.

Live PSN and console registration, physical focus/input, graphics performance,
packages, and the combined method chooser remain in ticket 23. The existing
native PS4/PS5 same-nickname selection rule was preserved, not certified. No
new layout work below 1024 CSS pixels was done.
