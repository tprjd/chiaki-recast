# Console setup implementation

Integration branch: `codex/console-setup-integration`
Feature baseline: `0bc02e96f59666dc42ac8325645e6fea5bcaad80`

The baseline preserves the existing Library changes and the approved plan.
The original checkout and index remain unchanged.

| Ticket | Dependencies | Work branch |
| --- | --- | --- |
| [01: Manual host entry](issues/01-manual-host-entry.md) | None | `codex/console-setup-manual` |
| [02: Registration form](issues/02-registration-form.md) | None | `codex/console-setup-form` |
| [03: Progress and results](issues/03-registration-progress-results.md) | 02 | Assigned after 02 merges |
| [04: Native acceptance](issues/04-integrated-native-acceptance.md) | 01, 03 | Assigned after 01 and 03 merge |

Each ticket records its status and evidence. Completion requires the integrated
screen tests, native contract checks, native UI checks, and final code review.

## Decisions

Use the approved test boundaries in [the spec](spec.md). Real authentication,
console pairing, and physical controller input remain separate hardware checks.
