# Linux build baseline

Source: integration revision 547c608e, before native ownership changes.
Environment: Ubuntu 26.04 WSL2, GCC 15.2, Qt 6.10.2, libplacebo 7.360.0,
FFmpeg avcodec 62.11.100, and SDL2 2.32.10.

The full production chiaki target configured and linked with optional
WebEngineQuick available. WebEngineWidgets and WebChannel development
modules are installed. This baseline does not yet contain the web setup UI.

Build directory: /home/david/.cache/chiaki-web-ui/build-baseline.
Executable: gui/chiaki within that directory.
Executable SHA256: 191fca56bfaa26dc798e7d8061450c52dc160d732b6ce9198fd408b26e2a8f38.

Configuration uses Ninja, Release, tests enabled, CLI disabled, and Steam Deck
native integration disabled. Desktop submodules use the pinned revisions.
The native source was not changed for this build. The process-local Linux
PATH excludes inherited Windows paths. Those paths caused the first nanopb
generation command to fail because of shell quoting.

The compile log ends with Linking CXX executable gui/chiaki. A nested shell
wrapper then returned an error because it expanded a status variable before
assignment. The build was not repeated solely to change that wrapper status.
The produced executable ran --help with QT_QPA_PLATFORM=offscreen and exited 0.
FFmpeg 8 Vulkan queue fields generated deprecation warnings, not compile errors.

The persistent build log is at
/home/david/.cache/chiaki-web-ui/baseline-build.log. Reusable environment notes
and a corrected build command are outside the repository in the task's
chiaki-web-ui-migration temporary directory.

## Limits

The graphics query reports Accelerated: no and llvmpipe LLVM 21.1.8.
This environment can test real embedded transport and native services.
It cannot establish GPU coexistence, HDR, physical input, real-console
playback, or performance equivalence. No such result is claimed here.
