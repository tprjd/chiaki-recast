# 22: Resume interrupted sessions and recover web setup

**What to build:** Users can recover from setup reloads, a setup renderer crash, suspend and resume, or an interrupted connection without duplicate work or damage to native playback.

**Blocked by:** 08: Stream from a registered console through PSN.

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 29, 30, 33, 34, 43, 46, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] Reloading or recreating setup reads current native state and reconnects subscriptions without restarting a pending request.
- [ ] A setup renderer crash leaves healthy native playback running. Setup can be recreated and restored through the web path.
- [ ] Native local and PSN suspend and resume behavior retains the existing policy and prepares resources before a replacement session needs them.
- [ ] Late cancellation, old worker replies, overlapping input owners, and teardown callbacks cannot change a newer attempt or session.
- [ ] Handoff and restoration preserve held-input release, gameplay capture, focus, geometry, error presentation, and explicit application exit behavior.
- [ ] Resource measurements cover the hidden or frozen page. Page destruction is added only if required, with defined non-secret state recovery.
- [ ] Embedded tests cover reload during a request, setup renderer failure during playback, suspend and resume, first and subsequent sessions, and stale completions. Add cycles only when resource trends, uncertain cleanup, or a known race require them.

## Comments

- Claimed after ticket 08. Prepare native setup recovery and existing suspend/resume policy. Measure the hidden or frozen page first; do not add optional page destruction without evidence that it is needed. Reuse verified native lifetime and cleanup checks, and run focused recovery cases.
