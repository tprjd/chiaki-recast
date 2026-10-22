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
- [04: Local native stream handoff](issues/04-local-native-stream-handoff.md): resolved for local connection and native lifetime. Real decoded GL/Vulkan cleanup, initialization recovery, hidden-page resume, sign-in close, and direct native caller checks pass. Live-console, actual focus, full presets/platforms and hardware acceptance remain in 23.
- [06: PSN sign-in](issues/06-psn-sign-in.md): resolved for standalone web account status and native embedded/external sign-in. Native transport, persistence, cookie rules and responsive checks pass. Live PSN and visible desktop acceptance remain outside this evidence.
- [11: General and stream settings](issues/11-general-and-stream-settings.md): resolved for the settings shell and general session controls. Real-bridge writes, rejection, search/focus, privacy, reload, and separate-process persistence pass. Physical and platform acceptance remains in the later acceptance tickets.
- [12: Video and audio settings](issues/12-video-and-audio-settings.md): resolved for native Video/Stream/Audio settings. Real-bridge writes, validation, profile drafts, device refresh through a controlled SDL driver, reload/restart and layout checks pass. Hardware/platform acceptance remains in later tickets.
- [13: Display and native renderer settings](issues/13-display-and-renderer-settings.md): resolved for display targets, native renderer controls, and safe native file actions. Focused bridge, file preservation, native QML caller, search, and layout checks pass. Immediate native restart policy and launch failure feedback also pass. Live display, GPU, successful OS relaunch, and platform acceptance remain open.
- [15: Keyboard and mouse controls](issues/15-keyboard-and-mouse-controls.md): resolved.
- [14: Native color mapping](issues/14-color-mapping-settings.md): claimed after ticket 13.
- Tickets [16](issues/16-controller-setup-and-mapping.md) and [17](issues/17-profiles-files-and-logs.md): ready after ticket 11.
- [18: Setup reminders and Steam shortcuts](issues/18-reminders-and-steam-shortcuts.md): ready after ticket 06.
- [05: Console management and manual addresses](issues/05-console-management-and-manual-addresses.md): resolved for guarded details/actions, manual saves/association, saved login PIN edits, and registered manual native handoff. Embedded validation, privacy, focus, native state, reload, and handoff checks pass. Live-console, physical access, platform, and hardware acceptance remains in 19 through 23.
- [07: Pairing and Account ID lookup](issues/07-pairing-and-account-id.md): claimed. Tickets 04 and 06 are integrated; the isolated worktree and native dependencies are prepared.
- [08: PSN native streaming](issues/08-psn-native-streaming.md): claimed after tickets 04 and 06. The isolated worktree is being prepared.
- Tickets [19](issues/19-windows-web-native-package.md), [20](issues/20-linux-web-native-packages.md), and [21](issues/21-macos-web-native-package.md): ready after ticket 04.
- Other tickets 05 through 23: waiting for their declared blockers.

## Integration history

