# Graphics build provenance audit

Read-only audit: no header/import-library/staged-DLL mismatch found. This audit does not establish the runtime cause of black video. No build, test, GUI launch, repository mutation or credential access occurred.

## Libplacebo source, actual headers and compiler

Root `C:/Users/david/.cache/chiaki-msvc-native/` contains clean libplacebo source HEAD `cee9b076f2c63104ccfd497fa79c39a867293ec4`. `git status --short` was empty.

Ticket03 `ticket03-build/compile_commands.json` and Ninja's recorded valid dependency entries for `gui/CMakeFiles/chiaki-gui.dir/src/qmlmainwindow.cpp.obj` and `gui/CMakeFiles/placebo-libav-impl.dir/src/libav_impl.c.obj` both select `libplacebo-prefix/include/libplacebo/` and the same prepared FFmpeg include root. The dependency evidence is saved beside this note in `graphics-build-actual-includes.txt`. This checks actual recorded includes, not only CMake search preferences.

All 33 source public headers that have installed counterparts match byte-for-byte between `libplacebo/src/include/libplacebo/` and `libplacebo-prefix/include/libplacebo/`. This includes renderer.h, colorspace.h, opengl.h and utils/libav.h. The generated prefix config.h also matches `libplacebo-build/src/include/libplacebo/config.h`, SHA256 `4dae6e44c23671f0403b2e3dc44a0573bf379321cc98eb4672b5c21972ec672c`.

Config: PL_MAJOR_VER 7, PL_API_VER 360; pkgconfig reports 7.360.1. OpenGL/Vulkan/D3D11/shaderc/lcms/DOVI enabled; libdovi/xxhash/glslang absent. No alternate UCRT/MinGW libplacebo headers occur in these recorded dependencies.

Both application and libplacebo use the prepared LLVM19 clang-cl 19.1.7 x64 MSVC ABI and /MD release runtime. The renderer is compiled as C11; QmlMainWindow as C++17; the application's PL_LIBAV_IMPLEMENTATION wrapper is C using the same installed header. All 64 libplacebo compilation commands and all application commands were checked: no /Zp, -fpack, -mms-bitfields or -fshort-enums flags. No packing pragma or C/C++ conditional struct layout was found in the public renderer/color/OpenGL headers; extern-C wrappers control linkage only. No obvious renderer-struct packing mismatch was found. No new compiled sizeof/offset probe was run.

## Linked versus staged binary

`ticket03-build/CMakeCache.txt:1153` and build.ninja LINK_LIBRARIES select exactly `libplacebo-prefix/lib/libplacebo.lib`. It is an x64 MSVC-compatible import library whose DLL name is `libplacebo-360.dll`.

- Prefix import lib and `libplacebo-build/src/libplacebo.lib` SHA256 both `8a76f457fe8dde7299435531b41b8e9fdf3792ef22a1c3bbc9c3229e9dfc74ab`.
- Prefix DLL `libplacebo-prefix/bin/libplacebo-360.dll`, build DLL `libplacebo-build/src/libplacebo-360.dll`, and staged `C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket03-package/libplacebo-360.dll` SHA256 all `cf4e4dccea7d0e9924206902a2b37513d2ad5b4fbc6631f2f54595e726bbeb20`.
- DLL machine is PE8664 x64. Staged chiaki-embedded-setup-tests.exe imports libplacebo-360.dll by that name. The libplacebo DLL imports shaderc_shared/vulkan/lcms/MSVC runtime, not FFmpeg.

Full path/hash records are in `graphics-build-hashes.json` beside this note.

## FFmpeg boundary

Both QmlMainWindow and libav_impl.c actual dependency records select `C:/Users/david/.cache/chiaki-ui-native/ffmpeg/ffmpeg-n7.1-latest-win64-gpl-shared-7.1/include/`. Link paths select that same prefix's avcodec.lib/avutil.lib/avformat.lib. Headers report avcodec 61.19.101 and avutil 59.39.100, matching their .pc/CMake component versions and imported avcodec-61.dll/avutil-59.dll names. CMake's aggregate FFMPEG version 59.39.100 is the avutil component version, not evidence of an avcodec 59 mismatch.

Staged DLL hashes exactly match their source prefix bin files:

- avcodec-61.dll: `dbe62f12cc2c8d67b68ac59ea310af8d76cf8d1a8c7742d39df49be4e6dd15a1`.
- avutil-59.dll: `52ae921161e25b1d4c4872f3d661dcc6ecf158d2f46ee64b9d5cae1df3fd7737`.
- avformat-61.dll: `9066fdef1e03a3bb6970e2dda10665ba8e344e95f539e3d83fe205427a74d172`.

No FFmpeg major-version/path mixture found. The FFmpeg distribution uses its existing C ABI across the MSVC-compatible import library; this bounded audit does not prove every AVFrame field offset without a new compiler/runtime probe, which was prohibited.

## Loaded-module limit

No process with a name beginning chiaki was live during either read-only process/module inventory. Therefore actual loaded DLL path/hash for the failed run cannot be verified here. The staged identity is clean; do not describe it as proof of loaded identity. Ticket03 was asked for any already-recorded loaded-module evidence, without a new launch. Runtime owner can inspect an already-running failing probe's module path if one becomes available.

Audit file hashes for reproducibility:

```text
e5ca70729901c7f1427d1e512e6ed8b6b1794e8fde1114837d438349e5db400e  ticket03-build/compile_commands.json
0ccf3ea0178e8dbab7a3f689380fa5375739040de235e069103a1db3278491e2  libplacebo-build/compile_commands.json
d4312f9fd3550ee5f1ece4cbb894c57dab74ecd2b139cf39775957dbda5aa903  prefix/include/libplacebo/renderer.h
c0f43fea41792891f63ab79f74ccbd7038f7764703bff415fa137f736e4bf072  prefix/include/libplacebo/colorspace.h
287f9ac873c254a4f4fdabd86fc0ee8ea2fb1eb99f418ad278e9ae474e02c65b  prefix/include/libplacebo/opengl.h
cfe445e9ef07df1476c6a4281e7fe62c99e44d636833ce632e7b3f860591894c  prefix/include/libplacebo/utils/libav.h
```