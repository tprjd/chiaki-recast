# 01: Share the existing stream interface

Status: claimed
Type: task
Blocked by: None (can start immediately)

**What to build:** Keep the current stream controls working in both renderer modes
while giving them one shared production menu and one composed test boundary.
This preparation must preserve current appearance and behavior so the later
design changes can use the same tested bindings and input lifecycle.

Parent and specification: [implementation map](../map.md).

## Acceptance criteria

- [ ] Inline and separate-window menus load shared production content and action bindings. Remove the duplicate control implementation without changing native control values, available actions, or persistence.
- [ ] Add only the top-level supplied-state boundary needed for the composed production stream interface. The application defaults to its real backend, settings, window, and navigation. Do not add a test API to every control or alter public session contracts.
- [ ] Extend the existing Qt Quick screen runner using the established Disconnect, LiveConsolePin, and Connection fixture patterns. The fixture loads real production content and wrappers; it supplies state and records commands without copying menu logic.
- [ ] Baseline tests cover menu opening/closure, both renderer routes, volume and microphone bindings, picture modes and zoom, preset selection, linked settings navigation, and the existing session-close route.
- [ ] Preserve renderer selection, host ownership, input capture/release, return focus, repeat-input protection, and separate-window cleanup. Verify that closure does not release input twice or leave a visible stale window.
- [ ] Preserve exclusion rules for connection progress, console login PIN, and disconnect dialogs. Null sessions, session replacement, and destruction do not cause stale actions or invalid property access.
- [ ] Keep existing statistics and notices working through their current production paths. Make their composed state available through the same test boundary for tickets 03 and 04.
- [ ] Tests assert displayed state, enabled actions, emitted commands, and focus behavior rather than private implementation details. Existing related QML checks pass after the refactor.
- [ ] Build the affected native application target and document the baseline verification and limits. Test fixtures and hooks do not enter shipped resources or change the user's settings.
- [ ] Record the shared entry points and fixture usage for the next tickets. Do not include the browser prototype or introduce a general-purpose overlay framework.

## Comments

This is the approved preparation ticket. It deliberately preserves the old
presentation; the complete visual changes belong to the following feature tickets.
