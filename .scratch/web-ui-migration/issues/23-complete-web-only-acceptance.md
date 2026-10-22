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
- [ ] Resolve the HighQuality native processing failure recorded in ticket 03, using a matched native baseline and actual loaded dependency identities. Verify the unchanged default and required presets; Fast-preset coexistence alone cannot pass this criterion.
- [ ] Compare matched native baseline and migrated measurements against tolerances fixed before evaluation. Reuse valid baseline variation data. Collect additional independent runs when variation is unknown, results approach a limit, or a regression appears. Inconclusive data cannot establish a pass.
- [ ] Complete the live baseline acceptance carried from [01](01-application-owned-native-services.md). Pin the build, machine, console, network, decoder, renderer, profile, display mode, and measurement conditions before evaluation. Use the [predeclared comparison conditions and tolerances](../evidence/ticket01/baseline.md). The loopback native baseline does not prove live performance. Retain the required combined accelerated WebEngine and native graphics evidence from [03](03-accelerated-web-console-library.md) and [04](04-local-native-stream-handoff.md).
- [ ] Record frame timing, queue depth, pending frame age, dropped frames, audio underruns, CPU, GPU, memory including Chromium, and time to first frame. Require no repeatable streaming regression beyond measured baseline variation.
- [ ] For materially different native and browser graphics paths, observe sustained playback for an interval justified by warm-up, resource trends, and known time-dependent failures. Record the duration and its limits. Cover supported SDR, HDR, decoder, and fallback cases; use longer runs when the evidence requires them.
- [ ] Exercise the first connection and a subsequent connection with recreated resources. Verify release of windows, callbacks, threads, GPU resources, held input, and browser helpers as appropriate to their lifetime. Target cancellation, failure, and late completion at native boundaries. Extend loops only for growing counters, uncertain cleanup, or a known race.
- [ ] Complete the remaining live playback, launch behavior, and live-frame cleanup acceptance from [01](01-application-owned-native-services.md). Reuse applicable [04](04-local-native-stream-handoff.md) evidence. The native service loopback checks do not establish live console playback or safe teardown with real decoded frames.
- [ ] Measure input latency externally when equipment is available. Report unavailable measurements and unsupported combinations explicitly instead of treating them as passed.

## Comments

### Ticket 02 acceptance limits

[02](02-native-stream-only-interface.md) preserves the native media and device
pipelines in source. Its checked pipeline criterion does not establish live
or hardware acceptance. The [recorded checks](../evidence/ticket02/verification.md)
use loopback sessions without live decoded frames.

Retain real-console playback, live-frame cleanup, microphone, physical
controller, touch, haptics, HDR output, external latency when available, and
matched performance acceptance. The combined accelerated WebEngine and native
renderer checks remain required in [03](03-accelerated-web-console-library.md)
and [04](04-local-native-stream-handoff.md). Reuse valid focused native evidence
without treating these unverified combinations as passed.

### Ticket 03 preset and desktop acceptance limits

[03](03-accelerated-web-console-library.md) uses the production native Fast
preset in its controlled GPU fixture. Production defaults remain unchanged.
Its [native diagnosis](../evidence/ticket03/verification.md) records correct
uploaded Y/U/V data and a transparent Quick center, but black video with
HighQuality in the Windows MSVC/Qt 6.9.3 environment. An old-import/no-page
control also fails; this is not a complete pre-migration baseline or proof
that every native configuration is affected. No permanent preset fallback
is authorized. Resolve the default processing issue before release acceptance.

The source/dependency audit found no header, import-library, staged-DLL,
FFmpeg, or packing mismatch. Loaded module paths were not observed during
the read-only audit. Retain that distinction in follow-up evidence.
Desktop screen results made while Windows Security or another input desktop
is active are inconclusive. Actual native handoff focus, physical controller,
touch, screen reader, translated locales, display scaling, Linux/macOS GPUs,
live console decode, HDR, audio/microphone, cleanup, and matched performance
remain required acceptance work.
