# Windows native build baseline with WebEngine

Source: 547c608e3bd8441705af8696edbc11b74b5247db, before native ownership changes.
Result: the full production chiaki target linked and one --help startup exited 0.
No application source fix or private QA hook was needed.

The fixed external source snapshot contains the pinned desktop submodules.
It applies only the existing VC workflow's gf-complete intrinsic-header patch.
The production Steam tools build patch runs in its normal build-directory copy.
The source audit compared 9,376 archived inputs. The gf-complete file is the
only changed archived input. Generated resources and Python caches are recorded.

Toolchain: clang-cl 19.1.7, MSVC 14.44 headers and libraries, Qt 6.9.3 MSVC x64,
libplacebo 7.360.1, FFmpeg 7.1, and the existing VC workflow's vcpkg baseline.
Qt WebEngineWidgets, WebChannel, and their matching runtime modules are installed.
The baseline app links its existing WebEngineQuick/Core support. The new setup
view and bridge do not yet exist in this source.

Executable:
C:/Users/david/.cache/chiaki-msvc-native/baseline-547c608e-build/gui/chiaki.exe.
SHA256: ade7f04e9a3e302a8b257b00a8628c5355040e7a216c79cedebf2120eae278e7.
The main agent checked the actual executable hash and saved help output/exit value.
It did not repeat the passing build or startup check.

Reusable dependency environment:
C:/Users/david/.cache/chiaki-msvc-native/environment.cmd.
Use a separate build directory for each ticket source. Do not repoint this
baseline's CMake cache. GNU libraries and Qt modules are kept separate.

Exact configure/build commands, module versions, dependency failures and fixes,
source audit, artifact hashes, and raw logs are retained outside the repository:
C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/msvc-baseline.md.
The sibling environment.md records dependency setup. Required Python generator
packages are in the external dependency prefix, not the user's global Python.

## Limits

This check establishes a loadable native application and compatible dependencies.
It does not prove accelerated Chromium, the new QWebEngineView/WebChannel path,
native renderer coexistence, live playback, HDR, physical devices, latency,
or release deployment. The MSVC build omits GNU-only native input targets.
No regression suite or GPU probe ran. Tickets 03, 04, and 23 retain those checks.
