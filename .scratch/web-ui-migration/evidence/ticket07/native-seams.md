# Ticket 07 native and web seams

Production source: `codex/web-ui-ticket-07` at `878d1af6` (GUI tree
`c3e47fc49ca8c566079614c2eb3d7b773c9b5823`). Later `3d07a8b2` adds only the
ticket 10 claim documentation. Worktree:
`C:/Users/david/.codex/worktrees/web-ui-ticket-07/chiaki-ng-ui`.

## Constructors and transport ownership

The canonical optional dependency order is:

```text
QmlNativeServices(settings, PsnRequests, ProcessLauncher, RegistrationRequests, DiscoveryRequests)
QmlBackend(settings, parent, PsnRequests, ProcessLauncher, RegistrationRequests, DiscoveryRequests)
```

All defaults remain production functions. RegistrationRequests in
`gui/include/registrationrequests.h` owns raw `start`, `stop`, and `fini`
functions. QmlRegist owns the native registration lifecycle, joins during
destruction, and queues real completion signals. QmlBackend owns attempt state,
validation, profile/target guards, persistence, and manual entry association.
The deleted QML callback is not part of the shared completion path.

Ticket 08 PsnRequests worker/startSession functions and DiscoveryRequests remain
independent. Test instances inject discovery control before construction. Their
unexpected raw sessions fail closed except the explicit loopback handoff case.
No global transport override or JavaScript fixture API exists.

## Registration actions

The SetupBridge methods are:

```text
openRegistration(key, profile)
submitRegistration(context, fields)
connectRegistration(attempt)
```

The corresponding TypeScript client actions in `gui/web/src/bridge.ts` are
`registrationOpen`, `registrationSubmit`, and `registrationConnect`.
`Pairing.svelte` owns the manual guided form, progress, retry, and success
presentation. It opens a new entered-address context with an empty key, or a
current selected local/manual target with its opaque key. It does not save a
manual entry before registration; ticket 05 still owns the separate manual form.

The submit fields are `address`, integer `target`, `account`, text `pin`, and
text `consolePin`. Temporary codes are cleared after dispatch. The native
context never crosses as a native index or address identity. Native callers
use `createRegistrationContext(index)` and the shared `registerHost(...,
context)`. The `registrationAttempt()` state and `registrationAttemptChanged`
signal replace callback-only completion. The bridge snapshot projects only
safe progress, message, and filtered diagnostics. It exposes no native keys,
saved PIN, credentials, or Account ID.

Ticket 09 may add automatic registration presentation and its guarded native
entry. RegistrationMethodDialog remains only for that ticket until its callers
are removed. A discovered local console with PSN metadata needs an affected
context check when 09 permits its manual pairing entry; remote-only PSN rows
must remain rejected by the manual context.

## Account source actions

The SetupBridge methods are `beginRegistrationAccount`,
`lookupRegistrationAccount`, `readRegistrationAccount`, and
`endRegistrationAccount`. The client calls are `accountBegin`, `accountLookup`,
`accountRead`, and `accountEnd`. Begin accepts `saved`, `lookup`, or `signIn`.
The native context, route UUID, request ID, profile generation, and PSN attempt
baseline guard the result. Only the explicit current read command returns a
validated Account ID. Generic PSN snapshots still omit it.

Public lookup reuses PsnRequests::networkManager as its external HTTP boundary.
QmlBackend owns its fixed public service URL, native 30-second timer, bounded
response, canonical Base64 validation, cancellation, and one completion. It
creates no credentials. The web page shows the service/privacy explanation
before sending the username. Existing `PsnAccount.svelte` accepts an optional
registration route/close action so sign-in returns to the same valid form.

## Verification limits

The real compiled Svelte/WebEngine/WebChannel/native path has focused controlled
registration, lookup, OAuth, validation, and stale-target checks. The address,
form, and progress correction passes 5/0. The separate normal-Windows loopback
handoff passes 3/0, including pending reload without replay, native window/session
creation, setup hiding, supported dismissal, cleanup, and setup restoration.
Earlier checks inherited LAN discovery traffic; that limit remains recorded.
Final source builds and bounded comparisons preserve valid earlier evidence.
No claim covers live services, real-console pairing, actual foreground/focus,
GPU acceleration, physical input, live media, packages, or performance.
