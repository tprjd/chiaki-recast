# 01: Manual host entry with layout A

**What to build:** A native guided workspace for adding a manual console address.
The user can associate the address with an existing console registration or
choose registration on first connection. The screen shows a saved result or
a recoverable error and returns to the console library without connecting.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

**Source:** [Console setup: guided workspace](../spec.md), selected layout A.
The user approved this ticket and its dependencies on 2026-10-20.

## Acceptance criteria

- [ ] The library's existing Add console action opens the redesigned native
  manual-entry screen. It uses A's left form panel, right instruction column,
  and the completed library and Settings visual conventions. It retains one
  application navigation owner rather than mounting another independent sidebar.
- [ ] The form contains a console address and registered-console selector.
  Help explains manual addresses, existing registrations, and registration on
  first connection. New UI text uses the existing translation mechanism.
- [ ] The address accepts the existing IP-address and hostname forms. Empty
  or whitespace-only input prevents submission. Trim the address at dispatch
  without imposing an IPv4-only validator or checking network reachability.
- [ ] Existing registrations have clear identities, including enough information
  to distinguish equal names. When registrations exist, require a deliberate
  choice. Keep Register on first connection available. With an empty list,
  deferred registration remains usable and can be the default.
- [ ] Selection follows stable identity through list reordering. Resolve the
  current native index at submission. Removal, ambiguity, or a profile change
  invalidates the selected registration without silently selecting another.
  Retain the entered address and show a recoverable error.
- [ ] An explicit deferred-registration choice uses the existing native
  unregistered-host behavior. An invalidated existing-console choice must not
  accidentally become deferred registration through an out-of-range index.
- [ ] Add console sends one local save action with the intended registration
  and trimmed address. Repeat activation cannot duplicate the save. Opening,
  editing, canceling, and navigating the form do not write settings.
- [ ] After the local action completes, show the saved result and relevant
  console details. Deferred registration explicitly states that pairing opens
  on first connection. Back to consoles returns to the updated library without
  a connection, wake, or streaming command.
- [ ] An actual rejected input, invalidated selection, or exposed backend
  failure produces a recoverable error. Back to details retains the address
  and any still-valid selection. Do not fabricate disk-write failure details
  or promise durable storage that the current backend does not report.
- [ ] Local saving has no simulated network progress, registration milestones,
  percentage, or artificial delay. Preserve existing persistence semantics and
  storage format. Any necessary native return-status change updates all callers.
- [ ] Cancel, Back, and Escape return through the existing caller route without
  saving. Child popups handle Escape first. Restore the opener's focus or a
  valid fallback when the opener no longer exists.
- [ ] Mouse, Tab, Shift+Tab, and existing controller-key conventions can reach
  every control and result action. Focus is visible, editing keys stay in their
  fields, and scrolling reveals the whole focused control. Controls have useful
  accessible names and result changes receive a useful focus target.
- [ ] Form, saved result, and failure fit the 900-pixel minimum width and work
  at 900 by 600 and 1280 by 720 logical pixels. Vertical scrolling is allowed.
  Long labels, help, and console names wrap without hiding actions.
- [ ] Streamer mode masks private addresses and identifiers throughout the
  form, selector, and results. Console-name privacy matches the library.
  Any diagnostic display respects masking.
- [ ] Production-QML screen tests exercise input, selection, dispatch arguments,
  action counts, results, no-write navigation, focus, and privacy through user
  actions. Cover existing and empty registration lists, deferred registration,
  equal names, reorder, removal, ambiguity, and profile changes, including a
  change away and back to the original profile.
- [ ] Tests use the existing Qt Quick runner and Settings-style injected backend
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
