# Ticket 13 verification

Display and Renderer settings use compiled Svelte, QWebEngineView, Qt WebChannel,
SetupBridge, QmlSettings, and the existing native Settings store. The file tests
complete real QFileDialog instances. File paths and file contents stay native.
No browser bridge fixture or production test API was added.

## Source and environment

The final source includes integration `79c79ea8` through merge `8e37069e`.
The final native restart source is `5a83520f`. The prior native merge is `dab7697b`. It retains ticket 04 connection state
and its suspended notification guard beside the renderer snapshots.
`identity.json` records full source commits, source hashes, artifact hashes,
and eight source comparisons against earlier passing sources.
The comparisons match. Five complete file-service and file-test bodies are unchanged.
The settings test adds only a controlled external process launcher. Two comparisons
exclude the intentionally changed restart branches and retain the unrelated code. CMake registers the renderer settings, file, layout, and restart policy cases.
`identity-before-restart.json` retains the earlier layout artifact identity.

The private build directory is
`C:/Users/david/.cache/chiaki-msvc-native/ticket13-build`.
`ticket13-build.cmd` records the configure arguments and build target.
The environment uses Qt 6.9.3 MSVC2022 x86_64, clang-cl 19.1.7, and MSVC 14.44.
Dependencies come from the prepared `chiaki-msvc-native/environment.cmd`.
The existing Windows gf-complete vendor patch is local and excluded from commits.
The final native build passed. Its output is `ticket13-restart-final-build.log`.
The earlier merged build output remains in `ticket13-merged-build.log`.
The Svelte and TypeScript check reports zero errors and zero warnings.

## Focused results

| Case | Source | Result |
|---|---|---|
| `rendererSettings` first red | `ae754f2b` plus the new test | Expected failure because Renderer was absent |
| `rendererSettings` | `62f7f839`, included in `abd6826f` | Passed inside the first renderer group |
| First `rendererFiles` | `abd6826f` | Failed because a valid export could not replace its locked temporary staging file |
| Corrected `rendererFiles` | `5b5f341b` | 3 passed, 0 failed, 3252 ms |
| `rendererLayout` first capture | `1fbdb138` | 3 passed, 0 failed, 1766 ms |
| Native QML `SettingsRenderer::test_configActionsAndConstruction` | `1fbdb138` | 3 passed, 0 failed, 69 ms |
| Corrected `rendererLayout` capture after integration merge | `dab7697b`, included in `1d84618b` | 3 passed, 0 failed, 2529 ms |
| `rendererRestartPolicy` red | `b055ee12` | Expected failures for both missing immediate triggers |
| `rendererRestartPolicy` green | `5a83520f`, included in `8e37069e` | 4 passed, 0 failed, 2808 ms |

Every runtime group held the root-coordinated slot. The commands set
`QT_QPA_PLATFORM=offscreen`, `QT_QUICK_BACKEND=software`, and the test-only
Chromium `--disable-gpu` flag. These checks make no GPU or focus claim.
The QML caller fixture is supplementary. The embedded cases use the real bridge.
No full suite ran. Passing behavior was reused after exact source comparisons.

The first launch lacked the private steam-tools DLL directory. The corrected
command adds that directory to PATH. The GUI-subsystem process needs QtTest's
`-o path,txt` option to save its report. The recorded red report contains the
expected assertion failure.

The first renderer group passed `rendererSettings` but failed `rendererFiles`.
Export now creates a normal INI child inside QTemporaryDir. QSettings can replace
that file without an open QTemporaryFile handle. The corrected file case passes.
The executable for that passing file run was not hashed before the next rebuild.
Its exact source and test reports remain recorded. `identity.json` states that
digest gap and proves that both file-service methods, both QmlSettings file
methods, and the file test body remain unchanged in the final source.
The earlier layout executable was hashed before the corrected layout run.
The final restart executable was also hashed before its focused run.

## Verified behavior

The display case checks all 18 primaries and 17 transfer-characteristic choices.
It changes primaries, transfer characteristics, numeric peak, numeric contrast,
and infinite contrast through the compiled page. Native reads confirm the values.

