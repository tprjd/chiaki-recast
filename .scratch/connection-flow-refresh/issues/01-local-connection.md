# 01: Local connection progress, cancellation, and retry

**What to build:** A user can connect to a local console through layout A, distinguish session connection from video waiting, cancel safely, read a persistent failure, and retry the intended console. Successful connection enters the stream automatically.

**Blocked by:** None (can start immediately).

**Status:** claimed

Parent: Connection progress and console login PIN: guided workspace. Specification commit: `cb1f76e9`.
Selected design: A on prototype branch `codex/prototype-connection-flows`, implementation commit `7a0111b6`.

- [ ] Local entry points show the selected console, route, task help, and actual connecting/video-waiting state using A. Reuse the existing setup buttons and visual conventions.
- [ ] Complete any necessary narrow refactoring first without changing the existing PSN, wake, PIN, or stream-menu behavior. Add only the presentation and backend/navigation boundary required by this complete local flow.
- [ ] Ready enters the stream without confirmation. Video-disabled sessions finish loading on connection. Session connection alone does not claim that a video frame has arrived.
- [ ] Connection start errors and session errors remain visible with details, Back to consoles, and Try again. Unrelated root errors retain their existing behavior.
- [ ] The attempt retains the intended target, profile, and route context. Non-library entry points retain their own valid connection context. Retry resolves the target again and starts once, only after old cleanup ends.
- [ ] Removed, hidden, ambiguous, or profile-invalid targets cannot be retried. Explain the invalid target and provide Back to consoles. Reordering cannot change the selected console.
- [ ] Cancel and existing keyboard/controller inputs stop once without sleeping the console. Show cancelling until actual cleanup completes, then cancelled with Back to consoles. Existing input routes cannot bypass a disabled action.
- [ ] Old completion, error, and PIN events cannot replace a newer attempt or closed view. Leave compatible entry points for the PSN, wake, and live-PIN tickets to use the same attempt ownership and cancellation rules.
- [ ] Streamer mode hides private console values and details. Untrusted names and messages use plain text. Long content wraps, status does not depend on color alone, and navigation restores a valid focus target.
- [ ] Production-QML tests in the existing runner drive all local states, start failure, video-disabled readiness, retry, duplicate actions, cancellation races, late events, view removal, and target/profile invalidation. Use a fake backend at the screen boundary, not a copy of screen logic.
- [ ] Focused native contract checks prove target/session ownership and local retry/stop dispatch. Keep these checks isolated from user settings and real consoles.
- [ ] The local screen is usable at 900 × 600 and 1280 × 720. Relevant existing console/setup tests and the normal build pass. Active-stream controls and current PIN routes still work.

Demo: connect a synthetic local console, advance to video waiting, produce a failure, retry once, cancel, and return to the library. No real network connection is required for this demonstration.

Scope limit: PSN and wake presentation changes belong to 03 and 04. PIN redesign belongs to 02 and 05. Do not introduce a general connection workflow framework.
