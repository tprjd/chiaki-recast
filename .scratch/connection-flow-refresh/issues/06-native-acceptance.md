# 06: Integrated connection and PIN native acceptance

**What to build:** Deliver one integrated native application in which every approved connection and PIN route uses A and transitions correctly. Prove the complete flow with isolated native checks, retain evidence, and produce a normal build without test helpers.

**Blocked by:** 03: PSN connection progress and recovery; 04: Auto-connect and console wake-up; 05: Live console login PIN across connection routes.

**Status:** claimed

Parent: Connection progress and console login PIN: guided workspace. Specification commit: `cb1f76e9`.
Selected design: A on prototype branch `codex/prototype-connection-flows`, implementation commit `7a0111b6`.

- [ ] Integrate 01–05 without losing the completed Library, Settings, console setup, active-stream, or existing authentication behavior. Resolve shared routing/backend changes and any duplicate presentation introduced by parallel work.
- [ ] Demonstrate all spec states across local connection, PSN, automatic registration, startup auto-connect, manual wake, saved PIN, and live PIN. Ready states hand off automatically. Saved, registered, cancelled, and failure results provide their specified actions.
- [ ] Verify the complete wake-to-local and wake-to-PSN paths, PSN-to-video waiting, pending PIN during handoff, and retry after stopped/failed attempts. Repeated actions, target reorder/removal, profile changes, and late events cannot affect the wrong session or console.
- [ ] Run the existing Qt Quick runner with the new production-view tests and relevant console, setup, and Settings regressions. Record actual pass/fail results and resolve failures caused by this feature.
- [ ] Native acceptance uses synthetic consoles, isolated settings, disabled discovery, and controlled backend events. Do not use the user's real account or saved console records for automation.
- [ ] Verify 900 × 600 and 1280 × 720 logical pixels at measured DPR 1, 1.5, and 2. Capture representative progress, error, result, and PIN states. Check expanded translated labels, long values, wrapping, scrolling, visible focus, and reachable actions.
- [ ] Check the in-window PIN routes and separate OpenGL window, including host movement, scale changes, input capture/release, blocked background commands, Escape, window close, and return focus. Cover mouse and keyboard behavior and report physical-controller coverage separately.
- [ ] Verify saved-PIN persistence after process exit for only the intended synthetic console. Verify that live PIN entry leaves stored PINs unchanged and no PIN appears in affected logs, error details, or evidence.
- [ ] Verify persistent connection errors, cleanup-gated retry, correct cancellation, unchanged auto-connect preference, suppressed late fallback, and video-disabled readiness through native handlers rather than QML fake behavior alone.
- [ ] Build the normal application after acceptance and verify that production sources match the tested change. Test fixtures, synthetic data, browser prototypes, captures, and QA helpers must be absent from shipped resources and the executable.
- [ ] Review the completed change against repository standards and the approved spec. Resolve actionable findings. Retain concise source/build manifests and acceptance evidence with the resulting revision.
- [ ] Report real PSN authentication, physical console wake-up, hardware PIN acceptance, live streaming, and physical-controller checks as unverified unless they were actually run. A normal build and fake-backend checks do not prove them.

Demo: run the integrated connection-to-PIN flow in the native app at both target sizes, cancel and retry it, save a PIN in isolated storage, and show a normal executable built without QA instrumentation.

Dependency reason: 03, 04, and 05 transitively include 01 and 02. This ticket checks cross-route behavior and native integration; it does not defer the individual tickets' implementation or tests.
