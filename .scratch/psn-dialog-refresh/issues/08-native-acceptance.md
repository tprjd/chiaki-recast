# 08: Integrated native acceptance

Status: resolved
Type: task
Blocked by: 02, 03, 04, 05, 06, 07

**What to build:** Deliver a verified normal application build in which all refreshed routes work together, with reproducible acceptance evidence and explicit external-service limits.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [x] Build the normal application with the production resources and without test fixtures. Integrate the completed slices and fix cross-flow regressions within the specification.
- [x] Run the focused feature tests and relevant Library, Settings, registration, connection, and PIN regressions. Feature tests are prerequisites from their own tickets, not work deferred to this ticket.
- [x] Exercise Settings/startup/expiry sign-in, both browser selections, registration return, sign-in closure and re-entry, reminder sequencing after Steam/PSN results, and console-management return routes together.
- [x] Complete visible native checks at 900 by 600 and 1280 by 720, compact-window sizes, and display scaling. Check actual focus, input capture/release, closure, and both renderer-dependent disconnect routes. Final desktop composition remains unverified, as recorded below.
- [x] Use isolated settings, synthetic credentials, temporary Steam data, and controlled session/service responses. Verify the tested source and build and retain sanitized evidence.
- [x] Separate controlled outcomes from real PSN sign-in, real Steam visibility after restart, console sleep/disconnect, and physical-controller results. Clearly record external or hardware checks not performed.

## Coverage and dependencies

User stories: integration coverage for 1-63, with emphasis on 1-7 and 63.

Ticket 01 is a transitive blocker through 02. No redundant direct edge is needed.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.

## Answer

Resolved on 2026-10-21 from integration base `d5655228`.
The [acceptance record](../acceptance.md) links the results, source manifests,
rendered captures, environment limits, and replay commands.

- All 38 production screen suites pass: 869 checks, no failures or skips.
- Integrated native checks pass 297 checks. They cover real isolated PSN saves,
  closed and replaced registration forms, Settings and expiry routes, startup
  reminders, actual temporary Steam writes, and About and renderer focus.
- Actual Vulkan and OpenGL disconnect routes pass 309 and 329 checks at DPR 1.25.
  Both required sizes, the compact window, native input capture, closure, and
  controlled stop and sleep dispatch pass.
- The native Steam command passes 31 assertions with temporary files and actual file failures.
- The normal executable builds from all 332 matching staged inputs and eight pinned
  submodules. Its 72 resources include every changed screen and exclude fixtures.
  QA symbols are absent. `--help` exits with code 0 before Settings construction.
  The [build record](../evidence/ticket08/normal-build.json) includes executable and DLL hashes.
- The shared cache has all 332 original source hashes and original build options.
  Both original targets rebuilt. The final executable and DLL remain unchanged.

Acceptance exposed a Qt 6.11 keyboard regression after a popup closed.
The opener retained PopupFocusReason, so the custom button's Return handler did nothing.
The shared confirmation now clears the opener's focus before Tab focus restoration.
The Steam controller notice also releases its popup focus first.
Existing tests now check visible focus and Return reopening. Native checks confirm
the same behavior. The prompts keep their existing design and operation callbacks.

The normal build and adjacent `launch-chiaki.ps1` are in
`C:/Users/david/.cache/chiaki-ui-native/psn-dialog-final-bin/`.
The [retained launch source](../evidence/ticket08/launch-final.ps1) sets the dependency paths.

All native service replies and session commands are controlled. Credential saves
and temporary Steam writes are real. The native build has no QtWebEngine, so its
embedded route falls back to the external route. A supplied browser covers the
embedded route in screen tests. Real services, browser navigation, Steam visibility,
console power effects, and physical controllers remain untested.

Final desktop composition remains unverified. Retained captures use actual GPU
targets or the separate QQuickWindow, with native focus and window evidence.
Earlier GDI and Windows Graphics Capture images were stale and are excluded.
This resolution covers the attainable controlled checks and states those limits.
No push or PR is part of this ticket. The parent task owns the final two-axis review.

### Final review follow-up

The [review fix verification](../evidence/review-fixes/README.md) records both final review fixes. Shared dialogs now scroll long text with Page Up and Page Down from the focused actions. Steam completion uses the existing PSN reminder function. The final-source affected suites pass 142 checks. Native runs pass 297 integrated, 337 Vulkan, and 357 OpenGL checks. The normal executable and DLL hashes are refreshed. The shared cache is restored and its original targets rebuilt. The earlier complete 869-check screen run remains prior-source evidence for unchanged paths. The parent task owns the final review resolution and specification status.
