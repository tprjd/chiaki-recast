# 01: Preserve native playback with application-owned services

**What to build:** The current application can discover consoles, connect, stop, and connect again after backend and QML engine lifetime are separated from the native window.

**Blocked by:** None (can start immediately).

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 3, 4, 78, 79, 83, 86, 87, 88.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] Before ownership changes, perform a bounded check of GPU-accelerated WebEngine beside the current native renderer. Record the compatible toolchain and actual graphics state. Keep this check within the ticket.
- [ ] Pin the baseline build, machine, console, network, decoder, renderer, profile, display mode, and measurement conditions. Record numerical tolerances before evaluating migrated results. Reuse applicable evidence in later tickets.
- [ ] One application-owned backend and one native QML engine serve the existing application. Creating another window does not create another backend or register another Chiaki singleton.
- [ ] The native stream window attaches and detaches explicitly. Existing constructor callers, renderer helpers, profile changes, and native tests use the new ownership rules.
- [ ] Settings, discovery, controller activity, and asynchronous operations behave correctly when a stream window is absent. No setup operation dereferences an absent stream window.
- [ ] Session and worker cleanup complete before dependent window and GPU resources are destroyed. Queued frames are released even when their callbacks do not run.
- [ ] The QML engine outlives window-specific roots and is destroyed before the backend. Image providers and incubation control remain valid across window recreation.
- [ ] Existing local playback, stop and reconnect, direct streaming, profile selection, launch options, and separate saved geometries still work.
- [ ] Production native regression checks cover window recreation and held-input release. This is preparation of existing behavior, with no added runtime UI selector.
