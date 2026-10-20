# 03: PSN connection progress and recovery

**What to build:** A remote user follows the real PSN connection stages through layout A, can cancel at supported stages, can read and retry persistent errors, and continues into video waiting and the stream. Automatic registration remains a separate conditional route.

**Blocked by:** 01: Local connection progress, cancellation, and retry.

**Status:** ready-for-agent

Parent: Connection progress and console login PIN: guided workspace. Specification commit: `cb1f76e9`.
Selected design: A on prototype branch `codex/prototype-connection-flows`, implementation commit `7a0111b6`.

- [ ] Use A's status panel and help for waiting for internet, initiating connection, linking, data connection, video waiting, and ready. Show actual console/route context and actual completed stages.
- [ ] Automatic registration shows its own progress and completed result. Registration is not shown as a mandatory step for every PSN session. Completion offers Back to consoles; registration-only retry preserves that intent.
- [ ] Start failure, general failure, and unreachable-console outcomes remain visible with useful details, Back to consoles, and retry when valid. Unrelated authentication/token screens retain their existing routes.
- [ ] Reuse 01's active-attempt and target validation behavior. PSN retry starts one attempt after cleanup and cannot act on a changed profile, missing target, or stale session. No automatic retry loop is added.
- [ ] Cancel uses the existing PSN stop behavior once and respects genuine non-cancellable stages. Explain disabled cancellation. Keyboard, controller, right-click, and Ctrl+Q inputs follow the same decision.
- [ ] Cancelling remains visible until cleanup completes. Show cancelled with Back to consoles. Late PSN success, failure, registration, or PIN events cannot reopen the old route or affect another attempt.
- [ ] Data connection completion hands off to the existing session/video waiting condition. It must not show video ready before a frame, except for the existing video-disabled mode. Success never adds an extra confirmation step.
- [ ] PIN requests still reach the current live-PIN presentation while 05 is pending. The PSN-to-stream transition must not lose a pending PIN request or open duplicate prompts.
- [ ] Private console data and diagnostics follow streamer mode. Text wraps, keyboard/controller focus stays visible, and return focus is valid at both target sizes.
- [ ] Production-QML tests drive all native PSN states and transitions, conditional registration, disabled cancellation, cleanup, persistent errors, retry, route handoff, video-disabled mode, stale events, and profile/target changes.
- [ ] Focused isolated native contract checks verify PSN event ownership, cancel dispatch, registration-only retry intent, and the PSN-to-stream handoff. The normal build and affected routing regressions pass.

Demo: show PSN linking with cancellation disabled, advance to data connection, cancel and complete cleanup, retry a failed attempt, then hand off to video waiting. Demonstrate automatic registration as a separate branch.

Dependency reason: 01 provides the production attempt ownership, validated retry, and stream-waiting destination. 04 is not a blocker; ordinary PSN entry already exists.
