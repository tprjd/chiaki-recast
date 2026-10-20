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

## Integration evidence

- The complete baseline suite passed in 407.94 seconds before feature changes.
- Ticket 01 and its long-label correction merged at `71be21f0`.
  [Integrated manual tests](evidence/ticket01-screen-tests.txt): 23 passed.
  [Earlier complete screen run](evidence/ticket01-full-screen-tests.txt): 586 passed.
- Ticket 02 merged at `8c8486ab`.
  [Integrated registration form tests](evidence/ticket02-screen-tests.txt): 22 passed.

These logs are retained here so that removing an implementation worktree does
not remove its test evidence. Native acceptance remains ticket 04's work.

## Decisions

Use the approved test boundaries in [the spec](spec.md). Real authentication,
console pairing, and physical controller input remain separate hardware checks.

[02: Registration form](issues/02-registration-form.md) is resolved. The native guided workspace retains all console targets and account routes. It keeps native registration arguments and the current log dialog. The focused production-QML checks passed at both target sizes. Ticket 03 can replace the result dialog through the same backend and navigation boundaries.

[01: Manual host entry](issues/01-manual-host-entry.md) is resolved. The guided
workspace preserves native save semantics and binds selection to MAC identity.
The focused production-QML suite passed 23 checks. Form, saved result, and failure
captures passed visual inspection at both required sizes. Native application
acceptance remains in ticket 04.
