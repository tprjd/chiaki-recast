# 05: Manage consoles and manual addresses from the web library

**What to build:** Users can inspect and manage consoles, save manual addresses, associate existing registrations, update a saved console login PIN, and connect a registered manual host through native playback.

**Blocked by:** 04: Connect locally from web setup to native playback.

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 10, 13, 14, 15, 23, 35, 36, 37, 38, 39, 69, 70.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] Details and action menus show the intended console type, status, registration state, and permitted actions using the selected design.
- [ ] Hide, unhide, manual entry deletion, and registration deletion retain their existing distinctions and confirmation behavior.
- [ ] Each management action resolves the current native target. Reordered discovery, removed consoles, ambiguous targets, and stale profiles cannot affect a different console.
- [ ] Saved console login PIN changes preserve leading zeroes and native validation. Saved PINs and registration keys are not returned to the page.
- [ ] Streamer mode masks identifiers and addresses throughout cards, details, confirmations, and errors.
- [ ] Library selection and focus return correctly after menus, dialogs, deletion, and live updates.
- [ ] Manual host entry retains address validation, existing registration choices, and the separate explanation of address saving and console registration.
- [ ] Saving reports the native result and refreshes the library. It does not simulate a network operation.
- [ ] Unregistered manual entries can be saved and are clearly marked as needing registration. A registered entry can start the native connection path.
- [ ] Editing or removing a target while a form is open cannot save changes to an unrelated host.
- [ ] Errors retain useful non-secret form input, and keyboard or controller focus returns to the appropriate action.
- [ ] Embedded tests verify native management state changes, cancelled confirmations, stale targets, privacy masking, saved manual hosts after reload, and connection of a registered manual entry through native handoff.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.
