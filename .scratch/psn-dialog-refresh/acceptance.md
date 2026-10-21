# Integrated native acceptance

Date: 2026-10-21. Integration base: `d5655228`.
The tested source includes the focus fixes in ticket 08.

The normal Windows application builds from all 332 staged build inputs.
The stage includes root and third-party CMake files, the SteamTools patch,
GUI sources, headers, resources, and tests. All eight submodule revisions match
the repository. Two consecutive configurations pass.

The executable is `C:/Users/david/.cache/chiaki-ui-native/psn-dialog-final-bin/chiaki.exe`.
[The launch helper](evidence/ticket08/launch-final.ps1) sets the UCRT and Qt paths.
An identical `launch-chiaki.ps1` is beside the executable.
The final directory takes precedence for the patched SteamTools DLL.
`chiaki.exe --help` exits with code 0 before Settings construction.
[The build record](evidence/ticket08/normal-build.json) contains executable and DLL
hashes, the exact 72-resource list, and the submodule revisions.
All required screens are present. Fixtures and QA resources are absent.
The normal source matches the worktree, and QA symbols are absent from the executable.

## Results

| Check | Final result |
| --- | --- |
| Complete production screen suite | [38 suites, 869 passed, no failures or skips](evidence/ticket08/screen-summary.json) |
| Integrated native PSN and navigation | [297 checks passed](evidence/ticket08/native-integration.json) |
| Actual Vulkan inline disconnect route | [309 checks passed](evidence/ticket08/native-vulkan.json) |
| Actual OpenGL separate disconnect route | [329 checks passed](evidence/ticket08/native-opengl.json) |
| Native Steam command with temporary data | [31 assertions passed](evidence/ticket08/steam-native.txt) |
| Shared native cache | [332 original hashes restored, original targets rebuilt](evidence/ticket08/cache-restoration.json) |

The full screen run covers Library, Settings, registration, connection, PIN,
PSN setup, public lookup, Steam results, reminders, confirmations, and information.
Each suite has a separate text result in `evidence/ticket08/`.

Native checks use production Main navigation, controls, backend, and session objects.
They cover Settings, startup, expiry, registration return, closed requests,
re-entry, replacement forms, profile changes, persistent credentials, and reminder chains.
The integrated Steam form writes a shortcut and five artwork files in a temporary tree.
The separate native command check also verifies updates and actual file failures.

Both native disconnect routes check 900 by 600 and 1280 by 720 clients at measured DPR 1.25.
The OpenGL dialog uses a 552 by 348 logical client.
Checks cover visible initial Cancel focus, keyboard and controller key mappings,
input capture and release, Enter, Escape, mouse input, native window close,
centering, cleanup, and unchanged preferences. Each route dispatches four Stop
requests and two GoToBed requests at controlled transport boundaries.

About and renderer notices restore visible focus in Settings when the opener is disabled.
About and the Steam controller notice can reopen with Return after closure.
The initial full run exposed a Qt 6.11 focus-reason error in this sequence.
The fix resets the opener's focus before Tab focus restoration.
The Steam notice also releases its popup focus before restoration.
The [failed controller result](evidence/ticket08/initial-screen/screen-tst_SettingsControllers.txt),
[failed About result](evidence/ticket08/about-keyboard.txt), and final suites retain the regression evidence.

## Evidence limits

Each native process uses a fresh INI directory and synthetic account or console data.
Network replies and session commands are controlled. Native settings saves and
temporary Steam file writes are real. No user settings or Steam installation are changed.

Retained images come from the actual Vulkan Qt Quick GPU target, the OpenGL
render-control framebuffer, or the separate QQuickWindow. Changed states produce
different pixels. Native window and focus checks accompany those images.
Final desktop composition remains unverified. GDI and Windows Graphics Capture
returned stale Codex pixels for the correct foreground Chiaki window in earlier checks.
Those images are excluded. No claim about desktop composition follows from GPU readback.

The native Qt build has no QtWebEngine. Native checks verify external fallback.
Screen tests verify embedded-browser selection and recovery with a supplied browser.
Real PSN sign-in, live public lookup, actual external-browser navigation, Steam
visibility after restart, real console sleep or disconnect, and physical controllers
remain untested. The renderer notice uses a supplied failure reason.

The [replay procedure](evidence/ticket08/README.md) identifies the commands and isolation hooks.
The final normal executable and DLL remain usable after cache restoration.
