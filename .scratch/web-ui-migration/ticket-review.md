# Astra High review of the web UI ticket proposal

Date: 2026-10-22
Reviewer: gpt-6-astra, High effort.
Outcome: recommend 23 tickets instead of 26.
Scope: review recommendations only. The review left the specification and 26 draft tickets unchanged.
Adoption: approved by the user on 2026-10-22. See the [23 published tickets](ticket-breakdown.md).

The reviewer read all 26 ticket drafts, the current specification, and connected
native code. The main agent checked the source behind the dependency changes
and validated the revised graph.

GPU-accelerated web setup and separate native video, input, and overlays remain
required. This review adds no QML setup fallback, runtime UI selector, or
additional approval gate.

## Recommended merges

The numbers in this section refer to the original drafts.

| Merge | Result | Why it is a reasonable size |
| --- | --- | --- |
| 05 + 06 | Console management and manual addresses | Both use the console library, native target keys, confirmations, host stores, and form handling. Native handoff stays in 04. |
| 07 + 08 | Pairing and Account ID retrieval | Public lookup is a branch of the registration form. One ticket can own retained input, replacement requests, profile checks, and focus return. PSN authentication stays separate. |
| 17 + 18 | Keyboard and mouse bindings, response, and reset | These are already one native settings page. One reset operation covers both tabs. The native input and response algorithms remain unchanged. |

The existing [keyboard and mouse settings page](../../gui/src/qml/KeyboardMousePage.qml)
contains the binding and response tabs and invokes one reset operation. Keeping
those controls together avoids an intermediate reset that covers only part
of the feature.

## Boundaries to keep

- Keep 01, 02, 03, and 04 separate. Native ownership, stream-root extraction,
  initial WebEngine integration, and handoff each have different failure cases.
- Keep sign-in, PSN streaming, and automatic PSN registration separate.
  They handle credentials, connection workers, and registration-only sessions.
- Keep startup auto-connect separate from session and setup recovery.
- Keep the settings shell separate from video and audio configuration.
- Keep renderer settings and color mapping separate. Their current pages have
  1,170 and 928 lines, with many dependent controls.
- Keep controller mapping separate from keyboard and mouse configuration.
  Device loss and native mapping input require their own checks.
- Keep profiles and files separate from reminders and Steam shortcuts.
  Profile replacement and file failure handling need native work.
- Keep Windows, Linux, and macOS packages separate. Their deployment and signing
  requirements differ, and one unavailable platform must not block the others.

## Required corrections before publication

1. Narrow original 09 to independent PSN sign-in and native account state.
   Move its return-to-registration acceptance into merged 07 + 08.
   Make the merged registration ticket depend on sign-in, rather than
   making sign-in wait for registration.

2. Remove original 07's dependency on 06. The native registration method accepts
   an entered address. Its success path creates or associates a manual host.
   The separate manual address UI is not a prerequisite.
   Evidence: [registration context and method](../../gui/src/qmlbackend.cpp:1653)
   and [manual host association](../../gui/src/qmlbackend.cpp:1739).

3. Remove original 11's dependency on 07. Automatic registration has its own
   native registration-only session. Give that ticket a complete automatic
   registration action with the correct context. Verify the combined
   two-method chooser at final acceptance, after both registration flows exist.
   Evidence: [automatic registration](../../gui/src/qmlbackend.cpp:1762).

4. Change original 25's blocker from 12 to 10. Recovery needs the local and PSN
   session paths. Native resume calls those paths directly rather than the
   startup auto-connect UI.
   Evidence: [native resume](../../gui/src/qmlbackend.cpp:373).

5. Remove original 21's dependency on 05. Reminders need sign-in. Native Steam
   shortcut creation takes a name, launch options, and a directory. It does not
   require the console management result.
   Evidence: [Steam shortcut creation](../../gui/src/qmlbackend.cpp:2817).

6. Give settings work one owner. Original 13 owns navigation, search, and
   general session settings. Original 14 owns video profile and audio fields.
   Original 15 owns renderer file actions. Original 20 owns profile and general
   settings file actions and reuses renderer actions already delivered.

7. Retire replaced setup routes, resources, adapters, and tests in each migration
   ticket. Preserve shared native stream components. Original 26 performs the
   final remaining-caller and resource check rather than collecting all cleanup.

Original 20 must retain its file failure safety requirement. Current
[profile import](../../gui/src/settings.cpp:253) and
[renderer import](../../gui/src/settings.cpp:292) clear destination settings
before completing the copy. A web wrapper alone cannot establish the promised
failure safety. Keep that native work within the respective file-action tickets.

At the start of 01, include a bounded prerequisite check of accelerated
WebEngine beside the current native renderer. Pin the baseline build and
measurement conditions. Reuse applicable evidence in 03 and 04. This does not
need another ticket or a broad feasibility project.

The startup [GPU-disable override](../../gui/src/main.cpp:91) and the native
[Qt Quick graphics selection](../../gui/src/qmlmainwindow.cpp:1783) make the
shared-process graphics check relevant.

## Revised order and blockers

