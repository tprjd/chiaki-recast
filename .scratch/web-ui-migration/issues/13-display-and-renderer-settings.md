# 13: Configure display targets and the native renderer

**What to build:** Users can configure display targets, renderer selection, presets, and custom native renderer options from web setup.

**Blocked by:** 11: Open settings and change general session options.

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 50, 51, 52, 56, 75.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] The web UI retains existing display controls, renderer choices, presets, and custom renderer fields with their actual ranges and conditions.
- [ ] Values are converted and validated by the existing native settings logic and persist in the existing store.
- [ ] Renderer changes retain the existing restart behavior and actual runtime renderer reporting. A saved choice does not falsely report a live switch.
- [ ] This ticket owns renderer settings import, export, and documentation entry points through native dialogs and services. File contents do not pass through the frontend. General settings and profile file actions belong to their own ticket.
- [ ] Cancelled or failed renderer file actions preserve existing values and report the actual result. Validate the import before replacing saved values; a web wrapper alone does not establish failure safety.
- [ ] Live stream display and renderer settings stay native and use the same saved values and notifications.
- [ ] Embedded tests verify native writes, rejected values, preset changes, restart requirements, renderer import and export, failed or cancelled file actions, and consistency with native live settings.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.
