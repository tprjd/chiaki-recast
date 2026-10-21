# 06: Disconnect confirmations in both renderer routes

Status: resolved
Type: task
Blocked by: None (can start immediately)

**What to build:** Deliver the same explicit disconnect choices in the in-window popup and separate native window, including correct session and console-power commands.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [x] Use the new design and existing production controls. Verify reachable actions at 900 by 600 and 1280 by 720 logical pixels, long text, keyboard/controller navigation, visible focus, return focus, and applicable privacy rules. Keep test fixtures and synthetic data out of production resources.
- [x] Use Disconnect and sleep, Disconnect, and Cancel with Cancel initially focused in both presentations.
- [x] Dispatch stop-with-sleep only for Disconnect and sleep; dispatch stop-without-sleep only for Disconnect. Cancel, Escape, and separate-window close keep the stream running.
- [x] Preserve saved disconnect preferences, renderer-dependent routing, placement, and input capture/release.
- [x] Verify production presentation behavior, actual native stop dispatch with controlled session operations, and visible focus/close behavior for both window routes.
- [x] Keep the active-stream menu and transport behavior outside the change.

## Coverage and dependencies

Primary user stories: 57-59. Shared user stories: 1-7, 63.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.

## Answer

Implemented `StreamDisconnectDialog` with one production `ConfirmDialog` for
both renderer routes. The choices are Disconnect and sleep, Disconnect, and
Cancel. Cancel has initial visible focus. Only the explicit disconnect choices
call `stopSession`, with the matching sleep argument. Escape and window close
leave the session running. The saved disconnect preference is unchanged.

`StreamView` retains its existing input handlers. The separate dialog uses
`Chiaki.window` for its native parent and centered placement. Native checks found
that the hidden render-control window had a zero-size overlay. `Main` now binds
that overlay to the viewport for all compact dialogs. This also fixes native
confirmation and reminder layouts.

Validation and replay files are in [ticket06 evidence](../evidence/ticket06/README.md).
The final focused QML checks passed 83 tests. Earlier connection and PIN
regressions passed another 83 tests. The complete pre-integration screen suite
passed 795 tests. Native Vulkan passed 307 checks, and OpenGL passed 327 checks.
Both native runs covered both supported sizes, actual controlled stop and sleep
dispatch, cancellation, focus, close, ownership, placement, and input release.
Post-exit checks confirmed unchanged synthetic host records and preferences.

The native source included integration through `599413ba`. The final branch also
merges `ae85efab`. Ticket 08 owns final integrated acceptance and the normal
production build. QA render-target readback and all test fixtures remain outside
production resources.

Desktop compositor capture was unavailable. Windows GDI and Windows Graphics
Capture returned stale Codex pixels despite the correct foreground native window.
Those images are excluded. The retained Vulkan images read the actual Qt Quick
GPU target and change with driven dialog state. Native window and input checks
passed, but they do not prove final desktop composition. Real console power
changes, physical controllers, and display scaling beyond measured DPR 1 remain
unverified in this ticket.
