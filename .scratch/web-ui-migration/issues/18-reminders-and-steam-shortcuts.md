# 18: Complete setup reminders and Steam shortcuts

**What to build:** Users can complete the existing setup reminders, confirmations, and supported Steam shortcut workflow from the web UI.

**Blocked by:** 06: Sign in to PSN independently from web setup.

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 38, 60, 61, 62, 69.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] Startup and PSN reminders retain their eligibility, dismissal, remembered choice, and repetition rules.
- [ ] Sign-in reminders reach the migrated account route and preserve focus. Other reminders retain their native eligibility and available destinations without depending on console management or manual address forms.
- [ ] Supported Steam shortcut creation uses native folder selection and completion signals and retains its validation and error handling.
- [ ] Repeated activation, cancelled dialogs, stale target selection, and changed profiles cannot create duplicate or unrelated shortcuts.
- [ ] Information and confirmation dialogs, permitted help links, privacy masking, and keyboard or controller focus return remain usable.
- [ ] Embedded tests verify reminder persistence, shortcut native results, cancelled actions, and stale completion.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Comments

- Claimed after ticket 06. Prepare the native reminder and Steam shortcut workflow in an isolated worktree. Use the 1024 CSS pixel design scope and focused embedded checks.
