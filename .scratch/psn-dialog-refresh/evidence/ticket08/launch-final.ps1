$ErrorActionPreference = 'Stop'
$nativeRoot = 'C:/Users/david/.cache/chiaki-ui-native'
$bin = "$nativeRoot/psn-dialog-final-bin"
$env:PATH = "$bin;$nativeRoot/msys64/ucrt64/bin;" + $env:PATH
$env:QT_PLUGIN_PATH = "$nativeRoot/msys64/ucrt64/share/qt6/plugins"
& "$bin/chiaki.exe" @args
if ($LASTEXITCODE) { throw "chiaki-ng exited with $LASTEXITCODE" }
