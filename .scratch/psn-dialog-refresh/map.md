# PSN sign-in and dialogs implementation map

Date: 2026-10-21
Breakdown: approved by the user; all eight tickets published
Specification: [PSN sign-in and remaining dialogs](spec.md)

## Tickets

| Ticket | Status | Blocked by |
| --- | --- | --- |
| [01: PSN setup and credential renewal](issues/01-psn-setup-and-renewal.md) | resolved | None |
| [02: Account-ID retrieval for registration](issues/02-registration-account-id.md) | resolved | 01 |
| [03: Steam shortcut creation and results](issues/03-steam-shortcut-results.md) | resolved | None |
| [04: Startup reminder choices](issues/04-startup-reminders.md) | resolved | None |
| [05: Explicit confirmations and registration choice](issues/05-confirmations-and-registration.md) | resolved | None |
| [06: Disconnect confirmations in both renderer routes](issues/06-disconnect-confirmations.md) | resolved | None |
| [07: About and renderer notices](issues/07-about-and-renderer-notices.md) | resolved | None |
| [08: Integrated native acceptance](issues/08-native-acceptance.md) | resolved | 02, 03, 04, 05, 06, 07 |

## Execution order

- All eight tickets are resolved. The parent task owns the final two-axis review.
- Ticket 08 verified the integrated production build and cross-flow behavior after all feature dependencies were resolved.
- Each feature ticket retains its focused test evidence. The shared native build cache is restored and released.
- The parent specification remains claimed until the final code review is complete.

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


2026-10-21: Resolved [ticket 07](issues/07-about-and-renderer-notices.md). Merge `9b249964` integrates `codex/psn-information` at `52b241f4` without conflicts. About and
renderer notices reuse the refreshed compact dialog, retain all required content,
and provide one Close action with scrolling and focus restoration. The
[evidence](evidence/ticket07/README.md) records 23 passing focused checks, 141 passing
regression checks, five passing post-merge checks, and eight inspected captures.
Native focus/input/scaling and the Main non-Library fallback remain in ticket 08.
Integration also removes one extra blank line at the end of the information test file so the diff check passes.

2026-10-21: Resolved [ticket 02](issues/02-registration-account-id.md). Registration reuses the PSN sign-in screen and has a separate public lookup form. Public lookup validates responses, recovers after a 30-second timeout, and saves no credentials. Both routes return only to the original valid registration form. The [verification record](evidence/ticket02/verification.md) contains production-screen red/green checks, focused regressions, and inspected captures at both required sizes. The implementation merged integration tip `599413ba` before final verification. Native build and visible native acceptance remain in ticket 08.

2026-10-21: Merge `e71a49c2` integrates ticket 02 from `codex/psn-registration` at `70f6891e` without conflicts. Reviewed evidence records 95 passing post-merge checks. Removed trailing whitespace from one saved red log and an extra blank line from the registration test fixture. The diff check passes. Ticket 06 is the only unresolved dependency of ticket 08.

2026-10-21: Resolved [ticket 06](issues/06-disconnect-confirmations.md). Merge `9e3b24c3` integrates `codex/psn-disconnect` at `f74a445f` without conflicts. Both renderer routes use explicit disconnect, sleep, and cancel choices. The native fix supplies the real parent window and viewport-sized overlay. The [evidence record](evidence/ticket06/README.md) includes 307 Vulkan and 327 OpenGL native checks, controlled stop and sleep commands, balanced input capture, and current render-target captures. Desktop compositor captures were stale and excluded. The [cache restoration record](evidence/ticket06/cache-restoration.json) confirms 209 original source hashes and both normal targets rebuilt. Real console effects, physical controllers, final desktop composition, and the final integrated build remain for ticket 08. All seven feature tickets are resolved; ticket 08 can now start.

2026-10-21: Resolved [ticket 08](issues/08-native-acceptance.md). The [acceptance record](acceptance.md) records 869 passing production screen checks, 297 integrated native checks, 309 Vulkan checks, 329 OpenGL checks, and 31 native Steam assertions. The normal build matches all 332 staged inputs and includes 72 production resources. Acceptance fixes visible keyboard focus after shared confirmations and the Steam controller notice. The shared native cache is restored and both original targets rebuilt. Real services, physical controllers, and final desktop composition remain unverified. The final executable, patched SteamTools DLL, and launch helper remain in the separate final build directory. The parent task owns the final two-axis review.

2026-10-21: Merge `846a9ab5` integrates ticket 08 from `codex/psn-acceptance` at `478b5d69` without conflicts. All eight implementation tickets are resolved. The parent specification remains claimed pending the required final code review.
