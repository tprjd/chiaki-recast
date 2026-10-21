# 04: Auto-connect and console wake-up

**What to build:** Startup auto-connect and explicit wake-and-connect use layout A to show the selected console, actual waiting and wake progress, timeout, cancellation, and handoff to the applicable local or PSN connection route.

**Blocked by:** 01: Local connection progress, cancellation, and retry.

**Status:** resolved

Parent: Connection progress and console login PIN: guided workspace. Specification commit: `cb1f76e9`.
Selected design: A on prototype branch `codex/prototype-connection-flows`, implementation commit `7a0111b6`.

- [x] Startup auto-connect and a manual connection to a sleeping console both show A with the correct context. Do not describe every wake request as startup auto-connect.
- [x] Show waiting for the selected console, waiting for a requested wake-up, actual PSN fallback, console found/handoff, timeout, cancelling, and cancelled. No fake percentage, completed step, or countdown is shown.
- [x] Preserve current console selection, wake eligibility, local discovery, PSN fallback, and transport timeout policy. Show PSN fallback only when it starts. The local/PSN handoff reuses the existing route contract and current attempt context.
- [x] Console-found progress continues to the relevant connection route automatically. It does not claim that the stream is ready.
- [x] Wake timeout remains visible with useful help, Back to consoles, and a validated retry. The earlier wait and session cleanup must finish before retry. Re-resolve the intended console/profile without using a stale row index.
- [x] Cancel dispatches once and reflects actual completion. Preserve native cancellation restrictions and explain disabled actions. Repeated input and competing timeout/cancel events cannot produce duplicate commands or navigation.
- [x] Cancelling stops the current wait without sleeping the console or removing the saved auto-connect preference. It cannot start a delayed local connection or PSN fallback afterwards.
- [x] Old discovery, wake, timeout, and session callbacks cannot affect a replaced attempt, closed view, or another profile. Missing or invalid targets provide a clear route to select a console again.
- [x] The form follows streamer mode, plain text rendering, visible focus, long-text wrapping, and return-focus rules at 900 × 600 and 1280 × 720.
- [x] Production-QML tests cover startup and manual-wake entry, all waiting/result states, real-event handoffs, cancellation races, timeout retry, preference preservation, and late/profile-invalid events.
- [x] Isolated native contract checks verify wait cancellation, fallback suppression, unchanged saved preference, and current-target retry. The normal build and existing startup/console regressions pass.

Demo: start auto-connect, switch to wake waiting, cancel while discovery completes, and verify that no session starts. Show a timeout and retry. Drive an actual fallback event into the PSN route.

Dependency reason: 01 provides active-attempt ownership, validated retry, and local handoff. 03 is not a blocker because the existing PSN route can receive the handoff; 06 verifies both completed designs together.


## Answer

Implemented the selected layout A in production `AutoConnectView.qml`, which
uses the shared `ConnectionView.qml`. Startup auto-connect and manual console
wake-up have separate headings. Native events control waiting, wake progress,
console-found handoff, PSN fallback, timeout, and cancellation. There is no
presentation timer that completes the operation or closes a failure.

The existing attempt owns the selected MAC and profile during startup.
Discovery and actual PSN fallback retain its ID and context. A wake timeout
keeps its error and return actions. Retry resolves the current target again
and starts one new attempt. A removed target or changed profile blocks retry.
The existing five-second fallback, ten-second PSN suppression, and
25-second wake timeout remain unchanged.

Cancellation stops the current timers and prevents late discovery or fallback
from starting a session. An allocated wake session has not started, so cleanup
does not wait for a quit event. A session that already started still uses its
native stop completion. Late standby discovery through a manual address shows
wake progress and keeps that session. Ready discovery does not start it twice.
Cancellation does not send a sleep command or change the saved auto-connect
choice.

The shared view also captures its attempt before an exit transition. An old
view cannot dismiss a new attempt or take its focus. The ownership guard comes
from ticket 05 commit `e13e95df`. Ticket 04 adds the real `StackView` regression
and the related focus guard in commits `3d7ad7d0` and `7762cd51`.

Verification completed:

- The initial production-QML check failed because the old wake view had no
  shared backend/navigation interface. The fallback check failed before its
  event-controlled label existed.
- The real `StackView` check first proved that an exiting wake view dismissed
  a new attempt. A second check proved that it took focus during the exit.
  Both checks pass with the shared guard.
- The final wake suite reports 13 checks passed. Local connection reports
  11 checks passed. PSN reports 11 checks passed after the route merge.
- Existing console management, library, navigation, manual-host, and Settings
  console suites report 44, 35, 71, 23, and 20 checks passed.
- The isolated native startup check failed against source `9739e883` because
  startup did not own an attempt. The changed native code passed all 40
  assertions at production revision `2a854392`.
- Those native checks use real `QmlBackend`, production Main and views, real
  session objects, discovery handlers, session events, and the fallback timer.
  Start, Stop, wake, PSN, and token transport are controlled at their native
  boundary. Settings use explicit isolated INI paths. The window stays hidden.
- Native checks cover startup and manual context, timeout, reordered-target
  retry, immediate cleanup before Start, cleanup after Start, cancellation at
  discovery and entry, profile changes, removed targets, late callbacks,
  manual-address wake, and actual PSN fallback. They recorded three starts,
  three stops, seven wake requests, and no sleep request.
- Five post-exit INI checks confirm the saved auto-connect choice, registered
  targets, saved PINs, manual targets, and the empty second profile.
- Six production-QML captures show waiting, wake progress, and timeout at
  900 by 600 and 1280 by 720. The parent agent inspected the captures.
- The normal application and screen-test target built at `2a854392`. All
  174 staged GUI source hashes matched that worktree revision. The normal
  executable contains no QA helper tokens, and production QML resources
  contain no test fixture or prototype.
- The shared native cache was restored by all 174 original hashes. Its
  original output configuration and normal application/test build were also
  restored before release to ticket 05.

The native and normal-build records remain tied to `2a854392`, before the
shared exit guard. The guard has the final production-QML proof above.
Ticket 05 owns the later integrated normal build. Ticket 06 owns visible
native input, renderer, window, and display-scale acceptance.

Evidence is in `../evidence/ticket04-*`. The normal executable is
`C:/Users/david/.cache/chiaki-ui-native/connection-wake-bin/chiaki.exe`.
The helper, exact QA patch, backups, preparation script, and restoration
script remain in
`C:/Users/david/.cache/chiaki-ui-native/wake-contract-green-prep/`.
The helper scripts use this worktree and fresh isolated run directories.
Change their source path before reuse. Cache ownership remains exclusive.

No real console wake-up, PSN authentication, hardware PIN acceptance, video
transport, or physical controller check is claimed.

### Interface added for the wake flow

- `context` is `startup` or `wake` while the attempt retains that entry context.
- `waitingForWake` gives the wait view ownership before local or PSN handoff.
- `phase` also accepts `waiting`, `waking`, and the brief `found` handoff.
- `fallback` becomes true when the actual PSN fallback route starts.
- The existing attempt ID continues through handoff. Explicit retry gets a
  new ID after current-target validation and completed cleanup.
