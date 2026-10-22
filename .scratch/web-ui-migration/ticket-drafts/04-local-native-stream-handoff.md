# 04: Connect locally from web setup to native playback

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** A user selects an active registered local console in the web UI, streams in a separate native window, disconnects, and returns to setup.

**Blocked by:** 02: Run native playback with a stream-only interface; 03: Show live consoles in the GPU-accelerated web UI.

**Status:** needs-info

**Spec stories:** 31, 33, 34, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83.

- [ ] A connect action resolves the current native console key and creates one connection attempt. Repeated activation cannot start a second attempt.
- [ ] The native renderer is prepared before session creation needs decoder or GPU information. Accelerated Chromium and the supported native OpenGL and Vulkan paths coexist.
- [ ] Setup remains visible until the native window can show progress and accept input, then hides. Native connection progress and console login PIN interaction remain usable.
- [ ] No video frames, decoded audio, raw game input, or GPU handles pass through the browser or bridge. The native video-disabled route remains supported.
- [ ] Native menu, notices, live settings, disconnect confirmation, console sleep policy, and held-input release work during the session.
- [ ] On stop, cancellation, or initialization failure, native cleanup completes before window destruction. Setup restores current state, selection, focus, and any retained error once.
- [ ] The hidden page stops unnecessary work and uses the supported freeze policy. Resume reads fresh native state without restarting an operation.
- [ ] Embedded integration checks cover success, cancellation, PIN handling, failure, repeated sessions, and window visibility. A real console run confirms native playback.
