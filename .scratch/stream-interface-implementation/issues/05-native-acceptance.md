# 05: Complete native acceptance

Status: resolved
Type: task
Blocked by: 02, 03, 04

**What to build:** Establish that the complete approved toolbar, statistics, and
notices work together in the native application. Verify the player-visible result
in both renderer modes and produce a normal production build with an accurate
acceptance record.

Parent and specification: [implementation map](../map.md).

## Acceptance criteria

- [x] Confirm that 02, 03, and 04 are resolved and their production QML checks pass on the integrated source. Fix failures caused by integration before marking this ticket complete.
- [x] Run combined production QML cases through the single composed boundary. Cover menu/pop-up transitions, statistics suppression and return, simultaneous notices, blocked content, disabled media, session replacement/destruction, and connection/PIN/disconnect priority.
- [x] Use isolated settings and controlled session state for native checks. Exercise the real application bindings and handlers while intercepting transport commands. Do not change the user's settings or operate their real console.
- [x] Exercise both the inline and separate-window renderer routes. Verify the approved A appearance, transparent backgrounds, opaque readable text, visible focus, and absence of a large opaque window or full-screen dimming layer.
- [x] Verify final placement and movement at 900x600 and measured DPR 1 on both renderer routes, per the user-reduced remaining test scope. See the acceptance record for earlier-source resize/fullscreen and three-size results. Final-host resize/fullscreen and live DPI transitions remain unverified; no final pass is claimed for them.
- [x] Check long translated labels, the longest quality preset, Zoom and Custom, large metric values, and notice combinations. Controls and notices do not clip or overlap, and no popup is cut off by its native host.
- [x] Verify actual window activation, initial Resume focus, keyboard/controller key handling, Escape order, focus restoration, single input release, input return to the stream, and no unwanted interception outside interactive regions.
- [x] Confirm that passive statistics and routine notices do not take focus. Check that session teardown or route changes cannot leave overlays visible or input captured.
- [x] Re-run relevant existing connection, console login PIN, disconnect, and linked-settings checks. Verify the configured disconnect policy and keep controlled command outcomes separate from real console effects.
- [x] Inspect production overlays against dark, bright, and detailed backgrounds. Save reliable visual evidence and associate each result with its source revision and renderer.
- [x] Distinguish component render-target captures from desktop composition. Reject unrelated desktop pixels. Record unavailable renderers, scaling modes, or unreliable capture methods as unverified; never report them as passed. Obtain reliable alternative evidence for required behavior or leave the affected acceptance item open.
- [x] Build the normal production application after all fixes. Verify that prototype files, comparison controls, fixtures, sample data, and controlled-backend hooks are absent from shipped resources.
- [x] Publish an acceptance record with pass/fail/unverified results, reproduction instructions, source/build identity, and remaining limits. Identify live media, real network degradation, console power effects, and physical-controller input as separate unless actually verified.
- [x] Do not create a release package, push, publish, or create a pull request as part of this ticket. Do not merge the throwaway prototype into production.

## Comments

Tickets 02, 03, and 04 are direct blockers. Ticket 01 is already their prerequisite.
This ticket verifies the integrated result; it does not replace feature-level tests.

Claimed after tickets 02, 03, and 04 were resolved. Integration merge `81a9550e`
contains all three features. Build and verify this final combined source, including
the recorded focus-outline fix and notice integration after the toolbar's normal build.

## Answer

Resolved after the user limited remaining tests to one resolution and one scale.
Final source `62f9676e`, with integration/review merge `faadb689b`, passes native
900x600 DPR 1 acceptance: OpenGL 270 checks and Vulkan 251 checks. Real Windows
pointer input, native focus, outside-click Escape, actual Main settings navigation,
modal priority, session replacement and cleanup pass. Reliable desktop duplication
captures show production composition. The final Qt 6.7 affected cases pass.

The final normal build matches 342 production inputs and 77 generated resource
inputs; QA/test/prototype resources and hooks are absent. The original native cache
sources, 37 options and output are restored, and both original targets are rebuilt.
The final normal executable remains separate. Both source review axes are clear.

See [acceptance and replay](../evidence/ticket05/acceptance.md) for exact source/build
hashes, passed and failed regressions, earlier-source coverage, screenshots and
unverified live media/network/power/physical-controller/DPI effects. No package,
push or PR was created, and no prototype code was merged.