- d0e2f052: preserved current source and approved planning baseline.
- 42bffb7d: recorded the Linux WebEngine build baseline and graphics limits.
- 2ac4e5dc: merged ticket 01 commit 8f1bacb4 without conflicts. The merged native source matches the tested ticket tree. Reused the recorded passing checks without a build or test rerun.
- 9fbc05a2: recorded the compatible Windows native and WebEngine build baseline. Build availability does not establish accelerated coexistence.
- d893e46f: merged ticket 02 branch d5826be0 and source/evidence commit 15bc0f9c without conflicts. The merged native source matches the tested ticket tree. Reused the recorded passing checks without a build or test rerun.
- fd207526: merged ticket 03 commit ec92fb73 without conflicts. The merged tree matches the worker tree. Source, package executable, frontend lock, and image digests match the recorded evidence. Production source matches the passing library build 0e2702b7; final graphics source matches passing build 713b5861. Reused the passing checks without builds or test reruns. HighQuality/default processing, actual handoff focus, initialization failure, live media, and platform hardware acceptance remain in tickets 04 and 23.
- 322c486b: merged ticket 11 commit ceae3eb7 without conflicts. The merged tree and all 14 recorded source, lock, executable, JavaScript, and CSS hashes match the tested artifacts. Reused save/reload 3/0 and validation/restart 4/0 without builds or test reruns. The checks used offscreen rendering and a test-only GPU-disable flag. Physical focus, visual layout, Deck hardware, live session effects, platform packages, and GPU acceptance remain open. Ticket 03 graphics evidence is unchanged.
- abd776d6: merged ticket 06 commit a75ebe09 without conflicts. The merged source matches tested source d1f643d7. All 32 recorded source, lock, executable, asset, and capture hashes match. Read the five focused case results and inspected all four captures. Reused passing evidence without builds or test reruns. Ticket 11 settings and shared keyboard guards remain. No live PSN, physical focus, or GPU claim is made. Ticket 03 graphics evidence is unchanged. Ticket 04 must close the optional sign-in window in SetupWindow::suspend before setup hides or freezes.
- fd41b8d2: merged ticket 12 commit cca37231 without conflicts. The merged source matches tested source 3f19a3c6; production source matches b3dbb435. All 16 recorded source/artifact/patch hashes and four compared capture hashes match. Reused expanded 3/0, state/restart/general 5/0, restart child 3/0, and controlled SDL dummy device/layout 4/0 without builds or test reruns. Ticket 06 account and ticket 11 general controls remain. Physical audio, hotplug, GPU, live playback, and platform acceptance remain open.
- 53295e54: merged ticket 04 commit 3e5bc93b without conflicts. GUI source matches final build effd472a. All 45 final source/artifact/lock/patch hashes and 29 independent source comparisons pass. Read and reused the focused handoff, initialization, decoded-cleanup, sign-in suspend, and direct-native results without builds or test reruns. Earlier initialization/decoded binary digests remain unavailable. Ticket 06 sign-in closure, suspended bridge emission guard, ticket 12 media fields, and ticket 15 claim remain. Closure covers the local handoff and native lifetime milestone. Unchecked real-console, actual focus, video-disabled, default/full-preset, supported-platform GPU, hardware, and performance acceptance remains in ticket 23.

- 7e29e313: merged ticket 13 commit fe0cfb0f without conflicts. GUI source matches compiled restart source 5a83520f. All 20 final source/executable hashes, eight bounded function comparisons with hashes, and four compared capture hashes pass. Read the saved settings, native file-dialog, layout, QML caller, and restart-policy results. Reused passing evidence without builds or test reruns. The earlier file-run executable digest remains null. Controlled restart failure passes 4/0 without a process launch. Actual successful OS relaunch/quit, physical display, GPU, live playback, platforms, packages, and HighQuality acceptance remain open. Ticket 04 lifetime, 06 account, 12 media, and 05/15/07 preparation claims remain.

- 7f202b35: merged ticket 15 commit 48427a70 without conflicts. GUI source matches tested settings merge 280989cf. All 26 recorded normalized source/lock/application/harness/asset/image/patch hashes and 45 independent preserved-source comparisons with hashes pass. Inspected all six layout captures. Read and reused valid focused evidence, including shared settings 5/0, capture/presentation 4/0, and current GNU numeric response 10/0. No build or test was repeated. Renderer 13, native lifetime 04, and media 12 behavior remain. Physical calibration, Raw Input, actual focus, live console, GPU, package, and platform desktop acceptance remain open. Numeric defaults remain provisional.

## Decisions

- [Ticket 05](issues/05-console-management-and-manual-addresses.md) adds current opaque target keys for console actions, hidden consoles, and registration choices. Manual entries retain their native identity and address. [Verification](evidence/ticket05/verification.md) records native saves and association, deletion distinctions, PIN leading zeroes, secret-free replies, streamer masking, stale target/profile rejection, focus, reload, and registered manual handoff. Native connection state remains in QmlBackend. Ticket 03/04 graphics and decoder evidence is reused. Ticket 07 owns pairing; tickets 19 through 23 retain physical, platform, hardware, and performance acceptance.

