# 05: Live console login PIN across connection routes

**What to build:** A current session can request a console login PIN through layout A, report an incorrect PIN, and continue after acceptance. The PSN prompt, stream prompt, and separate OpenGL window share validation, privacy, cancellation, and focus behavior.

**Blocked by:** 01: Local connection progress, cancellation, and retry; 02: Save a console login PIN.

**Status:** resolved

Parent: Connection progress and console login PIN: guided workspace. Specification commit: `cb1f76e9`.
Selected design: A on prototype branch `codex/prototype-connection-flows`, implementation commit `7a0111b6`.

- [x] Reuse 02's PIN content and controls for live initial entry, incorrect PIN, submitting/waiting, accepted/handoff, cancelling, and cancelled. A's guidance explains that live entry applies to this session only.
- [x] Forward the native incorrect-PIN flag to the current prompt. A rejected saved PIN produces a correction prompt; automatic saved-PIN use otherwise retains its current behavior.
- [x] Exactly four ASCII digits enable Sign in; preserve leading zeros. Prevent repeated submission while waiting. Clear rejected/transient input for a new request without losing the current session context.
- [x] Use actual native/session progress for accepted continuation. Do not infer PIN acceptance from a timer. Continue to the appropriate connection or stream state without an extra success confirmation.
- [x] Live submission never changes the saved PIN. The entry value is masked by default; Show/Hide and streamer mode use the same behavior as saved PIN entry.
- [x] Remove PIN values from the existing live-PIN log output and any new diagnostics. Do not include PINs in test reports or captures. Keep the current saved-PIN storage model unchanged.
- [x] The PSN and stream routes show one active prompt. Repeated requests update it. Requests during route handoff, old-session events, profile changes, and session removal cannot create duplicate or obsolete prompts.
- [x] Use 01's current-attempt ownership and cancellation handling. Cancel, Escape, and separate-window close stop the current pending session once, without sleeping the console. An action from an old prompt cannot stop or sign into a replacement session.
- [x] The separate OpenGL window uses the same live content in a compact layout. Position it relative to its host and update geometry after moves or scale changes. Keep all controls reachable. Logical geometry is tested here; visible native scale changes remain in 06.
- [x] Capture and release input exactly once per active presentation. Focus starts in the PIN field. Background commands cannot pass through the prompt. Closing, replacement, and session removal clear transient input and restore valid focus. Visible native focus and input delivery remain in 06.
- [x] Production-QML tests exercise all live states, the wrong saved-PIN path, repeated requests/submission, leading zeros, privacy changes, route handoff, stale-session callbacks, and consistent cancellation inputs. Reuse the existing test runner and screen-level fake backend.
- [x] Focused native checks verify incorrect-PIN propagation, submission to only the current session, unchanged saved PIN, absence of PIN logging, and both in-window and separate-window cancellation/input release. The normal build and relevant stream/connection regressions pass.

Demo: request a PIN, reject it, enter a corrected synthetic PIN, and continue. Repeat in the separate window; Escape and window close must cancel rather than hide a still-pending session.

Dependency reason: 01 owns current-session effects and cancellation; 02 supplies the shared PIN entry behavior. 03 is not a blocker because the PSN prompt exists before its visual refresh. Reconcile shared PSN/stream edits during integration and verify the finished routes in 06.

## Answer

Implemented A with one `LiveConsolePinHost` in `Main.qml`. Both renderers use `LiveConsolePinContent`, which reuses 02's controls. The native `sessionPinRequest` map contains request and attempt IDs, phase, incorrect status, and console name. It contains no PIN. Each form captures its request ID. `enterPin(requestId, value)`, `cancelSessionPin(requestId)`, and `dismissSessionPin(requestId)` reject obsolete requests. Submission and cancellation also recheck the captured session after synchronous notifications, so a replacement session cannot receive the old action.

Native connection events drive submission, correction, acceptance, and cancellation. A pending PSN prompt stays active during the StreamView handoff. Repeated requests do not duplicate it. Saved-PIN automatic use is tried once, and a rejected saved PIN opens correction entry. Live entry does not write settings. The old value log is removed.

The native handoff test found an unmatched `StreamView.releaseInput()` during view construction. The wrapper now releases only a recorded grab. The same test then passed with two captures while PSN progress and the prompt overlap, one after PIN acceptance, and zero after ready. Captured attempt IDs and the retiring-view guard also prevent a delayed StackView destructor or focus handler from changing the replacement attempt.

Verification:

- Final production-QML suites passed: live PIN **23**, local connection **12**, PSN **11**, wake **13**, and saved PIN **23**. The [red log](../evidence/ticket05-tdd-red.txt) and final `ticket05-*-final.txt` logs are retained. An earlier full screen run passed **681** checks; the final integrated source was checked with the focused suites above.
- Actual `StreamSession` event dispatch passed **99 assertions per mode**: [inline](../evidence/ticket05-native-inline.json) and [separate hidden window](../evidence/ticket05-native-separate.json). These checks include wrong saved PIN, correction, repeated submission, stale events, notification reentry, Cancel, Escape, close, profile invalidation, and the full PSN-to-StreamView pending-prompt composition. The [capture regression](../evidence/ticket05-native-capture-red.json) records the failure before the guarded release.
- Each mode passed **5 post-exit checks** for unchanged synthetic settings records and absence of PIN values in runtime logs: [inline](../evidence/ticket05-post-exit-inline.json), [separate](../evidence/ticket05-post-exit-separate.json). All settings used a new explicit isolated INI path. Discovery and network transports were disabled at the native boundary; the real StreamSession constructor and event handler ran.
- Empty-field A captures are retained at [900](../evidence/ticket05-A-900.png), [1280](../evidence/ticket05-A-1280.png), and [compact](../evidence/ticket05-A-compact.png). The normal native application and screen-test targets built. The [production manifest](../evidence/ticket05-normal-production-manifest.json) records all 178 GUI source hashes and the unlaunched executable hash. The [cache record](../evidence/ticket05-cache-restoration.json) confirms restored baseline hashes, configuration, normal rebuild, and absence of test hooks.

Reusable native helpers remain outside production at `C:/Users/david/.cache/chiaki-ui-native/live-pin-contract-prep01/`, with the tested source hashes, patch, helper source, and restoration manifest. Run 08 and run 09 keep the full native outputs in adjacent `live-pin-contract-run08/` and `live-pin-contract-run09/` directories. The worktree's `build/prepare-live-pin-native.py`, `build/run-live-pin-native.ps1`, and `build/check-live-pin-post-exit.py` provide preparation, execution, and settings checks.

Limits: native windows were hidden, and the separate test window was not shown. These results prove native request ownership, state, stop calls, and capture accounting. They do not prove visible native focus, input delivery, display-scale changes, or physical-console behavior. Those acceptance checks remain with 06. No normal native window was launched for this ticket.
