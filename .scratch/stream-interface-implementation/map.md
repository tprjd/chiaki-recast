# Stream interface implementation

Breakdown approved and published: 2026-10-21.

Specification: [Stream menu, statistics, and session notices](../stream-interface-refresh/spec.md).
Parent: [Implement the approved stream interface](../stream-interface-refresh/issues/01-stream-interface.md).
Both source records remain unchanged.

The approved design is A: the descriptive third-revision toolbar and the
second-revision statistics and notices. The design reference is branch
`codex/stream-interface-prototype`, commit `56869701`. The specification was
published in `fff89373`. Keep the prototype out of production integration.

| Ticket | Delivers | Blocked by | Status |
| --- | --- | --- | --- |
| [01: Share the existing stream interface](issues/01-share-stream-interface.md) | Shared production menu and one test boundary, with current behavior preserved | None | resolved |
| [02: Build the approved A toolbar](issues/02-stream-toolbar.md) | Complete descriptive toolbar, pop-ups, settings navigation, audio, Resume, and Disconnect | 01 | claimed |
| [03: Build the approved A statistics overlay](issues/03-stream-statistics.md) | Compact six-value readout with real bindings and visibility rules | 01 | claimed |
| [04: Build the approved A session notices](issues/04-session-notices.md) | Network, blocked-content, and disabled-media notices with actions and cleanup | 01 | claimed |
| [05: Complete native acceptance](issues/05-native-acceptance.md) | Combined native verification, regressions, and normal production build | 02, 03, 04 | ready-for-agent |

## Work order

Ticket 01 is resolved. Tickets 02, 03, and 04 are claimed for implementation.
They use the same production test boundary but do not require one another's
completed feature. Ticket 05 remains blocked by 02, 03, and 04.

Shared source edits may need coordination. That is not an extra blocking edge.
Each feature ticket must pass its own production QML checks before resolution;
ticket 05 does not replace those checks. Do not start implementation merely
because these tickets have been published.

## Requirement coverage

| Spec area | Owner |
| --- | --- |
| Shared menu content, native defaults, one supplied-state test boundary, baseline behavior | 01 |
| Toolbar design, opacity, labels, volume, microphone, picture, zoom, presets, linked settings, Resume, Disconnect | 02 |
| Pop-up geometry, navigation, focus, input ownership, configured disconnect policy | 02, with combined native acceptance in 05 |
| Six metrics, units, finite values, zero values, preference, visibility, passive input | 03 |
| Network thresholds and timing, audio/video flags, blocked-content action, notice stacking and cleanup | 04 |
| Connection and console login PIN priority, session replacement, no stale windows | 01 baseline; 02–04 affected behavior; 05 combined checks |
| Both renderer routes, minimum size, translation, accessibility, privacy | Each affected feature; 05 native verification |
| Native transparency, move/resize/fullscreen, scaling, focus, capture limits, regression suite, production resources | 05 |
| No new protocol, schema, saved opacity setting, metrics, or redesigned linked screens | All tickets inherit specification exclusions |

## Acceptance limits

Use isolated settings and controlled session state. Production QML checks and
native application checks are required. Intercept transport commands. Record
physical-controller, live-console, unavailable-renderer, and desktop-capture
limits explicitly. Browser prototype checks do not establish native acceptance.

## Comments

Implementation started on `codex/stream-interface-integration` from production
commit `746218af`. The approved specification and tickets are recorded in
`401559f7`. The prototype remains on its reference branch. Ticket 01 was claimed.
The parent issue remains open.

Ticket [01](issues/01-share-stream-interface.md) is resolved in integration merge
`fe4850b8`, with implementation `c07c4421`. Both renderer routes use the shared
production menu. `StreamView` provides one supplied-state boundary for the menu,
statistics, and notices. The current presentation remains in place.
See [baseline evidence](evidence/ticket01/README.md) for passing production QML,
controlled native checks, normal build records, and verification limits.

Tickets 02, 03, and 04 are now claimed. Ticket 05 remains blocked by all three.
The parent issue and approved specification remain unchanged.
