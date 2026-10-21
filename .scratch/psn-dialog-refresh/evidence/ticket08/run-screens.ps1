$ErrorActionPreference = 'Stop'
$nativeRoot = 'C:/Users/david/.cache/chiaki-ui-native'
$repo = (Resolve-Path "$PSScriptRoot/../../../..").Path
$env:PATH = "$nativeRoot/msys64/ucrt64/bin;" + $env:PATH
$env:QT_QPA_PLATFORM = 'offscreen'
$env:QT_QUICK_BACKEND = 'software'
$env:QT_QUICK_CONTROLS_CONF = "$repo/gui/src/qml/qtquickcontrols2.conf"
$env:QT_QPA_FONTDIR = "$env:WINDIR/Fonts"
$results = @()
foreach ($test in Get-ChildItem "$repo/gui/tests/qml/tst_*.qml" | Sort-Object Name) {
    $output = "$PSScriptRoot/screen-$($test.BaseName).txt"
    $timer = [Diagnostics.Stopwatch]::StartNew()
    & "$nativeRoot/psn-dialog-final-bin/chiaki-console-screen-tests.exe" -input $test.FullName -o "$output,txt"
    $code = $LASTEXITCODE
    $results += [pscustomobject]@{ Suite=$test.BaseName; ExitCode=$code; Seconds=[Math]::Round($timer.Elapsed.TotalSeconds, 2) }
    $results | ConvertTo-Json | Set-Content "$PSScriptRoot/screen-runs.json"
    Write-Output "$($test.BaseName): exit $code after $([Math]::Round($timer.Elapsed.TotalSeconds)) seconds"
}
if ($results.ExitCode | Where-Object { $_ -ne 0 }) { throw 'Screen suite failed' }
