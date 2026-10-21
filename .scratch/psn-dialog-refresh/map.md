# PSN sign-in and dialogs implementation map

Date: 2026-10-21
Breakdown: approved by the user; all eight tickets published
Specification: [PSN sign-in and remaining dialogs](spec.md)

## Tickets

| Ticket | Status | Blocked by |
| --- | --- | --- |
| [01: PSN setup and credential renewal](issues/01-psn-setup-and-renewal.md) | resolved | None |
| [02: Account-ID retrieval for registration](issues/02-registration-account-id.md) | ready-for-agent | 01 |
| [03: Steam shortcut creation and results](issues/03-steam-shortcut-results.md) | resolved | None |
| [04: Startup reminder choices](issues/04-startup-reminders.md) | resolved | None |
| [05: Explicit confirmations and registration choice](issues/05-confirmations-and-registration.md) | resolved | None |
| [06: Disconnect confirmations in both renderer routes](issues/06-disconnect-confirmations.md) | claimed | None |
| [07: About and renderer notices](issues/07-about-and-renderer-notices.md) | claimed | None |
| [08: Integrated native acceptance](issues/08-native-acceptance.md) | ready-for-agent | 02, 03, 04, 05, 06, 07 |

## Execution order

- Tickets 06 and 07 are claimed. Ticket 02 is open and unblocked. The first eligible ticket is 02.
- Ticket 08 starts after 02, 03, 04, 05, 06, and 07 are resolved. Ticket 01 is included through 02.
- Every feature ticket includes its own focused tests. Ticket 08 verifies the integrated production build and cross-flow behavior.
- Shared-file overlap requires coordination, not extra blocking edges. Allow only one writer to the shared native build cache.

## Coverage

| Spec user stories | Primary owner |
| --- | --- |
| 1-7, 63 | Every affected feature ticket; integrated acceptance in 08 |
| 8, 10-13, 36 | 02 |
| 9, 14-35, 37 | 01, reused by 02 where applicable |
| 38-47 | 03 |
| 48-50 | 04 |
| 51-56 | 05 |
| 57-59 | 06 |
| 60-62 | 07 |

## Scope and verification

Use the new design from the completed refresh work. Preserve the specification's authentication, storage, and operation-closure decisions. No separate refactor ticket is required.

Use the confirmed production-QML, isolated-native, and visible-native test approach. Keep real PSN, Steam installation, console, and physical-controller results separate from controlled checks. The parent specification is unchanged.

## Comments

2026-10-21: The user approved the recommended breakdown and requested publication of all eight tickets. All tickets are ready-for-agent and unclaimed. Publication does not start implementation.

2026-10-21: The user invoked implement-spec. Implementation is active on `codex/psn-dialog-integration`, based on `9b138cc43a5d667933284f1c24e8dabb721b01ad`. Tickets 01, 03, and 05 are claimed in separate managed worktrees. Each implementer uses TDD and merges the integration tip before handoff. A merger agent integrates completed branches. Ticket 01 currently owns the shared native build cache.

Final review compares the integration branch with `9b138cc43a5d667933284f1c24e8dabb721b01ad`. The approved specification and published tickets are the review contract. No PR or push is part of this request.

2026-10-21: Resolved [ticket 05](issues/05-confirmations-and-registration.md). Merged `codex/psn-confirmations` at `29d55e6c` without conflicts. Confirmations use explicit action labels and initial Cancel focus. Registration has separate automatic, manual, and cancel choices. The [focused confirmation results](evidence/ticket05/confirm-final.txt) and other evidence linked in the ticket record 111 passing checks and no failures. Visible native and integrated acceptance remain in ticket 08.

2026-10-21: Resolved [ticket 03](issues/03-steam-shortcut-results.md). Merge `39bea6eb` integrates `codex/psn-steam` at `8e764707` without conflicts. Steam creation retains editable input, reports terminal results, and applies dependency fixes to a build copy. The [focused screen results](evidence/ticket03-screen-green.txt) record 21 passing checks. The [native results](evidence/ticket03-native-green.txt) verify actual writes and controlled failures in a temporary Steam directory. The [broad regression run](evidence/ticket03-screen-regressions.txt) was stopped and is incomplete. Normal application, visible native, and real Steam acceptance remain in ticket 08.

2026-10-21: Resolved [ticket 04](issues/04-startup-reminders.md). Merge `2c05f543` integrates `codex/psn-reminders` at `216d5d62` without conflicts. Reminders have separate Set up now, Later, and Don't ask again choices. Dismissal preserves preferences and retains Steam-then-PSN order. The [reminder results](evidence/ticket04/reminder-final.txt) record 28 passing checks. The ticket links another 50 passing focused regression checks for confirmations, registration choices, and Steam reminder results. Visible native and integrated acceptance remain in ticket 08.

2026-10-21: Resolved [ticket 01](issues/01-psn-setup-and-renewal.md). Merge `d22b035b` integrates `codex/psn-setup` at `c4fff081` without conflicts. PSN setup uses guarded native requests, masked redirect input, persistent results, and explicit closure wording. The [full screen results](evidence/ticket01-screen-full.txt) record 721 passing checks, and the [post-merge PSN results](evidence/ticket01-screen-after-merge.txt) record 10 passing checks. The [controlled native evidence](evidence/ticket01-native-contracts.json) records 125 checks, actual isolated persistence, and visible native input at both required sizes. The normal build matches source `19c81712` before integration and does not prove the final integrated build. Real PSN sign-in, physical controller input, and final integrated acceptance remain in ticket 08. Ticket 02 can now start.

