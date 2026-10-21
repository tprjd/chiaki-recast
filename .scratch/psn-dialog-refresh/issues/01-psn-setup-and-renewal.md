# 01: PSN setup and credential renewal

Status: resolved
Type: task
Blocked by: None (can start immediately)

**What to build:** Deliver PSN remote-access setup and expired-credential renewal through the current embedded and external browser routes, including input, actual operation progress, results, retry, and screen closure.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [x] Use the new design and existing production controls. Verify reachable actions at 900 by 600 and 1280 by 720 logical pixels, long text, keyboard/controller navigation, visible focus, return focus, and applicable privacy rules. Keep test fixtures and synthetic data out of production resources.
- [x] Use the new design, preserve platform browser defaults, retain external-browser fallback and browser recovery actions, and preserve Settings, reminder, and expiry entry routes.
- [x] Explain the complete Sign-in redirect URL and saved credentials. Provide explicit copy/paste actions, masked input, reveal controls, and streamer-mode protection.
- [x] Reject invalid redirect input at the native boundary and provide a route to fresh sign-in for expired or used codes.
- [x] Show progress, terminal success, or persistent failure with retry and collapsed sanitized details. Correct expiry routing and embedded/external reminder-completion parity.
- [x] Before submission, Cancel exits. After submission, Close explains that sign-in can finish and save credentials. Prevent duplicate requests across closure and re-entry.
- [x] Associate requests with their original profile and presentation. Late results cannot save into another profile or change a replacement screen. Preserve compatibility for existing registration callers while the next ticket refreshes that route.
- [x] Verify production QML behavior and actual native persistence with isolated settings and controlled responses. Cover failure, replacement, duplicate actions, late results, and sensitive-value exclusion.

## Coverage and dependencies

Primary user stories: 9, 14-35, 37. Shared user stories: 1-7, 63.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.


## Answer

Implemented in `19c81712` on `codex/psn-setup`.

The production PSN task now uses the refreshed controls, actual request progress,
a persistent result, retry, and a fresh sign-in route. The external field starts
masked. Copy, Paste, Show, and streamer-mode protection are explicit. Browser
selection keeps the platform defaults, reload, cookie clearing, and fallback.
The expiry event now supplies the correct argument. Both browser routes save
the setup reminder state only after a valid final response.

`psnAuthAttempt` owns the native request across screen closure. Native validation
requires the exact redirect destination and one nonempty authorization code.
The request saves credentials only after valid token and Account ID responses.
Profile changes invalidate pending work and defer deletion until callbacks return.
Results cannot save in a replacement profile or update a retired presentation.
The existing registration sign-in APIs remain available for ticket 02.

Verification:

- The independent screen build passed 721 tests, with no failures or skips.
  The final focused PSN screen run passed 10 tests. The focused run also passed after the integration merge.
- Controlled native run 07 passed 125 checks. It reads actual saved credentials
  through fresh native Settings objects. It covers invalid input, malformed
  responses, duplicate requests and results, profile changes from progress and
  completion observers, and the existing registration APIs.
- Visible native checks passed at 900 by 600 and 1280 by 720 logical pixels,
  with measured DPR 1.25. Native keyboard input covers visible action focus,
  explicit paste and reveal, immediate privacy masking, expiry routing, closure
  during work, re-entry without a second request, and final success.
- The normal application build passed. Its 83 production source files match
  source commit `19c81712`. Its resources and executable exclude test fixtures and QA hooks. This build precedes the other integrated tickets.
- Native output contains no synthetic authorization code or token values.
  The shared cache has all 49 original source hashes and its original output
  configuration restored. The original app and screen targets rebuilt.

Evidence is in [ticket01-native-contracts.json](../evidence/ticket01-native-contracts.json),
[ticket01-screen-full.txt](../evidence/ticket01-screen-full.txt),
[ticket01-normal-build.json](../evidence/ticket01-normal-build.json), and
[ticket01-cache-restoration.json](../evidence/ticket01-cache-restoration.json).
The capture files use the `ticket01-form-*` and `ticket01-success-*` names.
The native assertion source and cache preparation helper are retained beside them.

Real PSN sign-in and physical controller input were not used. The native Qt
build lacks QtWebEngine. Its real browser check covers fallback; the embedded
route uses a supplied browser in production-screen tests. The integrated native
acceptance ticket remains responsible for the final cross-dialog focus and
scaling matrix.

Merged integration tip `3f998dd3` into this branch as `baadbcc3`. Both README test sections are retained. The final integrated application build remains part of ticket 08.
