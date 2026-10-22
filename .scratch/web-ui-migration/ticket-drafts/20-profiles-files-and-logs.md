# 20: Manage profiles, settings files, and logs

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can create, switch, and delete profiles, import and export settings, and open native log and file actions from web setup.

**Blocked by:** 13: Change general and stream settings from web setup.

**Status:** needs-info

**Spec stories:** 4, 46, 48, 50, 51, 52, 53, 54, 55, 60.

- [ ] Profile creation, switching, and deletion retain existing names, validation, confirmations, defaults, and persistent storage.
- [ ] Replacing the active native Settings instance refreshes the bridge and all visible controls. Old signals and late replies cannot write to the new profile.
- [ ] Native file dialogs handle settings and renderer import and export. File contents and stored credentials do not pass through the frontend.
- [ ] Cancelled or failed file operations preserve existing data and show the actual result.
- [ ] Log directory and supported file or help actions retain their native behavior and streamer mode masking.
- [ ] Embedded tests verify native stores across profile changes, restarts, import and export outcomes, cancellation, and pending operations from an old profile.
