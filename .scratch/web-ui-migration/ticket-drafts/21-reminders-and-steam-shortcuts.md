# 21: Complete setup reminders and Steam shortcuts

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can complete the existing setup reminders, confirmations, and supported Steam shortcut workflow from the web UI.

**Blocked by:** 05: Manage consoles from the web library; 09: Sign in to PSN from web setup.

**Status:** needs-info

**Spec stories:** 38, 60, 61, 62, 69.

- [ ] Startup and PSN reminders retain their eligibility, dismissal, remembered choice, and repetition rules.
- [ ] Reminder actions reach the migrated sign-in or console flow and preserve the intended target and focus.
- [ ] Supported Steam shortcut creation uses native folder selection and completion signals and retains its validation and error handling.
- [ ] Repeated activation, cancelled dialogs, stale target selection, and changed profiles cannot create duplicate or unrelated shortcuts.
- [ ] Information and confirmation dialogs, permitted help links, privacy masking, and keyboard or controller focus return remain usable.
- [ ] Embedded tests verify reminder persistence, shortcut native results, cancelled actions, and stale completion.
