# Final native Settings acceptance

Final revision: `e0e33b830d0f5f7f8eb5101657b16845e1f4d9ba` on `codex/settings-refresh-integration`.
The main checkout contains this revision and the preserved original console-library edits.

## Results

The native Windows application passes at 1280 by 720 and 900 by 600 logical pixels. The minimum supported width is 900 pixels.

| Run | Measured application scale | Result |
| --- | --- | --- |
| Keyboard Search lifecycle and immediate Escape | 100% | Pass at both sizes |
| Full Settings and linked-page matrix | 100% | Pass at both sizes |
| Full Settings and linked-page matrix | 150% | Pass at both sizes |
| Full Settings and linked-page matrix | 200% | Pass at both sizes |
| Full matrix with expanded translated labels | 100% | Pass at both sizes |

Each normal matrix checks 714 page/control focus targets across both sizes, including associated numeric values and bitrate descriptions. It covers all nine categories, both console stream forms, Display, all seven Renderer groups, all five Color Mapping groups, Profiles, and controller-selection cancellation. Tab, category keys, wheel scrolling, linked-page return focus, and library return pass. Browsing causes no persisted writes.

The translated matrix checks 722 focus targets and expanded Search result text. No translated label is elided. The final lifecycle test includes six Escape cancellations while the key-capture popup is visible but not yet fully opened. Settings and its key opener remain active.

All 290 final PNG/state pairs match the measured window size and scale. Root independently checked all 278 matrix/translated pairs and inspected the final 900-pixel Video and translated Color Mapping captures. The native reviewer inspected final Config actions, Renderer values, sidebars, and Search results. All five QA processes exited.

See the [native source/result manifest](native-previews/13-final-native-manifest.json), [200% Video capture](native-previews/13-final-matrix-e0e33b8-dpr2-video-900x600.png), and [translated Color Mapping capture](native-previews/13-final-long-e0e33b8-dpr1-color1-900x600.png).

Earlier bounded native checks verify actual isolated persistence for keys, numeric settings, profiles, debanding, and console actions. Native file-dialog cancellation and repeated Search routes also passed. The original user application was not stopped or changed.

## Production build

The temporary INI, discovery, framebuffer and QA helper changes were removed after native acceptance. The normal executable built successfully. Its `--help` startup check passed before any Settings object was constructed. Fourteen production source/resource files match the delivered checkout after line-ending normalization; all 12 original console files remain byte-identical. The build graph and executable contain no QA helper. Production resources contain no fixtures or prototype files.

Launch with [Launch-chiaki.cmd](C:/Users/david/.cache/chiaki-ui-native/settings-bin/Launch-chiaki.cmd). This supplies the existing local MSYS2 runtime paths. The executable is [chiaki.exe](C:/Users/david/.cache/chiaki-ui-native/settings-bin/chiaki.exe).

See the [build manifest](production-build.json), [build log](production-build.txt), and [startup log](production-help.txt). Existing compiler warnings about ignored file-open return values remain outside this change. The native QA logs retain the existing Main.qml Shortcut messages; no new QML error was found.

## Limits

Physical controller input, live-stream entry points, real HDR output, authentication, accepted registration/import/export, native profile deletion, and external documentation/clipboard actions were not exercised. Profile deletion restrictions and dispatch were checked through production-QML tests. Native deletion was excluded because its separate QSettings constructor was outside the test isolation patch.

Scaling used measured process-level Qt scale factors. Global Windows display settings were not changed. The normal executable's full UI was not launched against user settings; full UI checks used the same production QML and native handlers with isolated test storage and disabled discovery.
