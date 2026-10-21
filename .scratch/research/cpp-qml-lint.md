# C++ and QML lint research

Research date: 2026-10-21.

The smallest useful setup is `clang-tidy` for C++ and `qmllint` for QML, with one command to run both. Full checks across the C++ and QML interface need type descriptions for the application module. Adding two executable calls alone does not provide those checks.

## Project evidence

- `gui/CMakeLists.txt` creates `chiaki` with `add_executable`. QML files enter through `gui/src/qml/qml.qrc`. It does not use `qt_add_qml_module`.
- `gui/src/qmlbackend.cpp:151-154` registers `Chiaki`, `ChiakiWindow`, `ChiakiSettings`, and `ChiakiSession` at runtime.
- `gui/include/qmlbackend.h` exposes typed settings and session objects, but also `QVariantMap` and `QVariantList` values. Type descriptions alone cannot define every key in these dynamic values.
- `gui/tests/imports/org/streetpea/chiaking/Chiaki.qml` contains only `property var settings: null`. This test substitute does not describe the production API.
- `.github/workflows/build-windows-x86.yml` already uses Ninja and exports `compile_commands.json`. Its Qt version is `6.9.*`.
- Local screen-test builds use Qt 6.9.3 and Qt 6.7.3. They do not contain the full application build.

## C++ checks

`clang-tidy` reads compile commands to obtain include paths, compiler definitions, and language options. It can select individual checks and make selected warnings fail the command. A narrow initial check set is preferable to all style and modernization checks. [Clang-Tidy documentation](https://clang.llvm.org/extra/clang-tidy/index.html)

CMake exports the command database with `CMAKE_EXPORT_COMPILE_COMMANDS=ON`. This works with Makefile and Ninja generators. Visual Studio generators ignore the option. [CMake command database documentation](https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html)

CMake can also run the tool during compilation through the `CXX_CLANG_TIDY` target property. A separate lint command is a better first step here because it can restrict checks to application sources and keep lint cost separate from normal builds. The latter is a project recommendation. [CMake target property](https://cmake.org/cmake/help/latest/prop_tgt/LANG_CLANG_TIDY.html)

Clazy adds Qt-specific checks. Examples include a connection that uses a non-signal and a connection lambda that captures a local variable by reference. It can use the same command database. Treat it as an optional later addition because it adds another tool installation. Its optional clang-tidy plugin also requires a compatible build. [Clazy documentation](https://github.com/KDE/clazy), [connection check](https://raw.githubusercontent.com/KDE/clazy/master/docs/checks/README-connect-non-signal.md), [lambda check](https://raw.githubusercontent.com/KDE/clazy/master/docs/checks/README-lambda-in-connect.md)

Cppcheck also accepts a command database. It is an alternative or an additional analyzer, but this research found no need to add it to the initial setup. [Cppcheck manual](https://cppcheck.sourceforge.io/manual.html)

## Local C++ trial

Visual Studio Build Tools contains `clang-tidy` 19.1.5, although the executable is absent from the current `PATH`.

I exported commands from the existing Ninja screen-test build into `cpp-lint-smoke/compile_commands.json`. I checked the real `gui/src/qmlsvgprovider.cpp` without automatic fixes.

```text
clang-tidy -p .scratch/research/cpp-lint-smoke -checks=-*,clang-analyzer-*,bugprone-* gui/src/qmlsvgprovider.cpp
```

Result: five application warnings, comprising three narrowing conversions and two adjacent-parameter warnings. Exit code: 0. The large suppressed warning count comes from non-user code, not thousands of application findings. See `cpp-lint-smoke/output.txt`.

A second run selected only analyzer and narrowing checks and added `-warnings-as-errors=*`. Result: three application diagnostics and exit code 1. See `cpp-lint-smoke/strict-output.txt`. This verifies that the tool can reject a check on this Windows setup. These warnings need review before they are classified as bugs.

## QML checks and implementation order

See [QML research and local trials](qml-lint.md) for the type metadata options, warning thresholds, and Qt version differences.

A tooling-only `qmldir` and `.qmltypes` description can preserve the existing runtime setup, but requires maintenance when the C++ API changes. A full module migration can generate metadata. It also needs a singleton registration change: Qt restricts `qmlRegisterSingletonInstance` to modules whose types are all registered procedurally. Changing CMake alone is insufficient. [Qt singleton registration documentation](https://doc.qt.io/qt-6/qqml-h.html#qmlRegisterSingletonInstance)

Recommended order:

1. Pin the lint tool versions and use the matching Qt import directory.
2. Add a separate C++ lint command for project sources. Exclude third-party sources and generated sources from its input list.
3. Run QML lint in report mode to measure existing warnings. Require valid imports before interpreting type diagnostics.
4. Add accurate metadata for `org.streetpea.chiaking`. Prefer generated metadata when maintaining a full API description by hand becomes too costly.
5. Make a small agreed set of checks block CI. Verify the native exit code reaches the CI result.

Formatting can be a separate decision. `qmlformat` formats QML according to Qt conventions. It does not replace semantic lint checks. [Qt qmlformat documentation](https://doc.qt.io/qt-6/qtqml-tooling-qmlformat.html)

## Limits

No application files, build configuration, or CI workflows were changed. This is a research result, not an installed lint system. C++ checks covered one screen-test source file. No Clazy trial or full application analysis was run. Existing runtime QML tests remain necessary for behavior, dynamic data, and visual output.
