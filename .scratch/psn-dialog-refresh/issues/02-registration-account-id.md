# 02: Account-ID retrieval for registration

Status: ready-for-agent
Type: task
Blocked by: 01

**What to build:** Deliver the refreshed registration routes for PSN sign-in and public Account-ID lookup, returning a valid result directly to the intended registration form.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [ ] Use the new design and existing production controls. Verify reachable actions at 900 by 600 and 1280 by 720 logical pixels, long text, keyboard/controller navigation, visible focus, return focus, and applicable privacy rules. Keep test fixtures and synthetic data out of production resources.
- [ ] Reuse the completed PSN request and browser behavior from 01 for the registration sign-in route, including privacy, closure, recovery, and credential-storage explanation.
- [ ] Keep public lookup distinct from sign-in. Explain the third-party service and public-search requirement. Do not save PSN credentials from public lookup.
- [ ] Handle empty input, network failure, malformed responses, missing Account IDs, and retry without leaving the form permanently disabled.
- [ ] Return success directly to the valid registration caller. Preserve its original console/profile context; closed or replaced callers cannot receive a stale result.
- [ ] Verify both paths through the production registration screen and supplied service outcomes. Check that lookup does not write authentication credentials.

## Coverage and dependencies

Primary user stories: 8-13, 36. Shared user stories: 1-7, 14-35, 63.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.

