# 09: Manage profiles, settings files, and diagnostics

Publication: draft; awaiting breakdown approval

**What to build:** Use Config and the linked Profiles page to manage profiles,
access native import/export, and configure logging.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [ ] Show the real active profile and retain profile selection, creation, deletion, and native restrictions on protected or active profiles.
- [ ] After profile switching, Settings displays current values and does not write stale values into the new profile.
- [ ] Preserve profile-name validation and each action's existing commit timing.
- [ ] Settings import/export and About open their existing native flows. Keep formats, cancellation, and error handling intact.
- [ ] Sanitize Logs and Verbose Logging use their existing defaults and handlers.
- [ ] Child flows restore focus. No profile, file, or log change occurs merely by opening the page.
- [ ] Production-QML tests verify profile actions, validation, protected cases, refresh, logging writes, file-action dispatch, and cancellation.
- [ ] Build and demonstrate native Config and Profiles with isolated profiles and temporary files for persistence checks.

