# Chiaki Recast web setup UI specification

Status: ready-for-agent
Date: 2026-10-22
Publication: published to the local issue tracker through to-spec.
Test boundary: confirmed by the user on 2026-10-22.
Scope update: GPU-accelerated web setup without a QML setup fallback, confirmed on 2026-10-22.
Ticket and test-policy update: approved by the user on 2026-10-22.
Layout scope update: minimum design width is 1024 CSS pixels, confirmed by the user on 2026-10-22.
Task scope: the user invoked implement-spec on 2026-10-22. Implement the approved specification and all 23 tickets on the integration branch. The earlier planning-only restriction is superseded.

## Problem Statement

The QML setup UI limits how maintainers can change the application layout and
style. The current application also connects setup window lifetime to native
streaming resources. This connection makes a frontend migration difficult.

Users need a more maintainable setup UI without loss of features, saved
settings, or Remote Play quality. A UI change must not add browser work to
the native media and input paths or increase input delay.

## Solution

Use Svelte 5, TypeScript, HTML, and CSS for console discovery, registration,
connection setup, and settings. Embed the compiled frontend in a
GPU-accelerated Qt QWebEngineView window. Use Qt WebChannel to communicate
with existing native services.

When a connection needs native streaming resources, prepare a separate native
stream window. Hide the setup window after the native window can show progress
and accept input. Keep video, audio, input, HDR, and stream overlays native.
When the stream ends, finish native cleanup and restore the setup UI.

Migrate screen by screen on this branch. The existing main branch remains
available separately. This branch has one web setup UI and no runtime QML
setup fallback or UI selector. Keep the selected card library, settings
sidebar, and guided setup designs. Preserve the existing native backend
and the QML components used by native stream overlays and live settings.

## User Stories

