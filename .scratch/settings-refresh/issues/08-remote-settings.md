# 08: Manage PSN access and remote connection options

Status: resolved

**What to build:** Use Remote to enter the existing PSN login flow, clear tokens
when applicable, and configure port guessing.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [x] Login and Clear PSN Token appear under their existing native credential conditions.
- [x] Reuse the current authentication flow; account-state changes refresh the page without exposing credentials.
- [x] Include forced STUN port guessing, port count, and socket count with native defaults, limits, and update timing.
- [x] Returning from login, cancellation, or an error restores usable Settings focus.
- [x] Production-QML tests verify token-state transitions, intended login/clear dispatch, numeric writes, and navigation with fake credentials and handlers.
- [x] Build and demonstrate the native page using isolated state. Automated tests must not authenticate or clear the user's tokens.



## Answer

Implemented in 5010ec3 and merged at 8b9042b. All 31 Remote checks pass, including all credential-state combinations, clearing only the four native token fields, login navigation return, numeric bounds and account-change focus. The post-merge full CTest run passed. Native Remote navigation, focus bounds and no-write checks pass at 1280 by 720 and 900 by 600 using isolated state. Evidence: ../native-previews/07-09-management-dpr1-checks.json. Real authentication, network behavior and login errors remain untested; fake handlers exercised routing and cancellation without touching user tokens.
