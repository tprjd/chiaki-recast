# 01: Share the existing stream interface

Status: resolved
Type: task
Blocked by: None (can start immediately)

**What to build:** Keep the current stream controls working in both renderer modes
while giving them one shared production menu and one composed test boundary.
This preparation must preserve current appearance and behavior so the later
design changes can use the same tested bindings and input lifecycle.

Parent and specification: [implementation map](../map.md).

## Acceptance criteria

- [x] Inline and separate-window menus load shared production content and action bindings. Remove the duplicate control implementation without changing native control values, available actions, or persistence.
- [x] Add only the top-level supplied-state boundary needed for the composed production stream interface. The application defaults to its real backend, settings, window, and navigation. Do not add a test API to every control or alter public session contracts.
- [x] Extend the existing Qt Quick screen runner using the established Disconnect, LiveConsolePin, and Connection fixture patterns. The fixture loads real production content and wrappers; it supplies state and records commands without copying menu logic.
- [x] Baseline tests cover menu opening/closure, both renderer routes, volume and microphone bindings, picture modes and zoom, preset selection, linked settings navigation, and the existing session-close route.
- [x] Preserve renderer selection, host ownership, input capture/release, return focus, repeat-input protection, and separate-window cleanup. Verify that closure does not release input twice or leave a visible stale window.
- [x] Preserve exclusion rules for connection progress, console login PIN, and disconnect dialogs. Null sessions, session replacement, and destruction do not cause stale actions or invalid property access.
- [x] Keep existing statistics and notices working through their current production paths. Make their composed state available through the same test boundary for tickets 03 and 04.
- [x] Tests assert displayed state, enabled actions, emitted commands, and focus behavior rather than private implementation details. Existing related QML checks pass after the refactor.
- [x] Build the affected native application target and document the baseline verification and limits. Test fixtures and hooks do not enter shipped resources or change the user's settings.
- [x] Record the shared entry points and fixture usage for the next tickets. Do not include the browser prototype or introduce a general-purpose overlay framework.

## Comments

This is the approved preparation ticket. It deliberately preserves the old
presentation; the complete visual changes belong to the following feature tickets.

## Answer

Both renderer routes now load `gui/src/qml/StreamMenuContent.qml`. The shared
component retains the old backgrounds, controls, native values, and actions.
`StreamView` supplies backend, settings, native window commands, and navigation
with real application defaults. The existing statistics presentation is now
`StreamStatsContent.qml`. Its two wrappers and the notices remain in `StreamView`.

The complete production interface is tested through `StreamHarness` in the
existing screen runner. The baseline stream suite passes 37 checks. Seven
related suites also pass. Controlled native menu-to-disconnect checks pass on
Vulkan and OpenGL, with 327 and 347 checks. The normal `chiaki` target builds.
Input fixes cover native menu close, closure repeats, repeated configured
disconnect, destruction, and dialog focus after the menu hides.

Next entry points are `StreamMenuContent.qml` and `StreamMenuWindow.qml` for 02,
`StreamStatsContent.qml` and its wrappers for 03, and the notice sections in
`StreamView.qml` for 04. Fixture state includes all existing metric, session,
connection, PIN, and disabled-media fields. See `gui/tests/README.md` and
`../evidence/ticket01/README.md` for replay details and verification limits.

Native checks use isolated settings and intercepted session transport. They
establish neither live console behavior nor desktop composition. Final layout,
fullscreen, display-scale changes, physical controllers, and live media remain
for ticket 05. The shared native cache is restored and rebuilt before handoff.