Only the blocker column creates dependencies. Independent tickets can proceed
together.

| New | Original | Ticket | Blocked by, new numbers |
| --- | --- | --- | --- |
| 01 | 01 | Application-owned native services | None |
| 02 | 02 | Native stream-only interface | 01 |
| 03 | 03 | Accelerated web console library | 01 |
| 04 | 04 | Local native stream handoff | 02, 03 |
| 05 | 05 + 06 | Console management and manual addresses | 04 |
| 06 | 09 | Independent PSN sign-in | 03 |
| 07 | 07 + 08 | Pairing and Account ID retrieval | 04, 06 |
| 08 | 10 | PSN native streaming | 04, 06 |
| 09 | 11 | Automatic PSN registration | 08 |
| 10 | 12 | Console wake-up and startup auto-connect | 08 |
| 11 | 13 | Settings shell and general session settings | 03 |
| 12 | 14 | Video profiles and audio devices | 11 |
| 13 | 15 | Display and renderer settings | 11 |
| 14 | 16 | Color mapping settings | 13 |
| 15 | 17 + 18 | Keyboard and mouse bindings, response, and reset | 11 |
| 16 | 19 | Controller setup and mapping | 11 |
| 17 | 20 | Profiles, settings files, and logs | 11 |
| 18 | 21 | Reminders and Steam shortcuts | 06 |
| 19 | 22 | Windows package | 04 |
| 20 | 23 | Linux packages | 04 |
| 21 | 24 | macOS package | 04 |
| 22 | 25 | Session and setup recovery | 08 |
| 23 | 26 | Complete application and final acceptance | 05, 07, 09, 10, 12, 14, 15, 16, 17, 18, 19, 20, 21, 22 |

The final blockers include new 05 and 10. Console management and auto-connect
become independent end branches when the false dependencies are removed.
Final acceptance must wait for both.

The graph covers all 26 original tickets exactly once. It has no cycles,
missing blockers, or redundant transitive edges. Final acceptance reaches every
other ticket.

## Conservative test policy

Each ticket runs the smallest test set that covers its changed behavior and
affected callers. Use the production Svelte UI, real WebChannel transport,
production bridge, and native services. Control external results at the
approved native request boundaries.

Run additional tests when a change, failure, or unresolved risk gives a
specific reason. Record passing evidence and reuse it while the tested code,
dependencies, and relevant environment remain applicable. Stop after the
required checks pass unless a concrete concern remains.

- Establish the embedded harness once in original 03, then extend it for
  new behavior. Do not create another mocked bridge or test API per screen.
- Keep meaningful failure cases for stale targets, cancellation, invalid writes,
  file failures, and late replies. Another passing happy path does not replace
  a different failure case.
- Retain ADR 0001 and native output checks. Run the affected set after ownership,
  stream-root, handoff, input, or recovery changes. An unrelated setup form
  change does not require every native hardware check.
- Report the native input test compiler conditions accurately. The current
  [GNU-only target condition](../../gui/tests/CMakeLists.txt:41) means that an
  MSVC run does not provide that coverage.
- Perform an initial packaged startup check in original 03 on the first
  supported target. Keep complete platform handoff checks in original 22
  through 24, after native handoff works.
- Check each final package once and check the affected migrated features.
  Reuse deeper hardware evidence unless relevant native code, Qt or WebEngine,
  package contents, drivers, or configuration changed.

Most existing per-ticket checks cover different behavior and should stay.
The draft does not require a full suite after every ticket. Statements that
package checks can be repeated are not themselves requirements to repeat them.

### Replace blanket repeat counts

Apply these changes to the specification and final acceptance ticket when the
revised proposal is adopted.

| Current requirement | Recommended replacement |
| --- | --- |
| At least three baseline comparisons | Compare matched baseline and migrated measurements using a tolerance fixed before evaluating the result. Reuse valid baseline variation data. Collect additional independent runs if variation is unknown, a result is near the limit, or a regression appears. Do not claim a pass from inconclusive data. |
| A 30-minute stream per renderer | Perform sustained acceptance for materially different native and browser graphics paths. Choose the observation interval from warm-up, resource trends, and known time-dependent failures. Record its duration and limits. Use 30 minutes or longer when those facts justify it. |
| Twenty connect-stop cycles per renderer | Exercise first connection and a subsequent connection with recreated resources. Check release after each. Test cancellation, failure, and late completion at their specific native boundaries. Extend the loop when counters grow, cleanup is uncertain, or a known race needs repeated cycles. |

A subsequent connection tests resource recreation. Repeated measurements can
establish variation. A sustained run can expose growth or thermal effects.
Those repetitions have a purpose. An unchanged passing check does not need to
run again merely because another ticket completed.

Keep real GPU, HDR, microphone, controller, focus, and available external input
latency acceptance. Offscreen tests do not establish hardware results.

## Review limits

During this review, no application code, parent specification, or ticket draft changed.
The later publication applies the approved specification amendments and marks the old drafts as history.
No builds, test suites, native sessions, or hardware checks ran.
Ticket sizes are estimates based on the current source.
