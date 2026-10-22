# Controller setup verification

Date: 2026-10-22

## Result

The shared web settings page has controller options, connected device names and
types, and all 23 native primary and alternate mapping targets. Native
`ControllerManager`, `Controller`, and `QmlBackend` still own capture,
normalization, conflict movement, apply, reset, and cancellation. The page does
not receive controller samples or an arbitrary key command.

An opaque mapping operation and profile generation guard each mapping command.
Profile changes, page reload, setup hide, device removal, and native window
attachment end the operation. Removal of another device does not end it.
Connect and retry cannot start from the setup bridge during mapping.

Native `QmlController` sends its existing Qt navigation keys to the embedded
widget. Native stream windows keep their existing target and visibility rule.
Native modal dialogs retain the active focus target. Capture and controller
selection stop navigation. Navigation resumes after the current SDL event and
held button release.

## Sources and artifacts

The first implementation is `9b0de943`. Merge `f7524810` includes integration
`915be521`, including the ticket 07 request constructor and ticket 14 controls.
The final tested source is `f7524810` plus [final-source.patch](final-source.patch).
[source-sha256.csv](source-sha256.csv) records the actual GUI source files before
the final launch. Later commits add evidence and merge documentation only.

Merge `5ba1ca0c` includes the latest integration `5cd42106`. Its GUI tree is
unchanged from integration `915be521`. The final worker GUI source is unchanged
from `92a70fe`, which records the final tested patch. The read-only
[source-preservation.json](source-preservation.json) comparison checks all 189
recorded source hashes with no mismatch. It also records both executable hashes
and the compiled JavaScript and CSS hashes. No build or runtime check follows
this documentation merge.

| Run | Executable SHA256 | Result |
| --- | --- | --- |
| Settings and mapping, green02 | `b50630841f0fa1c2dc31e772de4a06851c3a13a52bcdb2860be1eec77728147d` | 4 passed, 0 failed, 0 skipped |
| Final mapping, green03 | `3215abb2f83a98a3f4222339e5d48a3c3e6d185bfec3781eddc1b39812f9dc59` | 3 passed, 0 failed, 0 skipped |
| Affected QML cases | `4cb35676642607be05f3defeaca8c069a68ec6874eebd2aca78af8d1ff93ec04` | 5 passed, 0 failed, 0 skipped |

Counts include Qt Test initialization and cleanup. The native executable is
`C:/Users/david/.cache/chiaki-msvc-native/ticket16-build/gui/tests/chiaki-embedded-setup-tests.exe`.
The QML executable is in the same directory. The checked source is
`C:/Users/david/.codex/worktrees/web-ui-ticket-16/chiaki-ng-ui`.

Before every launch, the external `ticket16-red`, `ticket16-red02`,
`ticket16-green01`, `ticket16-green02`, `ticket16-green03`, and `ticket16-qml01`
directories under
`C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/` recorded source,
full diff, new files, file hashes, and executable hashes. These include the
earlier source identities and unsuccessful attempts. The final full diff is
relative to integration `915be521`. The prepared Windows VC vendor patch is
local, pre-existing, and excluded from this change.

The target builds with the prepared MSVC dependency environment. The focused
commands are in [run-settings.cmd](run-settings.cmd),
[run-mapping.cmd](run-mapping.cmd), and
[run-affected-qml.cmd](run-affected-qml.cmd). They initialize `environment.cmd`
before adding the worker's `third-party/steam-tools` directory to `PATH`.
Windows Qt Test receives an explicit text log path.

`npm.cmd run check` reports 0 errors and 0 warnings. `git diff --check` passes.
The embedded and QML test targets build. Prepared shared dependency prefixes were not rebuilt or changed.

## Observed behavior

The native fixture sets controlled PSN list and discovery transports before it
constructs `QmlNativeServices`. Production transports keep their real defaults.
The fixture uses fresh test profiles and SDL virtual devices with vendor
`0xfafa` and products `0x0016` and `0x0017`. It does not change a physical
controller's mappings.

