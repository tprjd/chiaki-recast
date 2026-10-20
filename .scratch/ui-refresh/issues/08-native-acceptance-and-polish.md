# 08: Verify the native library against B

**What to build:** A visually reviewed, usable native console library with recorded acceptance results and any remaining layout defects corrected.

**Blocked by:** 07: Complete keyboard and controller navigation.

**Status:** resolved

**Spec stories:** 1, 15, 21, 22; final verification of all specification stories.

- [x] Compare native screenshots against the approved B snapshot at matching sizes. Correct spacing, typography, artwork, and alignment without restoring decorative text.
- [x] Check 1280 by 720 and 800 by 600 logical pixels, plus the supported narrower minimum if applicable. Exercise long names, absent fields, and many consoles.
- [x] Check 100%, 150%, and 200% Windows scaling. Essential actions remain readable, accessible, and reachable without clipping.
- [x] Build and launch the application. Run the screen tests and relevant existing native tests if backend code changed. Record reproducible commands and results.
- [x] Exercise real local-ready, standby, manual, registration, PSN, discovery, and return-from-stream flows where hardware/accounts permit. Record each unavailable case as untested rather than passed.
- [x] Verify startup reminders, automatic connection, profile changes, error recovery, streamer mode, and optional Steam commands in the integrated screen.
- [x] Confirm production resources contain no prototype routes, comparison controls, fake hosts, or test-only dependencies. Preserve the prototype snapshot as the design reference.
- [x] Review the final change and record screenshots, test results, and remaining limitations for user review. The approved specification remains unchanged unless separately authorized.

**Scope limit:** No unrelated performance, Settings, streaming, Android, or protocol changes.

**Specification:** [Main console screen: card library](../spec.md).

**Design reference:** B after text cleanup, preserved at `codex/prototype-console-b`, commit `73051300680f89e5c0f6dc3db71c7b581b8176e0`.

## Comments

Implementation review notes for final acceptance:

- The current visual guards in `tst_ConsoleNavigation.qml` compare red-channel
  values with `0.3` and `0.8`. Qt's image channel methods return byte values,
  as shown in the [Qt Quick Test documentation](https://doc.qt.io/qt-6/qml-qttest-testcase.html#grabImage-method).
  Use thresholds on that scale so nearly black artwork cannot pass a contrast
  guard. Verify the rendered image after changing the check.
- Rewrite `gui/tests/README.md` around the final API and verification commands.
  Remove obsolete row-embedding and temporary source-map descriptions. Keep
  actual verification limits clear; avoid contradictory historical claims.
- Compare the search field with B's compact desktop field. Ticket 04 currently
  fills the toolbar width. Keep narrow-window behavior while correcting any
  unnecessary difference in desktop spacing.
- Native build and isolated-launch commands are in the external task notes at
  `C:\Users\david\.codex\ui-refresh-native-toolchain.md`. The build helper uses
  cache-only settings routing. Keep that instrumentation out of production.

The preserved prototype was also checked at 390 pixels wide. Check the native library at 390 by 600 as the narrower case, including optional PSN/Steam commands, many cards, long names, and details. Keep any narrow-size fix local to this library. Do not add a global window minimum that changes stream sizing. Record the smallest size actually verified.

Ticket 07 found another weakness in the earlier pixel checks. In this offscreen setup, grabImage(control) can return an origin-cropped image rather than that child's rendered region. Its new focus checks capture the complete screen and map the control region. Use that approach when strengthening the older button/artwork contrast checks, and verify a dark body or absent background actually fails.

## Answer

Merged as 43a839d8c528c574f6e2e2e7ede961c253cf3c3b. The final library matches B with compact desktop search and usable 390-pixel spacing. Native window wheel forwarding is fixed under the existing UI-input gate. All 190 screen checks pass, integration CTest passes, and the complete Windows application builds with both native CTest targets passing. The integration agent inspected final QML and actual native captures and verified the nine measured-DPR viewport records. DPR 1.0, 1.5, and 2.0 each pass at 1280 by 720, 800 by 600, and 390 by 600. Scaling uses process-local Qt overrides on Windows; global desktop settings remain unchanged.

Final startup, automatic-connect cancellation, profile isolation, backend error recovery, optional Steam entry/cancel, privacy, and synthetic PSN target checks pass. Native PIN, Delete, cancellation, stale-target, and focus replay pass. Both independent review axes have zero findings. The approved spec and prototype remain unchanged, and production resources contain no test fixture, capture, prototype route, or test dependency.

See ../acceptance.md and ../code-review.md for evidence and commands. Real console discovery, ready/standby connections, network Wake, registration acceptance, PSN authentication/connection, stream return, and physical controller input remain untested. Existing narrow Settings and registration content clipping is outside scope.