# 02: Save a console login PIN

**What to build:** The existing library PIN action opens layout A, accepts a four-digit console login PIN, saves it only for the intended console, and shows a confirmed saved result. Saving does not start a connection.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

Parent: Connection progress and console login PIN: guided workspace. Specification commit: `cb1f76e9`.
Selected design: A on prototype branch `codex/prototype-connection-flows`, implementation commit `7a0111b6`.

- [ ] The existing library and management entry points open A's PIN form with the intended console context and guidance. Clearly distinguish the four-digit console login PIN from the eight-digit Remote Play pairing code.
- [ ] Exactly four ASCII digits enable Save PIN. Preserve leading zeros as text. Enter submits only when valid; repeated activation cannot save twice.
- [ ] PIN entry is masked by default. Show/Hide works outside streamer mode. Entering streamer mode immediately hides a revealed PIN and private console values.
- [ ] Reuse guarded management callbacks and stable profile-scoped target resolution. Reorder cannot redirect a save. Removal, hiding, ambiguity, profile changes, and obsolete callbacks cannot save or report success.
- [ ] The saved result appears only after confirmed acceptance by the existing persistence path. Add a small save outcome if needed. An invalid target or rejected save remains actionable and provides a route back to the library.
- [ ] Done returns to the library and restores valid opener focus. Cancel and removal of an unaccepted form discard input without a save. Transient PIN input is cleared on completion or cancellation.
- [ ] No connection starts, no peer console is changed, and the existing storage model remains unchanged. Do not add PIN removal or reveal an existing stored PIN.
- [ ] Keep PIN input and its validation/privacy behavior usable by the live-PIN flow without copying the form implementation. Reuse existing shared controls; keep this reuse limited to the actual PIN requirements.
- [ ] Production-QML tests cover initial focus, invalid input, leading zeros, masking/reveal, privacy changes, save outcome, duplicate acceptance, cancellation, stale callbacks, and profile/target changes.
- [ ] An isolated native save check proves that the intended synthetic console receives the exact PIN, a peer remains unchanged, and the saved value remains correct after process exit. Test output must not expose PIN values.
- [ ] A is usable at 900 × 600 and 1280 × 720 with long names and expanded text. Mouse and keyboard actions work, and existing management tests pass.

Demo: open the library PIN action, enter a synthetic four-digit PIN with leading zeros, reveal and hide it, save, and return from the saved result. Repeat with an invalidated target and verify that no success is shown.

Scope limit: Live PIN requests, session cancellation, incorrect-PIN feedback, and native separate-window behavior belong to 05.
