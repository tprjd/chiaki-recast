# 01: Preserve native playback with application-owned services

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** The current application can discover consoles, connect, stop, and connect again after backend and QML engine lifetime are separated from the native window.

**Blocked by:** None (can start immediately).

**Status:** needs-info

**Spec stories:** 3, 4, 78, 79, 83, 86, 87, 88.

- [ ] One application-owned backend and one native QML engine serve the existing application. Creating another window does not create another backend or register another Chiaki singleton.
- [ ] The native stream window attaches and detaches explicitly. Existing constructor callers, renderer helpers, profile changes, and native tests use the new ownership rules.
- [ ] Settings, discovery, controller activity, and asynchronous operations behave correctly when a stream window is absent. No setup operation dereferences an absent stream window.
- [ ] Session and worker cleanup complete before dependent window and GPU resources are destroyed. Queued frames are released even when their callbacks do not run.
- [ ] The QML engine outlives window-specific roots and is destroyed before the backend. Image providers and incubation control remain valid across window recreation.
- [ ] Existing local playback, stop and reconnect, direct streaming, profile selection, launch options, and separate saved geometries still work.
- [ ] Production native regression checks cover window recreation and held-input release. This is preparation of existing behavior, with no added runtime UI selector.
