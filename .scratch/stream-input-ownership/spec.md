# Stream input ownership implementation specification

Status: claimed

Decision: [ADR 0001: Give stream input an owner lifecycle](../../docs/adr/0001-stream-input-ownership.md)

Vocabulary: [Project glossary](../../GLOSSARY.md)

## Problem Statement

During Remote Play, users move between console controls and local interactions.
These interactions include the stream menu, notices, settings, connection progress,
disconnect confirmation, and the Console login PIN. Each interaction must receive
the correct input and return focus at the correct time.

The current implementation spreads these rules across QML callers and native code.
Callers maintain capture flags, saved focus, and release order. Some callers also
know when native session changes reset input capture. This makes changes difficult
to verify across all interactions.

This is architecture work. No known input or focus defect prompted the change.
The refactor must preserve established user-visible behavior. Duplicate opens and
obsolete closes must be harmless by contract.

## Solution

Use one native owner lifecycle module for all current stream input callers.
The module owns input blocking, saved focus, cleanup, and session handoff rules.
Each caller reports when its stream input presentation opens, releases input, and
finishes its close.

Users retain the current controls, focus behavior, and window presentations.
Maintainers gain one ownership contract that they can test against production
native code. Keep the existing QML test scenarios and add a maintained native
test target.

## User Stories

1. As a Remote Play user, I want the stream menu to receive local input when it opens, so that menu actions do not also control the console.
2. As a Remote Play user, I want menu flyouts to share the menu's input ownership, so that navigation between flyouts does not change input capture.
3. As a Remote Play user, I want console input to resume at the established menu close point, so that the close animation does not delay controls.
4. As a Remote Play user, I want focus to return after the correct close event, so that a closing native window does not take focus back.
5. As a Remote Play user, I want menu reopen restrictions to remain in effect during close and pending navigation, so that transitions behave as before.
6. As a Remote Play user, I want actionable notices to receive input when eligible, so that I can use their controls.
7. As a Remote Play user, I want actionable notices to yield to other interactions, so that a notice does not take their focus.
8. As a Remote Play user, I want a temporarily suppressed notice to retain its return focus, so that closing it later returns me to the correct control.
9. As a Remote Play user, I want statistics and passive notices to remain input-transparent, so that they do not interrupt console controls.
10. As a Remote Play user, I want managed connection progress and errors to retain their existing input behavior, so that connection transitions remain usable.
11. As a Remote Play user, I want legacy connection and error screens to retain their timing and navigation, so that the refactor does not alter those flows.
12. As a Remote Play user, I want PSN progress to retain input through its retirement transition, so that the retiring screen does not release input too early.
13. As a Remote Play user, I want disconnect confirmation to capture input before it opens, so that the opening action cannot reach the console.
14. As a Remote Play user, I want Cancel to close disconnect confirmation and return focus, so that I can continue the current session.
15. As a Remote Play user, I want both disconnect actions to retain their current meaning, so that I can choose whether the console enters sleep mode.
16. As a Remote Play user, I want Display settings to work in both supported presentations, so that input and focus remain correct with either renderer.
17. As a Remote Play user, I want Renderer settings to keep the current focus behavior, so that I can continue editing the selected control.
18. As a Remote Play user, I want Color Mapping settings to return to Renderer settings on normal close, so that I can continue the settings flow.
19. As a Remote Play user, I want navigation to the main view to close stream settings directly, so that Color Mapping does not reopen Renderer settings during exit.
20. As a Remote Play user, I want the Console login PIN to block console controls while active, so that PIN input goes to the local form.
21. As a Remote Play user, I want a replacement Console login PIN request to keep input capture, so that replacing the form does not interrupt the interaction.
22. As a Remote Play user, I want an old close callback to leave a reopened screen alone, so that the new screen retains its input and focus.
23. As a Remote Play user, I want overlapping interactions to release only their own input ownership, so that closing one does not unblock another.
24. As a Remote Play user, I want removal of a screen to clean up its ownership, so that the removed screen cannot retain input or restore focus later.
25. As a Remote Play user, I want session replacement and PSN handoff to preserve the current input rules, so that connection changes do not alter local control behavior.
26. As a Remote Play user, I want held buttons to retain the existing release behavior, so that closing a local interaction does not change how controls resume.
27. As a Remote Play user, I want cursor visibility and native keyboard routing to remain correct, so that local windows remain usable.
28. As a maintainer, I want duplicate opens and closes to have no additional effect, so that callers do not need paired counter bookkeeping.
29. As a maintainer, I want the ownership module to hold focus and session rules, so that callers do not duplicate those rules.
30. As a maintainer, I want native tests to compile production sources, so that the tests detect failures in the implementation that ships.
31. As a maintainer, I want the existing QML scenarios to remain available, so that layout, navigation, and screen behavior retain their coverage.
32. As a maintainer, I want repeatable native test commands and clear environment requirements, so that native evidence does not depend on temporary source edits.

