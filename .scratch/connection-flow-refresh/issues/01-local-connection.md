# 01: Local connection progress, cancellation, and retry

**What to build:** A user can connect to a local console through layout A, distinguish session connection from video waiting, cancel safely, read a persistent failure, and retry the intended console. Successful connection enters the stream automatically.

**Blocked by:** None (can start immediately).

**Status:** resolved

Parent: Connection progress and console login PIN: guided workspace. Specification commit: `cb1f76e9`.
Selected design: A on prototype branch `codex/prototype-connection-flows`, implementation commit `7a0111b6`.

- [x] Local entry points show the selected console, route, task help, and actual connecting/video-waiting state using A. Reuse the existing setup buttons and visual conventions.
- [x] Complete any necessary narrow refactoring first without changing the existing PSN, wake, PIN, or stream-menu behavior. Add only the presentation and backend/navigation boundary required by this complete local flow.
- [x] Ready enters the stream without confirmation. Video-disabled sessions finish loading on connection. Session connection alone does not claim that a video frame has arrived.
- [x] Connection start errors and session errors remain visible with details, Back to consoles, and Try again. Unrelated root errors retain their existing behavior.
- [x] The attempt retains the intended target, profile, and route context. Non-library entry points retain their own valid connection context. Retry resolves the target again and starts once, only after old cleanup ends.
- [x] Removed, hidden, ambiguous, or profile-invalid targets cannot be retried. Explain the invalid target and provide Back to consoles. Reordering cannot change the selected console.
- [x] Cancel and existing keyboard/controller inputs stop once without sleeping the console. Show cancelling until actual cleanup completes, then cancelled with Back to consoles. Existing input routes cannot bypass a disabled action.
- [x] Old completion, error, and PIN events cannot replace a newer attempt or closed view. Leave compatible entry points for the PSN, wake, and live-PIN tickets to use the same attempt ownership and cancellation rules.
- [x] Streamer mode hides private console values and details. Untrusted names and messages use plain text. Long content wraps, status does not depend on color alone, and navigation restores a valid focus target.
- [x] Production-QML tests in the existing runner drive all local states, start failure, video-disabled readiness, retry, duplicate actions, cancellation races, late events, view removal, and target/profile invalidation. Use a fake backend at the screen boundary, not a copy of screen logic.
- [x] Focused native contract checks prove target/session ownership and local retry/stop dispatch. Keep these checks isolated from user settings and real consoles.
- [x] The local screen is usable at 900 × 600 and 1280 × 720. Relevant existing console/setup tests and the normal build pass. Active-stream controls and current PIN routes still work.

Demo: connect a synthetic local console, advance to video waiting, produce a failure, retry once, cancel, and return to the library. No real network connection is required for this demonstration.

Scope limit: PSN and wake presentation changes belong to 03 and 04. PIN redesign belongs to 02 and 05. Do not introduce a general connection workflow framework.

## Answer

Implemented local progress in production `ConnectionView.qml`, hosted by
`StreamView.qml`. Main routing also opens this screen for synchronous start
failures. Ready removes the progress screen automatically. Existing PSN, wake,
live-PIN, and active-stream presentations remain available for their tickets.

The native backend now owns each attempt and its profile generation. Library
targets use the existing discovered MAC, manual address and MAC, or PSN DUID
identity. Retry resolves that identity again. Direct entry retains its actual
address and credentials, and validates their registration in the same profile.
Session callbacks capture the session object and attempt generation. Old
callbacks cannot clear a replacement session or display an obsolete prompt.

Verification completed:

- The first production-QML test failed because `ConnectionView` did not exist.
  The final local suite reports 11 checks passed, with no failures.
- Existing console and setup suites reported 270 checks passed before the
  ticket 02 merge. After the merge, local, saved-PIN, and console-management
  suites reported 11, 23, and 44 checks passed.
- The hidden native run uses real `QmlBackend`, `Main.qml`, `StreamView.qml`,
  and `StreamSession` objects. It controls Start and Stop at the transport
  boundary. Run 12 passed with ten starts, four stops, and no sleep dispatch.