Renderer checks cover presets, queue depth, backend selection, spatial upscaler,
anti-ringing, debanding, sigmoidization, color adjustment, and peak detection.
The bridge rejects invalid types, ranges, unsupported names, and unavailable
custom controls. Native QmlSettings notifications refresh the same page.
Reload and a new Settings instance retain the saved values.
A changed backend starts the existing native restart immediately.
A changed vertical sync value restarts when the current native renderer is OpenGL.
Without a native window, the bridge uses the startup native backend selection.
The UI retains restart-required state if the process launch fails. With no native
stream window, it reports that no native renderer is active. It does not label
the saved backend as the live backend. The live report reads the native window
when that window exists. The production presets and defaults are unchanged.

The documentation action opens `https://libplacebo.org/options/`. Copy uses the
same URL. The shared native helper now returns that URL, instead of a PSN URL.
The native QML renderer caller still constructs and invokes its existing actions.
Its file actions also display native outcome messages.

File checks confirm successful native export and import. Invalid import and
cancelled import leave the renderer map unchanged. Cancelled export leaves the
saved state unchanged. A failed replacement of a read-only destination leaves
its exact bytes unchanged. An export to a missing parent reports failure.
A profile change during the picker prevents import completion from applying.
Snapshots contain neither the selected paths nor raw imported option strings.
Imports validate the INI version, group, and libplacebo option values before
replacement. The three native spatial upscaler names remain supported.
Exports check file writes and use QSaveFile to commit the destination atomically.
Native target-store errors restore the previous values and return failure.
That target-store failure branch was reviewed but was not forced at runtime.

Search reaches the required preset or enable control when a result is hidden.
The layout case checks horizontal bounds at 390 pixels and 200 percent zoom.
The first 200 percent captures showed the sidebar before category layout settled.
The corrected test waits for the requested heading, scrolls, and lets the page
paint. The four final images were inspected and show the actual controls.

## Immediate restart parity

The initial web implementation deferred restart until a button click. The
legacy QML callers restart immediately after a changed backend selection and
an OpenGL vertical sync toggle. The focused red test observed zero launch
attempts for both required triggers.

SetupBridge now starts the shared QmlSettings restart action after those changes.
It compares old and new values, so duplicate writes do not launch another process.
The active native window supplies the runtime backend when it exists. The startup
backend selection applies when no native window exists. QmlSettings retains the
production QProcess::startDetached executable and arguments. It quits the current
application only after a successful launch. Failure retains the warning log and
returns a message that the value was saved but the application could not restart.
The compiled page and the legacy QML callers display that result. The web retry
button also uses the same actual result.

ProcessLauncher is an optional native constructor dependency. Its production
default uses QProcess::startDetached. It is not a WebChannel property or a test
API. The focused page test supplies a launcher that records requests and returns
failure. That boundary prevents a test process from relaunching itself.
The older rendererSettings case now uses the same controlled launch boundary.
Its prior unrelated assertions remain unchanged and their results are reused.

The focused green test covers Vulkan and OpenGL startup selections. It checks
immediate attempts after required changes, no attempt for Vulkan vertical sync,
no-op writes, invalid values, and stale generations. It checks the original
application path and arguments, visible launch failure text, and retry failure.
No process was spawned. The production success launch and quit path remains
unchanged apart from its returned result and was not executed in this test.
The case does not create a native stream window or prove live OpenGL behavior.

## Retained components and limits

PlaceboSettingsDialog and DisplaySettingsDialog remain shared native live
components. SettingsDialog still has callers for other migration tickets.
Their resources and existing fixtures remain. No obsolete adapter was added.
The Color mapping category is the entry for ticket 14. Its fields stay in that
separate ticket.

These checks do not prove physical display output, live renderer changes,
HDR, GPU coexistence, focus, Linux, macOS, packaging, or playback performance.
The actual OS process launch and successful application quit were not executed.
Ticket 11 and ticket 12 retain the
separate-process native-store evidence. This ticket adds reload and new-store
instance checks, but no additional renderer-specific process restart check.
The known native HighQuality fixture failure remains assigned to the root
acceptance work. This ticket does not change or bypass that preset.
