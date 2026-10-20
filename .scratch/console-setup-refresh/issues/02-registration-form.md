# 02: Console registration form with layout A

**What to build:** A native guided registration form that preserves every
supported console choice, identity route, and validation rule. The user can
enter details, obtain an Account ID through existing routes, and submit a real
registration request. Retain the current registration result dialog until 03
replaces it, so this slice remains usable end to end.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Source:** [Console setup: guided workspace](../spec.md), selected layout A.
The user approved this ticket and its dependencies on 2026-10-20.

## Acceptance criteria

- [x] Existing registration entry points open the new native form. Settings
  retains its broadcast-address route and default console type. Discovery and
  connection routes retain the selected host, console type, and registration
  context. Automatic PSN registration remains outside this redesign.
- [x] The screen uses A's left form panel and right instruction column with the
  completed library and Settings style. Group console details, account identity,
  and PIN fields. Retain one navigation owner and a clear Back route. All new
  UI text uses the existing translation mechanism.
- [x] Preserve PS5 and all three PS4 firmware choices: before 7.0, from 7.0
  to before 8.0, and 8.0 or later. Dispatch the matching existing native target.
  Changing the choice updates the relevant identity field and console instructions.
- [x] Only the oldest PS4 choice uses PSN Online ID. Other choices use the
  Base64 PSN Account ID. Online ID help explains case sensitivity. Account ID
  help distinguishes it from the PSN username.
- [x] Initialize the host and console type from the caller. Prefill an available
  saved Account ID on first initialization. Returning from a linked screen
  must not overwrite a deliberate edit with a stale saved value.
- [x] PSN sign-in and public lookup use their existing availability rules,
  routes, and callbacks. A successful return fills the Account ID. An unsuccessful
  or canceled lookup leaves registration editable without starting registration.
  Retain the other entries and restore useful focus on return.
- [x] The host supports the existing IP-address, hostname, and local broadcast
  behavior. Trim the host and applicable identity on submission. Do not introduce
  IPv4-only validation or a network reachability probe.
- [x] Submission requires a nonempty trimmed host, the applicable identity,
  an eight-digit Remote Play PIN, and either no console PIN or four digits.
  Keep PIN values as text up to the native boundary, including leading zeroes.
  Preserve native Account ID validation.
- [x] Validation feedback appears near the relevant field. Invalid or incomplete
  input prevents dispatch. If the backend rejects a request synchronously, expose
  the available error and keep the form editable. Do not display progress for
  a request that did not start.
- [x] Console-specific help explains where to obtain the Remote Play code and
  distinguishes the temporary eight-digit pairing PIN from the optional four-digit
  console login PIN. Preserve how the backend uses both values.
- [x] A valid submission calls the existing native registration operation once
  with the correct arguments and shows its current log/result dialog. Keep that
  dialog's existing outcome handling usable until 03 replaces it. Opening the
  form, changing fields, or visiting account routes does not write settings.
- [x] Cancel, Back, and Escape before submission leave without saving and restore
  the valid opener or a fallback. Active child popups receive Escape first.
  Preserve the existing result-dialog route in this intermediate slice.
- [x] Mouse, keyboard, and existing controller-key navigation can reach every
  field, account action, and submission action. Focus is visible and scrolls
  into view. Directional navigation must not consume text-editing keys.
- [x] The form and help work at 900 by 600 and 1280 by 720 logical pixels.
  Long labels and help wrap, and all actions remain reachable through vertical
  scrolling when needed. Fields and actions have accessible names.
- [x] Streamer mode masks private host, identity, and PIN values. No new help
  text or summary exposes those values. Preserve existing privacy behavior in
  the retained result dialog; 03 owns its replacement and diagnostic privacy.
- [x] Production-QML tests in the existing Qt Quick runner cover all four targets,
  route defaults, host and broadcast arguments, stored Account ID, Online ID
  visibility, both account routes, retained input, whitespace, PIN lengths,
  leading zeroes, optional PINs, synchronous rejection, focus, and no-write
  navigation. Exercise controls rather than private helpers.
- [x] Use the Settings-style backend and navigation injection pattern with real
  production defaults. Test fixtures remain outside production resources. Any
  changed native argument or validation behavior has a focused contract check
  at the real boundary with isolated data.

## Completion evidence

Demonstrate registration submission for PS5 and each PS4 choice using controlled
backend responses. Demonstrate both Account ID return routes, including a canceled
return, without losing the other fields. Record the affected screen-test results.
The existing result dialog must still work; this ticket must not leave submission
as a dead end while waiting for 03.

## Scope and preservation

Ticket 03 owns the new progress and result panels, retry behavior, and request
lifetime safeguards. Do not redesign authentication pages or introduce a new
registration protocol. Ticket 01 is independent; reuse shared code only when
it exists and fits, rather than making this ticket depend on its implementation.

Use the selected prototype on `prototype/console-setup-guided-workspace` at
`d3b02f91db69c6b6dca2c89a69036da5edc74b4d` as the visual reference. Preserve
the existing dirty checkout and completed library and Settings work. Real PSN
authentication and physical console pairing are separate hardware checks.


## Answer

Implemented the native guided registration workspace in `RegistDialog.qml`.
The form has a bounded left panel, a separate instruction column, console,
account and PIN groups, and the library's `ConsoleButton` actions. The screen
keeps the existing native routes and log dialog. It uses the existing backend
validation and registration arguments. No C++ behavior or resource list changed.

Account lookup now keeps entered values after a return. A canceled return does
not replace a deliberate Account ID edit with a saved value. Both account routes
restore focus to the Account ID. The form shows field help and synchronous
native rejection text, and scrolls focused controls into view.

### Evidence

- Used the approved production-QML screen boundary with backend and navigation
  injection. The fixture uses a real `StackView` for account-screen returns.
- The first test failed because the old screen had no injected account route.
  Further failing checks found the console selector's keyboard activation and
  missing identity feedback. The production fixes passed the same checks.
- Built the standalone Qt Quick runner in `build/registration-screen` with
  Qt 6.9.3 and MSVC x64. Ran `tst_RegistrationForm.qml`: 22 passed, 0 failed.
  Output is in `build/registration-screen/final.txt` in this worktree.
- Controlled backend tests cover PS5 and all three PS4 targets, broadcast and
  hostname input, trimmed host and identity, leading-zero PINs, optional PINs,
  invalid input, synchronous rejection, both account routes, canceled return,
  retained entries, focus, Escape, privacy, no-submit navigation and log closure.
- Rendered the production QML at 900 by 600 and 1280 by 720. Inspected both
  captures and checked focus visibility for every control at both sizes.
  Temporary captures are outside the repository. Permanent tests contain no
  machine-specific capture path.
- Checked the unchanged entry points in `Main.qml` and Settings. Caller host and
  PS5 inputs still enter the same production screen. Test fixtures remain outside
  production resources.

Ticket 03 owns progress/result replacement and request lifetime safeguards.
Ticket 04 owns the integrated native build, full suite, native route checks,
display scaling and long-translation acceptance. Real PSN authentication,
physical controller input and console pairing were not tested here.
