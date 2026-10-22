# 07: Pair PS4 and PS5 consoles and retrieve an Account ID

**What to build:** Users can register a discovered or manually entered PS4 or PS5 console, retrieve an Account ID through public lookup or sign-in without losing valid form input, and connect after native registration succeeds.

**Blocked by:** 04: Connect locally from web setup to native playback; 06: Sign in to PSN independently from web setup.

**Status:** resolved for pairing and Account ID routes. Live-service, physical, GPU, and platform acceptance remains in ticket 23.

**Source:** [Web setup UI specification](../spec.md). Stories 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 44, 45, 46.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] The guided form retains supported console and firmware choices, broadcast registration, account identity input, saved Account ID use, and an optional console login PIN.
- [x] The Remote Play pairing code and console login PIN remain distinct. Leading zeroes survive, and invalid fields prevent native submission.
- [x] Registration uses a current native context for the discovered target, an existing manual entry, or an entered address. It accepts an entered address without the manual-address UI. Profile or address changes invalidate stale work.
- [x] Native completion signals replace callback-only integration where required. One activation starts one registration, and completion is delivered once.
- [x] Progress, success, failure, diagnostics, corrected input, and retry reflect native outcomes. Dismissal does not claim an unsupported cancellation.
- [x] Successful registration updates the correct native store and manual entry association. The console can then use the existing native connection path.
- [x] Streamer mode masks account and address fields, results, and diagnostics. Temporary codes are cleared when no longer needed.
- [x] The lookup route retains its service explanation and privacy requirement before sending the username.
- [x] A shared native operation owns the request, timeout, response validation, and completion. No authentication credentials are created by public lookup.
- [x] Only a valid Account ID fills the registration field. Unreadable responses, missing results, service failures, and timeouts show an actionable error.
- [x] Registration details survive lookup, cancellation, and retry. Stale results after navigation, replacement requests, or profile changes are ignored.
- [x] The account-source route offers saved Account ID use, public lookup, and the completed PSN sign-in flow. Returning from sign-in retains valid registration input and context, applies only a current native Account ID result, and restores focus.
- [x] Embedded tests cover field validation, leading zeroes, native registration results, duplicate activation, stale profiles and manual targets, connection after registration, successful lookup, invalid replies, timeouts, replacement and late replies, and form retention through lookup and sign-in.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.


## Answer

The compiled web setup page now has one guided registration form for a current
discovered target, an existing manual entry, or an entered address. It supports
PS5, the three supported PS4 firmware choices, broadcast registration, saved
Account IDs, and optional console login PINs. Pairing codes and login PINs use
separate text fields and native validation. Leading zeroes survive.

The shared native registration operation owns progress, completion, cleanup,
and store updates. Native context and attempt guards reject duplicate
activation, duplicate completion, stale profiles, and changed manual or
discovered targets. A corrected address can start a new attempt. Success
associates the correct manual entry and permits the existing native connection
path. Closing progress explains that registration continues; it does not claim
cancellation. A reloaded page restores pending progress without replay.

Public lookup explains the service and privacy condition before it sends a
username. The native operation owns HTTP, its real 30-second timeout, response
limits, canonical Account ID validation, and completion. It saves no sign-in
credentials. Saved Account ID use, lookup, and ticket 06 sign-in return to the
same form. Only the current native result can fill the Account ID field.
Cancellation, route replacement, and profile changes discard stale results.

Streamer mode masks form fields and suppresses address results and diagnostics.
Temporary codes are cleared after use or dismissal. Snapshots and browser
storage exclude credentials, registration keys, saved PINs, and Account ID
results. Only an explicit current account-source command returns an Account ID.
Setup confirmations and instructions remain HTML; native services own operations.

[Verification](../evidence/ticket07/verification.md) records the expected red
checks, the passing lookup/timeout/sign-in/validation/target checks, the focused
address/form/progress correction (5 pass, 0 fail), and the corrected Windows
loopback registration handoff (3 pass, 0 fail). The latter covers manual
association, page reload, native session/window creation, setup hiding,
supported dismissal/cancellation, cleanup, and setup restoration. The final
application and embedded target build; Svelte check reports no errors or
warnings. Bounded source comparisons preserve the passing behavior after the
05, 08, 14, and documentation merges.

The obsolete QML registration and public lookup routes, resources, callback
adapter, and replaced tests are removed. RegistrationMethodDialog remains for
ticket 09 automatic registration. Native stream components remain shared.

Earlier functional runs inherited LAN discovery broadcasts. Their registration,
lookup, and OAuth transports were controlled. Final fixtures now inject ticket
08 discovery control before construction and reject unexpected raw sessions.
Passing checks were reused after exact source review; they were not repeated
to remove the recorded discovery limit. The supported layout scope is at least
1024 CSS pixels, including 200 percent zoom in a sufficiently wide window.
Earlier narrow captures remain historical evidence.

These checks do not prove real-console pairing, live public lookup or Sony
sign-in, actual foreground/focus, GPU acceleration, live media, physical input,
platform packages, or performance. Those limits remain in ticket 23. Native
input/media behavior and valid ticket 04/05/08 evidence are preserved.
