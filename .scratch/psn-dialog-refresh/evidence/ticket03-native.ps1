$ErrorActionPreference = 'Stop'
$qtPrefix = 'C:\Users\david\.cache\chiaki-native-qt\6.9.3\msvc2022_64'
$vsRoot = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools'
Import-Module "$vsRoot\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath $vsRoot -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64'
$cmake = "$vsRoot\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$env:PATH = "$qtPrefix\bin;" + $env:PATH
& $cmake -S gui/tests/steam -B build/steam-native -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$qtPrefix"
if ($LASTEXITCODE) { throw 'Native configure failed' }
& $cmake --build build/steam-native
if ($LASTEXITCODE) { throw 'Native build failed' }
$env:PATH = (Resolve-Path build/steam-native/steam-tools).Path + ';' + $env:PATH
$env:QT_QPA_PLATFORM = 'offscreen'
& .\build\steam-native\chiaki-steam-shortcut-tests.exe
if ($LASTEXITCODE) { throw 'Native check failed' }
