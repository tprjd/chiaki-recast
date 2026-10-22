# Ticket 09 integration audit

Date: 2026-10-22

Integration started clean at `25ece857523668ac7b2af4cd54ff1f9039944cd9`.
The explicit `--no-ff` merge of worker
`d63554c4c2b02fd3440db9dc3b11a8029d944856` completed without conflicts at
`13b594df87f86da0412f7bd9d6dfde5dc17479b8`.
The complete merge tree equals the worker tree.

## Independent checks

[Identities](identities.json) record six current artifact SHA256 matches.
These cover the application, embedded executable, JavaScript, CSS, HTML, and
lockfile. The merged GUI tree is the rebuilt tree
`962d140dd463ad143e9f7b165b1a9e76708d92e5`.

The two final runtime records include their source, complete diff, executable
hash, launcher, and output. Their saved diff hashes match the pre-run records.
Applying each GUI diff to its recorded baseline in a temporary Git index
reconstructs the runtime source. Only the test file differs between the two
groups. Direct diff inspection finds the two recorded fixture corrections.
The final02 GUI tree equals production source `6faf8ff3` exactly.
CRLF patch bytes need LF conversion for this temporary-index comparison.
The first comparison failed before that conversion. No workspace source changed.

The existing [verify.py](../ticket09/verify.py) passes all 24 bounded source
comparisons at the merge. [The saved result](source-comparisons.json) records
the actual merge identity. Its only permitted difference is ticket 16's
controller-operation refusal in `connectConsole`. The registration fixtures
have no active mapping operation.

Twelve protected files equal the integration baseline. These include the native
window, stream session, `StreamRoot`, input ownership, controller navigation,
setup widget, and web controller components. Three additional backend sections
match exactly: constructor, session release, and controller mapping. Constructor
arguments remain `PsnRequests`, `ProcessLauncher`, `RegistrationRequests`, then
`DiscoveryRequests`. Production `git diff --check` passes.

## Source review

The change reaches the backend, application controller, setup bridge, web
connection and manual form controls, resource list, and affected embedded tests.
Manual and automatic entry points share native busy guards. The bridge refuses
stale opaque keys and profiles. Automatic completion checks the current target,
DUID, console type, returned MAC, and attempt phase before saving. Duplicate
completion cannot save twice. Manual contexts accept a discovered local DUID
target and retain address, duplicate-row, and profile guards.

Registration prepares a hidden native renderer. `show()` owns StreamRoot
creation, so this path creates no stream root. The controller leaves setup
visible and avoids setup suspension and activation requests. Registration skips
the stream geometry changes. Existing worker, session, queued-frame, and native
window cleanup order remains. Retry waits for cleanup. The success fixture also
checks a later controlled normal native handoff with the same controller.

The removed QML chooser, signal, resource, and screen test have no remaining
caller. Native overlays and stream tests remain. Native same-nickname PS4/PS5
lookup behavior is unchanged.

## Reused verification and limits

The inspected final01 log has 12 pass and 2 fail, including affected manual
form and progress passes. Final02 has 4 pass and 0 fail for the two corrected
fixture cases. Together they pass all ten automatic data rows and the affected
manual cases. The PS4 correction gives the PS5 a distinct nickname. The context
correction expects the existing stale-address error. Production code did not
change between these groups. Failed, offscreen, and invalid-invocation records
remain available in [ticket 09 verification](../ticket09/verification.md).

The passing context row closes ticket 07's carried local-DUID criterion.
It checks native manual completion, changed address, duplicate rows, remote-only
rejection, and old-profile rejection. Automatic rows cover the same local target,
changed DUID, removed target, returned MAC, and request exclusion.

The recorded application and embedded builds pass after ticket 16 integration.
No build, runtime check, capture, or graphics matrix ran for this merge.
The local `gf-complete` patch is excluded. Live PSN, real-console registration,
actual focus, physical input, GPU coexistence, packages, and performance remain
in ticket 23. The combined method chooser also remains in ticket 23.
No new layout or acceptance work below 1024 CSS pixels was added.
