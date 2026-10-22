# 01: Preserve native playback with application-owned services

**What to build:** The current application can discover consoles, connect, stop, and connect again after backend and QML engine lifetime are separated from the native window.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 3, 4, 78, 79, 83, 86, 87, 88.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] Before ownership changes, perform a bounded check of installed WebEngine modules and the current native graphics baseline. Record the toolchain, actual graphics state, and limits. Required accelerated WebEngine and native coexistence checks remain in [03](03-accelerated-web-console-library.md) and [04](04-local-native-stream-handoff.md).
- [ ] Pin the baseline build, machine, console, network, decoder, renderer, profile, display mode, and measurement conditions. Record numerical tolerances before evaluating migrated results. Reuse applicable evidence in later tickets.
- [x] One application-owned backend and one native QML engine serve the existing application. Creating another window does not create another backend or register another Chiaki singleton.
- [x] The native stream window attaches and detaches explicitly. Existing constructor callers, renderer helpers, profile changes, and native tests use the new ownership rules.
- [x] Settings, discovery, controller activity, and asynchronous operations behave correctly when a stream window is absent. No setup operation dereferences an absent stream window.
- [ ] Session and worker cleanup complete before dependent window and GPU resources are destroyed. Queued frames are released even when their callbacks do not run.
- [x] The QML engine outlives window-specific roots and is destroyed before the backend. Image providers and incubation control remain valid across window recreation.
- [ ] Existing local playback, stop and reconnect, direct streaming, profile selection, launch options, and separate saved geometries still work.
- [x] Production native regression checks cover window recreation and held-input release. This is preparation of existing behavior, with no added runtime UI selector.

## Comments

### Implementation finding, 2026-10-22

The pre-edit check found no WebEngine module in the GNU Qt prefix. The native
OpenGL baseline passed with controlled loopback transport. This check did not
establish GPU coexistence or real-console performance. See the recorded
[baseline](../evidence/ticket01/baseline.md) and
[Linux environment limits](../evidence/baseline-linux.md).

The prerequisite criterion now describes that bounded check. This changes the
sequence of acceptance checks. It does not remove an acceptance requirement.
[03](03-accelerated-web-console-library.md) must prove accelerated embedded
Chromium beside native graphics. [04](04-local-native-stream-handoff.md) must
prove accelerated setup and the supported native renderers during handoff.
[23](23-complete-web-only-acceptance.md) must retain the matched live baseline,
predeclared tolerances, hardware checks, and performance acceptance.

The parent task prepared compatible Windows modules and dependencies. Its
current external record is
`C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/environment.md`.
Module preparation does not prove GPU acceleration or renderer coexistence.

### Remaining acceptance owners

The unchecked criteria above remain required. Ticket 23 owns the unmeasured
live baseline conditions and matched performance comparison. Tickets 04 and 23
own live playback, stop and reconnect, launch behavior, and teardown with real
decoded frames. The native service check covers controlled session completion,
reconnect, direct launch exit, profile changes, and separate saved geometries.
It does not establish live playback or all launch options.

Source inspection confirms owned queued frames and cleanup before the remaining
native GPU objects are released. The window destroys its swapchain and
invalidates render control before backend detachment. The loopback checks do
not establish the full lifetime order with live decoded frames. Preserve this
check in tickets 04 and 23 before claiming the cleanup criterion complete.

## Answer

The application-owned native service milestone is complete. Commit
`8f1bacb42c9059680d493de7fd883eda9e9c2b9d` adds one backend and one native QML
engine, explicit window attachment, window-independent services, and maintained
native regression checks. Integration merge `2ac4e5dc00ae8e3b11452f74bf27f06539257940`
has no conflicts and preserves the ticket's tested `gui` tree exactly.

The [verification record](../evidence/ticket01/verification.md) and
[manifest](../evidence/ticket01/manifest.json) identify the tested source and
executables. All 13 ticket source hashes and the three current executable
hashes match. The merged files also match after line ending normalization.
Git checks out the three new files with CRLF. The final logs record 3 passing
service checks, 4 passing OpenGL input
checks, and 3 passing Vulkan input checks, with no failures or skips. Source
inspection and these focused results support the checked code criteria.

No build or test was repeated for this merge. The only other integration
change records Linux baseline evidence. No tested code changed during merge.
No live-console, accelerated Chromium, HDR output, physical-device, or latency
acceptance is claimed. Tickets 02 and 03 can now start under the declared graph.
