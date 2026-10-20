# 04: Auto-connect and console wake-up

**What to build:** Startup auto-connect and explicit wake-and-connect use layout A to show the selected console, actual waiting and wake progress, timeout, cancellation, and handoff to the applicable local or PSN connection route.

**Blocked by:** 01: Local connection progress, cancellation, and retry.

**Status:** ready-for-agent

Parent: Connection progress and console login PIN: guided workspace. Specification commit: `cb1f76e9`.
Selected design: A on prototype branch `codex/prototype-connection-flows`, implementation commit `7a0111b6`.

- [ ] Startup auto-connect and a manual connection to a sleeping console both show A with the correct context. Do not describe every wake request as startup auto-connect.
- [ ] Show waiting for the selected console, waiting for a requested wake-up, actual PSN fallback, console found/handoff, timeout, cancelling, and cancelled. No fake percentage, completed step, or countdown is shown.
- [ ] Preserve current console selection, wake eligibility, local discovery, PSN fallback, and transport timeout policy. Show PSN fallback only when it starts. The local/PSN handoff reuses the existing route contract and current attempt context.
- [ ] Console-found progress continues to the relevant connection route automatically. It does not claim that the stream is ready.
- [ ] Wake timeout remains visible with useful help, Back to consoles, and a validated retry. The earlier wait and session cleanup must finish before retry. Re-resolve the intended console/profile without using a stale row index.
- [ ] Cancel dispatches once and reflects actual completion. Preserve native cancellation restrictions and explain disabled actions. Repeated input and competing timeout/cancel events cannot produce duplicate commands or navigation.
- [ ] Cancelling stops the current wait without sleeping the console or removing the saved auto-connect preference. It cannot start a delayed local connection or PSN fallback afterwards.
- [ ] Old discovery, wake, timeout, and session callbacks cannot affect a replaced attempt, closed view, or another profile. Missing or invalid targets provide a clear route to select a console again.
- [ ] The form follows streamer mode, plain text rendering, visible focus, long-text wrapping, and return-focus rules at 900 × 600 and 1280 × 720.
- [ ] Production-QML tests cover startup and manual-wake entry, all waiting/result states, real-event handoffs, cancellation races, timeout retry, preference preservation, and late/profile-invalid events.
- [ ] Isolated native contract checks verify wait cancellation, fallback suppression, unchanged saved preference, and current-target retry. The normal build and existing startup/console regressions pass.

Demo: start auto-connect, switch to wake waiting, cancel while discovery completes, and verify that no session starts. Show a timeout and retry. Drive an actual fallback event into the PSN route.

Dependency reason: 01 provides active-attempt ownership, validated retry, and local handoff. 03 is not a blocker because the existing PSN route can receive the handoff; 06 verifies both completed designs together.
