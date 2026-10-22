# Ticket 12 verification

Video, stream profile and audio settings use compiled Svelte, QWebEngineView,
Qt WebChannel, SetupBridge, QmlSettings and the native Settings store.
No browser bridge fixture or web test API is used.

## Source and build

Final checked source is `3f19a3c6`. It includes integration `ae754f2b` through
merge `b3dbb435`. The merge retains both account state and media settings.
The last commit changes tests only. Production source is the same as the
passing state, restart and general-control group at `b3dbb435`.
`identity.json` records the full commit and source/artifact SHA256 values.

The private build directory is
`C:/Users/david/.cache/chiaki-msvc-native/ticket12-build`.
The configure/build commands are saved beside this file. Qt is 6.9.3 MSVC2022
x86_64. The compiler is clang-cl 19.1.7 with the installed MSVC14.44 STL.
Dependencies use the prepared MSVC environment recorded in environment.md.
Pinned submodules were initialized in this worktree. Only the existing
`scripts/windows-vc/gf-complete.patch` was applied locally to the vendor
submodule. That vendor edit is excluded from commits.

## Focused outcomes

| Case | Source | Result |
|---|---|---|
| First category red | `0b813ede` plus the initial uncommitted test | Expected failure, Stream category absent |
| Category green | ticket12 implementation before `8c4bb984` | 3 passed, 0 failed, 743 ms |
| `videoAudioSettings` | `8c4bb984` | 3 passed, 0 failed, 8289 ms |
| Numeric draft red | `8c4bb984` plus the new uncommitted state test | Expected failure, old draft 42 remained after profile switch instead of current 128 |
| `videoAudioState videoAudioRestart generalSettings` | `b3dbb435` | 5 passed, 0 failed, 5840 ms |
| Restart child `videoAudioRestartRead` | `b3dbb435` | 3 passed, 0 failed, 812 ms |
| `videoAudioDevices videoAudioPreview` | `3f19a3c6` | 4 passed, 0 failed, 2757 ms |
| Svelte/TypeScript check | production source at `b3dbb435` | 0 errors, 0 warnings |

Every runtime group held the root-coordinated application slot. Runtime used
`QT_QPA_PLATFORM=offscreen` and test-only `--disable-gpu`. The QTest text files
record actual outcomes. Earlier passing groups were reused. No full suite ran.

The first process launch lacked the private steam-tools DLL directory. It
failed before QTest. After adding that directory to PATH, the GUI-subsystem
process did not write its test report to redirected stdout. The recorded red
run uses QtTest `-o path,txt` and confirms the expected UI failure.

The expanded settings case checks independent resolution, frame rate and
bitrate writes for all four local/remote PS4/PS5 profiles. It checks PS5 codecs,
PS4 codec exclusion, automatic bitrate reset on resolution changes, software
decoder conversion, zero-copy, window dimensions and conditions, cursor and
fullscreen controls. Audio checks cover buffer conversion, volume, microphone,
Speex conditions/levels and connection feedback. Rejected commands include
invalid ranges/types, unsupported names/devices/codecs and an old profile.
Reload and native store reads preserve independent values.

The state case proves that an uncommitted numeric draft resets on a profile
change even if both profiles have the same saved value. Numeric drafts carry
the generation from the first input event. The native bridge rejects old
profile generations. Search reaches a PS5 remote setting and falls back to
the controls required for hidden custom-size and speech fields. Invalid UI
writes restore the current native value. Volume displays its percent value.
Automatic bitrate has a 2..100 Mbps manual range and a separate reset action.
The independent process reads all four saved bitrates, audio buffer and volume,
then verifies current audio fields through its own compiled setup page.

The first uninitialized embedded fixture reported 0/0 audio devices. It does
not establish the host inventory. Production main initializes SDL audio, while
the general embedded fixture does not. The dedicated device case uses the SDL
`dummy` audio driver at the native system boundary. The real async enumeration
service returns named input/output devices. UI selection removes missing saved
names, persists the selected names and retains them after refresh. SDL audio
is closed after that case's native services are destroyed. Production APIs
are unchanged.

The layout case checks the actual viewport and horizontal bounds before each
capture, then permits the renderer to paint. The saved 1280px Video/Stream,
390px Audio/Wifi and 200 percent Audio/Wifi captures were inspected. Controls
and text remain inside the scrollable layout. These captures use software
rendering and do not prove physical focus or GPU behavior.

## Scope and limits

Ticket12 owns Video decoding and stream-window controls, all four stream
profiles, Audio/Wifi and speech settings. Ticket13 owns rendering presets,
renderer backend/custom controls, display targets and queue depth. Native
setters retain persistence, notifications and the existing live-volume update
connection. Decoder choices retain the original constant native inventory;
`none` maps to an empty native decoder string. Audio device lists remain native.
No secrets or unrestricted QObject writer are exposed.

The shared SettingsDialog.qml resource still has MainView, ConsolePinDialog,
unmigrated-group and fixture callers. It remains until those callers migrate.
No new setup-only callback adapter was added. Final removal belongs to the
remaining migration/cleanup work. Native stream settings remain unchanged.

Physical speakers, microphones, device hotplug, decoder hardware, HDR, haptics,
console playback and audible live volume are not verified. Linux/macOS build
conditions are preserved in source but were not executed. Steam Deck haptics
remain with the general/controller tickets. GPU acceleration and physical
focus acceptance remain root-owned. These software checks do not establish
release packaging or hardware performance.
