# 23: Complete the web-only application and verify native performance

**What to build:** The complete packaged application provides GPU-accelerated web setup and separate native playback and overlays, with the required feature and performance evidence.

**Blocked by:** 05: Manage consoles and manual addresses from the web library; 07: Pair PS4 and PS5 consoles and retrieve an Account ID; 09: Register a console automatically through PSN; 10: Wake consoles and auto-connect from web startup; 12: Configure stream profiles and audio devices; 14: Configure native color mapping from web setup; 15: Configure keyboard and mouse bindings, response, and reset; 16: Configure controllers and their mappings; 17: Manage profiles, settings files, and logs; 18: Complete setup reminders and Steam shortcuts; 19: Ship the Windows web and native application; 20: Ship the Linux web and native application; 21: Ship the macOS web and native application; 22: Resume interrupted sessions and recover web setup.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 3, 48, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] Every specification story is mapped to an implemented flow and acceptance evidence. The application offers one web setup path without a QML setup fallback or runtime UI selector.
- [ ] Audit remaining callers, resources, build references, and adapters for obsolete setup QML. Each migration ticket removes its replaced setup code when no caller needs it. Final acceptance resolves remaining obsolete references and preserves native stream QML, overlays, live settings, and their tests.
- [ ] All settings categories, linked search routes, input flows, account routes, reminders, confirmations, profiles, files, and Steam actions are reachable in the packaged application. The combined registration chooser offers manual pairing and automatic PSN registration with the correct context.
- [ ] Access checks cover keyboard, controller, touch, screen-reader labels, focus return, reduced motion, privacy, long text, display scaling, and 200 percent zoom at supported widths of at least 1024 CSS pixels. No optimization or new acceptance work is required below that width.
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

### Deferred follow-up record, 2026-10-22

The user requested a saved record for a later return. See
[deferred validation and known issues](../deferred-work.md) for the skipped
checks, HighQuality reproduction, evidence links, and conservative resume policy.
The current Windows source was later built and the actual UI opened at the
user's request. This adds local compilation and startup evidence only.

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

### Ticket 04 lifetime evidence and remaining acceptance

[04](04-local-native-stream-handoff.md) resolves the implemented local handoff
and lifetime milestone. Its [controlled decoded-frame check](../evidence/ticket04/verification.md)
uses the real H.264 decoder and frame worker at advancing native PTS/cadence.
Normal OpenGL disconnect releases 51/51 tracked AVBuffers; active Vulkan
controller destruction releases 54/54, including queued GUI delivery. Native
initialization recovery, page freeze/fresh resume, merged sign-in closure, and
direct native callers pass. Reuse this evidence when the relevant source and
dependencies are unchanged.

These checks do not prove real-console playback, actual foreground/focus,
physical input or held-input release, console sleep, native video-disabled
operation, hardware decoding, audio/microphone, default/other presets,
supported platform packages, or matched performance. The minimal pure native
HighQuality control reproduces wrong pixels without Qt Quick or WebEngine;
it does not supply a complete old-application baseline. Preserve production
defaults and the user's preset while completing that acceptance.

### User priority update, 2026-10-22

Skip the remaining build verification and performance tests at the user's
request. These checks remain unverified, rather than passed. Complete the
web frontend's visual fidelity to the chosen B library, A settings sidebar,
and A guided setup prototypes. Keep all actions connected to existing native
services, preserve controller/keyboard semantics, and review the visual result
at 1024 CSS pixels or wider. Necessary frontend preview builds are separate from
the skipped release build and performance qualification.

## Answer

The production Svelte UI now follows the selected B library, A Settings
sidebar, and A guided setup prototypes. The app rail, per-card actions,
artwork/status treatment, search/refresh toolbar, category rows, compact
controls, current profile chip, pairing form, and instruction column match
the selected layouts. Related profile, controller, and keyboard controls use
the same groups and rows. Native handlers, target identities, capability
checks, privacy controls, and connection progress remain in use.

The manual and automatic registration routes share one chooser. Navigation
closes the pairing account route and clears PIN drafts before changing the
page. Auto-connect still supports Off and the existing saved native target.
The maintained embedded fixture selectors follow the new card and chooser
layout, but their runtime suites were not run.

The frontend check passes with zero errors and warnings. The frontend build
passes for the visual changes. The last pairing navigation guard passes a
direct Svelte compile and focused async assertions. The short visual review
uses real Svelte components with a local-only
fixture adapter. See [verification](../evidence/ticket23/verification.md).

This resolves the requested source and visual milestone under the user's
priority update. The remaining native, package, platform, hardware,
HighQuality, and performance acceptance is user-skipped and remains
unverified. The unchecked historical criteria above do not claim passes.

### Final review follow-up

The separate Standards review found zero violations. The Spec review found
three defects. The merged fixes restore controller access to actions, reopen
busy registration progress, and remove unsupported management actions. An
additional fix keeps required Widgets profile hints outside the optional Quick
guard. See [final review integration](../evidence/final-review/integration.md)
for the separate reports, fix evidence, source identities, and cleanup status.
The source and visual milestone remains resolved. All user-skipped acceptance
and historical unchecked criteria remain unverified.
