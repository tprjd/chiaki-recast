# Connection and console PIN implementation tickets

Published: 2026-10-21
Breakdown and dependencies: approved by the user
Parent: [Connection progress and console login PIN: guided workspace](spec.md)
Selected design: A for every screen and state

| Ticket | Blocked by | Status |
| --- | --- | --- |
| [01: Local connection progress, cancellation, and retry](issues/01-local-connection.md) | None | resolved |
| [02: Save a console login PIN](issues/02-saved-console-pin.md) | None | resolved |
| [03: PSN connection progress and recovery](issues/03-psn-connection.md) | 01 | claimed |
| [04: Auto-connect and console wake-up](issues/04-auto-connect-wake.md) | 01 | claimed |
| [05: Live console login PIN across connection routes](issues/05-live-console-pin.md) | 01, 02 | claimed |
| [06: Integrated connection and PIN native acceptance](issues/06-native-acceptance.md) | 03, 04, 05 | ready-for-agent |

Tickets 01 and 02 can start immediately. Start each later ticket only after all its blockers are resolved. Ticket 06 transitively depends on all five implementation tickets.

Each implementation ticket includes behavior tests. Narrow refactoring is the first step inside the relevant complete flow. Shared-file edits require integration, but file overlap alone does not add a dependency. Ticket 06 verifies the combined native behavior and normal build; it does not replace individual ticket tests.

The production baseline is the completed console-setup integration branch with specification commit `cb1f76e9`. The selected prototype remains on `codex/prototype-connection-flows`, implementation commit `7a0111b6`, with the A selection recorded in `7f8b119a`.

## Specification coverage

| Ticket | Primary user stories |
| --- | --- |
| 01 | 1–8, 11, 17–28, 36, 42–44 |
| 02 | 29–36, 42–43, 45 |
| 03 | 1–4, 8–12, 17–28, 36, 42–44 |
| 04 | 13–28, 36, 42–44 |
| 05 | 20–28, 29, 33–45 |
| 06 | Integrated acceptance of all 45 stories |

The parent specification remains unchanged. Tickets 03, 04, and 05 are in progress on `codex/connection-flow-integration`.

## Decisions

- [01: Local connection progress, cancellation, and retry](issues/01-local-connection.md) is resolved. Merged `4327e9ac` with the shared attempt contract, local A progress, guarded retry, and cancellation cleanup. [Local tests](evidence/ticket01-ConnectionLocal-final.txt) passed 11 checks. [Native run 12](evidence/ticket01-native-contracts.json) passed with ten starts, four stops, and no sleep dispatch. The [normal build](evidence/ticket01-production-build.json) passed. Visible native input and integrated acceptance remain with ticket 06.
- [02: Save a console login PIN](issues/02-saved-console-pin.md) is resolved. Merged `9f2b3bfb` with the reusable A form, guarded console target, and confirmed save outcome. [PIN tests](evidence/ticket02-ConsolePin-final.txt) passed 23 checks, [management tests](evidence/ticket02-ConsoleManagement-final.txt) passed 44 checks, and [native save evidence](evidence/ticket02-native-save.json) confirms isolated persistence after process exit. Native visual and integrated acceptance remain with ticket 06.
