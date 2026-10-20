# 04: Find consoles without losing the action target

**What to build:** Search, accurate counts, useful empty states, and stable selection as the real console list changes.

**Blocked by:** 03: Make B the native main console screen.

**Status:** resolved

**Spec stories:** 3, 18, 19.

- [ ] Search names and known console types, supporting mixed case, spaces, clearing, and no matches. Preserve backend ordering and hidden/duplicate suppression.
- [ ] Show totals for visible hosts and filtered counts during search. If an online count is retained, count only confirmed ready discovery results.
- [ ] Filtering to a later source entry dispatches to that console rather than the filtered row number. Reuse the identity handling from ticket 02.
- [ ] Preserve selected identity after live updates when possible. If it disappears, choose a valid remaining entry or clear selection. Clear stale target state on profile changes.
- [ ] Distinguish no visible consoles, discovery disabled, and no search matches. Provide Add console, Enable discovery, or Clear search as appropriate.
- [ ] Search accepts typing and text-editing keys without triggering card actions. Re-rendering does not lose cursor position or focus.
- [ ] Screen tests cover filtering, reordering, all-hidden lists, selected-host deletion, profile changes, and empty-list navigation without loops or stale actions.

**Scope limit:** Do not add sorting, favorites, saved searches, or a persistent host model.

**Specification:** [Main console screen: card library](../spec.md).

**Design reference:** B after text cleanup, preserved at `codex/prototype-console-b`, commit `73051300680f89e5c0f6dc3db71c7b581b8176e0`.

## Answer

Implemented in `2baa349`, merged as `02d5c62`. Search covers names and known
console types, case and whitespace. Counts respect display suppression.
Empty states provide the appropriate Add, Enable discovery, or Clear search
action. Selection and action identity remain safe during live updates.

The viewport determines card width, including partial rows and one search
result. No blank delegates are added. The integration agent inspected the
desktop result and narrow empty-state captures, then repeated CTest after
merge. The Qt suite reports 89 checks passed, zero failed, and zero skipped.

The normal Qt text field preserves editing keys and consumes unused keys
before they can activate a console. Whole-screen tests remain at the approved
boundary. Full native integration and final input checks follow later.
