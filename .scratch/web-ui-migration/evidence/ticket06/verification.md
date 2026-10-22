# Ticket 06 verification

The account route uses compiled Svelte, QWebEngineView, QWebChannel, SetupBridge,
and application-owned QmlNativeServices. QmlBackend::startPsnAuth and PSNAccountID
retain token exchange, Account ID validation, profile ownership, and persistence.
The sign-in page uses a separate persistent `psn-token` profile without a WebChannel.

## Request boundaries

`PsnRequests` is an instance dependency passed to QmlNativeServices and QmlBackend.
Its production defaults use QNetworkAccessManager and chiaki_holepunch_list_devices.
The tests provide controlled QNetworkReply objects and an empty native device-list
response. They do not replace Svelte, WebChannel, the bridge, native parsing,
credential storage, profile guards, or the queued device-list completion.
Fixture codes and credentials cannot reach the live OAuth or device-list services.

The embedded isolation check also blocks and counts WebEngine requests. This
prevents an accidental remote redirect request if native navigation rejection fails.
The external-browser check captures QDesktopServices at the native URL-open boundary.

Ticket 08 can reuse `PsnRequests::listDevices` for device-list response fixtures.
The pointer runs in the existing native worker. Returned device arrays must use
memory compatible with chiaki_holepunch_free_device_list.

## Environment and commands

- Windows 11, Qt 6.9.3 MSVC2022 x64, clang-cl 19.1.7.
- Source checkout: `C:/Users/david/.codex/worktrees/web-ui-ticket-06/chiaki-ng-ui`.
- Build directory: `C:/Users/david/.cache/chiaki-msvc-native/ticket06-build`.
- Dependency environment: `C:/Users/david/.cache/chiaki-msvc-native/environment.cmd`.
- Configure command: `C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket06-configure.cmd`.
- Build command: `C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket06-build.cmd`.
- Application link command: `C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket06-app-build.cmd`.

The configure command adapts ticket 03's prepared MSVC inputs to this source and
build path. It disables vcpkg manifest installation. Dependencies were not rebuilt.
The existing MSVC gf-complete compatibility patch is local and excluded from commits.

The behavior checks set `QT_QPA_PLATFORM=offscreen` and
`QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu`. These settings apply only to the test
process. Production acceleration remains enabled. Visible Windows tests require
the runtime slot and the active input desktop. No hardware result is inferred from
these offscreen checks.

## Results

| Check | Result |
|---|---|
| Initial desktop precondition | Screen-saver desktop, foreground HWND 0. No behavior conclusion. |
| Account route red | Expected failure: account status control absent. |
| Account status green | 3 passed, 0 failed. Native signed-out, expired and ready states; no snapshot or browser-storage secrets. |
| Sign-in red | Expected failure: redirect controls absent. |
| Sign-in green | 3 passed, 0 failed. External native URL; OAuth/account progress; native persistence; malformed and duplicate redirects; duplicate submission; failure and retry; incomplete credentials; stale profile reply; URL clearing. |
| Refresh red | Expected failure: incomplete response overwrote saved credentials and reported success. |
| Refresh green | Passed in the combined refresh/isolation run. Invalid replies retain credentials; unauthorized errors are safe; valid replies update native status. |
| Embedded isolation | 3 passed, 0 failed after the observer correction. No channel or qt transport; zero outgoing redirect HTTP; native account completion; failure cookie deletion; success persistence. |
| Account presentation | Passed. All four captures inspected. No horizontal overflow at 390 pixels or 200 percent. Fresh sign-in clears the URL. Escape closes the panel and returns focus. |
| Frontend type and Svelte check | 0 errors, 0 warnings. |
| Native application and harness links | Passed. |

The initial offscreen D3D attempt lost its Skia context. The behavior checks then
used software Chromium rendering. Environment failures are not passing evidence.
The failed isolation observation remains in green-refresh-isolation.txt.

## Cookie persistence observation

The cookie fixture has a one-day expiry and a random name. Before success,
`cookieAdded` confirms the in-memory cookie. SQLite metadata reports count 0 before
the profile flush. After success and profile destruction, the same fixture row
has count 1, persistent count 1, and a future expiry. The check never selects cookie
values. The exact owned profile and fixture identity are in
[green-isolation-metadata.txt](green-isolation-metadata.txt).

Qt 6.9.3 did not replay cookie notifications after cache reset in this fixture.
The final check uses read-only SQLite metadata after the profile flush. The earlier
missing-notification failures remain recorded and caused no production change.
The query uses the existing required Python interpreter and its sqlite3 module.

## Source and artifact identity

The final native isolation check uses source commit
`d1f643d7e4810c296e59141a9f0074d05b1ad233`. It includes integration tip
`0b813edee3605615db197b5e49136cabe9261373` and ticket 11's settings. The merge retains
both bridge snapshots and commands, plus the shared `settingsOpen` and
`data-setup-panel` keyboard guards. The earlier auth, status, refresh and presentation
passes remain applicable: the merge adds settings and preserves their code paths.
Frontend type checking and both native links passed after the merge.

[identity.json](identity.json) records exact changed source hashes, frontend asset
hashes, the lockfile hash, the excluded vendor patch hash and these binary hashes:

- Application: `182c7dd8ede7adb6a608114cb331bd937c48781a58b55c8d96c1dae5e9ed988e`.
- Embedded harness: `8fd6f032b97ee6906046971ca9e767e2a9e217535388135b734ecc0c2b23763a`.

The application was linked from the merged production tree at `e0338188`. Commit
`d1f643d7` changes only the test's persistence observation; production source is equal.
The four images predate the settings merge. The account component and its layout
are equal in the merged tree. No new physical focus or hardware claim is made.

## Scope and limits

No real PSN sign-in, user credential read, live OAuth request, or live device-list request
was performed. Live Sony page compatibility and Windows visible focus remain
outside this evidence. Ticket 03's unchanged library and GPU evidence is reused.
HighQuality processing and complete hardware acceptance remain with tickets 04 and 23.

PSNTokenDialog.qml remains because registration and retained settings still call it.
The unused initPsnAuth and handlePsnLoginRedirect adapters and unused InitPsnToken
operation were removed after a repository-wide caller search. Shared native stream
QML remains unchanged.
