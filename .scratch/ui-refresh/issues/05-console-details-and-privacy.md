# 05: Inspect console details without exposing private data

**What to build:** A compact details view from each card, showing only available fields and applying streamer-mode masking throughout the new screen.

**Blocked by:** 03: Make B the native main console screen.

**Status:** resolved

**Spec stories:** 10, 18, 20.

- [ ] The card's details control opens information for the selected console without also starting a connection.
- [ ] Show available address, ID, registration, route, running application, and title ID fields. Omit missing fields instead of inventing values.
- [ ] Respect streamer mode in cards, details, tooltips, accessible names, and fallback labels. Mask an address used as a manual console's name.
- [ ] Resolve the current host by identity. Update or close details if the host disappears, and close stale details when the profile changes.
- [ ] Support mouse and keyboard opening. Escape closes the popup; closing restores focus to the opener or a valid nearby control.
- [ ] Screen tests assert displayed values, missing-field behavior, absence of private values from accessible text, and safe behavior when hosts change while details are open.

**Scope limit:** Reuse the screen test boundary and existing dialog conventions. Management actions are added in ticket 06.

**Specification:** [Main console screen: card library](../spec.md).

**Design reference:** B after text cleanup, preserved at `codex/prototype-console-b`, commit `73051300680f89e5c0f6dc3db71c7b581b8176e0`.

## Answer

Implemented in `ca8183a` with literal-text fix `a0d1a17`, combined with ticket
04 in `f5cde23`, then merged as `689d56b`. The existing ellipsis opens live
console details. Missing fields are omitted. Address, ID, and address fallback
names are masked in streamer mode, including accessible and tooltip text.

Details follow the current identity and close for removed, hidden, ambiguous,
or old-profile targets. Closing restores focus. Long content scrolls while
Close and temporary Manage controls remain reachable. Names and details use
plain text; a rendered-link regression proves tag-like host text stays literal.

The combined suite reports 106 checks passed with no failures, skips, or
warnings. CTest passed from the integration checkout after merge. The
integration agent inspected normal and narrow long-text captures. Ticket 06
will replace the temporary Manage menu and complete delayed action checks.
