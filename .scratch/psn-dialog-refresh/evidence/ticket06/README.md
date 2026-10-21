# Disconnect verification

Production GUI source: `ff4ad3a6`, with integration through `599413ba`.
The final branch also merges `ae85efab`. The later registration changes are
outside this native run. Ticket 08 owns complete integrated acceptance.

| Check | Result |
| --- | --- |
| Full screen suite before integration | 795 passed |
| Focused connection and PIN regressions | 83 passed |
| Final Disconnect, Confirmations, and Reminders suites | 83 passed |
| Actual Vulkan inline native route | 307 checks passed |
| Actual OpenGL separate native route | 327 checks passed |
| Post-exit isolated settings | Host records and preferences unchanged |
| Cache restoration | 209 original source hashes match, both normal targets rebuilt |

Both native runs use real `QmlBackend`, `Main`, `StreamView`, controls, and
constructed `StreamSession` objects. Native connected and quit events control
readiness and cleanup. Start, Stop, and GoToBed stop at controlled transport
boundaries. Discovery, PSN refresh, and wake transport are disabled. Every process
uses a new explicit INI directory and a synthetic console. No real account or
console is used.

Each run checks 900 by 600 and 1280 by 720 logical clients at measured DPR 1.
The checks cover initial visible Cancel focus, Tab, Backtab, controller key
mappings, Enter, mouse acceptance, Escape, repeated close, native window close,
return focus, native parent, centering, and actual input-block state. Only
Disconnect and sleep dispatches GoToBed. Both explicit disconnect actions dispatch
Stop. Each run records four Stop calls and two GoToBed calls. Cancellation sends
neither. The saved disconnect action remains Ask. QML checks also cover long
translated text, disabled actions, repeated acceptance, and preference value 2.

`native-overlay-red.json` records the actual zero-size native overlay before the
Main viewport bindings. The shared native confirmation and reminder now have
positive bounds and visible initial focus. Escape preserves the reminder choice.

The retained Vulkan images use `pl_tex_download` on the actual Qt Quick GPU target
on its render thread. Distinct driven dialog states produce different pixels.
OpenGL uses the render-control framebuffer. Separate windows use
`QQuickWindow.grabWindow`. The native helper verifies the actual foreground HWND.
Image inspection confirms current labels and the Cancel focus ring.

Desktop compositor capture remains unavailable in this session. GDI and Windows
Graphics Capture return stale Codex pixels even for the correct foreground HWND.
Those images are excluded. The GPU images prove rendering of the current target.
They do not prove final desktop composition. Real console power changes, physical
controllers, and scaling beyond DPR 1 remain unverified here.

## Replay

Use the existing MSYS2 native cache and compiler environment. Run from the repo
root. `prepare-native.py` requires a new preparation name. It backs up every
staged path and rejects missing patch locations.

```powershell
$nativeRoot = 'C:/Users/david/.cache/chiaki-ui-native'
$env:PATH = "$nativeRoot/msys64/ucrt64/bin;" + $env:PATH
python .scratch/psn-dialog-refresh/evidence/ticket06/prepare-native.py disconnect-prep02
cmake -S "$nativeRoot/source" -B "$nativeRoot/build" "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$nativeRoot/disconnect-qa-bin"
cmake --build "$nativeRoot/build" --target chiaki -j 4
& .scratch/psn-dialog-refresh/evidence/ticket06/run-native.ps1 -Run disconnect-vulkan-replay01 -Renderer vulkan
& .scratch/psn-dialog-refresh/evidence/ticket06/run-native.ps1 -Run disconnect-opengl-replay01 -Renderer opengl
python .scratch/psn-dialog-refresh/evidence/ticket06/check-post-exit.py "$nativeRoot/disconnect-vulkan-replay01"
python .scratch/psn-dialog-refresh/evidence/ticket06/check-post-exit.py "$nativeRoot/disconnect-opengl-replay01"
python .scratch/psn-dialog-refresh/evidence/ticket06/restore-native.py disconnect-prep02
```

The restoration result names the original runtime output directory. Reconfigure
with that directory and rebuild `chiaki` and `chiaki-console-screen-tests`.
`cache-restoration.json` records the completed restoration for this ticket.

`run-native.ps1 -Hold` pauses at the first inline disconnect for up to five minutes.
Create its reported `resume.flag` to continue. The script uses the measured desktop
scale adjustment of 0.8 for DPR 1 on this machine.

Preparation stages GUI files only. Full integrated acceptance must also stage the
repo build inputs and SteamTools patch. The source manifests identify exactly
which production GUI files and QA hooks each retained native run used. QA readback
and all test fixtures are outside production resources. The production QML list
contains the new disconnect component and no fixture or native helper.

The complete native preparation and original backups remain at
`C:/Users/david/.cache/chiaki-ui-native/disconnect-prep01/`. Run directories are
`disconnect-vulkan-readback01` and `disconnect-opengl-final02` in the same cache.
