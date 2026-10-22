# 18: Complete setup reminders and Steam shortcuts

**What to build:** Users can complete the existing setup reminders, confirmations, and supported Steam shortcut workflow from the web UI.

**Blocked by:** 06: Sign in to PSN independently from web setup.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 38, 60, 61, 62, 69.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] Startup and PSN reminders retain their eligibility, dismissal, remembered choice, and repetition rules.
- [x] Sign-in reminders reach the migrated account route and preserve focus. Other reminders retain their native eligibility and available destinations without depending on console management or manual address forms.
- [x] Supported Steam shortcut creation uses native folder selection and completion signals and retains its validation and error handling.
- [x] Repeated activation, cancelled dialogs, stale target selection, and changed profiles cannot create duplicate or unrelated shortcuts.
- [x] Information and confirmation dialogs, permitted help links, privacy masking, and keyboard or controller focus return remain usable.
- [x] Embedded tests verify reminder persistence, shortcut native results, cancelled actions, and stale completion.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Comments

- Claimed after ticket 06. Prepare the native reminder and Steam shortcut workflow in an isolated worktree. Use the 1024 CSS pixel design scope and focused embedded checks.


## Answer

The web UI now owns startup reminders and the Steam form. Native state retains
Steam-before-PSN order, once-per-setup repetition, remembered choices, complete
credential suppression, and auto-connect suppression. PSN setup uses the
existing account route and native sign-in service.

Steam uses native folder selection, opaque form/profile identities, and native
progress/completion signals. Paths and profile defaults stay native in streamer
mode. Repeated, cancelled, obsolete, hidden, reloaded, and changed-profile
operations cannot write an unrelated shortcut. The existing Steam file code,
validation, backups, partial-failure warning, retry, and restart instruction
remain. Success alone clears the stored Steam ask flag.

The existing triangle/Y dismissal, F1/L3 open action, form Menu submission,
field navigation, DOM focus return, and a native help allowlist are verified.
Replaced reminder/Steam QML leaves, resources, fixtures, and QJS callback tests
are removed. Native stream components remain.

[Focused evidence](../evidence/ticket18/verification.md) records red/green
results, real temporary Steam files, saved settings, source/artifact identities,
and the final integration merge. App/test builds and frontend checks pass.
Actual focus, physical input, GPU, live services, Steam library visibility,
and platform acceptance remain with ticket 23.
