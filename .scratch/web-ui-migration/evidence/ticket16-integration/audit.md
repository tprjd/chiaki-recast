# Ticket 16 integration audit

Date: 2026-10-22

Integration started clean at `5cd421067e06eb803e21ecbd1dcc1b97c97a8804`.
The explicit `--no-ff` merge of worker
`382f078450e98382fa6edc268c0c6ac5bca5655b` completed without conflicts at
`56acff7791d1bb49ad4ade2e063f281a6b4bbbc4`.
The complete merge tree equals the worker tree.

## Independent checks

[Source and artifact identities](identities.json) record these direct checks:

- All 189 entries match the pre-run `source-sha256.csv`. Of these, 187 match
  byte for byte. The new `navigation.ts` and `Controllers.svelte` match after
  CRLF-to-LF conversion. Their Git blobs equal the worker blobs.
- Both current test executables and the compiled JavaScript and CSS match
  the four recorded SHA256 values in `source-preservation.json`.
- The saved source manifest, two application logs, named QML log, and normal
  capture equal the external pre-run records. Five SHA256 comparisons pass.
- All 21 protected native source files equal the integration
  baseline. These include controller input, stream session, input ownership,
  native window, application lifetime, settings, `StreamRoot`, and live dialogs.
- Seven bounded source comparisons pass. These cover the backend constructor,
  session release, native mapping methods, keyboard and mouse bridge methods,
  media writes, renderer writes, and color mapping field definitions.

The constructor retains `PsnRequests`, `ProcessLauncher`,
`RegistrationRequests`, then `DiscoveryRequests`. Ticket 07 registration and
ticket 08 discovery adapters remain in that order.

## Source review

The reviewed change reaches the shared settings page, bridge commands and
state, native controller navigation, setup lifecycle, QML resources, and
affected application and QML tests. The native mapping methods retain capture,
conflict movement, apply, reset, and cancel. The browser receives display data
and restricted commands with an operation identity and profile generation.

Selection and capture disable controller navigation. A queued generation check
and held-button release guard resume navigation after the SDL event. Native
window targets retain their visibility rule. Setup uses the embedded widget,
and native modal dialogs retain the active focus target. Mapping requires no
native stream window or session. Connect and retry refuse an active mapping.
Profile changes, reload, setup hide, selected-device removal, and native window
attachment end the operation. Removing another device preserves it.

`StreamRoot` has no route to the removed setup mapping dialog. Its live display,
renderer, color mapping, and keyboard and mouse routes remain. ADR 0001 source
and tests remain. Ticket 04 session release and ticket 15 native capture remain.
The shared Settings search changes remove mapping routes and retain the other
categories. The local `gf-complete` vendor patch is excluded.

## Reused verification and limits

The inspected logs pass settings and mapping 4/0, final SDL mapping 3/0, and
the three named QML cases 5/0. Counts include initialization and cleanup.
The worker records a frontend check with no errors or warnings and successful
embedded and QML target builds. The saved final build log links the executable
and reports unused-capture and unused-function warnings. No build or test ran
for this integration.

The normal 1280 by 900 image shows readable labels, primary and alternate
columns, visible focus, and scrolling in the visible area. Lower rows and
actions are outside this capture. The 200 percent image is INVALID visual
evidence because it contains stale backing pixels. The DOM check alone does
not establish zoom visual acceptance. No narrower-width work was added.

The runtime records use offscreen rendering and a test-only GPU-disable flag.
Physical controllers, haptics, calibration, foreground focus, actual console
input suppression, GPU coexistence, live playback, platforms, packages, and
latency remain acceptance work. Ticket 23 also retains zoom visual acceptance.
The imported evidence has whitespace warnings, including patch context lines
and extra blank lines. Production `git diff --check` passes. Evidence bytes
remain unchanged so their source and artifact records can still be compared.
