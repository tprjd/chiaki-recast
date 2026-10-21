# Give stream input an owner lifecycle

Use one native module for stream input ownership across every current caller.
The module owns input blocking, saved focus, cleanup, and session handoff rules.
Callers report their presentation lifecycle through `present`, `dismiss`, and
`settled`. This gives the interface more depth than a capture declaration and
keeps focus rules together instead of leaving them in callers.

`present` returns an opaque stream input presentation identity. A repeated open
of the same active presentation returns its existing identity. An open after
dismissal creates a new identity, even when it uses the same screen.
`dismiss` and `settled` accept the presentation identity. Deferred callbacks keep
the identity from the time they were scheduled. Close-completion adapters keep
the identity from the start of each close. Duplicate closes and obsolete
callbacks have no effect on a newer presentation. Owner destruction removes
only that owner's records.

Session capture reset and presentation replacement are separate events.
An internal capture reset does not replace a continuing presentation's identity.
The module owns the existing reset, preservation, and managed-connection rearm
rules. Internal delayed work also checks whether its capture state is current.

Preserve the established user-visible flows, priority rules, native input
routing, cursor behavior, and held-button release behavior. Keep the inline and
native-window adapters. In particular, menu input release and visual close
completion remain separate events. Passive overlays do not become input owners.

Keep the existing QML test scenarios. Add a maintained native test target that
compiles production sources and tests the ownership interface and its native
effects. Test overlap, duplicate opens, obsolete closes, owner destruction,
focus return, and session handoff. Replace the saved QA builds' source rewriting
with controlled adapters inside the test build.
