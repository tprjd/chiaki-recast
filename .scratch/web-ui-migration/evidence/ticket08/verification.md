# Ticket 08 verification

Date: 2026-10-22. Branch: `codex/web-ui-ticket-08`.

Web setup can refresh remote consoles and connect with the current opaque
console key. Native code owns token refresh, device listing, connection
attempts, session creation, worker results, and cleanup. The web page shows
the existing native attempt during token refresh. The existing native window
shows later progress and recovery actions. No media or raw input crosses
WebChannel.

## Source and build

The worker starts from integration `24f57a8c`. It merges current console
management through `21e01c9d`, including integration `fab6df5c` and ticket 05.
The native session-start transport is in `bf4c7e01`.

The private build is
`C:/Users/david/.cache/chiaki-msvc-native/ticket08-build`.
It uses the prepared Qt 6.9.3 MSVC toolchain, clang-cl 19.1.7, FFmpeg 7.1,
libplacebo 7.360.1, and Vulkan SDK 1.4.357.0. Shared dependencies were not
rebuilt. The expected local gf-complete VC patch is excluded from commits.

The application and embedded test executable build. The frontend check has
zero errors and zero warnings. Each runtime group has a source commit, the
full application diff, and the executable SHA256 recorded before launch.
The runtime PATH uses this build's SteamTools directory and the prepared
`environment.cmd`.

## Focused runtime results

| Group | Result | Scope |
|---|---|---|
| `red.txt` | 2 passed, 1 failed, 22321 ms | Missing remote refresh control through the real embedded page. |
| `green.txt` | 5 passed, 0 failed, 3674 ms | Remote refresh, affected credential refresh, and sign-in. |
| `expanded.txt` | 7 passed, 0 failed, 7091 ms | Delayed list, failure retention, selection, local suppression, stale profile, token cancellation, retry, and affected library behavior. |
| `isolated.txt` | 4 passed, 1 failed, 3629 ms | Ownership and token cases pass with controlled discovery. One obsolete identifier assertion fails after the ticket 05 merge. |
| `refresh-final.txt` | 3 passed, 0 failed, 1152 ms | Correct streamer-mode identifier masking and remote refresh. |
| `native.txt` | 3 passed, 0 failed, 2465 ms | Real web target dispatch, native unreachable failure, retry, late cancellation, obsolete attempt, stale profile, session/window cleanup, and setup restoration. |

The native group uses source `bf4c7e01` and executable SHA256
`fcd205eeb9ba9eeaee4978d9d5bc3a35063017824bbb4e9864fe3e5e19c085e4`.
Both late-success routes assert zero producer starts. The raw start callback
returns an error if it is unexpectedly called. The fixture cannot start
network playback even when an ownership check fails.

`identity.json` records current application, test, asset, and source hashes.
`record-identity.py` reproduces the source comparisons. Eight whole native
files match the integrated baseline. StreamSession matches after the three
listed transport substitutions. Nine backend methods match, with the single
constructor-argument substitution in `createSession` recorded separately.

The failing identifier assertion expected no DUID in normal mode. Ticket 05
permits the identifier in Console details. The corrected assertion enables
streamer mode before it checks masking. The command still uses an opaque key.

The first red launch returned Windows loader error `C0000135` before tests
started. The command had expanded PATH before `environment.cmd` ran.
Sequential commands in `run-red.cmd` corrected the launch. No product source
change was used for that error.

## Native boundaries and isolation

Fixtures control native HTTP replies, device-list completion, and the existing
PSN connection worker call. They use the production Svelte page,
QWebEngineView, WebChannel, SetupBridge, QmlNativeServices, and QmlBackend.
They do not inject page state or replace the bridge.

The inherited backend starts discovery in its constructor and after cleanup,
even when the test's saved auto-discovery preference is false. The early red,
green, and expanded groups did not isolate that LAN discovery transport.
Their PSN HTTP and device listing were controlled. No console session was
started. Those runs are not evidence of zero external discovery traffic.

The later ticket 08 fixtures pass per-instance DiscoveryRequests before
construction. IPv4, IPv6, manual discovery, and cleanup use its native init
and fini functions. Production defaults remain the real discovery service.
The raw session-start callback also defaults to `chiaki_session_start`.
The native fixture records and rejects any unexpected producer start.

## Preserved native behavior

The source audit compares native window, input-owner, progress, PIN, and
stream QML files with integrated baseline `5a61e86a`. It also compares the
connection phase, start, retry, cancellation, dismissal, session creation,
and cleanup methods.

StreamSession differs only at its constructor parameter, stored transport,
and raw session-start invocation. The producer-start logic, event processing,
native media, input, stop, and join logic remain the same. The backend passes
the transport at construction. The audit lists these exact substitutions.

Ticket 04 native lifetime, PIN, decoded-buffer cleanup, and setup restoration
evidence remains applicable. Ticket 03 accelerated Chromium and native
renderer evidence remains applicable. This ticket does not repeat their
graphics or decoder matrix.

## Captures and limits

`remote-connection-error-390.png` shows the token error and recovery actions
in a narrow window. It predates the small refresh-button style correction.
`remote-refresh-800.png` shows the final 44-pixel refresh button and native
refresh result. A zoom factor of 2 was requested, but the capture can precede
Chromium's asynchronous zoom repaint. The image does not prove 200-percent
layout acceptance. Neither image proves physical focus or GPU acceleration.

The legacy MainView and ConsoleLibrary resource chains still have dedicated
QML callers and tests for later migration work. The native ConnectionView,
StreamView, PIN controls, and stream resource chain remain. This slice adds
no replacement native connection state machine or callback adapter to retire.
Ticket 23 retains the final setup-only resource cleanup.

Live PSN availability is not established. Root coordinates the available
registered console separately. Real PSN playback and return, physical focus,
held input, live media, HDR, hardware decoding, performance, and supported
platform packages remain acceptance work. Only Windows hardware is available.
The recorded native HighQuality processing limit remains open. Production
presets are unchanged.
