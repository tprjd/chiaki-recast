param([string]$Output='screen-statistics.txt', [string]$Suite='StreamStatistics', [string]$Platform='offscreen', [string[]]$Cases=@())
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path "$PSScriptRoot/../../../..").Path
$native = 'C:/Users/david/.cache/chiaki-ui-native'
$env:PATH = "$native/msys64/ucrt64/bin;" + $env:PATH
$env:QT_PLUGIN_PATH = "$native/msys64/ucrt64/share/qt6/plugins"
$env:QT_QPA_PLATFORM = $Platform
$env:QT_QUICK_BACKEND = 'software'
$env:QT_QUICK_CONTROLS_CONF = "$repo/gui/src/qml/qtquickcontrols2.conf"
$env:QT_QPA_FONTDIR = "$env:WINDIR/Fonts"
& "$repo/build/gui-screen/chiaki-console-screen-tests.exe" -input "$repo/gui/tests/qml/tst_$Suite.qml" -o "$PSScriptRoot/$Output,txt" @Cases
$result = $LASTEXITCODE
Get-Content "$PSScriptRoot/$Output"
exit $result
