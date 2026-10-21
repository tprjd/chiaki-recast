param([Parameter(Mandatory=$true)][string]$Run, [ValidateSet('opengl','vulkan')][string]$Renderer='vulkan', [double]$Scale=1, [switch]$Baseline)
$ErrorActionPreference='Stop'
$nativeRoot='C:/Users/david/.cache/chiaki-ui-native'
$runRoot="$nativeRoot/$Run"
if (Test-Path -LiteralPath $runRoot) { throw 'Use a new isolated run directory' }
$env:PATH="$nativeRoot/msys64/ucrt64/bin;$nativeRoot/build/third-party/cpp-steam-tools;"+$env:PATH
$env:QT_PLUGIN_PATH="$nativeRoot/msys64/ucrt64/share/qt6/plugins"
$env:QT_SCALE_FACTOR=$Scale.ToString([Globalization.CultureInfo]::InvariantCulture)
Remove-Item Env:QT_SCREEN_SCALE_FACTORS -ErrorAction SilentlyContinue
$env:QT_FORCE_STDERR_LOGGING='1'
$env:CHIAKI_QA_RENDERER=$Renderer
$env:CHIAKI_DISCONNECT_QA_ROOT=$runRoot
Remove-Item Env:CHIAKI_DISCONNECT_QA_HOLD -ErrorAction SilentlyContinue
if ($Baseline) { $env:CHIAKI_QA_BASELINE='1' } else { Remove-Item Env:CHIAKI_QA_BASELINE -ErrorAction SilentlyContinue }
$process=Start-Process -FilePath "$nativeRoot/stream-acceptance-qa-bin/chiaki.exe" -WindowStyle Hidden -PassThru -RedirectStandardOutput "$runRoot.stdout.txt" -RedirectStandardError "$runRoot.stderr.txt"
Write-Output "Native PID $($process.Id); evidence $runRoot"
if (!$process.WaitForExit(180000)) { throw "Native acceptance timeout, PID $($process.Id)" }
$process.Refresh()
if ($process.ExitCode -ne 0) { Get-Content "$runRoot.stderr.txt" -Tail 16; throw "Native acceptance failed, exit $($process.ExitCode)" }
Get-Content "$runRoot/contracts.json"
