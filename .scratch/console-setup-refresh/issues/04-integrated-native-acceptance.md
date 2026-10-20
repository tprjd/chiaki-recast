# 04: Integrated native acceptance

**What to build:** A verified native console setup flow that matches layout A
from entry through results and return navigation. Check both screens together
in the normal application, close any integration gaps, and record evidence
that the accepted specification works without changing the user's console data.

**Blocked by:** 01: Manual host entry with layout A; 03: Registration progress, results, and safe retries.

**Status:** resolved

**Source:** [Console setup: guided workspace](../spec.md), selected layout A.
The user approved this ticket and its dependencies on 2026-10-20. Ticket 02
is a transitive prerequisite through 03, not an additional direct blocker.

## Acceptance criteria

- [x] Integrate the completed feature slices while preserving the existing
  library changes and Settings redesign. Inspect the current checkout before
  changes. Include only feature-related work in any integration commit.
- [x] Run the existing Qt Quick screen suite and the focused native contract
  checks delivered by the feature tickets. Verify that tests load production
  QML and controls. Resolve regressions caused by this feature.
- [x] Build the normal application. Confirm that production resources and the
  executable exclude prototype files, fake backends, sample data, and temporary
  QA controls. A successful build alone does not complete this ticket.
- [x] Check the native application with temporary settings and synthetic console
  records. Disable discovery and automatic connection in that test environment.
  The user's profiles, registered consoles, and manual hosts remain unchanged.
  Do not use real authentication or network pairing as an implicit test action.
- [x] From the console library, open manual entry, add an address for an existing
  registration, inspect the saved result, and return to the updated library.
  Repeat with registration deferred. Neither path starts a connection or stream.
- [x] Exercise registration from Settings and from a controlled unregistered-console
  route. Verify broadcast defaults, selected host and type, all four console
  choices, and both PSN identity-field modes.
- [x] Check return from the existing account screens using controlled responses,
  including success and cancellation. Verify retained form values and focus.
  Record that controlled return testing does not prove real PSN authentication.
- [x] Drive native registration progress, success, and failure with controlled
  events. Expand logs, correct a failure, retry, and return to the library.
  Confirm that displayed results agree with the isolated settings state.
- [x] Exercise progress dismissal, reopening, duplicate activation, late callbacks,
  and profile changes through the integrated flow. Verify no wrong-target save,
  wrong-profile save, stale navigation, duplicate command, or crash. Confirm that
  Close progress is not presented as native cancellation.
- [x] Exercise manual selection reorder, removal, empty lists, ambiguity, and
  profile changes. Verify recovery without losing the address or accidentally
  converting a removed registration into deferred registration.
- [x] Capture and inspect both forms and all result states at 900 by 600 and
  1280 by 720 logical pixels. Verify A's form and instruction columns, readable
  spacing, reachable actions, and fully visible focused controls. Vertical
  scrolling is acceptable; horizontal clipping is not.
- [x] Check supported display scaling using measured process-level scale factors,
  following the existing native acceptance approach. Include expanded translated
  text and long console names. Do not change global display settings to produce
  evidence. Record the dimensions and scale of each captured check.
- [x] Check mouse, Tab, Shift+Tab, directional controller-key mappings, activation,
  Back, and Escape. Child popups receive Escape first. Input editing remains
  usable, and each return route restores the opener or an appropriate fallback.
  State explicitly that key simulation does not prove physical controller input.
- [x] Check streamer mode on both forms, selectors, help, summaries, and results.
  Expanded diagnostics must not reveal masked private data. Opening, browsing,
  canceling, and account-screen navigation cause no unintended persisted writes.
- [x] Review the final feature diff against the specification and project rules.
  Confirm that no supported registration choice or existing route was lost,
  no unrelated redesign was introduced, and no test-only code remains in the
  normal application. Resolve actionable feature findings before completion.
- [x] Remove temporary QA instrumentation after the controlled native checks.
  Build the final normal executable and verify that its app window opens using
  the existing launcher with runtime paths. Process liveness alone is insufficient.
  Do not submit registration against the user's real data for this final launch.
- [x] Save a concise acceptance record containing the tested revision, test and
  build results, inspected native captures, isolated persistence evidence, and
  remaining limits. Keep real PSN authentication, real console pairing, and
  physical controller hardware explicitly unverified unless separately exercised.

## Completion evidence

The completion record must make it possible to distinguish source review,
automated screen tests, real native contracts, and inspected native UI evidence.
Show that all required feature behaviors pass in the integrated application.
Do not substitute the browser prototype's 42 layout checks for native acceptance.

This ticket owns integration corrections and final verification. It does not
defer the feature tickets' own behavior tests until the end. A native limitation
must be stated with its effect rather than reported as a passing check.

## Scope and preservation

The accepted design is A for two separate screens. No merge into a wizard,
PSN-authentication redesign, stream-view work, or sub-900-pixel layout is included.
The approved acceptance approach uses isolated settings and controlled events.
Hardware pairing is not required, and no data migration or deletion is authorized
as a way to make the checks pass.

## Answer

Integrated acceptance passed at production revision `a19f26f1`. The only ticket
04 source correction aligns the registration header, margins, and button
padding with manual entry. The final complete screen suite passed 627 checks.
Native contract and post-exit INI checks passed. The native UI runs passed 111
framebuffer captures at both window sizes and measured DPR 1, 1.5, and 2.
Expanded text, privacy, focus, route returns, controlled outcomes, dismissal,
late callbacks, and profile changes passed.

QA controls and isolation were removed from the staged production source.
The normal build passed. Independent Computer Use verified the visible normal
application and its manual-entry route. No registration, account request, or
connection was submitted. The user can inspect the window left open.

See the [acceptance record](../native-acceptance.md) and
[capture manifest](../native-acceptance-manifest.json) for the tested source
hashes, logs, captures, actual saved arrays, and remaining limits. Real
authentication, pairing, and physical controller hardware remain unverified.
