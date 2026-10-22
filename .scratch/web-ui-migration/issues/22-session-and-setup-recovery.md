# 22: Resume interrupted sessions and recover web setup

**What to build:** Users can recover from setup reloads, a setup renderer crash, suspend and resume, or an interrupted connection without duplicate work or damage to native playback.

**Blocked by:** 08: Stream from a registered console through PSN.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 29, 30, 33, 34, 43, 46, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] Reloading or recreating setup reads current native state and reconnects subscriptions without restarting a pending request.
- [x] A setup renderer crash leaves healthy native playback running. Setup can be recreated and restored through the web path.
- [x] Native local and PSN suspend and resume behavior retains the existing policy and prepares resources before a replacement session needs them.
- [x] Late cancellation, old worker replies, overlapping input owners, and teardown callbacks cannot change a newer attempt or session.
- [x] Handoff and restoration preserve held-input release, gameplay capture, focus, geometry, error presentation, and explicit application exit behavior.
- [x] Resource measurements cover the hidden or frozen page. Page destruction is added only if required, with defined non-secret state recovery.
- [x] Embedded tests cover reload during a request, setup renderer failure during playback, suspend and resume, first and subsequent sessions, and stale completions. Add cycles only when resource trends, uncertain cleanup, or a known race require them.

## Comments

- Claimed after ticket 08. Prepare native setup recovery and existing suspend/resume policy. Measure the hidden or frozen page first; do not add optional page destruction without evidence that it is needed. Reuse verified native lifetime and cleanup checks, and run focused recovery cases.


## Answer

Setup reload and actual renderer failure recover through a fresh restricted web
page and channel. The same native bridge retains allowed route/search state and
opaque selection. Pending native requests are read from snapshots and are not
replayed. A failed hidden page waits for native cleanup before replacement.

Page events invalidate capture, mapping, Steam forms, and web file actions.
Native stream file actions retain their own lifetime. Reminder history stays in
the bridge. Local resume retains its target, and obsolete profile, attempt, or
cancellation state cannot restore old credentials. Native rendering, media,
connection policy, and stream input ownership remain native.

[Verification](../evidence/ticket22/verification.md) records native local/PSN
recovery 4/0, actual crash and final focus/media 5/0, scoped file lifetime 4/0,
eight affected cases, and the final real-channel helper/media check 3/0 without
JavaScript callback errors. Native decoded buffers all release after joined
producer cleanup. Resource samples support retaining hidden/frozen setup.

Source-equivalence checks retain ticket 04 cleanup, error, geometry, direct
exit, and input-owner coverage, plus the later worker, capture, profile, and
reminder contracts. No full suite, extra native cycles, or optional page
destruction was added. Physical focus and held-device input, real OS power
restore, live-console behavior, GPU/default-preset, long-term resource,
performance, platform, and package acceptance remain in ticket 23.
