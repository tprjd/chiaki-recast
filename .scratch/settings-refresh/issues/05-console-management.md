# 05: Manage registered and hidden consoles from Settings

Status: resolved

**What to build:** Use Consoles to start registration, inspect registrations,
select auto-connect, delete a registration, and unhide a console.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [x] Retain all registered and hidden console information and actions, with empty-list states and long names handled.
- [x] Registration opens the existing native flow and returns to usable Settings focus.
- [x] Only the intended console can auto-connect. Selection remains correct when console lists or the active profile change.
- [x] Deletion retains its confirmation; cancellation changes nothing. Deletion and unhide never act on a different entry after list updates.
- [x] Streamer mode masks private values in visible text, tooltips, and accessible names.
- [x] Production-QML tests use synthetic identities to verify action targets, list updates, confirmation cancellation, exclusive auto-connect, privacy, and focus restoration.
- [x] Build and demonstrate the native page with isolated fixtures. Do not delete or alter the user's registrations for testing.



## Answer

Implemented in 5439061, with native unhide validation fix 0a656f1. All 20 Consoles screen checks pass. Native Windows navigation and focus checks pass at 1280 by 720 and 900 by 600, and the masked 900-pixel layout was inspected. Native registration cancellation, auto-connect persistence, deletion cancellation and acceptance, and unhide passed using disposable synthetic records. The native check exposed and verified the fix for valid MAC addresses with zero bytes. Evidence: ../native-previews/03-06-input-dpr1-checks.json and ../native-previews/05-06-actions-fixed-dpr1-checks.json. Registration acceptance was not run.
