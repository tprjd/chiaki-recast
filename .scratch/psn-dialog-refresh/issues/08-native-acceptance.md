# 08: Integrated native acceptance

Status: claimed
Type: task
Blocked by: 02, 03, 04, 05, 06, 07

**What to build:** Deliver a verified normal application build in which all refreshed routes work together, with reproducible acceptance evidence and explicit external-service limits.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [ ] Build the normal application with the production resources and without test fixtures. Integrate the completed slices and fix cross-flow regressions within the specification.
- [ ] Run the focused feature tests and relevant Library, Settings, registration, connection, and PIN regressions. Feature tests are prerequisites from their own tickets, not work deferred to this ticket.
- [ ] Exercise Settings/startup/expiry sign-in, both browser selections, registration return, sign-in closure and re-entry, reminder sequencing after Steam/PSN results, and console-management return routes together.
- [ ] Complete visible native checks at 900 by 600 and 1280 by 720, compact-window sizes, and display scaling. Check actual focus, input capture/release, closure, and both renderer-dependent disconnect routes.
- [ ] Use isolated settings, synthetic credentials, temporary Steam data, and controlled session/service responses. Verify the tested source and build and retain sanitized evidence.
- [ ] Separate controlled outcomes from real PSN sign-in, real Steam visibility after restart, console sleep/disconnect, and physical-controller results. Clearly record external or hardware checks not performed.

## Coverage and dependencies

User stories: integration coverage for 1-63, with emphasis on 1-7 and 63.

Ticket 01 is a transitive blocker through 02. No redundant direct edge is needed.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.
