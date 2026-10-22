# Ticket 20 source milestone

The user skipped the remaining build tests and performance tests to prioritize prototype fidelity. Ticket 20 is resolved for source implementation. Linux application and release package acceptance is unverified.

## Source

The branch is `codex/web-ui-ticket-20`. Integration `1d769ee9ec3bfd267137d8ca738255feed9c7f90` was merged without conflicts in `4c1e7a08`. This includes the ticket 19 shared `--check-package` preflight, mandatory frontend notice output and install, and ticket 22 recovery changes. The Linux workflow changes preserve the other platform job blocks.

The latest priority integration `06b38253e60cb6c4db1a3a2fa73a3b2d8bbba421` was merged without conflicts in `704e15bb`. Both the Linux source milestone and the appended prototype-fidelity priority remain in the map. No further build or test ran.

Implementation commits are `c1fc84f9`, `69b9a39b`, `bd17c1bf`, and `7831e7b7`. Checkpoint commit `c5cf60b5` retains the completed preparation and exact external cache paths.

The AppImage Dockerfile pins its builder base, complete Qt 6.9.3 SDK, matching WebEngine extension, and Node 24.16.0 with npm 11.13.0. The Flatpak main, legacy, and development manifests use runtime and WebEngine BaseApp 6.11, the selected checkout, and one shared application source module. Locked npm sources include the build dependencies. The mandatory CMake frontend target builds and embeds the page. Node is build-only.

Native Qt Quick deployment, FFmpeg, libplacebo, SDL, Steam shortcut support, and haptic dependencies remain. The Flatpak FFmpeg module now supplies avformat with its own avcodec and avutil. Package checks require embedded frontend assets, WebChannel, native QML plugins, helper execute permission, WebEngine resources and locales, and the shared frontend notice. AppImage checks reject Qt dependencies resolved from the host. The checks reuse the shared read-only preflight before native services start.

ARM AppImage and Qt 5 compatibility paths stop with an explicit unsupported result. Release, weekly, and PR workflows no longer expect an ARM AppImage artifact. ARM Flatpak remains configured without an ARM build or runtime result. Source-build instructions use the checkout and matching runtime.

## Reused preparation

[The checkpoint](checkpoint.md) records the matching AppImage SDK builder pass, Flatpak dependency build pass, exact versions and hashes, and source archive with six pinned desktop submodules. It also records four rejection fixture passes, shell and manifest syntax checks, and the three expanded manifests. Those checks were completed before the final integration merge. They are not final application or package passes.

The external caches, source archive, logs, and scripts are preserved. No new source export, build, package, runtime, performance test, or validation matrix was run after the user's override.

## Skipped validation

Final Linux application compilation, AppImage and Flatpak creation, actual package contents and offline startup, helper execution and Chromium sandbox operation, file dialogs, window handoff, and native playback checks are skipped at the user's request. Performance tests are also skipped. There are no final Linux application binary or package hashes.

Linux hardware GPU, HDR, X11, Wayland, gamescope focus, physical input, audio, microphone, and live-console acceptance remain unavailable on the Windows-only host. WSL software graphics cannot establish those results. The ticket's unchecked acceptance items remain unverified. This source milestone does not establish Linux release readiness.
