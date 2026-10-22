# Ticket 11 verification

Date: 2026-10-22. Branch: `codex/web-ui-ticket-11`.

The settings entry, category sidebar, shared fields, search, and general session
controls pass focused embedded application checks. Native settings remain the
only settings store. Rejected commands retain the native value and show an error.

## Source and environment

The clean starting commit and integration tip were
`b90c7f27d4f8c36c5272502fe764058fc9664fcf`. The final integration merge reported
`Already up to date.` The original main checkout was not changed.

[source-and-artifact.json](source-and-artifact.json) records SHA256 values for
every changed source, the frontend lock, the tested executable, and packaged
JavaScript and CSS. Unchanged source comes from the starting commit. The local
Windows gf-complete patch remains outside the commit.

The build uses the existing Qt 6.9.3 MSVC x64, clang-cl 19.1.7, FFmpeg 7.1,
libplacebo 7.360.1, and Vulkan SDK 1.4.357.0 dependencies. Its separate build
directory is `C:/Users/david/.cache/chiaki-msvc-native/ticket11-build`.
[configure.cmd](configure.cmd) and [configure.txt](configure.txt) record the
configuration. [build.txt](build.txt) records the final focused build.

## Focused results

- The initial missing settings entry failed as expected. See
  [red-missing-entry.txt](red-missing-entry.txt). The initial offscreen run
  reported Skia context warnings and makes no graphics claim.
- The first implemented run failed at the enum value. Source inspection found
  a string select value with numeric Svelte options. The correction keeps the
  native number. The test also waits for the enabled entry after channel
  initialization. See [red-select-value.txt](red-select-value.txt).
- `generalSettings` passed 3/0 in 1082 ms. It saves through the page, reads the
  native setting, reloads the page, and reads a new native Settings instance.
  See [green-save-reload.txt](green-save-reload.txt).
- `settingsValidation` and `settingsRestart` passed 4/0 in 4608 ms. See
  [green-validation-restart.txt](green-validation-restart.txt).
- `npm run check` passed with zero errors and zero warnings after the final
  panel marker change. `git diff --check` passed.

The validation case checks every shortcut field, session enums, invalid ranges,
fractional and string enum values, invalid boolean types, unknown properties,
unsupported Deck controls, and stale profile commands. Rejected public commands
use a second real WebChannel client. It restores the page client's receiver
after the reply. No bridge or native settings mock is used.

The same case checks visible rejection feedback, retained native values, safe
search metadata, target focus, the hidden-shortcut fallback, the shared keyboard
panel guard, streamer masking, focus return, and current-profile values.

The restart case saves through the compiled page, destroys the first native
services, then starts a separate executable process. That process opens the same
isolated native profile and checks the native value and the rendered field.

The final executable includes the shared panel marker and its focused keyboard
check. The earlier save/reload result remains applicable. The only later
production change isolates panel keyboard events; the validation case checks
that change. The native setters and save/reload behavior are unchanged.

## Settings scope and extension points

This slice owns disconnect, suspend, audio/video enablement, streamer mode,
stream-menu enablement, four shortcut enums, and conditional Deck settings.
Their defaults and options come from SettingsDialog.qml, QmlSettings, and
Settings. This group has no restart-only setting. Audio/video enablement applies
to the next session. Native setters retain their existing signals and effects.

`SettingField.svelte` handles labeled enum and boolean fields. `settings.ts`
contains public labels, descriptions, defaults, and prerequisite targets.
`Settings.svelte` uses that metadata for search and focus. Later categories can
use the same field format and reveal behavior. Their native snapshots and
commands must add explicit allowed values and current-profile validation.
Unsupported fields remain absent from the native snapshot and search results.

The snapshot contains only the allowed general values. The command uses
explicit native setters. It does not write arbitrary QObject properties or
publish all settings. No credential, profile name, or path is added to search.

SettingsDialog.qml still has real callers through MainView.qml and
ConsolePinDialog.qml. Native stream settings also retain their QML routes.
This slice does not remove those resources or their tests.

## Limits and reused evidence

Runtime checks were serialized with the other workers. They used
`QT_QPA_PLATFORM=offscreen` and test-only
`QTWEBENGINE_CHROMIUM_FLAGS=--disable-gpu`, after the offscreen D3D context
failure was identified. Production acceleration remains enabled. The logs
contain offscreen font and GLES context warnings.

These results establish compiled Svelte, real QWebEngineView and WebChannel,
the production bridge, native values, and persistence. They do not establish
physical keyboard focus, visual layout, Steam Deck hardware, live console
sleep/suspend effects, Linux/macOS packages, or GPU acceptance. Those checks
remain in the relevant hardware and acceptance tickets.

The unchanged ticket 03 library/package and native graphics evidence is reused.
No native media, renderer, input, or stream lifecycle source changed. No broad
test suite or hardware matrix was repeated.
