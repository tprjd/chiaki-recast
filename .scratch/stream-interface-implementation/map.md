# Stream interface implementation

Breakdown approved and published: 2026-10-21.

Specification: [Stream menu, statistics, and session notices](../stream-interface-refresh/spec.md).
Parent: [Implement the approved stream interface](../stream-interface-refresh/issues/01-stream-interface.md).
The specification includes the user's later acceptance-scope update.

The approved design is A: the descriptive third-revision toolbar and the
second-revision statistics and notices. The design reference is branch
`codex/stream-interface-prototype`, commit `56869701`. The specification was
published in `fff89373`. Keep the prototype out of production integration.

| Ticket | Delivers | Blocked by | Status |
| --- | --- | --- | --- |
| [01: Share the existing stream interface](issues/01-share-stream-interface.md) | Shared production menu and one test boundary, with current behavior preserved | None | resolved |
| [02: Build the approved A toolbar](issues/02-stream-toolbar.md) | Complete descriptive toolbar, pop-ups, settings navigation, audio, Resume, and Disconnect | 01 | resolved |
| [03: Build the approved A statistics overlay](issues/03-stream-statistics.md) | Compact six-value readout with real bindings and visibility rules | 01 | resolved |
| [04: Build the approved A session notices](issues/04-session-notices.md) | Network, blocked-content, and disabled-media notices with actions and cleanup | 01 | resolved |
| [05: Complete native acceptance](issues/05-native-acceptance.md) | Combined native verification, regressions, and normal production build | 02, 03, 04 | resolved |

## Work order

Tickets 01 through 05 are resolved. Combined native acceptance and the final
normal production build are complete within the user-reduced testing scope.

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
| Connection and console login PIN priority, session replacement, no stale windows | 01 baseline; 02â€“04 affected behavior; 05 combined checks |
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

Tickets 02, 03, and 04 were claimed after ticket 01 was resolved.
The parent issue and approved specification remain unchanged.

Ticket [03](issues/03-stream-statistics.md) is resolved in integration merge
`5a8b9404`, with implementation `67c3fa93`. Both renderer routes show the compact
six-value statistics row. Existing sources supply live values and units, finite
fallbacks preserve zero, and session changes clear the old window metrics.
The separate statistics window keeps its native owner and rejects input and focus.
See [statistics evidence](evidence/ticket03/README.md) for production QML results,
Windows focus checks, native renderer captures, source hashes, and build limits.

Tickets 02 and 04 remained claimed after ticket 03 was resolved.
Ticket 05's final normal build must include the recorded post-build shortcut changes.

Ticket [04](issues/04-session-notices.md) is resolved in integration merge
`ac69c8c6`, with final ticket commit `45e29e60`. Both renderer host routes use the
compact audio, network, blocked-content, and disabled-video notices. The existing
threshold, warning timing, disconnect policy, and guarded session actions remain.
Shared production content preserves input ownership and higher-priority dialogs.
See [notice evidence](evidence/ticket04/README.md) for production QML results,
Windows behavior, component captures, source hashes, and the normal native build.

Ticket 02 remained claimed after ticket 04 was resolved. Notice captures do not
establish actual OpenGL or Vulkan composition, fullscreen, physical controllers,
live media, or further display scales. These native limits remain for ticket 05.

Ticket [02](issues/02-stream-toolbar.md) is resolved in integration merge
`81a9550e`, with final ticket commit `c685c01f`. Both renderer routes use the
approved descriptive toolbar, one active flyout, live controls, settings links,
Resume, and the configured disconnect policy. Captured controller keys reach
the owned native menu or popup. The native toolbar window retains compact bounds.
See [toolbar evidence](evidence/ticket02/README.md) for native input checks,
component captures, long-label checks, source hashes, and post-merge QML results.

Ticket [05](issues/05-native-acceptance.md) is resolved. Final native acceptance
passes at 900×600 and measured DPR 1 on both renderers: OpenGL 270 checks and
Vulkan 251 checks. Native input/focus, settings and modal handoffs, session
replacement, desktop composition and the Qt 6.7 compatibility regression pass.
The final normal build excludes QA/test/prototype resources; the original native
cache is restored and rebuilt. Both code review axes are clear. See
[acceptance and replay](evidence/ticket05/acceptance.md) for source identities,
captures, earlier-source coverage and remaining limits. The specification records
the user's later restriction on repeated resolution and scale testing.
