$ErrorActionPreference = 'Stop'
$native = 'C:/Users/david/.cache/chiaki-ui-native'
$repo = (Resolve-Path "$PSScriptRoot/../../../..").Path
$env:PATH = "$native/msys64/ucrt64/bin;" + $env:PATH
$env:QT_PLUGIN_PATH = "$native/msys64/ucrt64/share/qt6/plugins"
$env:QT_QPA_PLATFORM = 'offscreen'
$env:QT_QUICK_BACKEND = 'software'
$env:QT_QUICK_CONTROLS_CONF = "$repo/gui/src/qml/qtquickcontrols2.conf"
$env:QT_QPA_FONTDIR = "$env:WINDIR/Fonts"
$results = @()
foreach ($suite in @('StreamInterface', 'Disconnect', 'LiveConsolePin', 'ConnectionLocal', 'ConnectionPsn', 'ConnectionWake', 'SettingsDisplay', 'SettingsRenderer')) {
    $output = "$PSScriptRoot/screen-$suite.txt"
    & "$repo/build/gui-screen/chiaki-console-screen-tests.exe" -input "$repo/gui/tests/qml/tst_$suite.qml" -o "$output,txt"
    $results += [pscustomobject]@{Suite=$suite; ExitCode=$LASTEXITCODE}
    $results | ConvertTo-Json | Set-Content "$PSScriptRoot/screen-runs.json"
    Write-Output "$suite exit $LASTEXITCODE"
}
if ($results.ExitCode | Where-Object {$_ -ne 0}) {throw 'Related screen suite failed'}
