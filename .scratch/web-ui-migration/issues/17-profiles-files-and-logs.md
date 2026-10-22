# 17: Manage profiles, settings files, and logs

**What to build:** Users can create, switch, and delete profiles, import and export settings, and open native log and file actions from web setup.

**Blocked by:** 11: Open settings and change general session options.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 4, 46, 48, 50, 51, 52, 53, 54, 55, 60.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] Profile creation, switching, and deletion retain existing names, validation, confirmations, defaults, and persistent storage.
- [x] Replacing the active native Settings instance refreshes the bridge and all visible controls. Old signals and late replies cannot write to the new profile.
- [x] This ticket owns native profile and general settings import and export dialogs and commands. Renderer-specific file actions belong to the renderer ticket and reuse the same native services. Do not duplicate their UI or implementation. File contents and stored credentials do not pass through the frontend.
- [x] Cancelled or failed profile and general settings file operations preserve existing data and show the actual result. Validate an import before replacing native values; include the required native failure-safety work.
- [x] Log directory and supported file or help actions retain their native behavior and streamer mode masking.
- [x] Embedded tests verify native stores across profile changes, restarts, import and export outcomes, cancellation, and pending operations from an old profile.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Comments

- Claimed after the settings shell. Prepare native profile/file actions and repair the common general settings import/export failure boundary. Validate before replacing stores and preserve data on failure. Use the current 1024 CSS pixel design scope.

## Answer

Profiles now use opaque native keys and the existing native stores. Creation,
switching, and confirmed deletion keep their explicit actions and defaults.
General import/export and log access use native services; credentials and files
stay native. Imports validate before replacement, and exports commit atomically.
The stable QmlSettings object refreshes after replacement and rejects obsolete
signals, commands, picker results, and profile replies. The setup-only profile
leaf, routes, resource, sentinel/deletion adapters, and obsolete tests are removed.

Focused real-bridge profile/file cases passed, including a separate-process
restart, native QFileDialogs, invalid/cancelled operations, read-only replacement,
legacy formats, default catalog restoration, privacy, and 200% zoom at least 1024 CSS
pixels. Four affected QML cleanup cases passed. A focused regression caught and
fixed profile drafts resetting on unrelated snapshots. No full suite or hardware
matrix ran. The native mid-write rollback branch was not forced at runtime.

See [verification](../evidence/ticket17/verification.md) for commands, pre-run
source/executable identity, individual results, source reuse, and limits.