1. As a maintainer, I want to change setup layouts with HTML and CSS, so that UI changes need less QML-specific work.
2. As a maintainer, I want reusable Svelte components, so that shared controls have consistent behavior and style.
3. As a desktop user, I want the new setup UI to retain existing features, so that the migration does not remove tasks I use.
4. As a returning user, I want my settings and registered consoles to remain available, so that I do not need to configure the application again.
5. As a desktop user, I want the web setup window to open at normal startup, so that I can find consoles and change settings.
6. As a local user, I want console discovery to update the library, so that I can see available consoles.
7. As a PS4 or PS5 user, I want the correct console type and registration state, so that I can select a supported connection action.
8. As a desktop user, I want console names, artwork, and status to remain clear, so that I can identify the intended console.
9. As a desktop user, I want search and console selection to survive discovery updates, so that changing results do not select another console.
10. As a desktop user, I want console details and management actions, so that I can inspect and maintain my console list.
11. As a local user, I want to control discovery, so that I can retain the current discovery behavior.
12. As a remote user, I want to refresh PSN consoles, so that I can update the remote console list.
13. As a desktop user, I want to add a manual address, so that I can connect to a console that discovery does not show.
14. As a desktop user, I want manual host entry to remain separate from registration, so that I understand whether I saved an address or paired a console.
15. As a desktop user, I want to use an existing registration for a manual host, so that I do not need to register the same console again.
16. As a desktop user, I want to register a console with its Remote Play pairing code, so that the console accepts this client.
17. As a PS4 user, I want the existing supported firmware choices, so that I can register my console with the correct method.
18. As a desktop user, I want the Remote Play pairing code and console login PIN to have distinct labels, so that I enter each code in the correct field.
19. As a desktop user, I want leading zeroes in code fields to remain intact, so that the application submits the code I entered.
20. As a local user, I want the existing broadcast registration option, so that I can use the supported local registration route.
21. As a desktop user, I want the supported PSN account sources during registration, so that I can provide the required account identity.
22. As a desktop user, I want registration details to remain while I retrieve an Account ID, so that I do not need to enter the form again.
23. As a desktop user, I want invalid fields to prevent submission, so that I can correct them before a request starts.
24. As a desktop user, I want one activation to start one registration request, so that repeated clicks do not create duplicate work.
25. As a desktop user, I want registration progress and results from the native backend, so that I know the actual outcome.
26. As a desktop user, I want useful diagnostics and a retry route after registration failure, so that I can correct the problem.
27. As a desktop user, I want dismissal and cancellation to have their existing meanings, so that closing a screen does not imply an unsupported cancellation.
28. As a desktop user, I want console wake-up to remain available, so that I can connect when my console is in rest mode.
29. As a desktop user, I want auto-connect to retain its selected console and profile, so that startup connects to the intended target.
30. As a desktop user, I want to stop auto-connect, so that I can remain in setup.
31. As a local user, I want to connect to a registered local console, so that I can start Remote Play.
32. As a remote user, I want to connect through PSN, so that I can use Remote Play away from the local network.
33. As a desktop user, I want native connection progress and cancellation rules, so that I can understand and control the current attempt.
34. As a desktop user, I want a failed connection to offer the existing retry and recovery actions, so that I can try again.
35. As a desktop user, I want delayed actions to retain the intended console, so that a discovery update cannot connect to or delete another console.
36. As a desktop user, I want missing or ambiguous targets to fail safely, so that the application does not guess which console I meant.
37. As a desktop user, I want to hide and restore consoles, so that I can manage the visible library.
38. As a desktop user, I want deletion and other destructive actions to retain confirmations, so that I can avoid accidental data loss.
39. As a desktop user, I want to update a saved console login PIN, so that the existing automatic sign-in behavior remains available.
40. As a PSN user, I want the existing embedded sign-in option, so that I can use the supported sign-in flow.
41. As a PSN user, I want the external browser fallback, so that I can sign in when embedded sign-in is unavailable.
42. As a PSN user, I want to submit a sign-in redirect URL, so that native code can complete sign-in.
43. As a PSN user, I want expired credentials and token refresh to have the existing recovery behavior, so that I can restore remote access.
44. As a desktop user, I want the current public Account ID lookup and service explanation, so that I can choose whether to use that service.
45. As a desktop user, I want lookup timeouts and invalid replies to show a useful error, so that I can retry without accepting an invalid Account ID.
46. As a profile user, I want replies from an old profile to be rejected, so that a delayed operation cannot change the current profile.
47. As a PSN user, I want stored credentials to remain native, so that the setup page does not receive authentication secrets.
48. As a desktop user, I want every existing settings category and search route, so that I can still find and change each supported setting.
49. As a PS4 or PS5 user, I want local and remote stream values to remain separate, so that one profile change does not overwrite another stream configuration.
50. As a desktop user, I want the same setting units, ranges, defaults, and restart requirements, so that existing configuration behavior remains predictable.
51. As a desktop user, I want unavailable platform and device options to retain their conditions, so that the UI does not offer unsupported actions.
52. As a desktop user, I want setting changes to persist across reloads and restarts, so that the browser page is not a second settings store.
53. As a profile user, I want to create, switch, and delete profiles, so that I can retain separate configurations.
54. As a desktop user, I want to import and export settings through native file actions, so that I can retain the current configuration workflows.
55. As a desktop user, I want failed file actions to preserve existing data and report the error, so that I can recover without data loss.
56. As a desktop user, I want display targets, renderer options, and color mapping controls, so that advanced stream configuration remains available.
57. As a keyboard and mouse user, I want native key capture and binding conflict handling, so that keyboard and mouse mapping retains its current behavior.
58. As a mouse user, I want the existing mouse response controls and reset actions, so that the migration preserves my calibration and defaults.
59. As a controller user, I want controller mapping and device updates, so that I can configure the controllers I use.
60. As a desktop user, I want supported Steam shortcuts, log access, and help links, so that the remaining setup actions stay available.
61. As a desktop user, I want startup reminders and confirmations to retain their rules, so that the migration does not repeat or suppress them incorrectly.
62. As a keyboard user, I want visible focus and predictable navigation, so that I can complete setup without a mouse.
63. As a controller user, I want the existing setup navigation and actions, so that I can use the application from a controller.
64. As a touch user, I want reachable controls and scrolling, so that supported desktop touch use remains possible.
65. As a desktop user, I want layouts that support widths of at least 1024 CSS pixels, long text, and display scaling, so that all actions remain readable and reachable.
66. As a user who enlarges text, I want usable layouts at 200 percent zoom within the supported design width, so that I do not lose controls.
67. As a screen-reader user, I want labels, status messages, and dialog focus handling, so that I can understand and operate the setup UI.
68. As a user who reduces motion, I want the UI to respect that setting, so that animations do not prevent comfortable use.
69. As a user in streamer mode, I want private identifiers, account details, profile names, and paths to retain their masking, so that setup does not reveal them on screen.
70. As a desktop user, I want console names and service replies treated as text, so that untrusted content cannot run code in the setup page.
71. As a Remote Play user, I want a separate native stream window, so that Chromium never renders or transports video frames.
72. As a Remote Play user, I want the setup window hidden during playback, so that it does not cover the stream.
73. As a Remote Play user, I want native video, HDR, audio, microphone input, haptics, and input handling, so that the frontend migration does not replace the stream pipeline.
74. As a Remote Play user, I want the native menu, statistics, notices, and disconnect dialog, so that stream controls retain their behavior.
75. As a Remote Play user, I want live display, renderer, color mapping, and keyboard and mouse settings to remain native, so that I can change them during playback.
76. As a Remote Play user, I want the console login PIN prompt to remain native during connection and playback, so that sign-in does not depend on the hidden setup page.
77. As a Remote Play user, I want overlapping stream input owners to retain their lifecycle, so that closing one dialog cannot release input blocked by another.
78. As a Remote Play user, I want held keys and buttons released at handoff and focus loss, so that input does not remain stuck or reach the console during setup.
79. As a Remote Play user, I want gameplay capture and focus to return through the existing rules, so that menu and settings interactions do not change game input behavior.
80. As a Remote Play user, I want the setup UI restored after stream cleanup, so that I can connect again or change settings.
81. As a Remote Play user, I want connection and stream errors shown once with a recovery action, so that the window transition does not lose or duplicate errors.
82. As a Remote Play user, I want close, disconnect, console sleep, suspend, and resume to retain their policies, so that the migration does not change session behavior.
83. As a frequent user, I want repeated connections to release old windows and native resources, so that later sessions remain stable.
84. As a Remote Play user, I want a setup renderer failure to leave native playback running, so that a Chromium failure does not stop a healthy stream.
85. As a desktop user, I want a restored setup page to read current native state without replaying requests, so that recovery does not start duplicate operations.
86. As a command-line user, I want direct streaming to bypass setup page creation, so that the existing native launch route remains available.
87. As a command-line user, I want fullscreen, zoom, stretch, profile selection, and stream-exit options preserved, so that existing launch commands continue to work.
88. As a desktop user, I want setup and stream geometry retained separately, so that each window returns in its expected state.
89. As a Windows, Linux, or macOS user, I want tested release packages, so that the application works without a development environment.
90. As an offline user, I want setup assets packaged with the application, so that the UI does not depend on a website or development server.
91. As a desktop user, I want GPU acceleration enabled for the web setup UI, so that Chromium renders the interface on the GPU.
92. As a maintainer, I want one web setup path on this branch, so that I do not need to maintain a second setup UI.
93. As a maintainer, I want obsolete QML setup code removed as its screens migrate, so that unused setup code does not remain in the application.
94. As a Remote Play user, I want measured streaming performance without repeatable regression, so that the new setup UI does not increase latency or reduce playback quality.
95. As a maintainer, I want staged delivery with acceptance evidence, so that each migrated flow works with the native stream window.

