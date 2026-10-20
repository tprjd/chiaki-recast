# 02: Preserve connection behavior for every host type

**What to build:** Accurate card status and primary actions for discovered, manual, unregistered, and PSN consoles, with reliable action targets during host updates.

**Blocked by:** 01: Connect through a native console card.

**Status:** resolved

**Spec stories:** 4, 5, 6, 7, 8, 18.

- [ ] Show Online for confirmed ready discovery results and In standby for confirmed standby results. Use Status unknown for absent or unrecognized state.
- [ ] Keep Connect available for registered manual hosts with unknown status. Show PSN for remote hosts without inventing a local address or power status.
- [ ] Route discovered standby consoles through the existing wake-and-connect handler with the nickname. Do not mark a console online when a wake request is merely sent.
- [ ] Route unregistered hosts through the existing registration entry point, preserving automatic registration when available.
- [ ] Keep hidden hosts and suppressed duplicates out of the displayed records. Use a neutral label or artwork when the console type is missing.
- [ ] Keep source identity separate from displayed position. Resolve the current source target at action time; cancel if it disappeared or is ambiguous.
- [ ] Screen tests cover every host case in the specification, missing fields, hidden entries, and reordering or removal before activation. Assert the target and handler, not private adapter structure.
- [ ] Exercise the native registration, standby, manual, and PSN entry points available locally. Record unavailable hardware cases for final verification.

**Scope limit:** Prefer a view-local adapter. Add backend identity only if existing identity cannot pass the target checks. Preserve protocol and storage behavior.

**Specification:** [Main console screen: card library](../spec.md).

**Design reference:** B after text cleanup, preserved at `codex/prototype-console-b`, commit `73051300680f89e5c0f6dc3db71c7b581b8176e0`.

## Answer

Implemented in `471b17d`, merged as `32a9711` on the integration branch.
Cards cover the specified host cases, suppress hidden entries, and use neutral
artwork when state or type is unknown. The local adapter resolves current
identity and cancels missing, ambiguous, hidden, and old-profile targets.

The backend PSN resolver and both remote automatic-connection callers now use
the same displayed host list. No protocol or storage format changed.

Canonical MSVC/Qt 6.9.3 CTest passed after the merge. The executable reports
37 checks passed, zero failed, and zero skipped. Tests use the production
Material configuration and Windows fonts. Build commands and the current
public QML API are in `gui/tests/README.md`.

The full backend integration build follows this merge. Real registration,
standby, manual streaming, and PSN connection remain untested.
