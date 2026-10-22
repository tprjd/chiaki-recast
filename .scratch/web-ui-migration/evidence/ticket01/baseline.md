# Ticket 01 baseline and prerequisite, recorded before ownership edits

Base: 547c608 on codex/web-ui-migration-integration. The ticket branch descends from this tip.
Machine: Windows 11, NVIDIA GeForce RTX 5090, driver 32.0.16.1742 / OpenGL 617.42.
Native toolchain: GCC 16.2.0, Qt 6.11.2, libplacebo 7.360.1, from C:/Users/david/.cache/chiaki-ui-native/msys64/ucrt64.

Bounded prerequisite: no Qt WebEngine CMake package exists in that native prefix. MinGW cannot build Qt WebEngine. A compatible WebEngine/native coexistence run is unverified. No GPU-disabling workaround is accepted. Ticket 03 owns removal of the existing startup --disable-gpu override.

Baseline command, with native prefix/bin and build/keyboard-mouse-native/third-party/steam-tools on PATH:
C:/Users/david/Projects/chiaki-ng-ui/build/keyboard-mouse-native/gui/tests/chiaki-stream-input-native-tests.exe sessionAndFeedback -o .scratch/web-ui-migration/evidence/ticket01/baseline-native-opengl.txt,txt
Result: 3 passed, 0 failed, 0 skipped, 2138 ms. This existing executable is baseline evidence only. The test used real native OpenGL rendering and controlled session transport, with isolated test settings. It did not stream a console.

Live baseline conditions remain unmeasured: console, network, decoder, saved profile, display HDR mode, microphone, physical controller, external input latency. The native probe uses loopback session objects and software decode, 900x600 test geometry. It cannot establish live performance equivalence.

Predeclared live comparison conditions for later acceptance: use the same machine, driver, console, wired network, decoder, renderer, profile, resolution, refresh rate and HDR state in baseline and migrated runs. Record each value before either run. Warm up both until frame timing is stable. Reject a repeatable increase above 1 ms in median or p95 native presentation age, above 1 ms in available external input latency, any new sustained frame-loss trend, or resource growth across repeated connections. Treat results inside run-to-run noise as inconclusive and repeat only the affected comparison. Functional tolerance is zero changed saved settings, held inputs, lost cleanup, or duplicate sessions. No current hardware result is claimed against these limits.
