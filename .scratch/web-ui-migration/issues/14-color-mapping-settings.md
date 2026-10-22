# 14: Configure native color mapping from web setup

**What to build:** Users can open color mapping from renderer settings and configure the existing native tone and color mapping options.

**Blocked by:** 13: Configure display targets and the native renderer.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 50, 51, 52, 56, 75.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] All existing color mapping controls retain their labels, ranges, choices, defaults, dependencies, and capability conditions.
- [x] Validated native setters own persistence and renderer notifications. The browser does not implement color or HDR processing.
- [x] Settings search reaches the linked page and returns to the expected renderer category and focus.
- [x] The native live color mapping page reads and changes the same settings without a second store.
- [x] Embedded tests verify representative dependent controls, invalid values, saved native values, and linked search navigation.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Comments

- Claimed after native renderer settings. Reuse the guarded native settings interface and shared fields. Preserve the native live color mapping page and renderer processing.

## Answer

Web setup now has Color Mapping Enabled and Color Mapping Preset in Renderer,
with a linked page for all 28 existing gamut and tone controls. Public metadata
retains labels, choice order, ranges, steps, default hints, and dependencies.
Native initialization and getter fallbacks are unchanged. Existing native
setters own persistence and renderer notifications. Profile generation guards
reject obsolete writes. No browser color processing or second store was added.

Search focuses a missing parent option before a dependent control. Back and
Escape return to Renderer and its opener. Native adapter changes update web
fields. Numeric fields show the precision of their existing step, so native
float values do not expose long representation digits. Shared profile draft
guards and conversion factors remain.

The embedded color, Video and Audio, and Keyboard and mouse cases pass 5/0.
The separate process persistence reader passes 3/0. The corrected layout case
passes 3/0, with all four captures inspected. Earlier renderer and shared search
cases pass and are reused. The merged target builds, and the frontend check
reports no errors or warnings. The independent audit checks 28 controls and
28 preserved source comparisons. Exact source identities, executable digests,
commands, failures, fixes, and results are in
[ticket 14 evidence](../evidence/ticket14/verification.md).

The native live QML pages still have StreamRoot callers and remain. No unused
setup-only leaf was removed. Ticket 08 request constructors and native remote
operations are retained after the integration merge. Native media, renderer
restart and file safety, keyboard capture, and console management remain.

Checks used offscreen rendering and a test-only GPU-disable flag. They do not
establish live playback, HDR output, GPU acceleration, physical focus, latency,
or platform package acceptance. The earlier fixtures do not establish zero
LAN traffic during backend construction or cleanup. See the evidence limits.

The user later limited supported layout acceptance to at least 1024 CSS pixels.
The inspected 1280-pixel capture is in scope. The completed 900-pixel,
390-pixel, and 200 percent capture at 400 CSS pixels are historical evidence.
No further narrow work or rerun followed that decision. A 200 percent viewport
of at least 1024 CSS pixels remains unverified in this ticket.
