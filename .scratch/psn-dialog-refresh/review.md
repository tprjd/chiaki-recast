# PSN and dialogs final review

Date: 2026-10-21
Base: `9b138cc43a5d667933284f1c24e8dabb721b01ad`
Reviewed integration: `29b421e2f9e5489c1aab989324f0c7eb8661919f`
Command: `git diff 9b138cc43a5d667933284f1c24e8dabb721b01ad...HEAD`

Two independent reviewers checked the final source and connected callers.
The standards reviewer used Sol High. The specification reviewer used Astra High
because the change includes authentication, profile context, and late results.

## Standards

0 documented-standard breaches; 1 heuristic finding.

- P3, possible Duplicated Code. `SteamShortcutDialog.qml:53` repeats the credential
  checks, preference update, and PSN prompt from `RemindDialog.qml:30`. The changed
  code includes `if (!settings.psnRefreshToken || !settings.psnAuthToken ...)` and
  `navigate.showRemindDialog(qsTr("PSN sign-in"), ...)`. Call the existing
  `navigate.showPsnReminder()` after closing the Steam screen. This keeps reminder
  eligibility and text in one place. This is a heuristic, not a hard repository rule.

The reviewer read the commit list, production changes, callers, screen tests,
native Steam test, and repository instructions. The glossary and local issue
layout follow the documented rules. No ADR directory or additional coding
standard exists. The reviewer did not change files or run tests.

## Spec

1 finding.

- P2, make long action-dialog text reachable by keyboard and controller.
  `ConfirmDialog.qml:76` sets its scroll area to `Qt.NoFocus` when a primary
  action exists. The buttons cycle focus between themselves. Long reminder,
  confirmation, and registration instructions can overflow without a keyboard
  route to read them. The specification requires: "Every field, explanatory
  message, and action remains reachable with long text and larger display scales."
  Allow focus or scrolling input to reach the content. Keep Cancel initially selected.

An isolated check loaded production `ConfirmDialog` at 900 by 600, verified
overflow, and sent Tab, Up, and Page Down inputs. The content stayed at the top.
The retained failing test is in
`C:/Users/david/AppData/Local/Temp/psn-spec-review-scroll/`.

The reviewer found no other actionable missing requirements, unrequested behavior,
or incorrect implementations. The review did not repeat full acceptance or test
real services, hardware, QtWebEngine, or final desktop composition.

Standards: 1 heuristic finding, worst P3 duplication. Spec: 1 finding, worst P2 keyboard access.

## Resolution

Both findings are resolved in `d12fe646`, merged into integration at `0c6aebd5`.
One implementer made both changes with failing and passing production-screen tests.

The standards reviewer confirmed that the shared reminder helper preserves
credential eligibility, preference updates, and the completion route.
The specification reviewer independently reran the original scrolling reproduction
and the complete Confirmations suite. All 3 reproduction checks and 40 suite checks passed.

The [final verification record](evidence/review-fixes/README.md) contains 142 passing
affected screen checks, with no failures or skips. Native checks pass with 297
integrated checks, 337 Vulkan checks, and 357 OpenGL checks. A separate test timing
failure was traced to overlapping buttons before rendering completed. The test
now waits for rendering. The full Steam suite then passed three times.

The normal build was refreshed and the native cache restored. The parent checked
both binary hashes, the native result records, the long-text capture, and the
integrated source against the 332 tested inputs. The only checkout byte differences
are line endings; there are no content differences.

Final open findings: Standards 0; Spec 0. The recorded external-service, hardware,
QtWebEngine, and desktop-composition limits remain in the acceptance record.
