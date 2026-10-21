# PSN sign-in and dialogs implementation map

Date: 2026-10-21
Breakdown: approved by the user; all eight tickets published
Specification: [PSN sign-in and remaining dialogs](spec.md)

## Tickets

| Ticket | Status | Blocked by |
| --- | --- | --- |
| [01: PSN setup and credential renewal](issues/01-psn-setup-and-renewal.md) | ready-for-agent | None |
| [02: Account-ID retrieval for registration](issues/02-registration-account-id.md) | ready-for-agent | 01 |
| [03: Steam shortcut creation and results](issues/03-steam-shortcut-results.md) | ready-for-agent | None |
| [04: Startup reminder choices](issues/04-startup-reminders.md) | ready-for-agent | None |
| [05: Explicit confirmations and registration choice](issues/05-confirmations-and-registration.md) | ready-for-agent | None |
| [06: Disconnect confirmations in both renderer routes](issues/06-disconnect-confirmations.md) | ready-for-agent | None |
| [07: About and renderer notices](issues/07-about-and-renderer-notices.md) | ready-for-agent | None |
| [08: Integrated native acceptance](issues/08-native-acceptance.md) | ready-for-agent | 02, 03, 04, 05, 06, 07 |

## Execution order

- Tickets 01, 03, 04, 05, 06, and 07 can start immediately. The first eligible ticket is 01.
- Ticket 02 starts after 01 is resolved.
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

