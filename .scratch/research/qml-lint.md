# QML lint options

Research date: 2026-10-21. This note covers the current QML build and the local Windows Qt SDKs.

## Recommendation

Use a fixed Qt version for `qmllint`. Start with a report that exposes missing type information. Do not use the current test imports as the app's lint model.

For useful type checks without an application change, add a separate import directory for lint. It can contain a `qmldir` file and a `.qmltypes` file for `org.streetpea.chiaking`. This is a short-term option that needs manual maintenance. Prefer generated metadata when the project later adopts declarative QML registration.

Do not migrate the application to `qt_add_qml_module` only to add a lint command. That change affects type registration, singleton creation, resources, and component imports.

These recommendations follow from the code and the checks below. They are project decisions, not Qt requirements.

## Current code

- `gui/src/qmlbackend.cpp:150-154` registers `Chiaki` with `qmlRegisterSingletonInstance`. It registers `ChiakiWindow`, `ChiakiSettings`, and `ChiakiSession` as uncreatable types.
- `gui/src/qmlmainwindow.cpp:577` loads `qrc:/Main.qml`. Its initial properties contain the parent item. The app does not expose `Chiaki` as a context property.
- A search for `setContextProperty`, `setContextObject`, and `rootContext(` found no matches in `gui/src` or `gui/tests`.
- `gui/CMakeLists.txt` uses `AUTOMOC`, `AUTORCC`, and `gui/src/qml/qml.qrc`. It has no `qt_add_qml_module` call.
- `gui/tests/imports/org/streetpea/chiaking/qmldir` exposes two mock QML types. `Chiaki.qml` declares only `property var settings: null`. `ChiakiWindow.qml` declares two enums. These types are sufficient for selected screen tests, not the full app API.
- `gui/tests/main.cpp` adds that test import directory to the test engine. The test setup also registers `Chiaki.Test.StreamTestWindow` at run time.

## Type information and context properties

`qmllint` needs type information in its import paths. `-I` adds an import directory. C++ calls made when the app starts do not supply that information to a separate lint process. The failed app import in the local check confirms this gap. [Qt qmllint documentation](https://doc.qt.io/qt-6.10/qtqml-tooling-qmllint.html)

Context properties have a separate limitation. Qt 6.8 states that `qmllint` cannot see them and reports their use as unqualified access. This project currently has runtime type registrations instead. [Qt context property documentation](https://doc.qt.io/qt-6.8/qtqml-cppintegration-contextproperties.html)

Qt 6.11 adds name-based context property settings and a CMake search for `setContextProperty()` calls. This support controls warnings. It does not provide the missing member types for this project's registered singleton. The local 6.7.3 and 6.9.3 tools predate this support. [Qt 6.11 context property support](https://www.qt.io/blog/whats-new-in-qmllint-for-qt-6.11-part-3)

## Metadata without an application migration

A module's `qmldir` file can name a type file with `typeinfo plugins.qmltypes`. Type files describe C++ types for tools and do not affect module behavior. Qt recommends generation with `QML_ELEMENT` and the build system. [Qt module definition files](https://doc.qt.io/qt-6.8/qtqml-modules-qmldir.html)

A separate lint import directory is therefore a possible local solution. The directory structure must match `org/streetpea/chiaking`. Its type file must describe the singleton, uncreatable types, properties, signals, methods, enums, and types returned by properties. This includes `QmlController` objects reached through `Chiaki.controllers`.

The model must preserve property types. Using `var` for all members would hide useful errors. A handwritten type file can also become incorrect after a C++ API change. This option needs an explicit check when those headers change. Keep this lint directory separate from `gui/tests/imports` and from application resources.

## Full module migration

`qt_add_qml_module` generates `qmldir`, type information, type registration code, and `_qmllint` targets. It also manages QML resources and cached compilation. An executable can be the module's target without a separate plugin library. [Qt QML module CMake API](https://doc.qt.io/qt-6.8/qt-add-qml-module.html)

Qt's port guide replaces manual registration with declarative macros such as `QML_ELEMENT`. A migration must move the QML resource list into the module and check resource URLs and directory imports. A CMake call alone cannot infer all current procedural registrations. [Qt QML module port guide](https://doc.qt.io/qt-6.8/qt6-port-to-qt-add-qml-module.html)

The existing `Chiaki` object needs special care. Qt restricts `qmlRegisterSingletonInstance` to modules whose types are all registered procedurally. A declarative migration needs a singleton factory that returns the existing backend object. That object must outlive its engine and have the same thread affinity. Do not let the migration create an independent backend. [Qt singleton registration rules](https://doc.qt.io/qt-6.8/qqml-h.html#qmlRegisterSingletonInstance)

## Local checks and exit codes

Both Windows SDKs contain a working `bin/qmllint.exe`. Their `--version` output confirmed 6.9.3 and 6.7.3. Each run below used `-I <sdk>/qml` from the same SDK. PowerShell captured `$LASTEXITCODE` immediately after each process.

| Qt version | Input and extra options | Exit code | Result |
| --- | --- | --- | --- |
| 6.9.3 | `Main.qml` | `0` | Failed app module import and unqualified access warnings. |
| 6.9.3 | `Main.qml --max-warnings 0` | `-1` | Same import failure causes failure. |
| 6.9.3 | `Main.qml --max-warnings 0 -I gui/tests/imports` | `-1` | Import resolves. Members such as `Chiaki.window` and `Chiaki.session` are missing from the mock type. |
| 6.9.3 | `controls/Button.qml --max-warnings 0` | `0` | No output. |
| 6.7.3 | `Main.qml` | `-1` | Failed app module import and warnings. |
| 6.7.3 | `Main.qml --max-warnings 0` | `1` | Unsupported option. This invocation produced no output. |

The full SDK roots are `C:/Users/david/.cache/chiaki-native-qt/6.9.3/msvc2022_64` and `C:/Users/david/.cache/chiaki-native-qt/6.7.3/msvc2019_64`.

The representative strict command was:

```powershell
$qmlSdk = 'C:/Users/david/.cache/chiaki-native-qt/6.9.3/msvc2022_64'
& "$qmlSdk/bin/qmllint.exe" --max-warnings 0 -I "$qmlSdk/qml" gui/src/qml/Main.qml
$LASTEXITCODE
```

The first warning reports the unresolved `org.streetpea.chiaking` import at `Main.qml:6`. With test imports, the first warning reports the missing `window` member at `Main.qml:13`.

The 6.9.3 `--help` output lists `disable`, `info`, and `warning` levels. Its default warning limit is `-1`, which permits warnings. The 6.7.3 help states that a warning causes a nonzero exit code. Thus, the same default command has different failure behavior across these SDKs.

`.qmllint.ini` can set warning levels. Command-line settings take precedence. `info` messages do not fail the command. JSON output is available with `--json <file>`. A fixed tool version and its own help output are required before an exit code becomes a reliable CI check. [Qt warning configuration](https://doc.qt.io/qt-6.10/qtqml-tooling-qmllint.html)

## Limits

No application code, build configuration, or test imports changed. These checks cover representative real files, not all QML files. No `.qmltypes` implementation or module migration was tested. Static lint cannot validate dynamic `QVariantMap` keys, connection state transitions, console behavior, or rendered layout. Keep the runtime screen tests.
