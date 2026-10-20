# 08: Manage PSN access and remote connection options

Publication: draft; awaiting breakdown approval

**What to build:** Use Remote to enter the existing PSN login flow, clear tokens
when applicable, and configure port guessing.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [ ] Login and Clear PSN Token appear under their existing native credential conditions.
- [ ] Reuse the current authentication flow; account-state changes refresh the page without exposing credentials.
- [ ] Include forced STUN port guessing, port count, and socket count with native defaults, limits, and update timing.
- [ ] Returning from login, cancellation, or an error restores usable Settings focus.
- [ ] Production-QML tests verify token-state transitions, intended login/clear dispatch, numeric writes, and navigation with fake credentials and handlers.
- [ ] Build and demonstrate the native page using isolated state. Automated tests must not authenticate or clear the user's tokens.