## Implementation Decisions

### Native ownership and window lifetime

- Keep the existing native backend, FFmpeg decoder, libplacebo renderer, frame
  queues, and platform input services. Do not move media processing into the
  frontend.
- Add a small application controller for setup window lifetime, native stream
  window lifetime, handoff, and application exit. Keep connection logic in the
  existing backend.
- Make QmlBackend application-owned. Add explicit attachment and detachment of
  its native stream window. Keep one backend for discovery, settings,
  registration, PSN work, connection attempts, and sessions.
- Retain QmlMainWindow as the native render and input window. Give it a
  non-owning backend reference. Update its constructor callers, renderer
  helpers, and native test adapters together.
- Move the native QQmlEngine to application ownership. Use one persistent
  engine for native stream roots. Register the Chiaki
  singleton once, keep backend and engine on the same thread, and destroy
  the engine before the backend.
- Keep the engine's image providers alive with the engine. Retarget its
  incubation controller when the active native render window changes.
  Destroy window-specific roots and connections with each window.
- Supply each native QML root with its actual host window. Keep the web setup
  window separate from the attached native stream window. Replace the current
  constant window binding with an appropriate native notification.
- Separate QML root creation from window presentation. Showing an existing
  window must not create a second root or apply setup geometry to playback.
- Extract the native stream root and its navigation functions. Keep the stream
  menu, notices, statistics, console login PIN host, error handling, renderer
  fallback dialog, and live settings routes together.
