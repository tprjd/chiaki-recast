# 03: Registration progress, results, and safe retries

**What to build:** Complete the guided registration workspace with truthful
progress, success, and failure panels. The user can inspect diagnostics, correct
details, retry, or dismiss progress. Each attempt retains its intended console
and profile, and an old callback cannot change a newer screen or save into the
wrong profile.

**Blocked by:** 02: Console registration form with layout A.

**Status:** resolved

**Source:** [Console setup: guided workspace](../spec.md), selected layout A.
The user approved this ticket and its dependencies on 2026-10-20.

## Acceptance criteria

- [x] Replace the retained log dialog with form, progress, success, and failure
  content in A's left panel. Keep task instructions and context visible. Preserve
  the registration screen's identity and the form behavior delivered by 02.
- [x] Enter progress only for an accepted request. Show an indeterminate indicator
  and useful status text. Do not show the prototype's fixed percentage or sample
  milestones as real progress. Repeated activation while active sends no second
  request, including through keyboard and controller shortcuts.
- [x] Nonterminal native callbacks append diagnostic messages. Terminal callbacks
  select success or failure from the actual outcome. Do not infer outcomes by
  parsing translated logs. A synchronous rejection leaves the form usable and
  exposes its error without entering a permanent progress state.
- [x] Success is reported only after the registered-console update and any
  manual-host association are complete under the current persistence contract.
  The result identifies the relevant console and provides Back to consoles.
  The library shows the updated console without starting a stream.
- [x] Failure uses a neutral message and real available details. Do not claim
  that a PIN expired or was rejected unless the backend reports that cause.
  Provide Back to details and the appropriate dismissal route.
- [x] An expandable diagnostic section supports long lines, scrolling, keyboard
  focus, and inspection during progress and after an outcome. Keep normal result
  text readable without requiring the log to be expanded.
- [x] Back to details retains the entered address, console type, applicable
  identity, Remote Play PIN, and optional console PIN. A new attempt starts a
  fresh log. Retrying after failure preserves the intended original console
  and manual-host association rather than creating an unrelated manual host.
- [x] Close progress dismisses through the existing caller route and explains
  that registration can continue. Escape and Back during progress use the same
  semantics. Do not call this cancellation or report a canceled outcome when
  no native registration-cancel command exists.
- [x] Dismissal prevents later UI callbacks from accessing destroyed controls,
  navigating, or modifying a newer screen. An old terminal callback cannot
  overwrite a newer attempt's state. Duplicate terminal notifications cannot
  repeat an action or replace a completed result.
- [x] Native completion uses the initiating request's console context rather
  than the current mutable global target. Prevent overlapping manual registration
  attempts from sharing that mutable context, including after one progress
  screen is dismissed. A narrow request guard is sufficient; no job framework
  or new cancellation protocol is required.
- [x] A stale form cannot submit into another profile. An in-flight request
  cannot write its result into a different active profile after dismissal.
  Preserve its initiating profile identity. If existing persistence cannot
  safely write that profile when inactive, reject that settings update and
  report the condition rather than silently changing the active profile.
- [x] Treat profile changes and attempt replacement as invalidation events,
  including a change away and back. A failed or completed request releases any
  request guard so a valid later attempt can proceed. Preserve the existing
  native operation lifetime without adding automatic registration retries.
- [x] Result actions do not resubmit registration. Returning to Settings, the
  library, or form details restores a valid focus target. Child popups handle
  Escape first. All new state labels and actions use the translation mechanism.
- [x] Every panel and expanded log works at 900 by 600 and 1280 by 720 logical
  pixels. Long text wraps or scrolls without hiding focused controls. Mouse,
  Tab, Shift+Tab, and existing controller-key conventions reach all actions.
  Accessible names, visible focus, and text descriptions identify state changes.
- [x] Streamer mode masks private addresses, account identifiers, and console
  details consistently with the library. Result summaries never echo PINs.
  Hide or sanitize diagnostics in streamer mode so expanding logs does not
  bypass privacy protection, including when the setting changes during a run.
- [x] Production-QML screen tests drive controlled nonterminal and terminal
  callbacks through the injected backend. Cover success, generic failure,
  immediate rejection, log expansion, retry, retained input, duplicate activation,
  dismissal, screen destruction, reopening, old callbacks, and profile changes.
  Assert displayed results, focus, and command counts through screen behavior.
- [x] Focused native checks use temporary settings and controlled completion
  events to verify context capture, profile ownership, update-before-success
  ordering, guard release, and absence of unintended writes on rejection.
  Include failure followed by retry for an existing manual host. QML fakes alone
  do not prove these C++ and persistence contracts.
