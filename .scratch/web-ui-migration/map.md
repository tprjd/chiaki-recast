# Web UI migration implementation

Status: active
Integration branch: codex/web-ui-migration-integration
Implementation baseline: d0e2f052cb076b7fda85c51f60dab1cc428951fb
Started: 2026-10-22

The user invoked implement-spec for the full approved specification. This
supersedes the earlier planning-only task restriction. Use the approved
[23-ticket graph](ticket-breakdown.md) and [specification](spec.md).

The integration baseline preserves the source workspace, including pending
keyboard and mouse changes. The original main checkout remains unchanged.

## Test policy

Use the approved embedded application boundary and existing native input
checks. Run focused checks for changed behavior and affected callers.
Reuse valid passing results. Broaden or repeat checks only for relevant
changes, failures, or unresolved concerns. Record hardware limitations.

## Frontier

- [01: Application-owned native services](issues/01-application-owned-native-services.md): resolved for the native ownership milestone. Required graphics, live playback, cleanup, and performance acceptance remains assigned to tickets 03, 04, and 23.
- [02: Native stream-only interface](issues/02-native-stream-only-interface.md): resolved for the native stream-root milestone. OpenGL and Vulkan production root/input checks pass. Live media and combined graphics acceptance remains with tickets 03, 04, and 23.
- [03: Accelerated web console library](issues/03-accelerated-web-console-library.md): resolved for the library and graphics foundation milestone. Real bridge/package checks and Windows hardware Chromium plus native Fast-preset GL/Vulkan/recreation/fallback pass. HighQuality/default processing and full hardware acceptance remain in 04 and 23.
- [04: Local native stream handoff](issues/04-local-native-stream-handoff.md): ready after ticket 03 is integrated. Ticket 02 is resolved.
- Tickets 05 through 23: waiting for their declared blockers.

## Integration history

- d0e2f052: preserved current source and approved planning baseline.
- 42bffb7d: recorded the Linux WebEngine build baseline and graphics limits.
- 2ac4e5dc: merged ticket 01 commit 8f1bacb4 without conflicts. The merged native source matches the tested ticket tree. Reused the recorded passing checks without a build or test rerun.
- 9fbc05a2: recorded the compatible Windows native and WebEngine build baseline. Build availability does not establish accelerated coexistence.
- d893e46f: merged ticket 02 branch d5826be0 and source/evidence commit 15bc0f9c without conflicts. The merged native source matches the tested ticket tree. Reused the recorded passing checks without a build or test rerun.

## Decisions

- [Ticket 01](issues/01-application-owned-native-services.md) completes application-owned native services and QML engine lifetime. The pre-edit check established installed module limits and native graphics behavior. It did not prove accelerated WebEngine coexistence or real-console performance. Tickets 03 and 04 retain the required combined graphics acceptance. Ticket 23 retains matched live baseline conditions, predeclared tolerances, live-frame cleanup, and hardware acceptance.

- [Ticket 02](issues/02-native-stream-only-interface.md) extracts `StreamRoot.qml` and removes setup navigation from the native host. `streamViewDismissed` is a presentation request; ticket 04 owns cleanup before setup restoration. [Native verification](evidence/ticket02/verification.md) records actual root, PIN, settings, and input checks.

- [Ticket 03](issues/03-accelerated-web-console-library.md) completes the restricted production bridge and static Svelte library. Its passing Windows hardware GPU fixture selects the existing native Fast preset; production defaults stay unchanged. HighQuality produces black native video in the recorded fixture, including a bounded old-import/no-WebEngine control. This is not a full pre-migration baseline. [04](issues/04-local-native-stream-handoff.md) and [23](issues/23-complete-web-only-acceptance.md) retain default/preset processing acceptance, real handoff focus, live media, hardware, and platform limits. See [verification](evidence/ticket03/verification.md).