- Preserve ADR 0001. Keep stream input owner presentation identities,
  present, dismiss, settled, cleanup, focus, and held-input release behavior.
  Passive notices must not become stream input owners.
- Derive application activity from the active setup, stream, or owned dialog
  window. Preserve controller navigation when the native stream window is
  absent or hidden.

### Connection handoff and cleanup

- Prepare and attach the native renderer before session creation uses decoder
  or GPU information. Do not wait for the first frame to create the renderer.
- Keep setup visible until the native window can show progress and accept
  input. Then hide setup and transfer focus once. Keep connection progress
  and console login PIN interaction native after handoff.
- Use existing attempt identities, phases, cancellation limits, retry rules,
  and PSN cleanup state. A repeated session notification must not create
  another window or session.
- Reject a second connection while the current attempt or its cleanup remains
  active. Retain the existing rules for auto-connect and console wake-up.
- Keep registration-only operations in setup. An internal registration
  session object must not trigger a playback handoff.
- Preserve the video-disabled connection route. Use the existing native ready
  conditions rather than a frontend timer or simulated progress.
- Keep the native stream window alive until the session, connection workers,
  queued frames, and decoder references can no longer use its resources.
  A cleared session property alone does not prove destruction.
- Release gameplay capture and held inputs before teardown. Stop frame
  production, finish or cancel connection work, free pending frames, and
  release decoder references before their GPU resources.
- Preserve render-thread requirements, swapchain cleanup, and worker shutdown.
  Ensure that frames held by callbacks are released even if those callbacks
  will never run. Add a cleanup completion signal only where the existing
  interface cannot report the necessary completion.
- Restore setup from fresh native state after cleanup. Retain console
  selection and focus where the target still exists. Report a retained error
  once.
- Preserve disconnect confirmation, console sleep, suspend and resume,
  fullscreen, zoom, stretch, profile selection, and exit-on-stream-end rules.
  Keep the direct stream launch route free of setup page creation.
- Control application exit during handoff. A temporary absence of visible
  windows must not quit the application. Page close requests must not stop
  native playback.
- Preserve the existing persistent setup and stream geometry separately.

### WebChannel interface and settings

- Publish one restricted SetupBridge. Keep backend QObjects, window pointers,
  stream objects, GPU handles, and raw input devices out of the web interface.
- Use Qt WebChannel's embedded transport. Do not add an HTTP API, WebSocket
  server, or local network listener.
- Exchange JSON-compatible state and commands. Treat all calls as asynchronous.
  Distinguish accepted commands from completed operations.
- Use a small TypeScript adapter for callback completion, subscriptions, and
  current-state reads. Keep connection state logic in native code.
- Read current state after channel initialization, page reload, and page
  resume. Prevent lost updates during initial subscription. Release old
  subscriptions when a page is replaced.
