# Ticket 20 checkpoint

Historical preparation checkpoint. The user subsequently skipped the remaining build and performance tests to prioritize prototype fidelity. Integration `1d769ee9ec3bfd267137d8ca738255feed9c7f90` was merged in `4c1e7a08`, including ticket 22 recovery and ticket 19 common package preflight and notices. The source milestone is now READY for merge. [Final evidence](verification.md) supersedes the waiting state and resume plan below. Do not run the remaining commands under the current request. No application package acceptance is claimed.

## Source and scope

The worktree is `C:/Users/david/.codex/worktrees/web-ui-ticket-20/chiaki-ng-ui`, branch `codex/web-ui-ticket-20`. The integration base is `41c6134c0c42227b7776f8df95a3fbbc821bd452`.

Implementation commits are `c1fc84f9`, `69b9a39b`, `bd17c1bf`, and `7831e7b7`. The tested source archive is from `7831e7b720549720a65bb4f9b679a9d574b94bf4`, SHA256 `d93558a4df248e00a10ab0c46172dec5b5b25963c4489fe449119594af30345d`.

Windows Git produced the archive with `git -c core.autocrlf=false archive HEAD`. This setting is also necessary for the six submodule archives. Without it, Windows Git writes CRLF into Linux shell scripts. The archives contain tracked source and the exact desktop submodule pins, without Git metadata, private caches, frontend node_modules, or saved QA evidence.

The source identity is also saved in `C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket20-source/source.json`.

Changes reach the maintained main, legacy, and development Flatpak manifests, manual workflows, and Linux jobs in release, weekly, and PR workflows. The manifests use the selected checkout through one shared source module. They no longer fetch an unrelated upstream branch. The frontend still builds through the mandatory CMake RCC target.

The obsolete ARM AppImage and Qt 5 compatibility paths fail explicitly. Release, weekly, and PR jobs no longer expect an ARM AppImage asset. The ARM Flatpak configuration remains, with no ARM build or runtime pass claimed.

## Completed checks

The saved LF script `ticket20-checkpoint-checks.sh` completed with exit 0 on 2026-10-22. It checked shell syntax, workflow YAML, manifest JSON and YAML, all three expanded Flatpak manifests, the rejection tests, and the actual installed tool versions.

`test/linux-package-check.sh` passed four rejection cases: missing executable, missing WebEngine helper, helper without execute permission, and missing Widgets library. The positive package check is pending. It requires the shared `--check-package` flag from ticket 19, the embedded frontend, WebChannel, native QML plugins, WebEngine files, locales, and common frontend notices. AppImage Qt libraries must resolve inside the extracted package. The check rejects a packaged Node executable.

The AppImage builder completed with exit 0. Its immutable image identity is:

```text
sha256:1837c17a33b22ad995a4f306e2459f07434aa1df083fa7e9720b8ad825cf1b07
```

The local image tag is `chiaki-appimage-ticket20`. Its Dockerfile uses the prepared immutable builder base, then installs the complete Qt 6.9.3 SDK and matching official WebEngine extension. A narrow adapter gives the old builder's pinned aqt installer the directory layout that its original relocation code expects. The build checks matching GuiPrivate and WebEngineWidgets CMake files.

Direct inspection returned Qt Core, Gui, Widgets, WebEngineCore, WebEngineWidgets, WebChannel, and Quick version `6.9.3`. Node is `24.16.0`, and npm is `11.13.0`. The helper's dependencies resolve with no missing library in the builder. This is a toolchain check, not an application package pass.