- [x] The registration form tests from 02 continue to pass. The final production
  resources contain no prototype switcher, sample values, fake state controls,
  or test fixtures. Remove the obsolete result-dialog code once its behavior
  is covered by the replacement.

## Completion evidence

Demonstrate a controlled success, a failure followed by correction and retry,
and dismissal followed by late completion. Demonstrate a profile switch and
an overlapping request without a wrong-target or wrong-profile save. Record
screen tests and the native contract evidence. These checks belong to this
ticket and must pass before integration acceptance in 04.

## Scope and preservation

Keep native changes limited to correct result reporting and request ownership.
Retain registration inputs, supported targets, storage formats, and the existing
account routes. Do not redesign automatic PSN registration, add cancellation,
or invent diagnostic capabilities that the backend does not provide.

Use layout A from `prototype/console-setup-guided-workspace` at
`d3b02f91db69c6b6dca2c89a69036da5edc74b4d`. Its sample error explanations
and progress milestones are not native behavior. Preserve unrelated local
changes. Real console pairing is not required for this ticket's acceptance.

## Answer

The guided panel now shows form, indeterminate progress, success, and failure.
Diagnostics expand inside the panel and are hidden in streamer mode. Failure
retains the form values. Each retry starts a new log. Close progress dismisses
the screen and states that registration can continue.

The form receives its console context when the route opens. Settings receives
an empty context. Native requests capture that context and the profile
generation. A profile change invalidates an old form and its pending result,
including a change away and back. One native guard prevents overlap after
dismissal. Terminal flags reject duplicate outcomes and late logs. The UI
callback scope prevents access after dismissal, destruction, or a new attempt.

`QmlRegist::start` now reports a rejected native start. Native success follows
the registered-console update and the manual-host association. Retry retains
the original manual ID and address. Completion also checks that the manual
target still exists. A profile or target change during update signals cannot
write an association into a different profile or console. The existing Settings
interface has no transaction or durable-write result. If a signal changes the
profile after the registration update, the callback reports the partial update
and asks the user to check the original profile.

Validation:

- [Form screen tests](../evidence/ticket03-form-screen-tests.txt): 22 passed.
- [Result screen tests](../evidence/ticket03-result-screen-tests.txt): 19 passed.
  These load production QML and house controls at both required sizes. They
  cover results, diagnostics, privacy, retry, retained values and context,
  duplicate activation, dismissal, destruction, old callbacks, profile changes,
  keyboard input, and long action labels inside the scroll viewport.
- [First failing test](../evidence/ticket03-first-red.txt) preceded the result
  implementation. The [layout failure](../evidence/ticket03-layout-red.txt)
  exposed a long-action focus problem at 900 by 600. Content-size changes now
  trigger focus visibility checks, and the final result suite passes.
- [Native contracts](../evidence/ticket03-native-contracts.json): all passed
  in the actual application with isolated INI settings, disabled discovery,
  controlled start results, and native `QmlRegist` signals. Checks cover
  update-before-callback, manual failure then retry, independent Settings and
  discovered routes, overlap, duplicate events, stale forms, both profile
  changes, removed targets, changes during persistence signals, start rejection,
  and a new request started inside the terminal callback.
- [Saved INI arrays](../evidence/ticket03-native-persisted-arrays.json): all
  passed. Fresh Settings reads checked saved records in-process. A separate
  Python check read the actual INI files after the application exited.
- [Normal application build](../evidence/ticket03-production-build.txt) passed.
  The [source manifest](../evidence/ticket03-production-build.json) confirms
  that the staged source matches production and all QA hooks were removed.
  Production resources contain no test fixtures or prototype controls.
- Ten offscreen form, progress, result, and log captures were generated and
  inspected at 900 by 600 and 1280 by 720. They remain under this worktree's
  `build/gui-screen/registration-*.png` for the integration review.

Reusable native QA files remain outside production under
`C:/Users/david/.cache/chiaki-ui-native/`: `native-qa-console-contract.cpp`,
`qa-console-contract.patch`, and `check-console-contract-files.py`.
The final isolated run is `console-contract-run03`. Apply the patch only in the
staged source tree and use a new `CHIAKI_NATIVE_QA_ROOT` for each run. The helper
bypasses console transport and uses controlled native completion signals. It
does not test real pairing or the library's transport callback conversion.

The complete screen run was stopped before completion. Ticket 04 owns that
integrated run, native captures and DPR checks, and the normal visible-window
launch. Real PSN authentication, console pairing, and physical controller input
remain unverified. Qt WebEngineQuick is absent from the cached native toolchain.
