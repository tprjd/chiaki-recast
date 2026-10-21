# Connection and console PIN implementation tickets

Published: 2026-10-21
Breakdown and dependencies: approved by the user
Parent: [Connection progress and console login PIN: guided workspace](spec.md)
Selected design: A for every screen and state

| Ticket | Blocked by | Status |
| --- | --- | --- |
| [01: Local connection progress, cancellation, and retry](issues/01-local-connection.md) | None | claimed |
| [02: Save a console login PIN](issues/02-saved-console-pin.md) | None | claimed |
| [03: PSN connection progress and recovery](issues/03-psn-connection.md) | 01 | ready-for-agent |
| [04: Auto-connect and console wake-up](issues/04-auto-connect-wake.md) | 01 | ready-for-agent |
| [05: Live console login PIN across connection routes](issues/05-live-console-pin.md) | 01, 02 | ready-for-agent |
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

The parent specification remains unchanged. Tickets 01 and 02 are in progress on `codex/connection-flow-integration`.
