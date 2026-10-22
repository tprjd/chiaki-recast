# 07: Pair PS4 and PS5 consoles and retrieve an Account ID

**What to build:** Users can register a discovered or manually entered PS4 or PS5 console, retrieve an Account ID through public lookup or sign-in without losing valid form input, and connect after native registration succeeds.

**Blocked by:** 04: Connect locally from web setup to native playback; 06: Sign in to PSN independently from web setup.

**Status:** ready-for-agent

**Source:** [Web setup UI specification](../spec.md). Stories 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 44, 45, 46.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] The guided form retains supported console and firmware choices, broadcast registration, account identity input, saved Account ID use, and an optional console login PIN.
- [ ] The Remote Play pairing code and console login PIN remain distinct. Leading zeroes survive, and invalid fields prevent native submission.
- [ ] Registration uses a current native context for the discovered target, an existing manual entry, or an entered address. It accepts an entered address without the manual-address UI. Profile or address changes invalidate stale work.
- [ ] Native completion signals replace callback-only integration where required. One activation starts one registration, and completion is delivered once.
- [ ] Progress, success, failure, diagnostics, corrected input, and retry reflect native outcomes. Dismissal does not claim an unsupported cancellation.
- [ ] Successful registration updates the correct native store and manual entry association. The console can then use the existing native connection path.
- [ ] Streamer mode masks account and address fields, results, and diagnostics. Temporary codes are cleared when no longer needed.
- [ ] The lookup route retains its service explanation and privacy requirement before sending the username.
- [ ] A shared native operation owns the request, timeout, response validation, and completion. No authentication credentials are created by public lookup.
- [ ] Only a valid Account ID fills the registration field. Unreadable responses, missing results, service failures, and timeouts show an actionable error.
- [ ] Registration details survive lookup, cancellation, and retry. Stale results after navigation, replacement requests, or profile changes are ignored.
- [ ] The account-source route offers saved Account ID use, public lookup, and the completed PSN sign-in flow. Returning from sign-in retains valid registration input and context, applies only a current native Account ID result, and restores focus.
- [ ] Embedded tests cover field validation, leading zeroes, native registration results, duplicate activation, stale profiles and manual targets, connection after registration, successful lookup, invalid replies, timeouts, replacement and late replies, and form retention through lookup and sign-in.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.