## Implementation Decisions

- Implement ADR 0001 through one native owner lifecycle module associated with
  the stream window. Migrate every current caller of the shared input counter,
  including indirect callers through QML wrappers and signals.
- Use the three lifecycle operations `present`, `dismiss`, and `settled`.
  `present` accepts an owner, its role, and a focus target. It returns an opaque
  stream input presentation identity. `dismiss` and `settled` accept that identity.
- Use roles to express the existing ownership and focus rules. Do not introduce
  a universal priority order or a configurable dialog framework.
- A repeated `present` for the same active owner returns its existing identity.
  It does not add a block, replace saved return focus, or repeat an opening focus
  side effect. An open after dismissal creates a new identity, even for the same
  QObject.
- `dismiss` releases that presentation's input at the caller's established release
  point. `settled` reports completion of its close. Keep input release separate
  from any focus return that must wait for visual close completion.
- Repeated or obsolete `dismiss` and `settled` calls have no additional effect.
  They cannot release another presentation, restore obsolete focus, or alter a
  newer presentation of the same owner. Preserve this guarantee across teardown
  and session changes.
- Deferred callbacks retain the identity that was current when the work was
  scheduled. Close-completion adapters retain the identity from the start of that
  close. Reading the owner's current identity when an untagged completion arrives
  does not meet this contract.
- Keep real animation and popup completion events. Do not substitute a queued
  callback for completion of an animation. Guard any close-related adapter state
  changes as well as the call into the module against obsolete completion events.
- Owner destruction retires only that owner's records and cancels its pending
  focus work. Use lifetime-safe references for owners and focus targets. Closing
  owners in a different order from opening them must remain safe.
- Move shared saved-focus records, return-focus eligibility, suppression rules,
  and deferred focus cancellation into the module. Preserve each flow's current
  visibility and enabled checks, focus reasons, fallback, and activation timing.
  Callers can still choose their initial content control and retain ordinary
  within-screen navigation state.
- Keep the existing inline Quick and native-window adapters. Adapters perform
  window activation and report actual lifecycle events. They must not duplicate
  ownership policy or maintain independent input counters and return-focus stacks.
- Keep input capture distinct from presentation identity. A native capture reset
  does not replace the identity of a continuing presentation. Internal delayed
  work also checks whether its capture state is still current.
- Move the existing session preservation and reset conditions into the module.
  Preserve capture when a Console login PIN request is pending. Also preserve it
  when a new non-null session replaces an existing session, or starts on the PSN
  route. Otherwise apply the existing reset behavior. Apply the resulting block
  state to the new session.
- Centralize managed-connection rearm after session changes. Callers must no longer
  infer whether native capture was reset. Do not automatically rearm every visible
  owner after a reset. Keep the established differences between roles.
- Preserve the native window's event routing and cursor effects. This includes
  global shortcuts, console key routing, UI key delivery, native child-window
  selection, and rejection of input-transparent or nonfocusable windows.
- Preserve deferred native key delivery after actual window focus changes. Cancel
  pending keys on hide, destruction, and every session change, including changes
  that preserve input capture. Preserve existing mouse, wheel, and touch handling.
- Preserve StreamSession's blocked and release-wait behavior. Keep the current
  button-based release condition and feedback processing order, including d-pad
  touch handling. Do not extend the release condition to sticks, triggers, or
  touches as part of this refactor.
- Remove obsolete raw grab/release entry points and caller bookkeeping after all
  callers migrate. Keep any required capture details private to the implementation.
  A module that only forwards to the old counter while callers keep the policy
  does not satisfy this spec.

The migration must cover these callers and their adapters.

