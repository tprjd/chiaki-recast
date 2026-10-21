# 02: Account-ID retrieval for registration

Status: resolved
Type: task
Blocked by: 01

**What to build:** Deliver the refreshed registration routes for PSN sign-in and public Account-ID lookup, returning a valid result directly to the intended registration form.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [x] Use the new design and existing production controls. Verify reachable actions at 900 by 600 and 1280 by 720 logical pixels, long text, keyboard/controller navigation, visible focus, return focus, and applicable privacy rules. Keep test fixtures and synthetic data out of production resources.
- [x] Reuse the completed PSN request and browser behavior from 01 for the registration sign-in route, including privacy, closure, recovery, and credential-storage explanation.
- [x] Keep public lookup distinct from sign-in. Explain the third-party service and public-search requirement. Do not save PSN credentials from public lookup.
- [x] Handle empty input, network failure, malformed responses, missing Account IDs, and retry without leaving the form permanently disabled.
- [x] Return success directly to the valid registration caller. Preserve its original console/profile context; closed or replaced callers cannot receive a stale result.
- [x] Verify both paths through the production registration screen and supplied service outcomes. Check that lookup does not write authentication credentials.

## Coverage and dependencies

Primary user stories: 8-13, 36. Shared user stories: 1-7, 14-35, 63.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.

## Answer

Registration reuses `PSNTokenDialog.qml` for sign-in. Public lookup has a separate
form and keeps the existing service. It validates HTTP and JSON responses and
the eight-byte Account ID. A 30-second timeout restores retry for stalled requests.
Only the valid original registration caller receives a result. Closure, profile
changes, replacement screens, and repeated results cannot update another form.

The production registration route uses a real `StackView` in the screen tests.
The supplied lookup returns HTTP status and response text to the production parser.
The supplied authentication backend tests sign-in completion and background work.
Public lookup leaves authentication commands and saved Account ID state unchanged.

The [verification record](../evidence/ticket02/verification.md) links red/green
results, focused regressions, and six inspected captures at both required sizes.
Native persistence is covered by ticket 01. The final native build, visible native
focus, and display scaling remain in ticket 08. Real PSN sign-in, public-service
availability, and physical controllers were not tested.

