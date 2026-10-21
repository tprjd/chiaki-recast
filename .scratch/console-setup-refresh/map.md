# Console setup implementation

Integration branch: `codex/console-setup-integration`
Feature baseline: `0bc02e96f59666dc42ac8325645e6fea5bcaad80`

The baseline preserves the existing Library changes and the approved plan.
The original checkout and index remain unchanged.

| Ticket | Dependencies | Work branch |
| --- | --- | --- |
| [01: Manual host entry](issues/01-manual-host-entry.md) | None | `codex/console-setup-manual` |
| [02: Registration form](issues/02-registration-form.md) | None | `codex/console-setup-form` |
| [03: Progress and results](issues/03-registration-progress-results.md) | 02 | `codex/console-setup-results` |
| [04: Native acceptance](issues/04-integrated-native-acceptance.md) | 01, 03 | `codex/console-setup-acceptance` |

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
not remove its test evidence. Native acceptance is recorded under ticket 04 below.

## Decisions

Use the approved test boundaries in [the spec](spec.md). Real authentication,
console pairing, and physical controller input remain separate hardware checks.

[02: Registration form](issues/02-registration-form.md) is resolved. The native guided workspace retains all console targets and account routes. It keeps native registration arguments and the current log dialog. The focused production-QML checks passed at both target sizes. Ticket 03 can replace the result dialog through the same backend and navigation boundaries.

[01: Manual host entry](issues/01-manual-host-entry.md) is resolved. The guided
workspace preserves native save semantics and binds selection to MAC identity.
The focused production-QML suite passed 23 checks. Form, saved result, and failure
captures passed visual inspection at both required sizes. Native application
acceptance remains in ticket 04.

[03: Progress and results](issues/03-registration-progress-results.md) is resolved.
The panel uses native outcomes and retains form values for retry. Each form owns
its console context and profile generation. Native guards prevent overlap,
duplicate completion, and wrong-target or wrong-profile saves. The focused
form suite passed 22 checks, and the result suite passed 19 checks. Controlled
native contracts and post-exit INI checks passed. QA hooks were removed, and the
normal application build passed. Ticket 04 can use the external native helper
and patch recorded in ticket 03 for integrated acceptance.

[04: Native acceptance](issues/04-integrated-native-acceptance.md) is resolved.
Both native screens retain layout A. The registration header now matches manual
entry. The final integrated suite passed 627 checks. Native contracts, post-exit
INI checks, and 111 framebuffer captures passed at both sizes and measured DPR
1, 1.5, and 2, including expanded text, privacy, focus, routes, and results.
QA hooks were removed. The normal build passed, and independent Computer Use
confirmed the visible application and manual-entry route. The
[acceptance record](native-acceptance.md) separates each check and its limits.

## Completion

The [specification](spec.md) and all four tickets are resolved. The
[final review](review.md) has no open Standards or Spec findings. The one P3
duplication finding is fixed by the shared SetupButton. The cleanup merged at
`36587541` and passed 64 focused production-QML checks and the final normal build.
See the [cleanup verification](review-cleanup-verification.md) for its source
revision, exact binary hash, and the checks retained from native acceptance.

The five implementation worktrees are archived. This integration worktree and
all tracked evidence remain available. The original checkout and its 18
recorded file hashes are unchanged. The final executable is in
`C:/Users/david/.cache/chiaki-ui-native/console-setup-final-bin/`; its launcher
supplies the runtime paths. It was not started, so the open app was untouched.
