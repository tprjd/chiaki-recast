# Wake and startup auto-connect verification

Production source: `462894efae3cbd2dbffee1ba1498cd5a83741e2f`.
This source includes integration `5b026017683c7f1fe0af74e21deebe098d75d07b`.
The later ticket 10 commit adds this record and closes the local issue.

## Behavior

The web setup page uses opaque native keys to save an auto-connect target and
send a wake request. QmlSettings remains the persistent store. Wake credentials,
PSN credentials, saved PINs, and the selected native MAC stay in native code.

QmlBackend retains its startup timer, local discovery priority, wake timer,
connection attempt, profile guards, and native session path. Startup does not
depend on a browser event. The application controller prepares the native window
before session creation needs its resources. The existing constructor and queued
native completions already satisfy this ordering, so no controller change was
needed for ticket 10.

A wake-only request uses the same native connection attempt and wake timer. A
ready reply for the intended console completes the wait without a session. Wake
and connect retains native session preparation and its later discovery reply.
Stop auto-connect accepts only the current startup attempt. A stale stop cannot
cancel a later manual request. The saved auto-connect preference is kept.

The startup attempt retains its selected registration MAC and nickname through
PSN fallback. The native worker validates that target before accepting a late
success. A late console list re-checks the current automatic wait after the
native timer expires. A single eligible PS4 registration can use the native
Main PS4 remote route. Multiple eligible PS4 registrations do not cause a guess.
Ordinary manual PS4 nickname selection is unchanged.

Hidden-host updates invalidate an active wait before the native console list
applies its existing registered-host unhide rule. That library rule is unchanged.

## Test boundary and isolation

The tests use compiled Svelte, a real WebEngine view, real WebChannel, production
SetupBridge, native services, and ApplicationController. Isolated Settings stores
use the test application name and random profile names.

Native request adapters control HTTP, PSN device lists, PSN worker completion,
raw session start, discovery initialization and cleanup, and wake transport.
Unexpected transport work fails closed. No fixture can send a real wake packet
through the new wake boundary. Successful raw starts create a real compatible
`chiaki_thread`. Native Stop and session cleanup join that thread.

Startup checks disable page JavaScript before native work. They inspect native
session and window creation, then reload the compiled page and check recovery.
Discovery results enter through the real queued native completion. Native timers
are shortened only at the time boundary in timeout and fallback tests.

Root granted a separate runtime slot before each group. Routine checks use
offscreen WebEngine with test-only `--disable-gpu`. Native resource checks use
bounded normal Windows because Windows offscreen cannot create native GL
resources. Every launch uses an explicit QtTest text log and the ticket's own
SteamTools directory on PATH.

## Results

The result counts include QtTest initialization and cleanup.

| Group | Result |
| --- | --- |
| ticket10-red | 2 pass, 1 expected failure. The auto-connect selector is absent. |
| ticket10-target-green | 3 pass, 0 fail. Native save, restarted Settings, reload, and Off pass. |
| ticket10-wake-red | 2 pass, 1 expected failure. The wake action is absent. |
| ticket10-wake-green | 3 pass, 0 fail. Exact wake transport, progress, reload, unrelated replies, ready, and dismissal pass. |
| ticket10-cancel-red | 2 pass, 1 expected failure. The startup stop action is absent. |
| ticket10-cancel-green | 3 pass, 0 fail. Current startup stop, saved preference, stale stop, later manual wake, and late ready pass. |
| ticket10-failures-red | 7 pass, 1 fail. Transport, timeout, retry, duplicate, removed, and profile cases pass. Hidden target invalidation fails. |
| ticket10-hidden-green | 3 pass, 0 fail for the hidden case. The five other passing cases are reused. |
| ticket10-startup-red | 5 pass, 5 fail. PSN ready, cancellation, and profile cases pass. Removed and replaced targets wrongly start. Two local checks use an unfilled native hostname field. The synthetic PS4 store lacks the startup PS4 count cache. |
| ticket10-startup-guards | 7 pass, 1 fail. The corrected local ready and wake checks pass. PSN ready and removed/replaced late-success guards pass. A restarted PS4 store lists Main PS4, but name-only auto-connect cannot select it. |
| ticket10-ps4-green | 4 pass, 0 fail. The sole PS4 route and the ambiguous PS4 case pass. |
| ticket10-late-list-red | 2 pass, 1 expected failure. A list after the timer does not start a session. |
| ticket10-late-list-green | 2 pass, 1 fail. The native session starts, but the test requests Stop during the existing non-cancelable linking stage. |
| ticket10-late-list-green02 | 4 pass, 0 fail. The test waits for the public cancelable state. Late-list startup and the affected ticket 09 automatic registration success case pass. Production cancellation is unchanged. |

The local address check now reads the native resolved address. The PS4 fixture
now constructs Settings from saved data, as real startup does. The intermediate
failures and their exact pre-run source and executable identities remain here.
Only failed or affected cases were repeated.

Each group has `source.txt`, `full.diff`, `status.txt`, `executable.sha256`,
`run.cmd`, and `qt.txt`. The new-file queries returned no entries. PowerShell's
piped Set-Content did not create empty `new-files.txt` files. Each saved pre-run
status also confirms that there were no untracked source files. No missing file
is presented as a saved manifest.

## Final checks and reuse

The application and embedded harness build successfully in `ticket10-build`.
`npm run check` reports zero errors and zero warnings. The final build command,
build output, source hashes, submodule identities, and executable hashes are
stored here.

`source-preservation.txt` records a direct comparison. The last passing native
backend was reconstructed from its saved source commit and full diff. Final
backend code matches after removal of only two unused signal emissions. The
embedded harness matches after line-ending normalization.

The native application controller, StreamSession, native service constructors,
StreamRoot, StreamView, and ConnectionView match integrated ticket 09 source
`13b594df87f86da0412f7bd9d6dfde5dc17479b8`. Existing ticket 03/04/08/09 native
graphics, stream input, and handoff evidence remains applicable within its stated
limits. The canonical constructor order remains PsnRequests, ProcessLauncher,
RegistrationRequests, and DiscoveryRequests.

The caller audit found no production caller for AutoConnectView. Its resource,
fake ConnectionWake tests, unused wake signals, and fixture branch are removed.
Native ConnectionView and its local fixture remain. The unchanged local fixture
component compiles. Its runtime was not repeated for this leaf deletion.

## Limits

No result proves real-console wake-up, live PSN, real playback, foreground focus,
physical input, GPU acceleration, HDR, audio or microphone quality, latency,
performance, or packages. Root retains registered-console Windows acceptance in
ticket 23. Linux and macOS desktop hardware are unavailable. The existing native
HighQuality limitation is unchanged. No new layout work or capture below 1024
CSS pixels was added. The vendor VC patch remains local and excluded.
