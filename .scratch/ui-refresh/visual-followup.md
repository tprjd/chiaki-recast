# Card library visual corrections

The console details modal is part of the approved scope. This follow-up corrects the native screen from the user's annotated screenshots.

The selected card border now has the same width around the artwork and body. Library buttons share an outlined style. Search has an icon, discovery has larger text and an explicit icon size, and the three-dot control uses a centred SVG. The selected-console shortcuts are clickable buttons.

The details modal now uses the prototype's title, corner close control, status row, labelled columns, spacing, and bottom actions. Available native fields and management actions remain present. Narrow windows use one column and a scrollable information area. Streamer mode still masks private identifiers. The primary action closes the modal before it resolves and connects the selected console.

The native `QQuickRenderControl` window reports a Controls overlay with zero width and height. Its default dimmer does not cover the screen or block background commands. A backdrop inside `ConsoleLibrary` supplies the dimmer and consumes outside clicks. This behavior was checked in the actual Windows renderer.

Verification:

- The original screen fails nine new appearance and primary-action checks. Output: `C:/Users/david/.cache/chiaki-screen-build-integration/appearance-red.txt`.
- The final Windows Qt 6.11.2 screen suite passes all 206 checks. Both CTest targets pass in 79.18 seconds. Record: `C:/Users/david/.cache/chiaki-ui-native/polish-native-ctest-final.xml`.
- Native captures cover 1280 by 720, 800 by 600, and 390 by 600 logical pixels at measured scale factors 1.25 and 2.0. Both native replay records have `passed: true`. They check visible controls, the backdrop, and blocked background commands. The 2.0 replay also checks real wheel input to reach the last field at minimum width.
- Native replays use separate settings, synthetic console entries, and disabled discovery. No session starts. Fixtures are removed at exit.

Native evidence:

- [Library at 125%](native-previews/polish-native-final125c-library-1280x720.png)
- [Details at 125%](native-previews/polish-native-final125c-details-1280x720.png)
- [125% replay](native-previews/polish-native-final125c-checks.json)
- [200% replay](native-previews/polish-native-final200-checks.json)
- [Minimum window after wheel input](native-previews/polish-native-final200-details-fields-390x600.png)
- [Normal build verification](native-previews/polish-production-summary.json)

The normal build is `C:/Users/david/.cache/chiaki-ui-native/polish-bin/chiaki.exe`. It is open in a new window with the user's normal settings. Its source matches the workspace. The QA helper and settings changes are excluded from this executable.

Real-console streaming and a physical controller were not checked in this follow-up.