- Use native-issued console keys scoped to the profile. Resolve each key
  against the current console list immediately before dispatch. Reject
  missing or ambiguous targets. Do not retain a row index across an
  asynchronous command.
- Preserve attempt, request, and profile identities across results. Keep
  identities that can exceed JavaScript integer precision as strings.
  Reject stale results and repeated activation.
- Adapt registration, PSN authentication, and Steam shortcut operations that
  currently depend on QJSValue callbacks. Share native completion signals
  with both UI adapters. Preserve logs, validation, timeout behavior, and
  cancellation limits.
- Move public Account ID lookup into a shared native operation when that
  screen migrates. Preserve the existing service explanation, timeout,
  response validation, and stale-result checks.
- Keep Settings as the single persistent store. Reuse QmlSettings conversion,
  notification, and side-effect logic through validated native operations.
  Do not create a browser settings store.
- Preserve existing keys, defaults, units, ranges, profiles, device conditions,
  and separate local and remote PS4 and PS5 values. Refresh both adapters
  when the active Settings instance changes.
- Keep file dialogs, settings import and export, log actions, controller
  mapping, input capture, and Steam integration native. Provide web setup
  flows that call those services. Keep live stream settings native.

### Content, sign-in, and privacy

- Load only packaged application content in the bridge-enabled page.
  Use a separate page without the bridge, or the existing external browser
  route, for PSN sign-in.
- Preserve embedded sign-in, external sign-in fallback, sign-in redirect URL
  submission, credential refresh, and current cookie handling rules.
- Keep token exchange and stored credentials native. Do not publish PSN
  tokens, registration keys, session keys, or saved console login PINs.
  Expose status or masked values when the UI needs them.
- Accept new secret values through specific native commands. Clear temporary
  PINs and sign-in redirect URLs after submission or dismissal. Keep them out
  of browser storage and logs.
- Preserve streamer mode across cards, details, account fields, profiles,
  paths, errors, and diagnostics. Treat console names and service responses
  as untrusted text.
- Block external navigation, remote frames, unexpected resource requests,
  popups, and downloads in the trusted setup page. Open permitted help links
  through native actions.
- Use a restrictive Content Security Policy. Keep the Chromium sandbox
  enabled and remote debugging disabled in release packages.

### Frontend and access

- Use Svelte 5, TypeScript, and Vite for a static application. Use CSS Grid
  and Flexbox for layout, CSS variables for shared values, and CSS for
  transitions.
- Start with reusable components needed by the current screen. Share buttons,
  fields, confirmations, status messages, and layout values where the
  application uses the same behavior.
- Reuse the selected card library, category sidebar, guided registration,
  and manual host designs. Reuse existing artwork. Do not ship prototype
  sample data or simulated actions.
- Preserve all existing setup categories, search routes, linked advanced
  pages, reminders, file actions, profiles, Steam shortcuts, input setup,
  and account flows.
- Retain native labels, translation behavior, conditional controls,
  confirmations, error recovery, and restart requirements. Select the web
  message catalog mechanism when the first screen migrates.
- Preserve keyboard, controller, and desktop touch operation. Keep device
  access and input capture native. Prevent duplicate activation from
  native controller navigation and browser events.
- Provide labels, visible focus, predictable dialog focus return, status
  announcements, scrolling, and reduced-motion support.
- Use 1024 CSS pixels as the minimum setup design width. Check 1024 by 600
  and 1280 by 720 CSS pixels, long text, and display scaling. At 200 percent
  zoom, use a window wide enough to retain at least 1024 CSS pixels.
  Keep every action reachable within this supported range.
- Do not optimize or add acceptance work below 1024 CSS pixels. Keep existing
  working responsive CSS. Earlier narrower checks remain historical evidence;
  they do not require repetition or new work. This scope change does not add
  a hard native window-size limit.

### Browser resource use and release support

- Require GPU acceleration for the web setup UI. Remove the unconditional
  --disable-gpu startup override from this application path. Verify actual
  Chromium acceleration in the embedded application. Software WebEngine
  rendering does not satisfy this requirement.
