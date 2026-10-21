# 06: Integrated connection and PIN native acceptance

**What to build:** Deliver one integrated native application in which every approved connection and PIN route uses A and transitions correctly. Prove the complete flow with isolated native checks, retain evidence, and produce a normal build without test helpers.

**Blocked by:** 03: PSN connection progress and recovery; 04: Auto-connect and console wake-up; 05: Live console login PIN across connection routes.

**Status:** claimed

Parent: Connection progress and console login PIN: guided workspace. Specification commit: `cb1f76e9`.
Selected design: A on prototype branch `codex/prototype-connection-flows`, implementation commit `7a0111b6`.

- [x] Integrate 01–05 without losing the completed Library, Settings, console setup, active-stream, or existing authentication behavior. Resolve shared routing/backend changes and any duplicate presentation introduced by parallel work.
- [x] Demonstrate all spec states across local connection, PSN, automatic registration, startup auto-connect, manual wake, saved PIN, and live PIN. Ready states hand off automatically. Saved, registered, cancelled, and failure results provide their specified actions.
- [x] Verify the complete wake-to-local and wake-to-PSN paths, PSN-to-video waiting, pending PIN during handoff, and retry after stopped/failed attempts. Repeated actions, target reorder/removal, profile changes, and late events cannot affect the wrong session or console.
- [x] Run the existing Qt Quick runner with the new production-view tests and relevant console, setup, and Settings regressions. Record actual pass/fail results and resolve failures caused by this feature.
- [x] Native acceptance uses synthetic consoles, isolated settings, disabled discovery, and controlled backend events. Do not use the user's real account or saved console records for automation.
- [x] Verify 900 × 600 and 1280 × 720 logical pixels at measured DPR 1, 1.5, and 2. Capture representative progress, error, result, and PIN states. Check expanded translated labels, long values, wrapping, scrolling, visible focus, and reachable actions.
- [x] Check the in-window PIN routes and separate OpenGL window, including host movement, scale changes, input capture/release, blocked background commands, Escape, window close, and return focus. Cover mouse and keyboard behavior and report physical-controller coverage separately.
- [x] Verify saved-PIN persistence after process exit for only the intended synthetic console. Verify that live PIN entry leaves stored PINs unchanged and no PIN appears in affected logs, error details, or evidence.
- [x] Verify persistent connection errors, cleanup-gated retry, correct cancellation, unchanged auto-connect preference, suppressed late fallback, and video-disabled readiness through native handlers rather than QML fake behavior alone.
- [x] Build the normal application after acceptance and verify that production sources match the tested change. Test fixtures, synthetic data, browser prototypes, captures, and QA helpers must be absent from shipped resources and the executable.
- [x] Review the completed change against repository standards and the approved spec. Resolve actionable findings. Retain concise source/build manifests and acceptance evidence with the resulting revision.
- [x] Report real PSN authentication, physical console wake-up, hardware PIN acceptance, live streaming, and physical-controller checks as unverified unless they were actually run. A normal build and fake-backend checks do not prove them.

Demo: run the integrated connection-to-PIN flow in the native app at both target sizes, cancel and retry it, save a PIN in isolated storage, and show a normal executable built without QA instrumentation.

Dependency reason: 03, 04, and 05 transitively include 01 and 02. This ticket checks cross-route behavior and native integration; it does not defer the individual tickets' implementation or tests.


## Answer

Integrated 01 through 05 and completed controlled native acceptance at production revision `125e9284`. All criteria above have evidence or an explicit hardware limit in [acceptance.md](../acceptance.md). Status remains claimed until the parent merges this work.

The full production-view runner passed 711 checks after the sidebar fix. The final focused suites passed 83 checks after the review fixes. Native PSN, wake, and live-PIN contracts passed 64, 40, and 89 assertions. Visible OpenGL checks passed 623 at DPR 1 and 388 each at DPR 1.5 and 2. Actual Vulkan inline PIN passed 78. Saved-PIN failure and registration-result checks passed 90. Both required client sizes passed, with 30 retained native captures. Root also verified OS mouse and keyboard input in the isolated separate PIN window.

Native handlers cover the required states, wake and PSN handoffs, pending PIN continuity, persistent errors, cleanup-gated retry, late events, profile changes, target changes, and cancellation. Post-exit INI checks confirm the intended saved PIN, unchanged peer and live-PIN storage, and unchanged preferences. No PIN appears in runtime logs, JSON reports, or captures.

The sidebar wrap and color-route guards have red and green proof. Review recovery fixes retain authentication after token cleanup and allow explicit-credential CLI retry without saved records. Profile changes and removal of originally stored targets still invalidate old actions. The duplicated reconnect timer callback is now one private handler. Root's final Standards and Spec reviews report zero unresolved findings against review base `a0f9dbf0` and final source `125e9284`.

The normal app built from all 178 source hashes used for acceptance. Its executable, resources, and link contain no QA artifacts. The normal build is `C:/Users/david/.cache/chiaki-ui-native/connection-flow-final-bin/chiaki.exe`, SHA256 `f63cb93305bbe58972e0deaa59cb1ab306c3e1a0641fcb6192244817f232219f`. It was not launched. All pre06 cache hashes and configuration are restored, and both baseline targets rebuilt. The cache is released. The complete reusable helper and backup manifest remain in `connection-acceptance-prep01` outside the worktree.

Evidence is in `../evidence/ticket06-*`. Real authentication, physical wake-up, hardware PIN acceptance, live streaming, a physical controller, and a live move between different monitor DPR values remain unverified. Both available screens used the same measured DPR during each process.