| Caller or flow | Required lifecycle behavior |
| --- | --- |
| Stream menu and flyouts | Acquire before the menu opens. Flyouts share its presentation. Release at close start while the menu can remain visible. Settle at actual close completion. Preserve reopen restrictions and pending settings or disconnect navigation. |
| Actionable notices | Preserve eligibility, suppression, and focus checks. Support yielding after another owner has acquired input. Retain return focus during temporary suppression without restoring it over the new owner. |
| Managed connection progress and errors | Cover all phases before readiness and the session handoff. Centralize reset and rearm rules. Preserve navigation when the session ends. |
| Legacy connection and error UI | Cover the error label's acquisition and teardown behavior. Preserve error timeout and navigation. Do not infer a new release point from visibility alone. |
| PSN progress | Acquire on creation and retain ownership until destruction, including StackView retirement. Preserve the handoff into the stream and any live PIN request. |
| Disconnect confirmation | Acquire before popup open. Retain capture through native-window hide and host activation. Release at the current popup-closed point. Cover cancel, both disconnect actions, window close, and destruction. |
| Display settings | Cover inline and separate-window loaders, opening focus, normal close, replacement by another settings view, and navigation to the main view. Preserve hide-before-release timing. |
| Renderer settings | Cover both window presentations, saved content focus, normal close, and transitions to and from Color Mapping. Preserve hide-before-release timing. |
| Color Mapping settings | Preserve release and focus return before opacity clears on normal close, then reopen Renderer settings. Main-view navigation closes it without reopening Renderer settings. |
| Live Console login PIN host | Keep ownership outside the replaceable content loaders. A nonempty request replacement keeps capture and the continuing presentation. Cancel old focus-return work when a new request arrives. Preserve close, destruction, and fallback focus behavior. |
| Statistics and passive notices | Retain nonfocusable, input-transparent behavior. Do not create stream input owners for them. |

- Keep backend Console login PIN request identities separate from stream input
  presentation identities. Preserve request validation and cancellation. Releasing
  ownership must not itself cancel a backend request.
- Preserve settings values, renderer selection, visual content, dimensions,
  animations, accessibility, and disconnect semantics. Limit changes to the
  ownership migration and the test support needed to verify it.

## Testing Decisions

- Use the agreed highest test seam: the production owner lifecycle interface,
  exercised with its real native effects. Test observable input, focus, cursor,
  and feedback behavior. Do not use private record counts as the main assertions.
- Add a maintained native executable and register it with CTest through the
  project's test build. Compile the production ownership module and the relevant
  QmlMainWindow, QmlBackend, and StreamSession integration. Reuse production build
  definitions where practical. Do not copy and rewrite production sources.
- Use narrow internal controlled adapters only where session creation, transport,
  or controller feedback requires control. Keep the real ownership, event routing,
  and block/release logic under test. Avoid a second ownership implementation in
  a fake and avoid public diagnostic APIs added only for tests.
- Keep the existing QML screen-test target, standalone configuration, and scenarios.
  Adapt fixtures to the new interface where needed. Retain assertions about screen
  behavior, navigation, layout, exclusions, teardown, and focus. Do not replace
  those suites with native tests or delete scenarios that expose migration failures.
- Use the existing StreamInterface, StreamNotices, and LiveConsolePin QML suites
  as prior art. The saved native acceptance work supplies cases for session
  replacement, settings handoff, and native windows. Move those cases into the
  maintained target without retaining its source-rewriting build method.

The acceptance matrix must include the following checks.

