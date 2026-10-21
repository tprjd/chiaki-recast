# Replay integrated acceptance

Run these commands from the repository root while the shared native cache is idle.
Use a new preparation name and a new run name for each replay.
A replay replaces result files in this worktree.
The native dependencies and populated submodules are in the existing cache.

## Build the normal application and run screens

```powershell
$native = 'C:/Users/david/.cache/chiaki-ui-native'
$e = '.scratch/psn-dialog-refresh/evidence/ticket08'
$env:PATH = "$native/msys64/ucrt64/bin;" + $env:PATH
python "$e/stage-native.py" psn-acceptance-prep-replay
cmake -S "$native/source" -B "$native/build" "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$native/psn-dialog-final-bin"
cmake -S "$native/source" -B "$native/build" "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$native/psn-dialog-final-bin"
cmake --build "$native/build" --target chiaki chiaki-console-screen-tests -j 4
& "$e/run-screens.ps1"
python "$e/summarize-screens.py"
```

Wait for the screen run to finish before rebuilding its executable.
The runner uses `-o <suite-file>,txt` for each of the 38 production QML suites.

## Check native Steam writes

```powershell
cmake -S "$native/source/gui/tests/steam" -B build/steam-acceptance -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$native/msys64/ucrt64"
cmake --build build/steam-acceptance -j 4
$env:PATH = (Resolve-Path build/steam-acceptance/steam-tools).Path + ';' + $env:PATH
$env:QT_QPA_PLATFORM = 'offscreen'
& ./build/steam-acceptance/chiaki-steam-shortcut-tests.exe
Remove-Item Env:QT_QPA_PLATFORM
```

The test uses a temporary Steam tree. It compiles the production command and
the real patched SteamTools dependency. It does not use an installed Steam client.

## Check integrated native navigation

```powershell
python "$e/prepare-integration.py" psn-setup-prep-replay
python "$e/snapshot-native.py" integration psn-acceptance-prep-replay psn-setup-prep-replay
cmake -S "$native/source" -B "$native/build" "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$native/psn-integration-qa-bin"
cmake --build "$native/build" --target chiaki -j 4
& "$e/run-integration.ps1" -Run psn-integration-replay125
python .scratch/psn-dialog-refresh/evidence/ticket01-prepare-native.py --restore psn-setup-prep-replay
```

The preparation reuses ticket 01's INI isolation, controlled network responses,
and OpenGL readback. The added routes use production Main, Settings, registration,
information, reminders, the Steam form, and the native controller notice.
The [source record](integration-source.json) lists the seven explicit QA overrides.

## Check both native disconnect routes

```powershell
python "$e/prepare-disconnect.py" disconnect-prep-replay
python "$e/snapshot-native.py" disconnect psn-acceptance-prep-replay disconnect-prep-replay
cmake -S "$native/source" -B "$native/build" "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$native/disconnect-qa-bin"
cmake --build "$native/build" --target chiaki -j 4
& "$e/run-disconnect.ps1" -Run disconnect-vulkan-replay125 -Renderer vulkan
& "$e/run-disconnect.ps1" -Run disconnect-opengl-replay125 -Renderer opengl
python .scratch/psn-dialog-refresh/evidence/ticket06/check-post-exit.py "$native/disconnect-vulkan-replay125"
python .scratch/psn-dialog-refresh/evidence/ticket06/check-post-exit.py "$native/disconnect-opengl-replay125"
python .scratch/psn-dialog-refresh/evidence/ticket06/restore-native.py disconnect-prep-replay
```

The preparation reuses ticket 06's transport boundaries and actual GPU readback.
The [source record](disconnect-source.json) lists the ten explicit QA overrides.
The scale factor is 1 on this desktop, which gives measured DPR 1.25.

## Rebuild the final application and restore the cache

```powershell
cmake -S "$native/source" -B "$native/build" "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$native/psn-dialog-final-bin"
cmake --build "$native/build" --target chiaki -j 4
python "$e/snapshot-native.py" normal psn-acceptance-prep-replay
python "$e/check-normal.py"
& "$e/launch-final.ps1" --help
python "$e/stage-native.py" --restore psn-acceptance-prep-replay
cmake -S "$native/source" -B "$native/build" "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$native/console-setup-final-bin"
cmake --build "$native/build" --target chiaki chiaki-console-screen-tests -j 4
python "$e/check-restoration.py" psn-acceptance-prep-replay
```

The final normal build has no native QA hooks. The launch helper uses the final
directory's SteamTools DLL before other paths. The restored cache uses the original
`console-setup-final-bin` output. The [acceptance record](../../acceptance.md) states
the controlled checks and the external-service, hardware, and desktop composition limits.
