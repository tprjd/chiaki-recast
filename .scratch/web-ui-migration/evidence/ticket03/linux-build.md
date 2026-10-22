# Ticket 03 Linux changed-source compile

PASS for committed source `0e2702b7ce333a5cb121c077dee17cde99b0f7e1` from `codex/web-ui-ticket-03`. The full native application and all requested embedded/native test executables compiled and linked. One offscreen `chiaki --help` exited 0. No test executable, GUI, focus check or GPU assertion ran.

## Inputs and location

- Source: `/home/david/.cache/chiaki-web-ui/source-ticket03-0e2702b7ce333a5cb121c077dee17cde99b0f7e1`.
- Independent build: `/home/david/.cache/chiaki-web-ui/build-ticket03`. The baseline cache was not repointed or rebuilt.
- Archive input directory: `C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/linux-ticket03-archives-0e2702b7ce333a5cb121c077dee17cde99b0f7e1`.
- Exact source/submodule commits and each archive SHA256: `input-manifest.json` in that archive directory and the evidence directory below. Archives came from git objects, not dirty worktree files. Main, integration and ticket source were not changed.
- Desktop submodules: munit `439de4a9b136bc3b5163e73d4caf37c590bef875`; cpp-steam-tools `d36565f8eaca113efd454efe714126cd77a01512`; curl `b1ef0e1a01c0bb6ee5367bd9c186a603bde3615a`; gf-complete `fa54a4670a5705c84abf6c24b92b0cd479625478`; jerasure `505ccb4bc69eee8d7ea40a6089a056b99671134f`; nanopb `cad3c18ef15a663e30e3e43e3a752b66378adec1`.
- Android oboe and Switch borealis are recorded but excluded because those platform targets are off.
- Post-build archive audit compared 9,445 regular input files: no changed archived file; no .git metadata. In particular, the MSVC gf-complete working patch was excluded. No vendor/application patch was applied for Linux. Existing CMake SteamTools compatibility handling works on its build-directory copy.

## Selected environment

WSL Ubuntu 26.04, GCC/G++ 15.2.0, CMake 4.2.3, Qt 6.10.2, libplacebo 7.360.0, FFmpeg avcodec 62.11.100, SDL2 2.32.10. Qt Widgets/WebEngineWidgets/WebChannel and GuiPrivate were found in `/usr/lib/x86_64-linux-gnu/cmake`. Pinned Node v24.16.0 and npm 11.13.0 were selected from `package-tools/node-v24.16.0-linux-x64/bin`. Initial WSL capacity was 657 GiB free.

The selected Python is `/usr/bin/python3`, with protobuf 4.21.12. Source the package-tool env for its isolated npm cache, then reset PATH so its Flatpak-generator Python virtualenv cannot replace system Python:

```sh
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
source /home/david/.cache/chiaki-web-ui/package-tools/env.sh
export PATH=/home/david/.cache/chiaki-web-ui/package-tools/node-v24.16.0-linux-x64/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
```

## Commands and results

Saved wrappers beside this note record the actual sequence. All shell variables execute through saved LF scripts with `wsl --exec /bin/bash`, not a nested default-shell wrapper.

```powershell
& 'C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/stage-linux-ticket03.ps1' -Commit 0e2702b7
wsl --exec /bin/bash /mnt/c/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/build-linux-ticket03.sh 0e2702b7ce333a5cb121c077dee17cde99b0f7e1
wsl --exec /bin/bash /mnt/c/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/resume-linux-ticket03.sh
wsl --exec /bin/bash /mnt/c/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/resume-linux-ticket03-c11.sh
wsl --exec /bin/bash /mnt/c/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/finalize-linux-ticket03.sh
```

First configure passed. Two environment issues stopped compilation and were corrected in the same independent build, without source edits:

1. `package-tools/env.sh` prepended its Flatpak-generator Python virtualenv. Nanopb could not import google.protobuf. Explicit system Python and corrected PATH fixed this. Preserved log: `build-first-python-environment-failure.log`.
2. GCC 15 defaults to C23. Pinned munit uses removed `ATOMIC_VAR_INIT` at `test/munit/munit.c:802,814`. Explicit `CMAKE_C_STANDARD=11` fixed compilation without a vendor patch. Preserved log: `build-second-munit-c23-failure.log`.

The effective configure for a future fresh build is:

```sh
cmake -S "$source_dir" -B "$build_dir" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCHIAKI_ENABLE_CLI=OFF -DCHIAKI_ENABLE_TESTS=ON -DCHIAKI_ENABLE_STEAMDECK_NATIVE=OFF -DCHIAKI_USE_SYSTEM_CURL=OFF -DPYTHON_EXECUTABLE=/usr/bin/python3 -DCMAKE_C_STANDARD=11
cmake --build "$build_dir" --target chiaki chiaki-embedded-setup-tests chiaki-native-services-tests chiaki-stream-input-native-tests chiaki-mouse-response-tests chiaki-stream-input-tests chiaki-console-screen-tests chiaki-unit --parallel 8
QT_QPA_PLATFORM=offscreen "$build_dir/gui/chiaki" --help
```

Final configure/build/help status files are all 0. The frontend custom command installed 46 pinned packages through npm ci and built 111 modules through Vite 8.3.5. Outputs include index.html/app.js/app.css compiled into the setup resource. No frontend test/type-check command ran. This build used the npm cache/network normally; it is not proof of a fully offline frontend build.

The native GUI sources including setupwindow.cpp, setupbridge.cpp and qmlmainwindow.cpp compiled with Qt 6.10.2/private QRhi. Final links include the executable, all seven requested test executables and native library dependencies. ldd has no missing dependency; it resolves Qt WebEngineWidgets/Widgets/Quick/Gui/WebChannel, native libplacebo and FFmpeg. Expected warnings include Qt private API version coupling, FFmpeg 8 deprecated Vulkan fields, old vendor C const conversions and ignored QFile::open return values. No application compile error remained.

One evidence-wrapper digest command initially used `gui/` instead of `gui/tests/` for test executable paths. The independent finalizer corrected the artifact paths and generated complete hashes, without rebuilding or repeating --help.

## Reusable evidence

Windows-accessible copied evidence: `C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/linux-ticket03-evidence/`.

It contains configure/build/help logs and exit codes, both earlier failed logs, environment.log, CMakeCache.txt, exact input-manifest.json, source-audit.json, runtime-links.txt and complete artifact-sha256.txt. Original logs remain under the WSL build path. Actual test executables are under `build-ticket03/gui/tests/`; chiaki-unit is under `build-ticket03/test/`.

Full native application SHA256:

```text
26ee1929e88a8b37411ae97d74df6e277774db88132f5be5538e02690976cd57  gui/chiaki
```

This proves changed-source Linux compilation, correct frontend tool selection and one command-line startup only. WSL graphics remain software llvmpipe; no hardware GPU, desktop focus, compositor, physical input, real-console, PSN, package or regression acceptance is claimed. No baseline rerun, test suite, GUI launch, sandbox override, OS package installation or credential access occurred.