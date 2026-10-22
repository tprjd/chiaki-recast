# Ticket 18 verification

Source `80503e5773f811b38c0df7aa7e3efe219313b6e1` includes integration
`478a116d`, tickets 10 and 17. The final application and embedded test build
pass. The final frontend check reports zero errors and zero warnings.

The runtime uses Qt 6.9.3 MSVC x64 on Windows 11, clang-cl 19.1.7, the prepared
MSVC dependencies, and the pinned frontend lockfile. Each saved `run.cmd` calls
`environment.cmd` first and adds this build's `third-party/steam-tools` DLL path.
All GUI runs used the root-controlled runtime slot. They use offscreen Qt and
test-only `--disable-gpu`.

## Focused results

| Record | Result | Observation |
| --- | --- | --- |
| `red` | 2 pass, 1 fail | The compiled page had no startup Steam reminder. |
| `reminder-green` | 3 pass, 0 fail | Steam before PSN, Later keeps its flag, Never persists and reopens, page reload does not repeat. |
| `steam-red` | 2 pass, 1 fail | Set up now had no Steam form. |
| `steam-green` | 4 pass, 0 fail | Reminder plus native picker, real shortcut creation/update, five artwork files, controller configuration, backup, partial failure and retry. |
| `guards-red` | 2 pass, 2 fail | Native triangle/Y was unhandled, and hiding setup retained the Steam form. |
| `guards-green` | 4 pass, 0 fail | One press advances only the current reminder. Autorepeat and release leave the next reminder open. PSN route and DOM focus, help allowlist, changed-profile picker, stale operations, masking, hide/reload, and queued cancellation pass. |
| `eligibility` | 7 pass, 0 fail | Four eligibility rows plus affected `wakeConsole`: auto-connect suppression, complete/incomplete credentials, Steam cancellation before complete PSN suppression, and the merged `woken` state. |
| `keys-red` | 2 pass, 1 fail | The native F1 event did not open the Steam form. |
| `keys-green` | 3 pass, 0 fail | The single `steamShortcutGuards` case passes with native F1, Down, and Menu, plus the merged profile/picker and stale-operation checks. |

All Steam writes used a `QTemporaryDir`. The tests parse the actual
`shortcuts.vdf`, read controller configuration, and count artwork files.
The partial failure removes only the temporary controller file. The shortcut
write remains, the UI reports failure, and the saved ask flag stays set.
Retry completes the operation without a duplicate shortcut and clears the flag.
No test writes the user's Steam configuration or restarts Steam.

HTTP, device listing, PSN worker connection, raw session start, registration,
and discovery boundaries are controlled before the native services are built.
The setup page, WebEngine, WebChannel, bridge, and native file code are real.
The native help handler is intercepted during its check; arbitrary URLs and
file URLs are rejected.

The eligibility layout row sets a 2048-pixel native client width and 200 percent
zoom. It first verifies an actual width of at least 1024 CSS pixels, then checks
document and dialog bounds. This is a DOM check, not visual acceptance.

## Source and artifacts

Each result folder has the exact command, Qt log, runtime output, and compressed
pre-run identity. The identity includes the source commit, status, submodule
revisions, full relevant diff, untracked files, and source/executable SHA256
values. Early untracked frontend files are included in those identities.
The final `keys-green` identity also records the application and compiled
frontend assets. Failed records are retained.

`source-preservation.json` verifies 21 native stream/input/service files and
14 existing bridge methods against integration `478a116d`. Backend code outside
the Steam operation is unchanged. The native Steam operation and five passing
test bodies are unchanged from `8aa79312`. The final guard case was rerun for
the actual profile merge and keyboard changes. Earlier valid passes were reused.
No full suite, graphics matrix, or source-equivalent group was repeated.

The caller audit found no remaining reminder/Steam QML leaf or QJS callback
caller. Those leaves, resource entries, fixtures, and the extracted standalone
callback test were removed. Shared MainView/SettingsDialog cleanup stays with
ticket 23. Native stream files remain. The local gf-complete workflow patch is
excluded from commits; its diff hash is recorded.

## Limits

This evidence does not establish actual foreground focus, physical controller
input, GPU composition, live PSN, real-console playback, or Steam library
visibility after a user restart. Native file-lock failure branches and the
Steam-disabled build configuration were not repeated. Steam file writes can
leave partial changes; the UI reports that result. Ticket 23 retains hardware,
platform, package, and visual acceptance.
