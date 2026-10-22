# Ticket 17 verification

Profiles and general settings files use compiled Svelte, QWebEngineView,
WebChannel, SetupBridge, QmlSettings, and native Settings. File tests complete
real QFileDialog instances. No file path or content is an argument from JavaScript.
Credentials remain native. Streamer mode masks profile labels, names, log paths,
and the native export title and suggested file name.

## Source and checks

The work started at `df307813`. Merge `569448d9` includes ticket 09 through
`8b28fc1d`. Merge `36f8378f` includes the later claim documentation at `5b026017`.
The ticket 09 merge retains the profiles snapshot, native registration busy guard,
and both focused test registrations. Merge `f2f84944` includes ticket 10 at
`5db47a2d`; merge `5be3f952` includes root `773e08e6`, with identical ticket 10
sources. These additive conflicts retain Profiles, autoConnectKey, terminal
`woken` handling, and all test slots. The shared controlled discovery fixture
includes the new fail-closed wakeup callback before every ticket 17 constructor.

Private build: `C:/Users/david/.cache/chiaki-msvc-native/ticket17-build`.
Qt 6.9.3 MSVC2022 x64 and clang-cl 19.1.7 use the prepared native dependencies.
`configure.cmd`, `build.cmd`, and `build.txt` record the configuration and final
application/test links. `frontend-check.txt` reports zero errors and warnings.
The local Windows gf-complete patch stays inside its submodule and is excluded.

| Focused run | Result |
|---|---|
| `red/qt.txt` | Expected failures: malformed host import cleared the hidden map; Profiles was absent. |
| `green/qt.txt` | `profileImportValidation`, `profileSettingsPage`, `profileFiles`, and affected `rendererFiles`: 6 passed, 0 failed, 8757 ms. |
| Child restart in `green/qt.txt` | A separate process reads the selected profile, audio volume, and log flags: 3 passed, 0 failed. |
| `cleanup/embedded.txt` | `profileDefaultImport` passed. `profileSearch` caught a draft reset after an unrelated native notification. |
| `cleanup/qml.txt` | Four named SettingsConfig/SettingsSearch cleanup cases: 8 passed, 0 failed, 2213 ms. |
| `search-fix/qt.txt` | Corrected `profileSearch`: 3 passed, 0 failed, 1637 ms. |

Each group held the root runtime slot. Each directory contains its pre-run full
source/diff/new-file record, executable SHA256, and sequential `run.cmd`.
The identity records use gzip to reduce duplicate source text. Tests ran offscreen
with software Qt Quick and test-only Chromium `--disable-gpu`. No GPU, desktop
focus, screenshot, live PSN, LAN, session, or hardware claim follows from them.
All profile/file fixtures control HTTP, console lists, worker connection, raw
session start, registration, and discovery before native services are constructed.

## Behavior proved

- Profile creation trims the name and needs an explicit action. Selection does
  not switch profiles. Deletion needs the checkbox and explicit action; native
  services reject deletion of the default and current profiles. Deletion preserves
  the active maps and removes the selected native store. Existing names and
  per-profile values survive switching and a separate-process restart.
- Settings replacement keeps the shared QmlSettings object, disconnects old native
  signals, refreshes values, and invalidates old keys/generations. Old profile
  commands and old picker completions cannot write into the new profile.
- Imports validate the INI version, array structure, required MAC/key lengths,
  registered targets, manual IDs/addresses, and mapping/profile entries before
  changing maps or stores. Version 1 host names/MACs and decoder migration remain
  supported, including inactive entries retained by older QSettings writers.
  Default imports restore the source catalog and default store; named stores stay
  separate. Failed validation leaves cached and reopened native values unchanged.
- Exports use a child INI in QTemporaryDir and QSaveFile. The decoder exports as
  Auto. Cancel, invalid import, read-only destination replacement, and missing
  parent failure report their actual result. Existing destination bytes remain
  unchanged after a failed replacement. A shared pending guard covers general
  and renderer file actions.
- Log sanitizing, verbose logging, native log-directory opening, streamer masking,
  search fallback, post-switch focus, and profile draft survival on unrelated
  snapshots pass. General and Profiles retain at least 1024 CSS pixels at 200%
  zoom, with no horizontal overflow. No narrow layout or capture work was added.

## Caller removal and reused evidence

The caller audit found ProfileDialog only in the retired setup SettingsDialog
route and its fixtures. Removed the leaf, resource entry, routes, focus target,
search entries, profile-only test, and unused QmlSettings sentinel getter and
invokable deletion adapter. Native profile services and catalog notifications
remain. The parent SettingsDialog/MainView chain, diagnostics/About fixtures,
and native stream settings stay for their remaining callers and final cleanup.
Only the four named affected QML cases ran; no linked inventory or narrow matrix.

`source-preservation.json` verifies the earlier passing service/test methods,
the final default-import implementation, and unchanged native stream input,
StreamRoot, Controllers, and keyboard/mouse components. The unused-adapter removal and actual ticket 10 merge rebuilt both tests and the
real application. The final source comparison retains incoming wake/auto-connect
methods, request controls, and the backend except the required old-signal
disconnect. No invoked ticket 17 file/profile method changed.
Passing cases were not repeated after additive merges or unused API deletion.

Native store sync errors return failure and attempt to restore the previous
snapshot. A mid-write native store failure was reviewed but not forced at runtime.
Physical/platform acceptance and native graphics checks remain with later tickets.
