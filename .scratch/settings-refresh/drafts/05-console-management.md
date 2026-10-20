# 05: Manage registered and hidden consoles from Settings

Publication: draft; awaiting breakdown approval

**What to build:** Use Consoles to start registration, inspect registrations,
select auto-connect, delete a registration, and unhide a console.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [ ] Retain all registered and hidden console information and actions, with empty-list states and long names handled.
- [ ] Registration opens the existing native flow and returns to usable Settings focus.
- [ ] Only the intended console can auto-connect. Selection remains correct when console lists or the active profile change.
- [ ] Deletion retains its confirmation; cancellation changes nothing. Deletion and unhide never act on a different entry after list updates.
- [ ] Streamer mode masks private values in visible text, tooltips, and accessible names.
- [ ] Production-QML tests use synthetic identities to verify action targets, list updates, confirmation cancellation, exclusive auto-connect, privacy, and focus restoration.
- [ ] Build and demonstrate the native page with isolated fixtures. Do not delete or alter the user's registrations for testing.