The full path is compiled Svelte, `QWebEngineView`, WebChannel, production
`SetupBridge`, native services, and the actual SDL virtual joystick API.
JavaScript activates visible controls and reads the resulting page. It does not
inject controller state into the page or substitute a backend.

The passing cases establish these results:

- Controller options use native setters and persist. The touch increment
  converts 0.75 mm to native value 75. Boolean, range, unknown-name, and disabled
  touchpad writes fail safely.
- SDL attachment updates device display. All 23 native mapping rows appear.
  Native Options opens Settings. Native Return opens mapping before editing
  and opens reset after apply. Native D-pad input moves to primary and alternate
  controls.
- Selecting a device does not also start capture. Capturing `b1` for the Cross
  alternate moves that input from Moon through the native conflict behavior.
  The input does not also close or activate the setup dialog.
- Cancel capture leaves the native draft unchanged. The analog option updates
  the existing native capture option. Cancel discards the draft. Apply saves
  the mapping, and a new `Settings` instance reads the same value. Reset restores
  the original SDL mapping and removes the saved override.
- A search for MIC alternate input selects the device first and then focuses
  the correct target without capture. Reload ends capture. A profile change
  closes the operation and rejects the old capture and apply requests. Setup
  hide ends capture. Removal of an unrelated device preserves the operation.
  Removal of the selected device ends it and rejects the old apply request.
- The three affected QML cases retain category routing, safe public search
  metadata, and search focus without settings writes.

The detailed logs are [settings-and-mapping.txt](settings-and-mapping.txt),
[final-mapping.txt](final-mapping.txt), and [affected-qml.txt](affected-qml.txt).

## Red checks and corrections

[first-red.txt](first-red.txt) fails at the absent Controllers category.
[navigation-red.txt](navigation-red.txt) fails when native Options leaves the
page on Consoles. Both failures precede their production changes. An initial
launch did not reach Qt because its runtime path named the wrong Steam tools
directory. The corrected sequential launcher reached the expected first red.

The first broader QML search run passed eight cases. It then continued into
the existing linked inventory. The worker stopped that owned test process to
release the shared runtime slot. The run is
[interrupted-qml.txt](interrupted-qml.txt), not a full suite pass. Its warning
identified one remaining legacy reference to the removed mapping opener. The
final named cases pass after that reference uses the remaining controller
option. The full inventory was not repeated.

## Removal and preserved native code

The caller audit found no StreamRoot route to `ControllerMappingDialog.qml`.
This change removes that setup-only file, resource entry, old settings/search
adapters, mapping test files, and their fixture methods. Other legacy settings
code remains for its current callers and the final setup cleanup.

Compared with integration `915be521`, `ControllerManager`, `StreamSession`,
`StreamInputOwnership`, `Settings`, `QmlSettings`, `StreamRoot`, and the native
live keyboard, display, renderer, and color dialogs have no changes. No gameplay,
calibration, haptics, decoder, or frame-processing algorithm changed.

## Visual evidence and limits

[controller-mapping.png](controller-mapping.png) is the inspected 1280 by 900
normal-size capture. Labels, both mapping slots, focus, and scrolling are usable
in the inspected area. The lower rows and actions are exercised by the native
tests, but they are not all visible in this one capture.

The 200 percent check uses a 2560 by 1800 native window. The DOM confirms DPR 2,
1280 CSS pixels, and no horizontal overflow. Both attempted widget grabs still
contain stale backing pixels. The final image is retained as
[invalid-controller-mapping-200-percent.png](invalid-controller-mapping-200-percent.png).
It is invalid visual evidence. Ticket 23 retains 200 percent visual acceptance.
No new work targets widths below 1024 CSS pixels.

All runtime checks use offscreen rendering and a test-only GPU-disable flag.
They do not establish physical controller behavior, foreground focus, haptics,
calibration, controller latency, real-console input suppression, GPU coexistence,
live playback, or non-Windows package behavior. Existing valid native stream
evidence is reused. These hardware and platform checks remain with ticket 23.
