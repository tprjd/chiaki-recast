# 05: Manage consoles and manual addresses from the web library

**What to build:** Users can inspect and manage consoles, save manual addresses, associate existing registrations, update a saved console login PIN, and connect a registered manual host through native playback.

**Blocked by:** 04: Connect locally from web setup to native playback.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 10, 13, 14, 15, 23, 35, 36, 37, 38, 39, 69, 70.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] Details and action menus show the intended console type, status, registration state, and permitted actions using the selected design.
- [x] Hide, unhide, manual entry deletion, and registration deletion retain their existing distinctions and confirmation behavior.
- [x] Each management action resolves the current native target. Reordered discovery, removed consoles, ambiguous targets, and stale profiles cannot affect a different console.
- [x] Saved console login PIN changes preserve leading zeroes and native validation. Saved PINs and registration keys are not returned to the page.
- [x] Streamer mode masks identifiers and addresses throughout cards, details, confirmations, and errors.
- [x] Library selection and focus return correctly after menus, dialogs, deletion, and live updates.
- [x] Manual host entry retains address validation, existing registration choices, and the separate explanation of address saving and console registration.
- [x] Saving reports the native result and refreshes the library. It does not simulate a network operation.
- [x] Unregistered manual entries can be saved and are clearly marked as needing registration. A registered entry can start the native connection path.
- [x] Editing or removing a target while a form is open cannot save changes to an unrelated host.
- [x] Errors retain useful non-secret form input, and keyboard or controller focus returns to the appropriate action.
- [x] Embedded tests verify native management state changes, cancelled confirmations, stale targets, privacy masking, saved manual hosts after reload, and connection of a registered manual entry through native handoff.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Answer

Implemented console details, guarded management actions, manual addresses,
existing registration association, saved console login PIN edits, and the
registered manual connection route. Native services retain settings and
connection state. Every management command resolves a current opaque key.
Manual identities include the native manual entry identity and its current
address. Profile changes, reordered discovery, removed or ambiguous consoles,
and edited manual targets cannot redirect an old action.

The page receives no saved PIN or registration key. Streamer mode masks
addresses and identifiers in the library, details, choices, confirmations,
and errors. Address aliases use a comparison without letter case. Form and
result focus, cancellation, deletion distinctions, native saves, reloads,
registration association, and leading zeroes pass embedded checks.
The manual connection case passes native window handoff, setup hide/freeze,
cancellation cleanup, and setup restoration.

Source commits are `5bf0b24d`, `8d4725db`, and `347b1488`. Final merge
`823cf551` includes integration through `24f57a8c`, with renderer/media fields,
ProcessLauncher, and native keyboard/mouse capture lifecycle preserved.
The two merged CSS additions retain their separate dialog styles.

[Verification and source/artifact identities](../evidence/ticket05/verification.md)
record the focused red/green results, final build, source comparisons, and
reused ticket 03/04 evidence. The old manual-host and saved-PIN QML screens,
callbacks, and dedicated tests are removed. Native live PIN components remain.
Shared legacy resource chains remain for later reminder, Steam, and settings
work. Ticket 23 owns their final removal.

Closure covers this implementation and embedded application milestone.
Ticket 07 owns unregistered pairing and Account ID lookup. Loopback handoff
does not prove real console playback. Physical focus, controllers, screen
readers, touch, HDR, sustained media, input latency, platform packages, and
GPU/default-preset acceptance remain in tickets 19 through 23. No new full
native decoder or graphics matrix was run for these setup forms.

Integration merge `ef14d52fc2e8ca9638babbc0e3e2bb934b542111` contains worker
commit `c3bf624c1068820aad4c8f333cab8bad2354cd44` without conflicts.
The merged tree matches the worker tree. GUI source matches tested merge
`823cf551b72002b9f0b39cc7ae5dd0705e101ed4`.
The merger independently verified 19 source hashes, four artifact hashes,
51 evidence hashes, and all 27 preserved-source comparisons. All three
captures were inspected. [The integration audit](../evidence/ticket05-integration/audit.json)
records the comparison hashes. Its adjacent `verify.py` can repeat these checks.

The CSS selector review confirms that ticket 15 keeps its reset dialog
declarations. The selector uses the existing `reset-keyboard-title` label.
Ticket 05 dialogs keep their tested styles. Native lifetime, PSN account,
media, renderer restart, and input capture code retain their integrated behavior.
The vendor patch remains excluded. Source whitespace checks pass.
Valid focused results were reused, including saved-PIN result focus 3/0,
privacy 3/0, and loopback native handoff 3/0. No build or test was repeated.
Live-console, physical-focus, GPU, and platform acceptance remain open.
