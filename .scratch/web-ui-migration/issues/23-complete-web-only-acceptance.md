# 23: Complete the web-only application and verify native performance

**What to build:** The complete packaged application provides GPU-accelerated web setup and separate native playback and overlays, with the required feature and performance evidence.

**Blocked by:** 05: Manage consoles and manual addresses from the web library; 07: Pair PS4 and PS5 consoles and retrieve an Account ID; 09: Register a console automatically through PSN; 10: Wake consoles and auto-connect from web startup; 12: Configure stream profiles and audio devices; 14: Configure native color mapping from web setup; 15: Configure keyboard and mouse bindings, response, and reset; 16: Configure controllers and their mappings; 17: Manage profiles, settings files, and logs; 18: Complete setup reminders and Steam shortcuts; 19: Ship the Windows web and native application; 20: Ship the Linux web and native application; 21: Ship the macOS web and native application; 22: Resume interrupted sessions and recover web setup.

**Status:** ready-for-agent

**Source:** [Web setup UI specification](../spec.md). Stories 3, 48, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] Every specification story is mapped to an implemented flow and acceptance evidence. The application offers one web setup path without a QML setup fallback or runtime UI selector.
- [ ] Audit remaining callers, resources, build references, and adapters for obsolete setup QML. Each migration ticket removes its replaced setup code when no caller needs it. Final acceptance resolves remaining obsolete references and preserves native stream QML, overlays, live settings, and their tests.
- [ ] All settings categories, linked search routes, input flows, account routes, reminders, confirmations, profiles, files, and Steam actions are reachable in the packaged application. The combined registration chooser offers manual pairing and automatic PSN registration with the correct context.
- [ ] Access checks cover keyboard, controller, touch, screen-reader labels, focus return, reduced motion, privacy, long text, required sizes, display scaling, and 200 percent zoom.
- [ ] The production embedded application checks use the real bridge and native services. Run checks affected by the final changes and reuse applicable passing feature evidence. Existing native input ownership and output checks retain coverage on their supported toolchains; an MSVC run must not claim coverage from GNU-only targets.
- [ ] Check each final Windows, Linux, and macOS package once with actual Chromium GPU acceleration enabled. Exercise the core native handoff and affected migrated features. Reuse deeper hardware evidence unless relevant native code, Qt, WebEngine, package contents, drivers, or configuration changed.
- [ ] Compare matched native baseline and migrated measurements against tolerances fixed before evaluation. Reuse valid baseline variation data. Collect additional independent runs when variation is unknown, results approach a limit, or a regression appears. Inconclusive data cannot establish a pass.
- [ ] Record frame timing, queue depth, pending frame age, dropped frames, audio underruns, CPU, GPU, memory including Chromium, and time to first frame. Require no repeatable streaming regression beyond measured baseline variation.
- [ ] For materially different native and browser graphics paths, observe sustained playback for an interval justified by warm-up, resource trends, and known time-dependent failures. Record the duration and its limits. Cover supported SDR, HDR, decoder, and fallback cases; use longer runs when the evidence requires them.
- [ ] Exercise the first connection and a subsequent connection with recreated resources. Verify release of windows, callbacks, threads, GPU resources, held input, and browser helpers as appropriate to their lifetime. Target cancellation, failure, and late completion at native boundaries. Extend loops only for growing counters, uncertain cleanup, or a known race.
- [ ] Measure input latency externally when equipment is available. Report unavailable measurements and unsupported combinations explicitly instead of treating them as passed.
