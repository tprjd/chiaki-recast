# 13: Configure display targets and the native renderer

**What to build:** Users can configure display targets, renderer selection, presets, and custom native renderer options from web setup.

**Blocked by:** 11: Open settings and change general session options.

**Status:** claimed

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
status remain distinct. The UI offers the existing native restart action.

Renderer import and export now return actual native outcomes. Import validates
all input before it replaces keys. Export checks writes and atomically commits
the file. Native picker completions retain their original settings generation.
Failed and cancelled actions preserve values. Failed export also preserves an
existing read-only destination. File paths and contents stay native.
The shared documentation helper returns the actual libplacebo URL.

Focused embedded checks passed for writes, invalid values, custom conditions,
presets, native notifications, reload, native store reads, file outcomes,
profile changes during a picker, and responsive layout. The retained native
QML action caller also passes. See [ticket 13 evidence](../evidence/ticket13/verification.md)
for exact commands, source comparisons, reports, captures, and the recorded
missing digest for the earlier file-only executable.

The native live dialogs and their resources still have callers and remain.
No new setup-only callback adapter was added. The Color mapping entry is ready
for ticket 14. Live playback, GPU, physical displays, application restart,
platform, and package acceptance remain unverified. Production presets and
the known HighQuality acceptance limit are unchanged.