- Start with a hidden setup window during playback. Stop unnecessary UI
  updates and animation. Freeze the hidden page where the selected Qt
  version and page state permit it.
- Measure total browser resource use. Hiding or freezing does not prove
  zero GPU use or memory use.
- Destroy or discard the setup page only if measurements justify the added
  lifecycle work. First define recovery of the route, selected console,
  and non-secret form state. Do not retain PINs or sign-in redirect URLs.
- Recreating a page must reconnect to the same native backend and current
  attempts. It must not restart an operation. Keep page, channel, bridge,
  and WebEngine profile ownership explicit.
- Require matching WebEngineWidgets and WebChannel modules for the desktop
  GUI. Build the frontend as part of that GUI. Remove WebEngineQuick if no
  remaining component needs it. Keep Qt Quick for native streaming and overlays.
- Use web setup as the sole setup UI on this branch. Do not add a web feature
  toggle, QML setup fallback, or runtime UI selector. Keep the existing main
  branch available separately.
- Pin the frontend toolchain and package lock. Check the selected Node,
  Svelte 5, Vite, Qt, and embedded Chromium versions together.
- Compile the frontend into packaged Qt resources as a native build
  dependency. Include all generated assets. Prove clean builds without
  an old frontend output directory.
- Use relative packaged asset URLs. First verify module loading, dynamic
  imports, styles, fonts, artwork, and WebChannel script loading from
  Qt resources. Add a custom scheme only if the packaged bundle requires it.
- Preserve Windows, Linux, and macOS release support with toolchains that
  supply the required WebEngine modules. Check each supported architecture
  and toolchain. Report unsupported combinations explicitly rather than
  retaining a second setup UI for them.
- Update existing release processes for widget libraries, helper processes,
  resources, locales, dependency notices, sandbox behavior, and signing.
  Test AppImage, Flatpak, and macOS package behavior as applicable.
- Keep Qt and Chromium security updates in the release process.

### Delivery order

1. Perform a bounded check of accelerated WebEngine beside the current
   native renderer. Pin the baseline build and measurement conditions.
   Keep this prerequisite within the native ownership ticket.
2. Separate application, backend, QML engine, and native window ownership.
   Extract the native stream root, overlays, and live settings routes.
3. Build the packaged Svelte setup window with WebEngine GPU acceleration.
   Remove the startup override that disables acceleration. Add the restricted
   bridge, current state reads, capabilities, and stable console targets.
   Reuse applicable graphics evidence from the prerequisite check.
4. Connect a registered local console from the web UI. Verify setup hide,
   native playback, cleanup, and setup restoration.
5. Migrate console management and manual addresses, independent PSN sign-in,
   pairing and Account ID lookup, remote connection, automatic registration,
   wake-up, and auto-connect. Use only genuine prerequisite dependencies.
6. Migrate settings, advanced pages, profiles, files, input setup, reminders,
   and remaining actions. Remove replaced setup routes, adapters, resources,
   and tests in each ticket. Preserve shared native stream components.
7. Check initial packaging with the first embedded web flow. Deliver Windows,
   Linux, and macOS packages after native handoff works. Complete access,
   recovery, and performance acceptance. Reuse applicable passing evidence.

The [approved ticket breakdown](ticket-breakdown.md) records the 23 tickets
and their blocking edges. Independent tickets can proceed together.

## Testing Decisions

### Agreed primary test boundary

- Use one primary feature boundary at the embedded application, as confirmed
  by the user on 2026-10-22.
- Exercise the compiled Svelte UI through a real QWebEngineView, real
  WebChannel transport, production SetupBridge, native application
  controller, and existing native services.
- Check external results: displayed state, native window visibility,
  saved settings, reported outcomes, and recovery behavior. Do not
  assert private helper calls, component structure, or implementation order.