- [Ticket 04](issues/04-local-native-stream-handoff.md) adds one application window controller while the backend retains connection state. [Verification](evidence/ticket04/verification.md) records real bridge handoff, native PIN/menu/disconnect, initialization recovery, decoded buffer cleanup (GL 51/51 and Vulkan 54/54), fresh page resume and merged sign-in closure. Ticket 03 graphics evidence is reused. Full live-console, foreground/focus, native video-disabled, default-preset, hardware/platform and performance acceptance stays open in 23; production presets are unchanged.

- [Ticket 12](issues/12-video-and-audio-settings.md) adds video decoding, four independent stream profiles, and native audio settings to the shared settings page. Numeric drafts retain their starting profile generation and reset on profile changes. [Verification](evidence/ticket12/verification.md) records real transport, native persistence, controlled SDL device enumeration, layout checks, and hardware limits. Ticket 13 retains display targets and renderer settings.

- [Ticket 11](issues/11-general-and-stream-settings.md) adds the shared settings shell and an explicit native allowlist for general session controls. Native setters retain persistence and side effects. Focused application checks pass save/reload 3/0 and validation/restart 4/0. [Verification](evidence/ticket11/verification.md) records the tested sources, binary hashes, offscreen test conditions, and remaining hardware limits. Existing QML settings routes remain because they still have real callers.

- [Ticket 01](issues/01-application-owned-native-services.md) completes application-owned native services and QML engine lifetime. The pre-edit check established installed module limits and native graphics behavior. It did not prove accelerated WebEngine coexistence or real-console performance. Tickets 03 and 04 retain the required combined graphics acceptance. Ticket 23 retains matched live baseline conditions, predeclared tolerances, live-frame cleanup, and hardware acceptance.

- [Ticket 02](issues/02-native-stream-only-interface.md) extracts `StreamRoot.qml` and removes setup navigation from the native host. `streamViewDismissed` is a presentation request; ticket 04 owns cleanup before setup restoration. [Native verification](evidence/ticket02/verification.md) records actual root, PIN, settings, and input checks.

- [Ticket 03](issues/03-accelerated-web-console-library.md) completes the restricted production bridge and static Svelte library. Its passing Windows hardware GPU fixture selects the existing native Fast preset; production defaults stay unchanged. HighQuality produces black native video in the recorded fixture, including a bounded old-import/no-WebEngine control. This is not a full pre-migration baseline. [04](issues/04-local-native-stream-handoff.md) and [23](issues/23-complete-web-only-acceptance.md) retain default/preset processing acceptance, real handoff focus, live media, hardware, and platform limits. See [verification](evidence/ticket03/verification.md).

- [Ticket 06](issues/06-psn-sign-in.md) adds standalone web PSN sign-in and account status. Existing native auth and profile guards own completion; remote sign-in has no setup channel. [Verification](evidence/ticket06/verification.md) records controlled native HTTP, persistence, safe failures, cookie rules and responsive captures. Live PSN and visible desktop acceptance remain outside this evidence.

- [Ticket 13](issues/13-display-and-renderer-settings.md) adds Display and Renderer settings, guarded native writes, accurate saved-versus-active status, and native renderer file actions. Imports validate before replacement. Failed export preserves existing bytes. Native picker results retain their profile generation. The shared documentation action returns the actual libplacebo URL. Backend and OpenGL vertical sync changes retain immediate native restart triggers, with actual launch failure feedback. [Verification](evidence/ticket13/verification.md) records focused passes, source comparisons, the earlier file-run artifact digest gap, and remaining live hardware limits.

- [Ticket 15](issues/15-keyboard-and-mouse-controls.md) adds native physical capture, all keyboard and mouse slots, response controls, legacy keys, and complete reset to the web settings page. Form/profile/window guards cancel stale capture and reset work across handoff. [Verification](evidence/ticket15/verification.md) records focused application passes, current-source GNU numeric response 10/0, source preservation, and hardware limits. Physical calibration remains provisional.
