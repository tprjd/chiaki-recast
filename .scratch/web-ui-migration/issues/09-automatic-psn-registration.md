# 09: Register a console automatically through PSN

**What to build:** Users can choose the existing automatic PSN registration method and receive its native result while remaining in web setup.

**Blocked by:** 08: Stream from a registered console through PSN.

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 16, 21, 24, 25, 26, 27, 32, 46.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] A complete automatic PSN registration action resolves the intended console and current registration context. It works independently of the manual pairing form; the combined two-method chooser is checked at final acceptance.
- [ ] Extend the shared native context so manual pairing accepts the same current discovered local console with a DUID that automatic registration accepts. Keep remote-only PSN rows rejected by manual pairing. Check both methods for that same local target, including changed or duplicate discovery rows, changed addresses, and changed profiles. This carries the unchecked discovered-DUID case from ticket 07.
- [ ] Automatic registration uses the native registration-only connection path and keeps setup visible.
- [ ] Any native renderer resources needed by the internal session are prepared without presenting a playback handoff.
- [ ] Progress, cancellation limits, success, failure, diagnostics, and retry use the existing native attempt state.
- [ ] The correct registration is saved once. Late callbacks, changed profiles, and removed targets cannot save an unrelated registration.
- [ ] Session, worker, and hidden native resources are cleaned up before another request or playback attempt starts.
- [ ] Embedded tests distinguish registration-only work from playback and verify saved native state and cleanup.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Comments

- Claimed after native PSN streaming. Reuse native registration-only attempts, profile/target guards, and hidden native resource preparation. Manual pairing is finishing separately; retain its form and completion interface.