| Area | Required evidence |
| --- | --- |
| Basic ownership | One owner blocks console controls. Dismissal releases its block at the correct point. Repeated open does not add blocking or change saved return focus. |
| Overlap | Two owners can overlap. Releasing either first does not release the other. Focus returns only when eligible. |
| Obsolete work | Open, dismiss, and reopen the same QObject. Deliver the old close and settlement afterward. The new presentation retains input, focus, and adapter state. Repeat with queued focus work and duplicate callbacks. |
| Destruction | Destroy an active owner and an owner awaiting settlement. Other owners remain correct. No later callback accesses destroyed content or restores its focus. |
| Focus eligibility | Exercise overlapping owners, suppressed notices, invalidated focus targets, fallback focus, and native-window activation. Verify where input is actually delivered. |
| Menu timing | While the close animation is still visible, input has the established released state. Actual completion performs only eligible focus work. Pending navigation and reopen guards remain effective. |
| Notices | Exercise blocked-content and video-disabled actions, menu suppression, settings handoff, PIN suppression, disconnect, and return after temporary suppression. Passive overlays never capture input. |
| Connection and PSN | Cover managed and legacy paths, readiness, errors, teardown, StackView retirement, and PSN handoff. Include capture reset followed by managed rearm. |
| Session replacement | Exercise the actual session-change path for non-null replacement, null transitions, PSN start, and pending PIN. Verify both capture preservation and reset. A continuing presentation keeps its identity. |
| Disconnect | Verify cancel, both disconnect actions, native close, destruction, and reopening. An obsolete popup completion cannot release or change the newer presentation. |
| Settings | Exercise Display, Renderer, and Color Mapping in inline and native windows. Verify normal close, replacement, the return to Renderer, and main-view exit. |
| Console login PIN | Replace a request while active, replace content loaders, close and reopen, change sessions, and destroy the host. Verify capture, focus, and backend request cancellation separately. |
| Native events and cursor | Verify cursor changes, UI key delivery, global shortcuts, eligible native child-window selection, and ignored passive windows. Cancel queued keys on hide, destruction, and every session change. |
| Controller feedback | Observe outgoing feedback while blocked and while release waits for held buttons. Verify eventual resumption through the production release gate. Checking only the fully blocked state is insufficient. |

- Use controlled event ordering for duplicate and obsolete callback tests. Use
  event-driven waits for focus and completion instead of arbitrary sleep delays.
- Separate tests that can run offscreen from tests that require desktop window
  activation. Register and document both. A skipped desktop check is not a pass.
- Exercise both supported renderer presentation paths, including native windows,
  on a suitable desktop. Use the established OpenGL and Vulkan acceptance cases
  where supported. Record environmental limitations explicitly.
- Synthetic QKeyEvent delivery does not prove the spontaneous physical-key route
  to the console. Verify that route with a suitable native input test or a recorded
  desktop acceptance check. Keep its evidence distinct from constructed-event tests.
- Document repeatable configure, build, and test commands with required dependencies
  and desktop conditions. Run the maintained native target and the existing QML
  suite after migration. Record failures and skips with their causes.
- Finish with a repository-wide caller audit. Every production raw grab/release
  caller and indirect wrapper must be migrated. Review the diff for remaining
  duplicate focus, ownership, and session-reset policy in QML.

## Out of Scope

- Fixing an unreported input defect or changing established interaction behavior.
- A general dialog system, a universal priority order, or new overlay types.
- Redesigning settings, connection screens, notices, or the Console login PIN.
- Moving unrelated application focus logic into the stream ownership module.
- Changing console transport, PIN validation, disconnect actions, controller
  mapping, or held-button release semantics.
- Renderer redesign, replacement of native-window adapters, or removal of the
  existing QML test target.
- Adding public diagnostic controls or keeping source rewriting as the maintained
  native test method.

## Further Notes

The user approved complete caller coverage, the owner lifecycle module, harmless
duplicate opens and obsolete closes, and the combined native and QML test approach.
The test seam in this spec records that agreement. No further design interview is
required before implementation.

ADR 0001 is the architectural authority. This spec adds the caller scope and
acceptance requirements. The concrete representation of opaque identities and
the internal CMake source grouping remain implementation choices.

Existing native acceptance evidence covers some real session replacement and
window cases. It does not provide the maintained production-source test target
required here. Saved evidence must not be reported as a fresh test run.

Implementation is complete when all callers use the lifecycle module, duplicated
ownership policy is removed, and the required native and QML checks pass.
Any unavailable desktop or hardware check must be named in the completion report.
This specification does not itself implement or validate the refactor.


## Answer

Implementation is available on `codex/kbm-input-ownership`. Acceptance remains
open. Do not mark this prerequisite resolved from the checks below.

`QmlMainWindow.streamInput` now owns presentation identities, input capture,
saved return focus, notice focus retention, destruction cleanup, and session
capture reset and managed-connection rearm. All production raw input-counter
callers now use `present`, `dismiss`, and `settled`. Menu and popup completion
adapters keep the identity from the start of their close. Native pending key
work also checks its capture generation. The existing spontaneous-event and
timestamp guards remain in place. StreamSession's feedback order and button
release condition are unchanged.

The `chiaki-gui` object target supplies the same production objects to the app
and the maintained native test executable. A private test boundary controls the
backend session pointer and its existing change signal. The test creates real
StreamSession objects without starting transport. GNU linker wrapping observes
outgoing controller state. The QML fixtures delegate ownership behavior to the
production module and retain their lifecycle-balance observations.