- Use an isolated native settings store. Reuse existing controlled
  native session adapters. Where a service lacks a test adapter,
  supply controlled results at its native asynchronous request boundary.
  Cover failures, delayed replies, and cancellation. Keep the production
  bridge and transport in this integration path.
- Reuse the existing native test harness where it supports this boundary.
  Add one embedded application harness for the missing production
  WebEngine path. Avoid separate new test APIs for each Svelte component.
- Keep browser presentation checks supplementary. Browser fixtures can
  check layout and interaction, but they cannot prove native state changes
  or window handoff.

### Conservative execution

- Run the smallest check set for each ticket's changed behavior and affected
  callers. A separate failure case is distinct coverage, not a duplicate run.
- Record passing evidence. Reuse it while the tested code, dependencies,
  configuration, and relevant environment remain applicable.
- Repeat or broaden checks only after a relevant change, failure, or unresolved
  concern. Stop when the required checks pass and no concrete concern remains.
- Establish the embedded application harness once, then extend it. Do not add
  a mocked bridge or a separate test API for each screen.
- Run affected native input and output checks after ownership, stream-root,
  handoff, input, or recovery changes. Unrelated setup forms do not require
  the complete native hardware matrix.
- Check packaged startup on the first supported target with the initial web
  flow. Keep full platform handoff checks in the platform package tickets.
- Check each final package once and exercise affected migrated features.
  Reuse deeper hardware evidence unless relevant native code, Qt, WebEngine,
  package contents, drivers, or configuration changed.

### Modules and prior art

- Test the setup application and its bridge together with application
  window ownership, connection setup, settings persistence, registration,
  and account operation results.
- Reuse existing console screen scenarios for the web UI tests. Replace
  tests of retired QML setup components as their web replacements land.
  Retain the QML tests for native stream components.
- Retain existing native stream input ownership and input output tests.
  They exercise production native sources and observable input effects.
- Preserve the ADR 0001 scenarios for overlap, duplicate opens, obsolete
  closes, owner destruction, focus return, and session handoff.
- Use the existing profile, registration, PSN, connection, and settings
  screen scenarios as behavioral prior art for the migrated flows.
- Account for the current compiler conditions on native input test targets.
  A test run that omits those targets must not claim native input coverage.

### Required application scenarios

- Initialize the page from native state. Receive updates during
  initialization. Reload or restore a page during a pending operation
  without losing state or replaying the request.
- Exercise console discovery reorder, selection preservation, missing
  targets, ambiguous targets, stale profiles, and repeated activation.
- Exercise local and remote connection success, wake-up, auto-connect
  cancellation, retry, PIN prompts, initialization failure, network
  failure, and native cleanup.
- Exercise close and disconnect policies, console sleep, suspend and
  resume, repeated sessions, application exit, and setup renderer failure
  during native playback.
- Verify settings through the web UI, native reads, reloads, and
  application restarts. Include profile replacement,
  devices, import and export failures, and restart-only settings.
- Exercise registration validation, leading zeroes, supported console
  choices, account lookup, progress, results, diagnostics, dismissal,
  cancellation limits, and stale results.
- Verify that external sign-in pages lack the setup bridge. Check
  navigation restrictions, untrusted text, invalid lookup replies,
  secret-free snapshots, and streamer mode.
- Check keyboard, controller, touch, focus restoration, screen-reader
  labels, reduced motion, long content, required window sizes, scaling,
  and zoom.
- Check clean release packages without a Qt SDK or Node installation.
  Verify offline assets, helper startup, sandbox operation, supported
  display systems, setup page recovery, and direct streaming.
- Verify actual Chromium GPU acceleration in the packaged application.
  Check that startup does not force software rendering. Test accelerated
  web setup alongside the supported native OpenGL and Vulkan paths.

### Native hardware and performance acceptance

- Use the same machine, console, network, decoder, renderer, profile,
  and display mode for baseline and migrated comparisons.
- Check native OpenGL and Vulkan paths where available, SDR, supported
  HDR modes, hardware decode, software decode, and decoder fallback.