- Native checks cover connecting, video waiting, readiness, normal quit and
  later discovery, start failure, retry after row reorder, duplicate actions,
  cancellation cleanup, old session events, error reentry, view removal, clean stop before readiness, empty-ID rejection,
  profile changes during an active attempt, and invalid retry targets.
- Native checks also prove video-disabled readiness and direct-entry retry
  to its original address. Post-exit INI checks confirm that cancellation
  preserves the saved auto-connect preference and leaves the other profile
  without registrations.
- Production-QML captures show progress and failure at 900 by 600 and 1280
  by 720. The parent agent inspected these captures.
- The normal application built with ticket 02 merged. Its source hashes
  match the worktree. The executable contains no native QA helper tokens.
  The shared native cache was then restored by hash and rebuilt normally.

Evidence is under `../evidence/ticket01-*`, including the native assertions,
observed results, post-exit checks, captures, and production manifest.
The normal executable is
`C:/Users/david/.cache/chiaki-ui-native/connection-local-bin/chiaki.exe`.

The hidden native window cannot prove visible keyboard input capture or native
display scaling. Visible input, separate PIN windows, active-stream interaction,
and cross-route acceptance remain explicit checks in ticket 06. No real PSN
authentication, console wake-up, hardware PIN acceptance, live video transport,
or physical controller check is claimed here.

### Interface for tickets 03, 04, and 05

`connectionAttempt` contains these public fields:

| Field | Meaning |
| --- | --- |
| `id` | Opaque string for one native attempt |
| `route` | `local` or `psn` |
| `phase` | `connecting`, `video`, `ready`, `failure`, `cancelling`, or `cancelled` |
| `name`, `address`, `ps5` | Known console presentation values |
| `errorTitle`, `errorText` | Persistent error values, masked by streamer mode in the view |
| `cleanupPending` | The attempt still has a session that must stop before retry or return |
| `targetValid` | Current resolution against the captured profile and target identity |
| `presented` | The attempt still owns its screen |
| `cancelable` | Local presentation allows Cancel. PSN must bind this to its genuine restrictions. |
| `waitingForWake` | Existing wake presentation owns the allocated local session |
| `registrationOnly` | Captured automatic-registration intent |

`retryConnection(id)`, `cancelConnection(id)`, and `dismissConnection(id)`
return whether the command was accepted. Retry requires failure, completed
cleanup, a valid target, and the same attempt ID. Dismissal cancels an active
local attempt before it removes screen ownership. Normal ready-session quit
also removes ownership, so later discovery cannot reopen StreamView.

Tickets 03 and 04 must extend the existing PSN and wake policies through this
contract. Their earlier token, internet, registration, and wake-wait stages still
use the old presentation. In particular, an allocated wake session has not
started yet and must use its existing cleanup condition. Do not wait for a quit
event from a session that never started. PSN cleanup must also update the
contract before retry becomes available.

`sessionPinDialogRequested(bool incorrect)` now carries the native incorrect
flag. Native session ownership guards suppress old requests. Ticket 05 must
bind prompt submission and dismissal to the captured attempt ID. Existing
`enterPin` remains the old submission entry point until that ticket.

The QA helper and backups are in
`C:/Users/david/.cache/chiaki-ui-native/connection-contract-prep-final/`.
`connection-qa.patch` records the exact temporary native changes. The helper
loads production Main through `QmlMainWindow::show`, while QA patches suppress
the native show methods. Settings use explicit INI paths, discovery is disabled,
and session log creation is disabled. Reuse a fresh run directory. The actual
assertion source is also retained as `ticket01-native-contracts.cpp`.

Preparation and restoration scripts remain in this worktree's `build/`
directory. `prepare-connection-native.py` accepts a fresh stage directory name.
`restore-connection-native.py connection-contract-prep-final` restores this
run. The preparation script uses this worktree as its source. Change that path
before reuse for a later integrated revision. Restoration verifies each
original hash and touches restored files
before rebuilding, so Ninja cannot reuse QA objects. Cache ownership must stay
with one agent at a time. The cache is restored and released.
