# 08: Stream from a registered console through PSN

**What to build:** Users can refresh remote consoles, connect to a registered console through PSN, and return to web setup after native playback ends.

**Blocked by:** 04: Connect locally from web setup to native playback; 06: Sign in to PSN independently from web setup.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 12, 32, 33, 34, 43, 46, 47, 81.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] Remote console refresh displays real native results while retaining the existing local and remote console selection rules.
- [x] The selected native console key starts the current PSN connection path, including token refresh and all native connection phases.
- [x] Native progress, cancellation limits, retry, console login PIN handling, and error recovery retain their current behavior.
- [x] Worker completion is tied to the correct session, attempt, and profile. Cancelled or obsolete work cannot start or restore another session.
- [x] Separate native playback and setup restoration work for remote sessions with no media or raw input through WebChannel.
- [x] Embedded tests cover delayed refresh, expired credentials, unreachable consoles, late cancellation, stale profiles, and native cleanup.
- [ ] A real PSN connection confirms native playback and return to the accelerated web setup UI.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Comments

- Claimed for implementation after local handoff and PSN sign-in. Use the existing native connection attempt, worker, token refresh, and cleanup services. A registered console is available on this Windows PC. Linux and macOS GPU hardware checks are unavailable on this host.

## Answer

The implementation milestone is complete. Native refresh results reach the
web library. Opaque current-target commands use the existing PSN connection
attempt. The web page presents native token-wait progress and recovery.
The existing native window owns later connection progress, PIN entry,
playback, and cleanup.

Focused embedded checks cover delayed refresh, list failure with retained
selection, local suppression, expired credentials, cancellation, retry, stale
profiles, and privacy. The normal Windows native worker check passes 3/0.
Late successful worker replies after cancellation or a profile change cannot
start the producer. Both routes assert zero raw session-start calls. Native
session/window destruction and setup restoration pass.

[Verification](../evidence/ticket08/verification.md) records source identities,
executable hashes, exact logs, preserved native code, and capture limits.
Production request defaults remain real. Later fixtures isolate native
discovery before construction. Earlier discovery-isolation limits remain
explicit in the evidence.

Real PSN playback is unchecked. Root owns that separate hardware check with
the available registered console. Tickets 19 through 23 retain platform,
package, focus, live-media, default-preset, and performance acceptance.
Shared legacy setup resources still have QML callers and tests for later
tickets. Native stream components remain intact.