Checks on 2026-10-21:

- The first native tracer failed configuration because the new production module
  did not exist. Its first green run passed duplicate-open and overlap checks.
- The maintained ownership test passes its four behavior cases, plus Qt setup
  and cleanup. Cases cover overlap, duplicate opens, obsolete close callbacks,
  destruction, reentrancy, and capture preservation and reset.
- The application and both maintained native test executables build with GNU
  UCRT64, Qt 6.11.2, FFmpeg, SDL2, and libplacebo 7.360.1.
- `ctest --test-dir build/input-native -R '^chiaki-stream-input' --output-on-failure`
  passes all three registered tests. The desktop runs use OpenGL and Vulkan.
  They check real session replacement, Display settings, Renderer settings,
  Color Mapping return, main-view exit, cursor state, blocked feedback, and
  resumption through both production button-release gates.
- One affected QML behavior pass selected StreamInterface, StreamNotices,
  LiveConsolePin, and Disconnect lifecycle scenarios. It passed 62 checks and
  found ten Disconnect fixture focus failures. The fixture now gives its opener
  initial focus. A run of only the ten failed choices passes all ten, plus setup
  and cleanup. The other 62 selected checks were not repeated.
- Earlier focused runs confirmed the native menu focus correction and eight
  notice checks for initial content, renderer handoff, focus return, and
  destruction. The standalone Qt 6.9.3 screen target still builds.
- A repository search finds no production or fixture `grabInput`, `releaseInput`,
  `grab_input`, caller capture flags, or shared return-focus stacks.
  `git diff --check` passes.

Limits and outstanding acceptance:

- A separate Win32 SendInput attempt returned success but did not observe mapped
  console feedback. That attempt preceded the consistent backend-session test
  boundary. This does not prove an OS input defect or an environment cause.
  The route remains unverified. The deterministic desktop target reports
  `spontaneousKeys` as skipped unless explicitly enabled. Its key injection sends
  down and up together, including on assertion failure.
- The complete native connection, PSN retirement, legacy-error timeout, pending
  key cancellation, native child-window selection, and obsolete adapter-event
  matrix has not been completed. The focused tests are not full acceptance.
- The full notice render matrix was not repeated. An earlier exploratory Qt
  6.9.3 pass reported two compact-warning width failures and one documented
  offscreen passive-window skip. No notice layout values changed in this work.
- Live console input, console sleep, physical controllers, desktop composition,
  and display scaling have not been checked. No hardware pass is claimed.

Repeatable configure, build, native, and separate OS-key commands are in
[`gui/tests/stream-input.md`](../../gui/tests/stream-input.md). The full existing
QML target and its scenarios remain available.

## Comments

### Acceptance status check, 2026-10-21

`Status: claimed` remains correct. Full acceptance requires the matrix above.
Later keyboard and mouse work adds evidence, but does not complete that matrix.
This check reviewed saved logs and current test sources. No tests ran again.

- The [saved native log](../keyboard-mouse-controls/evidence/keyboard-native.log)
  records passing OpenGL and Vulkan session and feedback checks.
  These checks cover controlled session changes and settings transitions.
  They do not complete the connection, PSN retirement, legacy-error timeout,
  pending-key cancellation, native child-window, and obsolete adapter-event cases.
- The [notice log](../keyboard-mouse-controls/evidence/chiaki-review-notice-qml.txt)
  records ten passing case rows across inline and separate windows.
  The [ownership log](../keyboard-mouse-controls/evidence/chiaki-review-notice-green.txt)
  confirms focus handoff between notice hosts. These focused checks do not
  establish a complete rerun of the required QML scenarios.
- The [test guide](../../gui/tests/stream-input.md#check-native-keys-separately)
  records partial Win32 injected-key success after the earlier failed attempt.
  Later attempts failed the Windows foreground precondition. The complete final
  injected-key case and physical-input acceptance remain unverified.
- The [later acceptance record](../keyboard-mouse-controls/acceptance.md#open-acceptance-gates)
  retains an unexplained Space-output failure in `platformKeyboard`.
  One diagnostic run passed, but did not establish the earlier failure's cause.
  Live console input, console sleep, physical controllers, desktop composition,
  and display scaling also lack complete acceptance evidence for this refactor.

Close this specification after the missing automated cases and QML checks pass,
with desktop and hardware results or explicit unavailable-check limits recorded.
