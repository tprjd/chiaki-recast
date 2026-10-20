# 09: Manage profiles, settings files, and diagnostics

Status: resolved

**What to build:** Use Config and the linked Profiles page to manage profiles,
access native import/export, and configure logging.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [x] Show the real active profile and retain profile selection, creation, deletion, and native restrictions on protected or active profiles.
- [x] After profile switching, Settings displays current values and does not write stale values into the new profile.
- [x] Preserve profile-name validation and each action's existing commit timing.
- [x] Settings import/export and About open their existing native flows. Keep formats, cancellation, and error handling intact.
- [x] Sanitize Logs and Verbose Logging use their existing defaults and handlers.
- [x] Child flows restore focus. No profile, file, or log change occurs merely by opening the page.
- [x] Production-QML tests verify profile actions, validation, protected cases, refresh, logging writes, file-action dispatch, and cancellation.
- [x] Build and demonstrate native Config and Profiles with isolated profiles and temporary files for persistence checks.



## Answer

Implemented in 1bd2fa7 and merged at ba83d9d. Config, Profiles and General pass 35 relevant screen checks. Native navigation, focus bounds and no-write browsing pass at both supported sizes. Native trimmed-name creation, profile switching, independent values, persistence and cancellation pass. Native import/export dialogs cancel with Escape without writes and restore focus; About return also passes. Evidence: ../native-previews/07-09-management-dpr1-checks.json and ../native-previews/09-profile-actions-final-dpr1-checks.json. Native deletion and accepted file operations were not run; screen tests cover deletion restrictions and action dispatch. The existing About dialog clips some prose at 900 pixels; its redesign is outside this specification.