The AppImage SDK helper SHA256 is `6aa071c1652ec5cb6eb78c9245cb33efb728c9d4e61ab9233db30f9b1fddf533`. The extension archive SHA256 is `8ac7e13152a1d3415e8d274a905c692633152286d815b165378d42a829801236`. Its official source is the [Qt 6.9.3 extension repository](https://download.qt.io/online/qtsdkrepository/linux_x64/extensions/qtwebengine/693/x86_64/). The Node archive SHA256 is `d804845d34eddc21dc1092b519d643ef40b1f58ec5dec5c22b1f4bd8fabde6c9`.

The Flatpak dependency build completed with exit 0 and stopped before `chiaki-ng`. It uses the already installed isolated KDE SDK, Platform, and WebEngine BaseApp 6.11. Direct inspection returned matching Qt Core, Gui, WebChannel, Quick, and WebEngineWidgets `6.11.2`.

The built dependency prefix reports:

| Module | Version |
| --- | --- |
| FFmpeg avcodec | 61.19.100 |
| FFmpeg avutil | 59.39.100 |
| FFmpeg avformat | 61.7.100 |
| libplacebo | 7.360.1 |
| SDL2 compatibility | 2.32.64 |
| FFTW | 3.3.10 |
| Opus | 1.5.2 |
| SpeexDSP | 1.2.1 |
| json-c | 0.18 |
| miniupnpc | 2.3.0 |

The avformat build is deliberate. The previous static FFmpeg module omitted it and could select the SDK's different FFmpeg version. The json-c and application CMake builds set the minimum policy required by the SDK's CMake 4.

The offline npm source list was generated from the current lockfile in clean external staging without node_modules. It includes build dependencies. There are 71 locked packages and 144 Flatpak sources. The generator is the prepared pinned flatpak-builder-tools checkout. No final application frontend build in Flatpak has run yet.

## Historical resume plan

No GUI or native runtime process was started. All builder and dependency processes have finished. There are no application binaries or final package hashes yet. The checkpoint check session also finished with exit 0.

The external tool and build root is `/home/david/.cache/chiaki-web-ui/ticket20`. Its retained files include:

- `appimage-builder.log` for the successful final builder.
- `aqt-latest.log` for the successful SDK relocation inspection.
- `flatpak-download.log` and `flatpak-deps-build.log`.
- `flatpak-state` with downloaded inputs and built dependency cache.
- `flatpak-build` with the dependency prefix, stopped before the application.
- `source` with the last archive for inspection.
- `*.yaml.expanded.json` for the three manifests.

The reusable scripts are under `C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/`. `ticket20-export.py` uses Windows Git and forces LF for the root and submodule archives. `ticket20-flatpak-deps.sh` is a dependency preparation script, not the final application build. `ticket20-builder.sh` already passed and must not rerun unless a relevant input changes.

After the main agent provides the approved integration revision:

1. Merge that revision into this worktree. Preserve the Windows and macOS workflow job blocks.
2. Confirm the shared `--check-package` flag and installed `share/licenses/chiaki-ng/frontend/THIRD-PARTY-NOTICES.txt` are present.
3. Export the exact current source with `ticket20-export.py`. Record the revision, diff, new files, and archive identity.
4. Build the application and Flatpak from the retained offline dependency cache. Use the manifest in the new exported checkout, `--disable-download`, the existing `flatpak-state`, and a separate package repository. Then run the package check with the Platform runtime, not the SDK.
5. Build the AppImage from the exported source with `chiaki-appimage-ticket20` and the maintained `scripts/build-appimage.sh`. A build-only `APPIMAGE_EXTRACT_AND_RUN=1` can run the deployment tools where FUSE is unavailable. It is not a Chromium or shipping sandbox option.
6. Record the full source, binary, helper, notice, and package hashes before runtime checks. Verify the actual extracted AppImage and installed Flatpak contents, missing helper rejection, and missing asset rejection.
7. Request the main agent's GUI slot before any application or harness launch. Use an isolated profile and prevent PSN, LAN, process launch, registration, and session side effects before native service constructors. Software-only checks must state their limits.
8. Check offline startup, WebEngine helper and sandbox operation, native file actions, and affected setup/native handoff behavior where the available environment permits. Check each relevant final artifact once. Repeat only for a relevant change or concrete failure.

Linux GPU, HDR, X11/Wayland/gamescope focus, physical devices, live console playback, audio and microphone acceptance, and performance measurements remain unavailable on this Windows-only host. WSL llvmpipe does not satisfy those gates. Record them in the final ticket answer and ticket 23. Do not turn this checkpoint into a release acceptance claim.
