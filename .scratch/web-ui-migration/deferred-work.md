# Deferred validation and known issues

Recorded: 2026-10-22.
State: deferred at the user's request; resume when requested.
Branch: `codex/web-ui-migration-integration`.
Implemented source and UI: `0d09776eda3ea5f383b5098bdecaf43b8dc005ea`.

The user requested that remaining build validation and performance tests be
skipped while the UI was brought to the selected prototype standard.
The migration tickets are resolved for that source and UI scope.
Their unchecked release and hardware criteria remain unverified.

## Current application and completed checks

The current Windows application was subsequently built at the user's request
and opened with the real embedded web UI. Its existing profile and registered
console appeared. This confirms compilation and normal startup on this PC.
It does not establish live playback or release readiness.

Local executable:
`C:/Users/david/.cache/chiaki-msvc-native/current-app/chiaki.exe`.
Executable SHA-256:
`603315773f4a7d80bcd71d7309abec0e98547800cf1061179740f2597d4d1f32`.
Runtime dependencies came from the earlier verified Windows package stage.
The copied package manifest describes that earlier package, not this new local
executable. `LOCAL-BUILD.txt` beside the executable records this distinction.

Completed evidence also includes:

- Frontend compilation and type checks, plus layout review at 1024 and 1280
  CSS pixels. No new acceptance work is required below 1024 CSS pixels.
- Focused browser navigation checks and fixes for controller focus, checkbox
  activation, registration progress reopening, and unsupported management actions.
- The final standards and specification review, with its findings fixed.
- Earlier controlled native service, decoder, cleanup, and Windows GPU checks,
  within the source and hardware limits stated in their individual reports.

See [UI verification](evidence/ticket23/verification.md),
[final review fixes](evidence/final-review-fixes.md), and
[final review record](evidence/final-review/integration.md).
These results do not replace the deferred checks below.

## Known issue: HighQuality native rendering

**State: reproduced in controlled tests; root cause unresolved.**
The effect on live console playback remains unverified.

The native HighQuality preset produced black or magenta output where a known
red test frame was expected. Native overlays still rendered correctly.
The Fast preset rendered the expected frame in the recorded OpenGL and Vulkan
checks. Production defaults and the user's preset were left unchanged.

A standalone libplacebo mixed-frame control reproduced incorrect output without
Qt Quick, WebEngine, WebChannel, screen capture, or a live console connection.
With compute enabled, both readbacks were `0/0/0/0`. With compute disabled,
both were `254/0/0/255`, the expected red. This result isolates a native test
failure from the HTML UI. The exact library, driver, shader, or cache cause
has not been identified.

Recorded environment: Windows, NVIDIA RTX 5090, driver 617.42, Qt 6.9.3,
libplacebo 7.360.1, and FFmpeg 7.1. These are the recorded test versions.
The controls do not establish a full pre-migration application baseline.

Evidence and reproducible source:

- [Standalone mixed-frame control](evidence/native-highquality-control/pure-libplacebo-mix-control/verification.md).
- [Standalone direct-render control](evidence/native-highquality-control/pure-libplacebo-control/verification.md).
- [Cadence control](evidence/native-highquality-control/highquality-cadence/verification.md).
- [Combined browser and native graphics findings](evidence/ticket03/verification.md).

Deferred diagnosis must compare the native controls and a matched application
baseline with actual loaded dependency identities. A permanent preset downgrade
or a compute-disable workaround has not been approved.

## Skipped and unverified checks

| Area | Remaining work | Existing record |
| --- | --- | --- |
| Current Windows package | Validate the final portable package and installer on a clean environment. Confirm contents, dependencies, notices, helper startup, sandbox behavior, and normal and direct launch for the final source. The local build and visible startup above cover this PC only. | [Windows evidence](evidence/ticket19/verification.md) |
| Windows installer | Compile and exercise the Inno Setup installer. An earlier local compiler installation was rejected by automatic approval review with only `blocked by policy` as its reason. Installer scripts exist; local installer validation remains incomplete. | [Windows ticket](issues/19-windows-web-native-package.md) |
| Linux | Build the final application, AppImage, and Flatpak packages. Check offline startup, contents, helpers, sandbox, dialogs, native handoff, and playback on actual Linux systems. Cover supported X11, Wayland, and gamescope conditions as applicable. | [Linux evidence](evidence/ticket20/verification.md) |
| macOS | Build both supported architectures and DMG packages. Check deployment, signatures, entitlements, notarization, Gatekeeper, clean-machine startup, helpers, GPU output, and native handoff on actual Macs. | [macOS evidence](evidence/ticket21/verification.md) |
| Live media and services | Verify real-console connection and disconnect, first and subsequent sessions, real decoded-frame cleanup, recovery, and relevant local and PSN routes. Check SDR, HDR, supported decoders, required presets, audio output, microphone, and haptics on available hardware. | [Native handoff evidence](evidence/ticket04/verification.md), [recovery evidence](evidence/ticket22/verification.md) |
| Desktop and physical input | Verify foreground focus during native handoff and setup restoration, held-input release, physical controller and keyboard/mouse behavior, touch, screen-reader labels, translated text, display scaling, and 200% zoom at widths of at least 1024 CSS pixels. Browser key checks do not establish physical controller operation. | [Final acceptance ticket](issues/23-complete-web-only-acceptance.md) |
| Sleep and teardown | Exercise actual OS suspend/resume, console sleep, cancellation, failure, and late completion with live media. Confirm release of workers, frames, GPU resources, callbacks, input, and browser helpers according to their lifetimes. | [Recovery evidence](evidence/ticket22/verification.md), [service evidence](evidence/ticket01/verification.md) |
| Performance and latency | Complete the matched native baseline comparison, sustained playback, and external input-latency measurement when equipment is available. Record frame timing, queue depth, pending frame age, drops, audio underruns, CPU, GPU, total memory including Chromium, and time to first frame. | [Baseline conditions and tolerances](evidence/ticket01/baseline.md), [final acceptance ticket](issues/23-complete-web-only-acceptance.md) |

Only this Windows PC was available. The user confirmed that the console uses
Ethernet. The PC's active physical connection was also observed as Ethernet.
Linux and macOS hardware results cannot be inferred from this host or WSL.

Windows ARM64 and MinGW packages, ARM AppImage, and Qt 5 compatibility packages
are explicitly unsupported by the current source. They are not pending passing
checks. ARM Flatpak remains configured but unverified.

## Resume policy

The smallest first live check is connection, visible native playback, disconnect,
and return to setup on this Windows PC. Record the exact source, package,
dependencies, driver, console, network, profile, renderer, decoder, and display
conditions before interpreting the result.

Reuse applicable passing evidence. Run focused checks for changed behavior or
unresolved risks. Repeat or broaden checks only for relevant changes, failures,
uncertain variation, results near a tolerance, or signs of a leak or race.
Performance conclusions require valid baseline variation and the predeclared
tolerances. Inconclusive measurements remain inconclusive.

The HighQuality diagnosis and platform qualification remain separate work.
No background test or follow-up schedule was created by this record.
