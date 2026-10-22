# Ticket 07 integration audit

The merger checked the source and saved evidence on 2026-10-22. No build,
runtime test, or new capture was run. The recorded passing results are reused.

- Prior integration: `42d5a5e0e61fb07d009aea121dbeadfe805e9979`.
- Worker: `5b13f539f983a8f1c42f3906a2d8590e9b2faa29`.
- Merge: `915be521cb6563cd946b4fd71e464e525114ec6e`.
- GUI tree: `c3e47fc49ca8c566079614c2eb3d7b773c9b5823`.
- The merge had no conflicts. Its complete tree equals the worker tree.
- The local gf-complete patch is excluded. Native library and vendor trees
  equal prior integration. The original main checkout was not changed.

The merger read the ticket Answer, verification, native interface note,
identity manifest, comparison script, actual native and web source, focused
reports, and final build reports. The read-only command
`python .scratch/web-ui-migration/evidence/ticket07/verify.py --check` passes
before and after the merge. All 37 bounded comparisons pass. An independent
hash check also matches the 21 source files, four current artifacts, and 50
evidence files. [audit.json](audit.json) records the identities and comparisons.

All 45 prior embedded test bodies remain equal after the required constructor
argument insertion in 10 cases. Eighteen complete files retain native window
lifetime, stream/media code, settings, account services, discovery, manual
management, connection UI, and shared CSS. The native registration operation
retains validation, current profile and target checks, completion once, and
store association once. Public lookup retains its native parser, actual
30-second timer, stale-result guards, and credential-free result. Explicit
current account reads preserve ticket 06 sign-in and snapshot privacy.

The combined dependency order remains `PsnRequests`, `ProcessLauncher`,
`RegistrationRequests`, `DiscoveryRequests`. Native request defaults remain
production functions. Final pairing fixtures inject discovery control before
construction and reject unexpected raw session starts. The explicit native
handoff retains the real raw session start and uses loopback only.

The actual focused report passes lookup, timeout, account sources, sign-in,
validation, stale targets, and console guards. Its form-focus and address-retry
failures have corrected passing evidence, 5 passed and 0 failed. The corrected
Windows native handoff report has 3 passed and 0 failed. Its earlier failure
was the missing fixture dismissal. Production native lifetime did not change.
Final saved application and embedded builds pass after tickets 08 and 14.
The frontend report has zero errors and zero warnings.

One requirement remains open in ticket 09. The current manual context rejects
every nonempty DUID, including a discovered local console with PSN metadata.
That local row can expose manual pairing. Ticket 07 now marks this case
unchecked. Ticket 09 tracks the exact shared context change and both methods
for the same current local target, with stale-target and profile guards.
Remote-only PSN rows must remain rejected. Ticket 09 confirmed ownership.

The merger inspected only the supported `registration-1280.png` capture,
SHA256 `91384ad628ede0e8dce7f39816ca6018b5789a72109b84926f3b0a3b1d54b847`.
Visible form labels and instructions are readable without horizontal clipping.
Narrow images remain historical. No new acceptance below 1024 CSS pixels was
done. Supported 200 percent zoom acceptance remains open in ticket 23.

Earlier runs inherited LAN discovery broadcasts. That limit remains recorded.
This audit does not establish zero LAN traffic, live services, real-console
pairing, physical focus/input, GPU acceleration, media quality, platform
packages, or performance.
