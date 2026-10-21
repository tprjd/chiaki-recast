# 05: Complete native acceptance

Status: claimed
Type: task
Blocked by: 02, 03, 04

**What to build:** Establish that the complete approved toolbar, statistics, and
notices work together in the native application. Verify the player-visible result
in both renderer modes and produce a normal production build with an accurate
acceptance record.

Parent and specification: [implementation map](../map.md).

## Acceptance criteria

- [ ] Confirm that 02, 03, and 04 are resolved and their production QML checks pass on the integrated source. Fix failures caused by integration before marking this ticket complete.
- [ ] Run combined production QML cases through the single composed boundary. Cover menu/pop-up transitions, statistics suppression and return, simultaneous notices, blocked content, disabled media, session replacement/destruction, and connection/PIN/disconnect priority.
- [ ] Use isolated settings and controlled session state for native checks. Exercise the real application bindings and handlers while intercepting transport commands. Do not change the user's settings or operate their real console.
- [ ] Exercise both the inline and separate-window renderer routes. Verify the approved A appearance, transparent backgrounds, opaque readable text, visible focus, and absence of a large opaque window or full-screen dimming layer.
- [ ] Verify toolbar and pop-up placement after window movement, resize, fullscreen changes, and display-scale changes. Check 900×600, 1280×720, and 1920×1080 logical layouts, plus 100%, 150%, and 200% scaling where available.
- [ ] Check long translated labels, the longest quality preset, Zoom and Custom, large metric values, and notice combinations. Controls and notices do not clip or overlap, and no popup is cut off by its native host.
- [ ] Verify actual window activation, initial Resume focus, keyboard/controller key handling, Escape order, focus restoration, single input release, input return to the stream, and no unwanted interception outside interactive regions.
- [ ] Confirm that passive statistics and routine notices do not take focus. Check that session teardown or route changes cannot leave overlays visible or input captured.
- [ ] Re-run relevant existing connection, console login PIN, disconnect, and linked-settings checks. Verify the configured disconnect policy and keep controlled command outcomes separate from real console effects.
- [ ] Inspect production overlays against dark, bright, and detailed backgrounds. Save reliable visual evidence and associate each result with its source revision and renderer.
- [ ] Distinguish component render-target captures from desktop composition. Reject unrelated desktop pixels. Record unavailable renderers, scaling modes, or unreliable capture methods as unverified; never report them as passed. Obtain reliable alternative evidence for required behavior or leave the affected acceptance item open.
- [ ] Build the normal production application after all fixes. Verify that prototype files, comparison controls, fixtures, sample data, and controlled-backend hooks are absent from shipped resources.
- [ ] Publish an acceptance record with pass/fail/unverified results, reproduction instructions, source/build identity, and remaining limits. Identify live media, real network degradation, console power effects, and physical-controller input as separate unless actually verified.
- [ ] Do not create a release package, push, publish, or create a pull request as part of this ticket. Do not merge the throwaway prototype into production.

## Comments

Tickets 02, 03, and 04 are direct blockers. Ticket 01 is already their prerequisite.
This ticket verifies the integrated result; it does not replace feature-level tests.

Claimed after tickets 02, 03, and 04 were resolved. Integration merge `81a9550e`
contains all three features. Build and verify this final combined source, including
the recorded focus-outline fix and notice integration after the toolbar's normal build.
