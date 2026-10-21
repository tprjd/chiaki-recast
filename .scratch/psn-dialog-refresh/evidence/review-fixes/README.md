# Final review fix verification

Date: 2026-10-21. Integration base: `242c2fa0`.
Both findings in [the final review](../../review.md) have fixes.
The parent task owns the review resolution and the specification status.

## Changes and regression evidence

The shared confirmation footer forwards unhandled keys to the existing scroll view.
Page Up and Page Down now scroll long text from a focused action.
These keys also map to the controller shoulder buttons.
Initial Cancel focus, arrow navigation, Tab navigation, and action activation remain covered.
The [new regression](scroll-red.txt) fails in all four size and action rows before the fix.
The [focused result](scroll-green.txt) passes after the one-line change.
The reviewer's original [reproduction](reviewer-scroll-repro.qml) and [failed result](reviewer-scroll-red.txt) are retained.

The Steam form calls `navigation.showPsnReminder()` after closure.
The existing `fromReminder` and `remotePlayAsk` conditions remain in place.
The shared reminder owns credential eligibility and wording.
The supplied navigation fixture uses the production reminder policy.
The [old route fails](reminder-route-red.txt) with that contract.
The [new route passes](reminder-route-green.txt).
The real reminder-chain tests cover configured and incomplete PSN setup after Steam cancel, success, and failure.

One initial full Steam run also exposed a test layout race.
[The failed probe](steam-layout-red.txt) has Retry at x = 0 while Edit is 94 pixels wide.
[The passing probe](steam-layout-pass-before-wait.txt) has Retry at x = 104.
Active focus arrived before the result Flow had its rendered positions.
The test now waits for rendering before the mouse click.
The final full Steam suite passes, followed by two complete passing repeats.
The production retry code is unchanged.
`steam-initial-retry-layout-red.txt` retains the initial failure.

## Final-source results

| Check | Result |
| --- | --- |
| Confirmations | 40 passed |
| Reminders | 31 passed |
| Information dialogs | 23 passed |
| Disconnect | 19 passed |
| Steam shortcut | 21 passed |
| Registration method | 8 passed |
| Integrated native routes and actual temporary Steam form writes | 297 passed |
| Actual Vulkan native route and long-text scrolling | 337 passed |
| Actual OpenGL native route and long-text scrolling | 357 passed |

[The affected screen summary](screen-summary.json) records 142 passed checks with no failures or skips.
Each `screen-*.txt` file is a complete production-screen suite.
The two additional Steam runs are `steam-repeat-1.txt` and `steam-repeat-2.txt`.
The first information run lacked the fresh worktree's `build/` capture directory.
The directory was created before the final passing run.

Native checks use 900 by 600 and 1280 by 720 clients at measured DPR 1.25.
Both actual renderers scroll overflowing reminder text with Page Down and Page Up
while Later keeps visible focus. Retained GPU captures include those states.
The existing native checks also verify actions, native window closure, input capture,
balanced release, controlled Stop and GoToBed requests, and unchanged saved preferences.
The integrated run verifies the actual Steam-to-PSN reminder chain through the shared function.

## Source and normal build

[The source comparison](source-delta.json) lists five changed inputs since ticket 08.
Only two production QML files and three test files changed.
The other 327 staged inputs match the earlier acceptance source.
The earlier complete 38-suite result of 869 passed checks remains prior-source evidence.
The final-source affected suites and native runs above cover the review changes.
The native C++ Steam command and dependency patch are unchanged, so their earlier
31 native file assertions were not repeated. The integrated form still makes actual temporary writes.

[The current normal build record](normal-build.json) supersedes ticket 08's executable and DLL hashes.
It verifies all 332 staged inputs, eight pinned submodules, all 72 production resources,
required screen inclusion, fixture exclusion, and absent QA symbols.
Two consecutive configurations pass. Normal `--help` exits with code 0 before Settings construction.
The executable, app-local SteamTools DLL, and adjacent `launch-chiaki.ps1` remain in
`C:/Users/david/.cache/chiaki-ui-native/psn-dialog-final-bin/`.
[The cache restoration record](cache-restoration.json) verifies all 332 original source hashes,
original build options, rebuilt original targets, and unchanged final artifacts after restoration.

## Replay and limits

The [ticket 08 procedure](../ticket08/README.md) supplies the compiler environment,
isolation steps, native build commands, and restoration sequence.
Use new preparation and run names.
Use this directory's `prepare-disconnect.py` to include the native long-text checks.
Use this directory's other native helpers and source-check scripts to record a replay separately.
Stage and restore all build inputs with ticket 08's `stage-native.py`.
Create the repository's `build/` directory before screen tests save captures.

The affected screen command is `chiaki-console-screen-tests.exe -input gui/tests/qml/tst_<suite>.qml -o <result>,txt`.
The six suite names are Confirmations, Reminders, InformationDialogs, Disconnect,
SteamShortcut, and RegistrationMethod.
Native preparation names were `disconnect-prep-review` and `psn-setup-prep-review`.
The full-source backup is `psn-acceptance-prep-review`.
Native run directories are `psn-review-vulkan125`, `psn-review-opengl125`, and `psn-review-integration125`.

Service replies and session commands are controlled. Settings saves and temporary Steam writes are real.
QtWebEngine is unavailable in the native build. Its external fallback and supplied-browser screen tests retain prior coverage.
Final desktop composition, real PSN sign-in, live lookup, browser navigation, Steam visibility,
console power effects, and physical controllers remain unverified.
Retained captures use actual GPU targets or the separate QQuickWindow.
No stale desktop capture is included.
