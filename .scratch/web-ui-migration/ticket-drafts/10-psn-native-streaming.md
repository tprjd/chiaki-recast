# 10: Stream from a registered console through PSN

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can refresh remote consoles, connect to a registered console through PSN, and return to web setup after native playback ends.

**Blocked by:** 04: Connect locally from web setup to native playback; 09: Sign in to PSN from web setup.

**Status:** needs-info

**Spec stories:** 12, 32, 33, 34, 43, 46, 47, 81.

- [ ] Remote console refresh displays real native results while retaining the existing local and remote console selection rules.
- [ ] The selected native console key starts the current PSN connection path, including token refresh and all native connection phases.
- [ ] Native progress, cancellation limits, retry, console login PIN handling, and error recovery retain their current behavior.
- [ ] Worker completion is tied to the correct session, attempt, and profile. Cancelled or obsolete work cannot start or restore another session.
- [ ] Separate native playback and setup restoration work for remote sessions with no media or raw input through WebChannel.
- [ ] Embedded tests cover delayed refresh, expired credentials, unreachable consoles, late cancellation, stale profiles, and native cleanup.
- [ ] A real PSN connection confirms native playback and return to the accelerated web setup UI.
