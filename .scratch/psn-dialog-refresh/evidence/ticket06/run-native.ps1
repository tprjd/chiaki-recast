param([Parameter(Mandatory=$true)][string]$Run, [ValidateSet('opengl','vulkan')][string]$Renderer='vulkan', [switch]$Hold)
$ErrorActionPreference='Stop'
$nativeRoot='C:\Users\david\.cache\chiaki-ui-native'
$runRoot=(Join-Path $nativeRoot $Run).Replace('\','/')
if(Test-Path -LiteralPath $runRoot) { throw 'Use a new isolated run directory' }
$env:PATH="$nativeRoot\msys64\ucrt64\bin;$nativeRoot\build\third-party\cpp-steam-tools;"+$env:PATH
$env:QT_PLUGIN_PATH="$nativeRoot\msys64\ucrt64\share\qt6\plugins"
$env:QT_SCREEN_SCALE_FACTORS='1'
$env:QT_SCALE_FACTOR='0.8'
$env:CHIAKI_QA_RENDERER=$Renderer
$env:CHIAKI_DISCONNECT_QA_ROOT=$runRoot
if($Hold) { $env:CHIAKI_DISCONNECT_QA_HOLD='1' } else { Remove-Item Env:CHIAKI_DISCONNECT_QA_HOLD -ErrorAction SilentlyContinue }
$process=Start-Process -FilePath "$nativeRoot\disconnect-qa-bin\chiaki.exe" -WindowStyle Hidden -PassThru -RedirectStandardOutput "$runRoot.stdout.txt" -RedirectStandardError "$runRoot.stderr.txt"
if($Hold) { [pscustomobject]@{Pid=$process.Id;Root=$runRoot;ResumeFile="$runRoot/resume.flag"}; return }
if(!$process.WaitForExit(60000)) { throw "Native check timeout, PID $($process.Id)" }
$process.Refresh()
Get-Content "$runRoot/contracts.json"
if($process.ExitCode -ne 0) { throw "Native check failed, exit $($process.ExitCode)" }
