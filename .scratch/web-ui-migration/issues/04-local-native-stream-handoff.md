# 04: Connect locally from web setup to native playback

**What to build:** A user selects an active registered local console in the web UI, streams in a separate native window, disconnects, and returns to setup.

**Blocked by:** 02: Run native playback with a stream-only interface; 03: Show live consoles in the GPU-accelerated web UI.

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 31, 33, 34, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] A connect action resolves the current native console key and creates one connection attempt. Repeated activation cannot start a second attempt.
- [ ] The native renderer is prepared before session creation needs decoder or GPU information. Accelerated Chromium and the supported native OpenGL and Vulkan paths coexist.
- [ ] Complete the combined graphics acceptance carried from [01](01-application-owned-native-services.md) and started in [03](03-accelerated-web-console-library.md). Observe actual accelerated Chromium during native handoff. Record each supported renderer result and all unverified combinations.
- [ ] Resolve or carry the recorded Windows HighQuality native processing failure with a matched native baseline. Preserve the user's preset and production defaults; Fast-preset coexistence evidence does not certify HighQuality or other presets.
- [ ] Setup remains visible until the native window can show progress and accept input, then hides. Native connection progress and console login PIN interaction remain usable.
- [ ] No video frames, decoded audio, raw game input, or GPU handles pass through the browser or bridge. The native video-disabled route remains supported.
- [ ] Native menu, notices, live settings, disconnect confirmation, console sleep policy, and held-input release work during the session.
- [ ] On stop, cancellation, or initialization failure, native cleanup completes before window destruction. Setup restores current state, selection, focus, and any retained error once.
- [ ] Verify the remaining cleanup criterion from [01](01-application-owned-native-services.md) with real decoded frames. Prove that session and frame workers cannot use released window or GPU resources, including swapchain teardown. Retain this evidence for [23](23-complete-web-only-acceptance.md).
- [ ] The hidden page stops unnecessary work and uses the supported freeze policy. Resume reads fresh native state without restarting an operation.
- [ ] Embedded integration checks cover success, cancellation, PIN handling, failure, repeated sessions, and window visibility. A real console run confirms native playback.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Comments

### Ticket 02 handoff

[02](02-native-stream-only-interface.md) delivers the production native
`StreamRoot.qml`. Its `streamViewDismissed` signal requests a presentation
change. It does not stop a session or release its resources. Complete native
session and frame cleanup before window destruction or setup restoration.

The [ticket 02 checks](../evidence/ticket02/verification.md) use controlled
loopback sessions. They prove native root, PIN, saved geometry, settings, and
input behavior on the recorded OpenGL and Vulkan paths. They do not prove
real-console decoded-media playback, physical-device behavior, or accelerated
WebEngine coexistence. These acceptance requirements remain open here and in
[23](23-complete-web-only-acceptance.md).

### Ticket 03 graphics and native failure limits

The [ticket 03 evidence](../evidence/ticket03/verification.md) separates
accelerated Chromium, native GPU-buffer output, screen observations, and
unverified combinations. Its controlled graphics fixture selects the existing
native Fast preset through Settings. This does not change production defaults.

HighQuality produces black native video in the recorded Windows MSVC/Qt 6.9.3
fixture. Uploaded Y/U/V bytes and the transparent Quick overlay center are
correct. A bounded old `fromOpenGLContext` control without a WebEngine page
also fails, but it is not a full pre-migration baseline. Keep this native
processing issue open through handoff and final acceptance; do not lower the
user's preset or treat all presets as passed. The dependency provenance audit
found no header/DLL mismatch; loaded module paths were not observed.

The native initializer retains fatal error paths, including QRhi creation
failure. A controller cannot catch `qFatal` to restore setup. Assess graceful
initialization failure at the real native boundary while implementing handoff.
Actual foreground ownership and focus return still need handoff acceptance;
the GPU observer can render a visible native window without foreground focus.

### Minimal native HighQuality control

The later [native control evidence](../evidence/native-highquality-control/pure-libplacebo-mix-control/verification.md)
reproduces wrong HighQuality pixels with `pl_render_image_mix` without Qt Quick,
QRhi, WebEngine, or WebChannel. The same executable passes with compute disabled.
Each input has a fresh signature. Direct rendering produces a correct second
frame; continuous native application input advances signatures but still has
wrong colors. Uploaded YUV values remain correct.

This is a concrete native dependency/control failure independent of the web UI
and the graphics import. It is not a full old-application baseline or a live
console result. Preserve the user's preset and production defaults. Full
HighQuality and hardware acceptance remains open in this ticket and 23.

### Ticket 06 sign-in window merge requirement

[06](06-psn-sign-in.md) adds an optional native `signInWindow` to `SetupWindow`.
When this ticket adds `SetupWindow::suspend`, close that window before setup
hides or its page freezes. Retain the sign-in profile cleanup rules. This is
required when the handoff source merges with the integrated sign-in source.
