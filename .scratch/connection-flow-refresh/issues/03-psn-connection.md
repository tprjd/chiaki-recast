# 03: PSN connection progress and recovery

**What to build:** A remote user follows the real PSN connection stages through layout A, can cancel at supported stages, can read and retry persistent errors, and continues into video waiting and the stream. Automatic registration remains a separate conditional route.

**Blocked by:** 01: Local connection progress, cancellation, and retry.

**Status:** resolved

Parent: Connection progress and console login PIN: guided workspace. Specification commit: `cb1f76e9`.
Selected design: A on prototype branch `codex/prototype-connection-flows`, implementation commit `7a0111b6`.

- [x] Use A's status panel and help for waiting for internet, initiating connection, linking, data connection, video waiting, and ready. Show actual console/route context and actual completed stages.
- [x] Automatic registration shows its own progress and completed result. Registration is not shown as a mandatory step for every PSN session. Completion offers Back to consoles; registration-only retry preserves that intent.
- [x] Start failure, general failure, and unreachable-console outcomes remain visible with useful details, Back to consoles, and retry when valid. Unrelated authentication/token screens retain their existing routes.
- [x] Reuse 01's active-attempt and target validation behavior. PSN retry starts one attempt after cleanup and cannot act on a changed profile, missing target, or stale session. No automatic retry loop is added.
- [x] Cancel uses the existing PSN stop behavior once and respects genuine non-cancellable stages. Explain disabled cancellation. Keyboard, controller, right-click, and Ctrl+Q inputs follow the same decision.
- [x] Cancelling remains visible until cleanup completes. Show cancelled with Back to consoles. Late PSN success, failure, registration, or PIN events cannot reopen the old route or affect another attempt.
- [x] Data connection completion hands off to the existing session/video waiting condition. It must not show video ready before a frame, except for the existing video-disabled mode. Success never adds an extra confirmation step.
- [x] PIN requests still reach the current live-PIN presentation while 05 is pending. The PSN-to-stream transition must not lose a pending PIN request or open duplicate prompts.
- [x] Private console data and diagnostics follow streamer mode. Text wraps, keyboard/controller focus stays visible, and return focus is valid at both target sizes.
- [x] Production-QML tests drive all native PSN states and transitions, conditional registration, disabled cancellation, cleanup, persistent errors, retry, route handoff, video-disabled mode, stale events, and profile/target changes.
- [x] Focused isolated native contract checks verify PSN event ownership, cancel dispatch, registration-only retry intent, and the PSN-to-stream handoff. The normal build and affected routing regressions pass.

Demo: show PSN linking with cancellation disabled, advance to data connection, cancel and complete cleanup, retry a failed attempt, then hand off to video waiting. Demonstrate automatic registration as a separate branch.

Dependency reason: 01 provides the production attempt ownership, validated retry, and stream-waiting destination. 04 is not a blocker; ordinary PSN entry already exists.

## Answer

Implemented PSN progress and recovery in production `ConnectionView`, `PsnView`,
`StreamView`, and Main routing. The view shows internet waiting, PSN initiation,
console linking, data connection, video waiting, errors, and cancellation. Only
native events add completed stages. A data connection does not prove that video
is ready. Video-disabled sessions still continue when the session connects.

Automatic registration uses a separate branch. Its completed result stays on
screen after session cleanup. Retry retains registration-only intent. The native
handler checks the original profile and console again. It consumes each automatic
registration confirmation, so an old callback cannot start another request.

The backend retains the attempt through token refresh. Worker results, session
events, token replies, and registration results must belong to that attempt and
profile. Cancel follows native stage restrictions and dispatches once. Worker
return or native quit completes cleanup. The shared token-finish handler also
clears cleanup after the existing resume refresh limit is reached. Errors stay
visible, and retry waits for cleanup. PSN hosts now require current registrations,
so a cached remote row cannot retry a removed registration. Host-list updates use
profile snapshots and apply their results only to that profile generation.

The stream view retains ownership after PSN handoff. A later error, cancellation,
or retry therefore cannot destroy the active attempt by replacing its host.
The earlier PIN signal and prompt remained functional before ticket 05. The
integrated PIN continuity proof below completes that criterion.

Verification completed:

- Production-QML tests load the actual connection view with the approved fake
  backend. Initial red checks showed the old local route text and the missing
  completed-stage output. The final PSN suite passed 11 checks.
- Relevant QML suites passed 188 checks: local connection 11, PSN connection 11,
  saved PIN 23, console management 44, library 35, registration form 22,
  registration results 19, and manual host 23. No checks failed.
- The final hidden native run passed 64 assertions. It used real `QmlBackend`,
  `Main`, `PsnView`, `StreamView`, and fully constructed `StreamSession` objects.
  It exercised 14 PSN workers, eight starts, and five PSN cancel commands.
- Native checks cover PSN stage restrictions, one cancel command, worker cleanup,
  start and general errors, unreachable consoles, synchronous start failure,
  persistent errors after stream handoff, retry, video-disabled readiness,
  late session events, profile changes, removed targets, token cleanup,
  resume refresh failure, consumed registration callbacks, registration-only
  retry, and persistent registration completion.
- Five checks read the actual isolated INI files after process exit. They confirm
  unchanged saved PINs, console records, and auto-connect preference. The other
  profile remains unregistered. Reports contain no PIN values.
- Production-QML captures at 900 by 600 and 1280 by 720 show A's guided workspace.
  The parent agent inspected linking and registration completion captures. Long
  text, details, privacy changes, and focused actions passed at both sizes.
- The normal application built from 173 GUI files with hashes equal to this
  worktree. Its executable contains no temporary native QA tokens. It was not
  launched against user settings. The shared native cache was then restored by
  hash and rebuilt with its original normal output configuration.

Evidence is in `../evidence/ticket03-*`. The native assertion source is retained
as `ticket03-native-contracts.cpp`. The final normal source and executable hashes
are in `ticket03-normal-production-manifest.json`.

The temporary QA helper, transport intercepts, restoration manifest, and scripts
remain outside production under
`C:/Users/david/.cache/chiaki-ui-native/psn-contract-prep02/`. The final hidden run
is `psn-contract-run11/`. Its transport intercepts prevent discovery, PSN refresh,
PSN worker network calls, session transport, and sleep commands. The constructor
continues through normal native initialization. QA disables the local automatic
retry branch because its isolated PSN transport has no real holepunch object.

No real PSN account, console, network connection, hardware PIN acceptance, video
transport, visible native input, physical controller, or native DPR check is
claimed. Ticket 05 owns pending PIN presentation continuity. Ticket 06 owns full
visible acceptance and hardware limits. No map or specification file changed.

### Integrated PIN continuity

Ticket 05 is integrated at `f54346ae`. Its Main-owned prompt keeps the same
request and field through the PSN-to-StreamView transition. Both [inline native
checks](../evidence/ticket05-native-inline.json) and [separate hidden-window
checks](../evidence/ticket05-native-separate.json) passed 99 assertions, including
prompt continuity and balanced input capture. This resolves the remaining PIN
criterion. Visible native focus, input delivery, and DPR checks remain with 06.
