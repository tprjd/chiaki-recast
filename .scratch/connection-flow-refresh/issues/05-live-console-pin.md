# 05: Live console login PIN across connection routes

**What to build:** A current session can request a console login PIN through layout A, report an incorrect PIN, and continue after acceptance. The PSN prompt, stream prompt, and separate OpenGL window share validation, privacy, cancellation, and focus behavior.

**Blocked by:** 01: Local connection progress, cancellation, and retry; 02: Save a console login PIN.

**Status:** ready-for-agent

Parent: Connection progress and console login PIN: guided workspace. Specification commit: `cb1f76e9`.
Selected design: A on prototype branch `codex/prototype-connection-flows`, implementation commit `7a0111b6`.

- [ ] Reuse 02's PIN content and controls for live initial entry, incorrect PIN, submitting/waiting, accepted/handoff, cancelling, and cancelled. A's guidance explains that live entry applies to this session only.
- [ ] Forward the native incorrect-PIN flag to the current prompt. A rejected saved PIN produces a correction prompt; automatic saved-PIN use otherwise retains its current behavior.
- [ ] Exactly four ASCII digits enable Sign in; preserve leading zeros. Prevent repeated submission while waiting. Clear rejected/transient input for a new request without losing the current session context.
- [ ] Use actual native/session progress for accepted continuation. Do not infer PIN acceptance from a timer. Continue to the appropriate connection or stream state without an extra success confirmation.
- [ ] Live submission never changes the saved PIN. The entry value is masked by default; Show/Hide and streamer mode use the same behavior as saved PIN entry.
- [ ] Remove PIN values from the existing live-PIN log output and any new diagnostics. Do not include PINs in test reports or captures. Keep the current saved-PIN storage model unchanged.
- [ ] The PSN and stream routes show one active prompt. Repeated requests update it. Requests during route handoff, old-session events, profile changes, and session removal cannot create duplicate or obsolete prompts.
- [ ] Use 01's current-attempt ownership and cancellation handling. Cancel, Escape, and separate-window close stop the current pending session once, without sleeping the console. An action from an old prompt cannot stop or sign into a replacement session.
- [ ] The separate OpenGL window uses the same live content in a compact layout. Position it relative to its host and update geometry after moves or scale changes. Keep all controls reachable.
- [ ] Capture and release input exactly once per active presentation. Focus starts in the PIN field. Background commands cannot pass through the prompt. Closing, replacement, and session removal clear transient input and restore valid focus.
- [ ] Production-QML tests exercise all live states, the wrong saved-PIN path, repeated requests/submission, leading zeros, privacy changes, route handoff, stale-session callbacks, and consistent cancellation inputs. Reuse the existing test runner and screen-level fake backend.
- [ ] Focused native checks verify incorrect-PIN propagation, submission to only the current session, unchanged saved PIN, absence of PIN logging, and both in-window and separate-window cancellation/input release. The normal build and relevant stream/connection regressions pass.

Demo: request a PIN, reject it, enter a corrected synthetic PIN, and continue. Repeat in the separate window; Escape and window close must cancel rather than hide a still-pending session.

Dependency reason: 01 owns current-session effects and cancellation; 02 supplies the shared PIN entry behavior. 03 is not a blocker because the PSN prompt exists before its visual refresh. Reconcile shared PSN/stream edits during integration and verify the finished routes in 06.
