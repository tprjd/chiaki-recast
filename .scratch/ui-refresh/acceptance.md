# Native console library acceptance

Implemented on `codex/ui-refresh-integration` at `43a839d8c528c574f6e2e2e7ede961c253cf3c3b`.
All eight tickets are resolved. The approved spec and prototype remain unchanged.

The native QML library has responsive cards, search and counts, private details, current-identity actions, and keyboard/controller navigation. It keeps the existing connection and settings handlers. Native acceptance also found and fixed delayed dialog focus and missing mouse-wheel forwarding.

## Tests and review

- 190 production-screen Qt Quick checks pass with Qt 6.9.3 and MSVC. Integration CTest passes in 64.20 seconds. [Actual test output](native-previews/screen-08-suite.txt).
- Dark PS4/PS5 artwork and an absent primary background each fail the appearance guards. Restored source passes. [Mutation proof](native-previews/screen-08-contrast-mutation-red.txt).
- The complete Windows application builds with UCRT64 GCC and Qt 6.11.2. Both native CTest targets pass in 68.94 seconds.
- Independent [Standards and Spec reviews](code-review.md) each report zero findings. A separate focused-card reorder probe preserves selection and dispatches once to the current source index.
- Production resource lists exclude prototype routes, fake hosts, captures, and test files. QuickTest links only to the test executable, which has no install rule.

Reproducible screen commands are in [gui/tests/README.md](../../gui/tests/README.md).
Full native build, isolated-launch commands, dependency versions, QA helpers, and logs are recorded in [the local toolchain notes](C:/Users/david/.codex/ui-refresh-native-toolchain.md).

## Native behavior

The [management replay](native-previews/07-native-checks.json) verifies PIN validation and cancellation, exact on-disk PIN `0042`, an unchanged peer PIN, intended Delete, stale Delete rejection, repeated confirmation, and opener focus. No session starts, and all disposable records are removed.

The final integrated application also passes these isolated checks:

- [Startup reminders](native-previews/08-startup-checks.json), including order and saved choices.
- [Automatic-connect waiting and cancellation](native-previews/08-autoconnect-checks.json), without a connection.
- [Profile isolation, error recovery, and Steam entry/cancel](native-previews/08-gates-checks.json), without creating a shortcut.
- Synthetic PSN mapping with twelve manual rows, Main PS4 suppression, sleeping-name suppression, and invalid indices. No token or network is used.

The earlier native Delete/Escape failure remains in the evidence directory. It is fixed by ticket 07. The earlier native wheel failure is fixed by ticket 08.

## Display checks

All three logical sizes pass at measured DPR 1.0, 1.5, and 2.0. The integration agent verified each record and its image dimensions.

| Logical size | DPR 1.0 image | DPR 1.5 image | DPR 2.0 image |
| --- | --- | --- | --- |
| 1280 by 720 | 1280 by 720 | 1920 by 1080 | 2560 by 1440 |
| 800 by 600 | 800 by 600 | 1200 by 900 | 1600 by 1200 |
| 390 by 600 | 390 by 600 | 585 by 900 | 780 by 1200 |

Results: [DPR 1.0](native-previews/08-matrix-dpr1-checks.json), [DPR 1.5](native-previews/08-matrix-dpr1_5-checks.json), [DPR 2.0](native-previews/08-matrix-dpr2-checks.json).

These runs use process-local Qt scale overrides on the Windows desktop at its existing 125 percent setting. Each run checks actual DPR. Global Windows display settings are unchanged. They are Qt scaling checks on Windows, not separate OS display-setting tests.

Actual application framebuffer captures show [many cards at 150 percent](native-previews/08-matrix-dpr1_5-many-800x600.png) and [scrolled, masked details at 200 percent](native-previews/08-matrix-dpr2-details-fields-390x600.png). The integration agent inspected both. Wheel, Tab, last-card focus, footer focus, long/missing fields, search recovery, details actions, and Close remain reachable. All fixture cleanup checks pass, with no session and discovery disabled.

The [B comparison capture](native-previews/screen-08-B-1280x720.png) uses the production QML screen and SVG provider with sample records. It is separate from the full-application captures above.

## Limits

Real console discovery, ready/standby connection, manual streaming, network Wake, registration acceptance, PSN authentication/connection, stream return, and physical controller input remain untested. Those checks need hardware or accounts. Controller key mappings and action eligibility pass the screen tests.

Existing Settings and registration forms can clip content at 800 by 600. Their contents are outside this main-screen change. The main library is verified down to 390 by 600; arbitrarily smaller windows are not verified.

## Cleanup

Seven managed ticket worktrees are archived. Codex re-attached the clean baseline worktree after archive, so Git removed that obsolete checkout after a clean-state and merged-ancestry check. `git worktree list` now contains only the main integration checkout. All ticket branches remain recoverable.

The native cache source is clean at the final integration commit. QA instrumentation is disabled, the normal production executable is rebuilt, and all final QA processes have exited. [Final native summary](native-previews/08-final-native-summary.json) records the executable and SHA256. The integration agent checked its actual file hash and the clean source state.

Steam shortcut writes and real discovery Hide/unhide recovery were not checked. Only dialog entry/cancellation and screen action dispatch were verified for those operations.
