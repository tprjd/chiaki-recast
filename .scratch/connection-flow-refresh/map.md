# Connection and console PIN implementation tickets

Published: 2026-10-21
Breakdown and dependencies: approved by the user
Parent: [Connection progress and console login PIN: guided workspace](spec.md)
Selected design: A for every screen and state

| Ticket | Blocked by | Status |
| --- | --- | --- |
| [01: Local connection progress, cancellation, and retry](issues/01-local-connection.md) | None | resolved |
| [02: Save a console login PIN](issues/02-saved-console-pin.md) | None | resolved |
| [03: PSN connection progress and recovery](issues/03-psn-connection.md) | 01 | resolved |
| [04: Auto-connect and console wake-up](issues/04-auto-connect-wake.md) | 01 | resolved |
| [05: Live console login PIN across connection routes](issues/05-live-console-pin.md) | 01, 02 | resolved |
| [06: Integrated connection and PIN native acceptance](issues/06-native-acceptance.md) | 03, 04, 05 | resolved |

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

All six tickets are resolved on `codex/connection-flow-integration`. The [acceptance record](acceptance.md) gives the results and remaining hardware limits.

## Decisions

- [01: Local connection progress, cancellation, and retry](issues/01-local-connection.md) is resolved. Merged `4327e9ac` with the shared attempt contract, local A progress, guarded retry, and cancellation cleanup. [Local tests](evidence/ticket01-ConnectionLocal-final.txt) passed 11 checks. [Native run 12](evidence/ticket01-native-contracts.json) passed with ten starts, four stops, and no sleep dispatch. The [normal build](evidence/ticket01-production-build.json) passed. Visible native input and integrated acceptance remain with ticket 06.
- [02: Save a console login PIN](issues/02-saved-console-pin.md) is resolved. Merged `9f2b3bfb` with the reusable A form, guarded console target, and confirmed save outcome. [PIN tests](evidence/ticket02-ConsolePin-final.txt) passed 23 checks, [management tests](evidence/ticket02-ConsoleManagement-final.txt) passed 44 checks, and [native save evidence](evidence/ticket02-native-save.json) confirms isolated persistence after process exit. Native visual and integrated acceptance remain with ticket 06.
- [03: PSN connection progress and recovery](issues/03-psn-connection.md) is resolved. Implementation is integrated from `1cc42ee8`. [Native checks](evidence/ticket03-native-contracts.json) passed 64 assertions, the retained QML results passed 188 checks, and [post-exit persistence](evidence/ticket03-post-exit.json) passed five checks. Ticket 05's [native PIN continuity proof](evidence/ticket05-native-inline.json) completes the PSN-to-stream criterion.
- [04: Auto-connect and console wake-up](issues/04-auto-connect-wake.md) is resolved. Merged `77039744` with A's startup and wake progress, validated retry, and cancellation. [Native evidence](evidence/ticket04-native-contracts.json) passed 40 assertions, and [post-exit persistence](evidence/ticket04-native-post-exit.json) passed five checks. The normal build covers revision `2a854392`. The later shared exit and focus guard passed [13 wake checks](evidence/ticket04-qml-green.txt) and [11 local checks](evidence/ticket04-ConnectionLocal-final.txt). Ticket 05 owns the integrated build; ticket 06 owns visible native acceptance.
- [05: Live console login PIN across connection routes](issues/05-live-console-pin.md) is resolved. Merged `f54346ae` with the shared live PIN host and guarded native request actions. [Inline](evidence/ticket05-native-inline.json) and [separate hidden-window](evidence/ticket05-native-separate.json) checks each passed 99 assertions, including PSN-to-stream continuity and balanced capture. The [normal build manifest](evidence/ticket05-normal-production-manifest.json) covers 178 GUI sources. Visible native focus, input delivery, and DPR checks remain with ticket 06.
- [06: Integrated connection and PIN native acceptance](issues/06-native-acceptance.md) is resolved. Merged `d13637b6` with [controlled native acceptance](acceptance.md), 711 full and 83 final focused QML checks, 30 captures, measured DPR 1, 1.5, and 2, and an actual Vulkan inline check. Both final reviews have no unresolved findings. The [normal manifest](evidence/ticket06-normal-production-manifest.json) covers 178 tested GUI sources. Real PSN authentication, physical wake-up, hardware PIN acceptance, stream transport, a physical controller, and a live move between different monitor DPR values remain unverified.
