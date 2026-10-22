# 06: Save manual console addresses and connect

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can save a manual console address, associate an existing registration, and connect to a registered manual host from the web UI.

**Blocked by:** 04: Connect locally from web setup to native playback.

**Status:** needs-info

**Spec stories:** 13, 14, 15, 23, 35.

- [ ] Manual host entry retains address validation, existing registration choices, and the separate explanation of address saving and console registration.
- [ ] Saving reports the native result and refreshes the library. It does not simulate a network operation.
- [ ] Unregistered manual entries can be saved and are clearly marked as needing registration. A registered entry can start the native connection path.
- [ ] Editing or removing a target while a form is open cannot save changes to an unrelated host.
- [ ] Errors retain useful non-secret form input, and keyboard or controller focus returns to the appropriate action.
- [ ] Embedded tests read the saved native host, reload the page, and connect a registered manual entry through the native handoff.
