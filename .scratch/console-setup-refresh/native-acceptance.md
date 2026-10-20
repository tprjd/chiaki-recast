# Console setup native acceptance

Status: passed
Date: 2026-10-20
Tested production revision: `a19f26f1b24844cbc2b050957e9b511b66a3093a`
Integration base: `c2e2acef66cec524613a1f56be5d282e27084606`

Both native screens retain layout A, separate routes, and the existing controls.
The acceptance correction aligns the registration header, margins, and button
padding with manual entry. No behavior or storage format changed in ticket 04.
Source review found no lost console targets or account routes. The original
checkout's 18 recorded file hashes remain unchanged. The parent agent's final
The [final Standards and Spec reviews](review.md) are complete with no open findings. The [cleanup verification](review-cleanup-verification.md) records the later shared-button extraction and final build.

## Screen tests

- [Final complete suite](evidence/ticket04-final-full-suite.txt): 627 passed,
  0 failed, in 418.218 seconds. The runner loads production QML and controls.
- [Initial integrated suite](evidence/ticket04-initial-full-suite.txt): passed
  in 422.50 seconds, before the header correction.
- Focused checks after the correction: [22 form checks](evidence/ticket04-registration-form.txt)
  and [19 result checks](evidence/ticket04-registration-results.txt) passed.

The MSVC Qt 6.9.3 runner used the documented offscreen/software configuration
and `QT_QPA_FONTDIR=C:\Windows\Fonts`. These checks cover native command
arguments, all targets, stored Account ID, validation, leading zeroes, return
routes, stable selection, reordering, ambiguity, privacy, and callback lifetime.

## Native contracts and persistence

The [native contract run](evidence/ticket04-native-contracts.json) passed in the
actual application. The [post-exit INI check](evidence/ticket04-native-contract-persisted-arrays.json)
also passed. These checks use real Settings arrays and production completion
handlers. Only transport start and completion events are controlled.

The contracts cover update-before-success, manual failure and retry, captured
target ownership, independent Settings and discovered routes, overlap rejection,
duplicate terminal events, late logs, stale forms, profile changes away and
back, removed targets, changes during update signals, native start rejection,
and starting another request inside a terminal callback. They create no session.

## Native UI checks

The [capture manifest](native-acceptance-manifest.json) lists 111 framebuffer
images, source hashes, dimensions, measured scale, and focus bounds. Each scale
run passed 37 captures and 30 control checks against both the window and clipped
scroll viewports. Images came from the production OpenGL framebuffer, with the
native app visible and exposed. They are separate from the offscreen tests.

| Measured DPR | 900 by 600 framebuffer | 1280 by 720 framebuffer | Evidence |
| --- | --- | --- | --- |
| 1 | 900 by 600 | 1280 by 720 | [UI checks](native-previews/console-ui-final-dpr1/checks.json), [saved arrays](native-previews/console-ui-final-dpr1/persisted-arrays.json) |
| 1.5 | 1350 by 900 | 1920 by 1080 | [UI checks](native-previews/console-ui-final-dpr1_5/checks.json), [saved arrays](native-previews/console-ui-final-dpr1_5/persisted-arrays.json) |
| 2 | 1800 by 1200 | 2560 by 1440 | [UI checks](native-previews/console-ui-final-dpr2/checks.json), [saved arrays](native-previews/console-ui-final-dpr2/persisted-arrays.json) |

Process scale factors produced these values. Global display settings did not
change. The DPR 1.5 run expands translated labels and help by at least 50 percent.
All runs use equal console names near the native nickname limit, with MAC
identity visible when privacy permits. Expanded labels did not truncate.
Vertical scrolling kept focused controls fully visible.

All captures were inspected through contact sheets, with full-size form,
action, progress, log, result, and privacy images inspected separately. Useful
examples are the [900-pixel form](native-previews/console-ui-final-dpr1/900x600-registration-form.png),
[expanded actions](native-previews/console-ui-final-dpr1_5/900x600-registration-form-actions.png),
[manual saved result](native-previews/console-ui-final-dpr1_5/1280x720-manual-saved-deferred.png),
and [success at DPR 2](native-previews/console-ui-final-dpr2/1280x720-registration-success.png).

Native checks passed for these actions:

- Library to manual entry, existing registration and deferred saves, duplicate
  activation, saved results, return to the updated library, selection removal,
  correction with the address retained, an empty list, and profile invalidation.
- Settings to registration with broadcast and PS5 defaults, all four console
  choices, and the Online ID and Account ID modes. The controlled unregistered
  route retained its address and target context.
- Both existing account screens, controlled success and cancellation, retained
  address and PINs, Account ID focus, and return to the Settings opener.
- Real native progress, expandable long diagnostics, failure, retained details,
  retry, ignored old callbacks, success consistent with saved records, dismissal,
  reopening, overlap rejection, and profile-invalid completion. Close progress
  remained a dismissal rather than cancellation.
- Mouse actions, Tab, Shift+Tab, Up and Down mappings, Return activation, popup
  Escape handling, and field editing keys. Input Escape ended editing before
  the next Escape closed the screen. Physical controller input was not tested.
- Streamer mode on forms, selectors, help, summaries, progress, and both results.
  Expanded diagnostics remained hidden. Browsing, cancellation, and account
  returns left the console records unchanged.

The UI matrix supplies controlled field values and selector choices, then
uses real controls and mouse actions. Separate native key checks exercise input
dispatch. Registration completes through real `QmlRegist` signals and production
handlers. Fresh Settings reads and post-exit INI reads verify the results.
The native map cannot contain duplicate MACs. Screen tests verify ambiguity and
reordering; native UI checks verify removal, recovery, and profile invalidation.

Every QA run used a new INI directory. The three primary Settings constructors
were isolated before startup. Discovery stayed disabled after profile changes,
automatic connection was stopped, tokens were absent, and no session existed.
QA did not call real authentication, public lookup, or console pairing.

The raw UI helper's revision label retains the initial base `c2e2acef`. Its final
build already included the header change in `a19f26f1`. The manifest records the
exact tested QML hashes and this distinction. Raw evidence remains unchanged.

## Final normal application

The [source manifest](evidence/ticket04-production-source-manifest.json) confirms
that all staged GUI files match production and QA hooks were removed. Normal
resources contain no prototype controls, fixtures, fake backends, or sample
records. The normal build has no QA helper object.
The [normal build](evidence/ticket04-production-build.txt) passed.

The executable is `C:/Users/david/.cache/chiaki-ui-native/console-setup-bin/chiaki.exe`.
`Launch-chiaki.cmd` beside it supplies the existing runtime paths. Its SHA256 is
`4b3187ff78789fd3cab16169f587ba55c557198726a2f3a115c3d0ef2d7cacfb`.

The root agent independently used Computer Use to verify normal window
`5704498`, titled `chiaki-ng:PS5-HDMI`. The library appeared. Add console opened
the completed manual-entry screen with an empty, focused address field and the
expected selector, instructions, and actions. No values or requests were
submitted. The window remains open for the user. This normal startup uses the
existing settings and discovery. It is distinct from isolated QA. No screenshot
of the user's console details was saved.

## Limits

Real PSN authentication, real console pairing, transport callback conversion,
and physical controller input remain unverified. Qt WebEngineQuick is absent
from the cached native toolchain; controlled account returns do not prove the
embedded authentication flow. Settings exposes no durable disk-write result or
transaction guarantee. Those limits are unchanged by this UI update.

Reusable QA helpers, patches, and launch scripts remain outside production in
the native build cache. They are not linked into the normal executable.
