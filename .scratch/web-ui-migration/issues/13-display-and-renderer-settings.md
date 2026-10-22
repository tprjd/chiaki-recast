# 13: Configure display targets and the native renderer

**What to build:** Users can configure display targets, renderer selection, presets, and custom native renderer options from web setup.

**Blocked by:** 11: Open settings and change general session options.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 50, 51, 52, 56, 75.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] The web UI retains existing display controls, renderer choices, presets, and custom renderer fields with their actual ranges and conditions.
- [x] Values are converted and validated by the existing native settings logic and persist in the existing store.
- [x] Renderer changes retain the existing restart behavior and actual runtime renderer reporting. A saved choice does not falsely report a live switch.
- [x] This ticket owns renderer settings import, export, and documentation entry points through native dialogs and services. File contents do not pass through the frontend. General settings and profile file actions belong to their own ticket.
- [x] Cancelled or failed renderer file actions preserve existing values and report the actual result. Validate the import before replacing saved values; a web wrapper alone does not establish failure safety.
- [x] Live stream display and renderer settings stay native and use the same saved values and notifications.
- [x] Embedded tests verify native writes, rejected values, preset changes, restart requirements, renderer import and export, failed or cancelled file actions, and consistency with native live settings.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Answer

Implemented Display and Renderer categories with the existing display targets,
backend choices, presets, queue depth, and custom renderer groups. Ticket 14
owns color mapping. The explicit native allowlist checks types, ranges,
conditions, and profile generations. Native QmlSettings setters retain saved
values and live notifications. Saved backend choices and active native renderer
status remain distinct. Backend changes restart immediately. OpenGL vertical
sync changes retain the same immediate trigger. Failed launches keep their
warning log and show the actual native failure result. Duplicate writes do not
restart. The focused test controls the external native launch boundary.

Renderer import and export now return actual native outcomes. Import validates
all input before it replaces keys. Export checks writes and atomically commits
the file. Native picker completions retain their original settings generation.
Failed and cancelled actions preserve values. Failed export also preserves an
existing read-only destination. File paths and contents stay native.
The shared documentation helper returns the actual libplacebo URL.

Focused embedded checks passed for writes, invalid values, custom conditions,
presets, native notifications, reload, native store reads, file outcomes,
profile changes during a picker, responsive layout, and immediate restart
policy with controlled native launch failures. The retained native
QML action caller also passes. See [ticket 13 evidence](../evidence/ticket13/verification.md)
for exact commands, bounded source comparisons, reports, captures, and the recorded
missing digest for the earlier file-only executable.

The native live dialogs and their resources still have callers and remain.
No new setup-only callback adapter was added. The Color mapping entry is ready
for ticket 14. Live playback, GPU, physical displays, successful OS relaunch,
platform, and package acceptance remain unverified. Production presets and
the known HighQuality acceptance limit are unchanged.

Integration merge `7e29e313` contains worker commit `fe0cfb0f` without conflicts.
The merged tree matches the worker tree. GUI source matches compiled restart
source `5a83520f`. All 20 final recorded source and executable hashes match.
The merger independently checked the eight bounded function comparisons and
their hashes, read the actual case logs, and inspected all four layout captures.
The four merged capture hashes match the worker copies. The earlier file-only
executable digest remains null. The layout images retain their earlier layout
artifact identity, separate from the final restart executable identity.

Passing checks were reused without builds or test reruns. The controlled restart
case passes 4/0 and did not start a process. Successful OS relaunch and application
quit remain untested. Ticket 04 lifetime, ticket 06 account, and ticket 12 media
controls remain. Claims for 05, 15, and pairing preparation remain unchanged.
Source-only whitespace checks pass. Physical display, GPU, live playback,
platform, package, and HighQuality acceptance remain open.
