# 17: Manage profiles, settings files, and logs

**What to build:** Users can create, switch, and delete profiles, import and export settings, and open native log and file actions from web setup.

**Blocked by:** 11: Open settings and change general session options.

**Status:** ready-for-agent

**Source:** [Web setup UI specification](../spec.md). Stories 4, 46, 48, 50, 51, 52, 53, 54, 55, 60.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] Profile creation, switching, and deletion retain existing names, validation, confirmations, defaults, and persistent storage.
- [ ] Replacing the active native Settings instance refreshes the bridge and all visible controls. Old signals and late replies cannot write to the new profile.
- [ ] This ticket owns native profile and general settings import and export dialogs and commands. Renderer-specific file actions belong to the renderer ticket and reuse the same native services. Do not duplicate their UI or implementation. File contents and stored credentials do not pass through the frontend.
- [ ] Cancelled or failed profile and general settings file operations preserve existing data and show the actual result. Validate an import before replacing native values; include the required native failure-safety work.
- [ ] Log directory and supported file or help actions retain their native behavior and streamer mode masking.
- [ ] Embedded tests verify native stores across profile changes, restarts, import and export outcomes, cancellation, and pending operations from an old profile.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.
