# 01: Manual host entry with layout A

**What to build:** A native guided workspace for adding a manual console address.
The user can associate the address with an existing console registration or
choose registration on first connection. The screen shows a saved result or
a recoverable error and returns to the console library without connecting.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Source:** [Console setup: guided workspace](../spec.md), selected layout A.
The user approved this ticket and its dependencies on 2026-10-20.

## Acceptance criteria

- [x] The library's existing Add console action opens the redesigned native
  manual-entry screen. It uses A's left form panel, right instruction column,
  and the completed library and Settings visual conventions. It retains one
  application navigation owner rather than mounting another independent sidebar.
- [x] The form contains a console address and registered-console selector.
  Help explains manual addresses, existing registrations, and registration on
  first connection. New UI text uses the existing translation mechanism.
- [x] The address accepts the existing IP-address and hostname forms. Empty
  or whitespace-only input prevents submission. Trim the address at dispatch
  without imposing an IPv4-only validator or checking network reachability.
- [x] Existing registrations have clear identities, including enough information
  to distinguish equal names. When registrations exist, require a deliberate
  choice. Keep Register on first connection available. With an empty list,
  deferred registration remains usable and can be the default.
- [x] Selection follows stable identity through list reordering. Resolve the
  current native index at submission. Removal, ambiguity, or a profile change
  invalidates the selected registration without silently selecting another.
  Retain the entered address and show a recoverable error.
- [x] An explicit deferred-registration choice uses the existing native
  unregistered-host behavior. An invalidated existing-console choice must not
  accidentally become deferred registration through an out-of-range index.
- [x] Add console sends one local save action with the intended registration
  and trimmed address. Repeat activation cannot duplicate the save. Opening,
  editing, canceling, and navigating the form do not write settings.
- [x] After the local action completes, show the saved result and relevant
  console details. Deferred registration explicitly states that pairing opens
  on first connection. Back to consoles returns to the updated library without
  a connection, wake, or streaming command.
- [x] An actual rejected input, invalidated selection, or exposed backend
  failure produces a recoverable error. Back to details retains the address
  and any still-valid selection. Do not fabricate disk-write failure details
  or promise durable storage that the current backend does not report.
- [x] Local saving has no simulated network progress, registration milestones,
  percentage, or artificial delay. Preserve existing persistence semantics and
  storage format. Any necessary native return-status change updates all callers.
- [x] Cancel, Back, and Escape return through the existing caller route without
  saving. Child popups handle Escape first. Restore the opener's focus or a
  valid fallback when the opener no longer exists.
- [x] Mouse, Tab, Shift+Tab, and existing controller-key conventions can reach
  every control and result action. Focus is visible, editing keys stay in their
  fields, and scrolling reveals the whole focused control. Controls have useful
  accessible names and result changes receive a useful focus target.
- [x] Form, saved result, and failure fit the 900-pixel minimum width and work
  at 900 by 600 and 1280 by 720 logical pixels. Vertical scrolling is allowed.
  Long labels, help, and console names wrap without hiding actions.
- [x] Streamer mode masks private addresses and identifiers throughout the
  form, selector, and results. Console-name privacy matches the library.
  Any diagnostic display respects masking.
- [x] Production-QML screen tests exercise input, selection, dispatch arguments,
  action counts, results, no-write navigation, focus, and privacy through user
  actions. Cover existing and empty registration lists, deferred registration,
  equal names, reorder, removal, ambiguity, and profile changes, including a
  change away and back to the original profile.
- [x] Tests use the existing Qt Quick runner and Settings-style injected backend
  and navigation pattern. Production defaults remain real application objects.
  If native behavior changes, a focused contract check verifies the real local
  save boundary with temporary settings. A fake backend alone is not that proof.

## Completion evidence

Demonstrate saving an address for an existing registration and for deferred
registration. Demonstrate a removed selection that produces an error without a
write, then correction and successful submission. Record focused test results
and native screen captures or clearly state which native checks remain for 04.
Tests for this ticket's behavior must pass before the ticket is complete.

## Scope and preservation

This ticket does not redesign console registration, PSN sign-in, discovery,
streaming, or existing library pages. Reuse the current controls and test patterns.
Add only shared layout elements that this screen actually needs. Ticket 02 can
start independently and must not depend on an anticipated shared component.

The selected prototype is preserved on `prototype/console-setup-guided-workspace`
at `d3b02f91db69c6b6dca2c89a69036da5edc74b4d`. It supplies layout guidance,
not sample data or fake actions for production. Preserve the existing dirty
checkout and completed Settings work. Real console pairing is not an acceptance
requirement for this ticket.
## Answer

Implemented the native guided workspace in `ManualHostDialog.qml`. The screen
uses the existing `ConsoleButton`, text field, and selector controls. Production
backend and navigation defaults remain the native application objects.

The selected registration follows its MAC identity. The screen resolves the
current native index only when the user selects Add console. Removed or duplicate
identities and profile changes clear the selection and show a recoverable error.
The address remains available. Deferred registration is an explicit choice and
uses the existing out-of-range native index convention.

The local save shows a saved result without network progress or a connection
command. An exposed rejection shows a neutral failure and retains valid details.
The C++ save boundary and persistence contract remain unchanged. Streamer mode
masks addresses and MAC identities in fields, options, and results.

### Evidence

- Applied TDD at the approved complete-screen boundary. Recorded failures before
  implementing deferred save, stable selection, invalidation, profile changes
  during failure, privacy for address names, and long option text.
- Built the standalone Qt Quick screen runner in `build/manual-screen` with
  Qt 6.9.3 and MSVC x64.
- `chiaki-console-screen-tests.exe -input gui/tests/qml/tst_ManualHost.qml`
  passed 23 checks, with no failures or skipped checks.
- Covered trimmed hostname and IPv6 input, empty addresses, existing and empty
  registration lists, explicit deferred registration, equal names, reordering,
  removal, ambiguity, profile changes away and back, rejection and correction,
  repeated activation, mouse selection, keyboard and controller keys, Escape,
  focus visibility, long names, privacy, and rendered results.
- Inspected production-QML captures of form, saved result, and failure at
  900 by 600 and 1280 by 720. Local captures are in
  `build/manual-screen/manual-{900,1280}-{form,saved,failure}.png`.

The native application build, library opener focus, isolated native persistence,
and display scaling checks remain in ticket 04. Physical controller input and
real pairing were not checked. The saved result follows the existing local
settings action; it does not promise a durable disk-write result.

### Integration follow-up

A direct integration run omitted `QT_QPA_FONTDIR`. The supported Windows test
environment passed. The fallback font produced an action wider than the window.
Longer labels also reproduced clipping with the required fonts.

The manual actions now wrap within the panel and stack when the row cannot fit.
Content-size changes also reveal the focused control. The test checks the full
control against the Flickable viewport, including the clipped top edge.

The existing full screen run passed 586 checks with no failures or skipped
checks. The final focused manual suite passed 23 checks after integration.
The two focus-size cases also passed without the Windows font directory.
Form and result captures retain layout A.
