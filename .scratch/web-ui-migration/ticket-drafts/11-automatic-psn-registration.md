# 11: Register a console automatically through PSN

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can choose the existing automatic PSN registration method and receive its native result while remaining in web setup.

**Blocked by:** 07: Register PS4 and PS5 consoles with a pairing code; 10: Stream from a registered console through PSN.

**Status:** needs-info

**Spec stories:** 16, 21, 24, 25, 26, 27, 32, 46.

- [ ] The registration method chooser retains the correct console and registration context and offers the existing supported methods.
- [ ] Automatic registration uses the native registration-only connection path and keeps setup visible.
- [ ] Any native renderer resources needed by the internal session are prepared without presenting a playback handoff.
- [ ] Progress, cancellation limits, success, failure, diagnostics, and retry use the existing native attempt state.
- [ ] The correct registration is saved once. Late callbacks, changed profiles, and removed targets cannot save an unrelated registration.
- [ ] Session, worker, and hidden native resources are cleaned up before another request or playback attempt starts.
- [ ] Embedded tests distinguish registration-only work from playback and verify saved native state and cleanup.
