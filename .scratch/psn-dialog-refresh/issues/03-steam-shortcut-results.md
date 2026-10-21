# 03: Steam shortcut creation and results

Status: resolved
Type: task
Blocked by: None (can start immediately)

**What to build:** Deliver the refreshed Steam form through actual create-or-update behavior, accurate final results, retry, and the manual restart instruction.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [x] Use the new design and existing production controls. Verify reachable actions at 900 by 600 and 1280 by 720 logical pixels, long text, keyboard/controller navigation, visible focus, return focus, and applicable privacy rules. Keep test fixtures and synthetic data out of production resources.
- [x] Preserve profile-based defaults, optional launch options, the optional Steam folder, and existing file pickers. Reject an empty trimmed name.
- [x] Preserve native matching, shortcut updates, artwork, and controller-configuration effects.
- [x] Show operation progress and a terminal result. Do not offer cancellation for synchronous writes or claim rollback after failure.
- [x] Treat only terminal success as success. An intermediate successful step followed by failure cannot disable the reminder or show the restart instruction.
- [x] Retain editable input and useful details on failure. Clear old outcome/details for retry and prevent duplicate creation while active.
- [x] Include the manual Steam restart instruction in success and preserve the eligible next reminder without restarting Steam or stacking dialogs.
- [x] Verify production QML results and actual native writes in a temporary Steam directory, including matching-entry updates and failure after partial progress.

## Coverage and dependencies

Primary user stories: 38-47. Shared user stories: 1-7, 63.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.


## Answer

Implemented the production Steam form, operation state, persistent failure, retry,
and terminal result with the refreshed controls. The form retains profile defaults,
custom options, the folder picker, and editable input. Streamer mode masks the
profile, options, folder, and diagnostic paths. Keyboard and controller key
mappings retain visible focus and scroll focused actions into view.

Only a terminal successful callback disables the Steam reminder and shows the
manual Steam restart instruction. The active synchronous write has no cancel
action. Failure explains that some files can already have changed. The completion
route retains the eligible PSN reminder and returns focus to the Library opener.

The native command rejects blank names, emits one terminal outcome, and retains
errors through intermediate progress. A patch to a build copy of the pinned
SteamTools dependency fixes custom folder checks, expected absent shortcut files,
Windows backup names, failed artwork saves, backup protection, and write results.
The source submodule remains unchanged. The normal application and the native
check use the same patched dependency.

Evidence:

- `evidence/ticket03-screen-red.txt` records the initial failing production-screen check.
- `evidence/ticket03-screen-green.txt` records 21 passing Qt Test checks, including lifecycle checks.
- `evidence/ticket03-native-red.txt` and `evidence/ticket03-native-backend-red.txt` record native failures before the fixes.
- `evidence/ticket03-native-green.txt` records actual temporary-file create, matching update, five artwork saves, controller configuration, partial failure, invalid names, artwork failure, backup denial, and shortcut/controller write denial.
- `evidence/ticket03-{minimum,large}-{form,progress,failure,success}.png` show the production screen at 900 by 600 and 1280 by 720.
- `evidence/ticket03-screen.ps1` and `evidence/ticket03-native.ps1` reproduce the focused checks. Repeated native configuration passes on the existing build copy.
- `evidence/ticket03-tested-source.json` identifies the tested source and unchanged dependency.

Limits: No real Steam installation was changed. Steam restart, live library
visibility, physical controller input, native display scaling, and a full normal
application build remain for ticket 08 acceptance.
The broad combined CTest run was stopped after 374.59 seconds without per-case output. Its partial result is retained in evidence/ticket03-screen-regressions.txt and is incomplete. The focused Steam and native checks passed again after the integration merge.
