# 02: Save a console login PIN

**What to build:** The existing library PIN action opens layout A, accepts a four-digit console login PIN, saves it only for the intended console, and shows a confirmed saved result. Saving does not start a connection.

**Blocked by:** None (can start immediately).

**Status:** resolved

Parent: Connection progress and console login PIN: guided workspace. Specification commit: `cb1f76e9`.
Selected design: A on prototype branch `codex/prototype-connection-flows`, implementation commit `7a0111b6`.

- [x] The existing library and management entry points open A's PIN form with the intended console context and guidance. Clearly distinguish the four-digit console login PIN from the eight-digit Remote Play pairing code.
- [x] Exactly four ASCII digits enable Save PIN. Preserve leading zeros as text. Enter submits only when valid; repeated activation cannot save twice.
- [x] PIN entry is masked by default. Show/Hide works outside streamer mode. Entering streamer mode immediately hides a revealed PIN and private console values.
- [x] Reuse guarded management callbacks and stable profile-scoped target resolution. Reorder cannot redirect a save. Removal, hiding, ambiguity, profile changes, and obsolete callbacks cannot save or report success.
- [x] The saved result appears only after confirmed acceptance by the existing persistence path. Add a small save outcome if needed. An invalid target or rejected save remains actionable and provides a route back to the library.
- [x] Done returns to the library and restores valid opener focus. Cancel and removal of an unaccepted form discard input without a save. Transient PIN input is cleared on completion or cancellation.
- [x] No connection starts, no peer console is changed, and the existing storage model remains unchanged. Do not add PIN removal or reveal an existing stored PIN.
- [x] Keep PIN input and its validation/privacy behavior usable by the live-PIN flow without copying the form implementation. Reuse existing shared controls; keep this reuse limited to the actual PIN requirements.
- [x] Production-QML tests cover initial focus, invalid input, leading zeros, masking/reveal, privacy changes, save outcome, duplicate acceptance, cancellation, stale callbacks, and profile/target changes.
- [x] An isolated native save check proves that the intended synthetic console receives the exact PIN, a peer remains unchanged, and the saved value remains correct after process exit. Test output must not expose PIN values.
- [x] A is usable at 900 × 600 and 1280 × 720 with long names and expanded text. Mouse and keyboard actions work, and existing management tests pass.

Demo: open the library PIN action, enter a synthetic four-digit PIN with leading zeros, reveal and hide it, save, and return from the saved result. Repeat with an invalidated target and verify that no success is shown.

Scope limit: Live PIN requests, session cancellation, incorrect-PIN feedback, and native separate-window behavior belong to 05.

## Answer

Implemented A in `ConsolePinDialog` with reusable `ConsolePinContent`. The form uses the existing controls, masks input, supports explicit reveal, and hides PINs and console context when streamer mode starts. The library passes a live console-name resolver and resolves the profile-scoped target again at acceptance. An unavailable target permanently invalidates its pending action, including when that target returns later.

`MainView` supplies the native boolean `savePin` outcome. The native handler accepts exactly four ASCII digits and rejects unavailable or unregistered targets. Success follows the existing Settings update. A profile change during that update cannot report success. The saved result has Done; rejection has Back to consoles. Completion, cancellation, removal, and duplicate close clear transient input or have no further effect. Saving starts no connection and retains the existing storage format.

Checks:

- Production-QML PIN tests: 23 passed. Management tests: 44 passed. See the retained [PIN results](../evidence/ticket02-ConsolePin-final.txt), [management results](../evidence/ticket02-ConsoleManagement-final.txt), and [TDD red checks](../evidence/ticket02-tdd-red.txt).
- Rendered production QML at 900 x 600 and 1280 x 720 passed with long names and expanded text. Form and saved-result captures were inspected against A's shell, card, and guidance hierarchy. The sidebar, breadcrumb, identity row, dividers, status line, and both guidance headings match that hierarchy. Captures: [900 form](../evidence/ticket02-A-900-form.png), [900 saved](../evidence/ticket02-A-900-saved.png), [1280 form](../evidence/ticket02-A-1280-form.png), [1280 saved](../evidence/ticket02-A-1280-saved.png). Mouse actions, Enter, Escape, focus, privacy changes, stale callbacks, and target/profile changes passed.
- The isolated native production save path preserved the exact leading-zero text for its synthetic target and left its peer unchanged. Invalid input, including a trailing newline and non-ASCII digits, and invalid target indices were rejected. A separate process checked the actual INI file after exit. No PIN values appear in the reports or runtime logs. See [native evidence](../evidence/ticket02-native-save.json).
- Native source files were restored by SHA256. The normal output configuration and executable were rebuilt. The final binary excludes the temporary helper. Helper, patch, checker, manifests, and logs remain outside production in `C:/Users/david/.cache/chiaki-ui-native/saved-pin-contract-prep/`.

Integration notes: `ConsolePinContent` exposes `pin`, `validPin`, `privateMode`, text/context properties, `phase`, `submitting`, `failureText`, `showSidebar`, guidance headings, and `actionHelp`; it emits `submitted`, `cancelled`, and `done`. Ticket 05 can reuse its field, validation, reveal, privacy, and workspace. `Main.showConsolePinDialog` now accepts a third argument, the console-name resolver. No map or spec change was made.

Native visual acceptance at measured DPR 1, 1.5, and 2, full route checks, and the final integrated application build remain with ticket 06. No real account, console, or user settings were used. The native Settings API confirms acceptance of its update; it does not expose a disk-write result to the UI.