- Compare matched baseline and migrated measurements using numerical
  tolerances fixed before evaluation. Reuse valid baseline variation data.
  Collect additional independent runs when variation is unknown, results
  approach a limit, or a regression appears. Inconclusive data cannot
  establish a pass.
- Record frame timing, queue depth, pending frame age, dropped frames,
  audio underruns, CPU use, GPU use, and memory. Include Chromium helper
  processes. Record time to the first frame separately.
- Require no repeatable streaming regression beyond measured baseline
  variation. Report setup startup time, package size, and idle resource
  use separately.
- Measure end-to-end input latency with external equipment where
  available. Keep that acceptance item unverified if the equipment
  is unavailable. Native frame statistics alone do not prove latency.
- Observe sustained playback on materially different native and browser
  graphics paths. Choose the interval from warm-up, resource trends, and
  known time-dependent failures. Record its duration and limits. Use
  30 minutes or longer when those facts require it.
- Exercise first connection and a subsequent connection with recreated
  resources. Check release of windows, callbacks, threads, GPU resources,
  held input, and browser helpers as appropriate to their lifetime.
- Target cancellation, failure, and late completion at their native
  boundaries. Extend connection loops only for growing counters, uncertain
  cleanup, or a known race.
- Use real desktop and console acceptance for GPU, HDR, microphone,
  controller, and focus behavior. Offscreen software tests cannot
  establish these results.

## Out of Scope

- Electron or a replacement native streaming backend.
- Video frames, decoded audio, GPU handles, or raw game input through
  JavaScript, Chromium, or WebChannel.
- A browser video player, web-based stream overlays, or migration of
  live stream settings to the setup page.
- A runtime Node service, remote frontend hosting, or a new local
  network API.
- Android, iOS, Nintendo Switch, and a mobile product redesign.
- New user features, a new settings schema, or credential migration
  that the frontend change does not require.
- Shipping prototype simulations or sample data.
- A second QML setup UI, runtime UI selection, or a QML setup recovery
  path on this branch. The existing main branch remains available separately.
- Support for a toolchain that cannot supply the required WebEngine modules.
- An additional process and protocol without evidence that the
  selected in-process design cannot meet the requirements.

## Further Notes

- The original code inspection and detailed work-package plan are
  preserved in the [historical migration plan](migration-plan.md).
  Its QML setup fallback and dual-UI rollout proposals are superseded.
- This specification changes the setup technology from the earlier UI
  refresh decisions. It retains their selected designs and behavior.
- ADR 0001 remains binding for native stream input ownership. The
  migration must not change that lifecycle.
- The user requires a GPU-accelerated web setup UI and separate native
  video and overlays. The main branch supplies the existing application.
  No QML setup fallback is required in this implementation.
- A separate window does not prove graphics isolation. Check native
  renderer initialization together with accelerated Chromium.
- The shared engine design preserves the restrictions described in the
  [Qt singleton guidance](https://doc.qt.io/qt-6/qml-singleton.html#exposing-an-existing-object-as-a-singleton).
- WebChannel timing and data conversion follow the
  [Qt WebChannel JavaScript API](https://doc.qt.io/archives/qt-6.9/qtwebchannel-javascript.html).
- Package validation must cover the
  [Qt WebEngine deployment requirements](https://doc.qt.io/archives/qt-6.9/qtwebengine-deploying.html)
  for the selected release.
- These are implementation requirements and proposed acceptance checks.
  Builds, native sessions, hardware measurements, and release packages
  have not been validated by this specification task.
- The user confirmed the application test boundary. No additional triage
  is required. Implementation is tracked in [the implementation map](map.md).

## User priority update, 2026-10-22

The remaining build verification and performance tests are skipped at the
user's request. Retain their unverified status and all completed evidence.
The next implementation priority is to match the selected prototypes in
`gui/prototype/`: B card library, A settings sidebar, and A guided console setup.
Preserve all native services, media paths, input ownership, and bridge behavior.
Review the actual visual result at supported widths of at least 1024 CSS pixels.
No new narrow layout optimization or broad test/build matrix is required.
