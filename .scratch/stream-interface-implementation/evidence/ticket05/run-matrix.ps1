param([Parameter(Mandatory=$true)][string]$Preparation, [string]$Suffix='replay01')
$ErrorActionPreference='Stop'
# The user limited remaining acceptance to 900x600 and one measured scale.
$here=$PSScriptRoot
foreach($renderer in @('opengl','vulkan')) {
    $run="stream-acceptance-$renderer-$Suffix"
    & "$here/run-native.ps1" -Run $run -Renderer $renderer -Scale 0.8 *> "$here/native-$renderer-$Suffix.txt"
    python "$here/record-native.py" $run "$renderer-$Suffix" $Preparation
    if ($LASTEXITCODE -ne 0) { throw 'Native evidence or settings check failed' }
}
