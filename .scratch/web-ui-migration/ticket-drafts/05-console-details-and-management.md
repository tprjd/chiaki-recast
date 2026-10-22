# 05: Manage consoles from the web library

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can inspect console details, hide or restore consoles, remove entries, and update a saved console login PIN from the web library.

**Blocked by:** 03: Show live consoles in the GPU-accelerated web UI.

**Status:** needs-info

**Spec stories:** 10, 35, 36, 37, 38, 39, 69, 70.

- [ ] Details and action menus show the intended console type, status, registration state, and permitted actions using the selected design.
- [ ] Hide, unhide, manual entry deletion, and registration deletion retain their existing distinctions and confirmation behavior.
- [ ] Each management action resolves the current native target. Reordered discovery, removed consoles, ambiguous targets, and stale profiles cannot affect a different console.
- [ ] Saved console login PIN changes preserve leading zeroes and native validation. Saved PINs and registration keys are not returned to the page.
- [ ] Streamer mode masks identifiers and addresses throughout cards, details, confirmations, and errors.
- [ ] Library selection and focus return correctly after menus, dialogs, deletion, and live updates.
- [ ] Embedded tests verify native state changes, cancelled confirmations, stale targets, and private display behavior.
