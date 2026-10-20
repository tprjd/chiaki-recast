# Console screen test commands

The test target uses the production `ConsoleLibrary.qml`, controls, console
artwork, and SVG image provider. Sample hosts exist only in `qml/`. Tests observe
action signals without opening a network session.

The normal application build includes this target when `CHIAKI_ENABLE_TESTS`
is enabled. A separate build needs only Qt Quick, Qt Quick Test, and Qt SVG.
No test target or sample host is part of application packaging.

## Windows Qt 6.9.3 commands

These PowerShell commands use the MSVC compiler and the Qt version from the
Windows build workflow. The Qt installation contains `qtbase`, `qtdeclarative`,
`qtsvg`, and `qttools`. The commands start at the repository root.

```powershell
$qtPrefix = 'C:\Users\david\.cache\chiaki-native-qt\6.9.3\msvc2022_64'
$vsRoot = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools'
Import-Module "$vsRoot\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath $vsRoot -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64'
$cmake = "$vsRoot\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$ctest = "$vsRoot\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe"
$env:PATH = "$qtPrefix\bin;" + $env:PATH
& $cmake -S gui/tests -B build/gui-screen -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$qtPrefix"
& $cmake --build build/gui-screen
& $ctest --test-dir build/gui-screen --output-on-failure
```

The registered test sets `QT_QPA_PLATFORM=offscreen` and
`QT_QUICK_BACKEND=software`. The executable also accepts Qt Test arguments.

The full application needs the additional dependencies and configuration in
`.github/workflows/build-windows-x86.yml`. After that configuration, the build
and launch commands are:

```powershell
& $cmake --build build --config Release --target chiaki
& .\build\gui\chiaki.exe
```

## Initial native behavior

Before this change, the main screen used a full-width `ListView`. A row click
or Return called `Chiaki.connectToHost(index, name)` for a discovered console.
Other entries used `Chiaki.connectToHost(index)`. The backend handled
registration, standby wake-and-connect, local sessions, and PSN sessions.

The baseline replaces the artwork area of a registered, ready row with the
production card. Its Connect action uses that same handler. The existing
toolbar, details, PIN, wake, hide, delete, discovery, and controller commands
remain in the screen until the complete card library replaces the list.

`ConsoleLibrary` accepts `hosts`, `sourceIndices`, and `streamerMode`.
`connectRequested(sourceIndex, nickname)` identifies the backend entry.
When `sourceIndices` is empty, the host's array index is the source index.
This baseline mapping is for immediate actions. Later tickets must resolve
host identity again for live updates and delayed confirmations.

## Baseline results, 2026-10-20

MSVC 19.44 and Qt 6.9.3 built the C++ screen test target. CTest discovered and
passed `chiaki-console-screen`. Three behavior checks passed, plus the Qt Test
setup and cleanup checks:

- A ready console displays its name and Online status. A mouse click emits
  one action for source index 7 with the expected nickname.
- Return emits one action from a Connect button with visible keyboard focus.
- Streamer mode masks an address used as a name in visible, accessible, and
  tooltip text.

The first test failed before `ConsoleLibrary.qml` existed, then passed after
the production component was added. Test runs use the real QML screen and the
existing C++ SVG provider.

The full application build and launch are not yet verified at this baseline.
A real registered local-console connection is untested. This test harness has
no hardware or account access and cannot prove discovery or streaming works.
