param([string]$Suffix='final01')
$ErrorActionPreference='Stop'
$here=$PSScriptRoot
$matrix=@(
    @{Renderer='vulkan';Scale=0.8;Label='vulkan100'},
    @{Renderer='opengl';Scale=1.2;Label='opengl150'},
    @{Renderer='vulkan';Scale=1.2;Label='vulkan150'},
    @{Renderer='opengl';Scale=1.6;Label='opengl200'},
    @{Renderer='vulkan';Scale=1.6;Label='vulkan200'}
)
foreach($case in $matrix) {
    & "$here/run-native.ps1" -Run "stream-acceptance-$($case.Label)-$Suffix" -Renderer $case.Renderer -Scale $case.Scale *> "$here/native-$($case.Label)-final.txt"
    python "$here/record-native.py" "stream-acceptance-$($case.Label)-$Suffix" $case.Label
    if ($LASTEXITCODE -ne 0) { throw 'Native evidence or settings check failed' }
}
foreach($renderer in @('opengl','vulkan')) {
    & "$here/run-native.ps1" -Run "stream-acceptance-$renderer-baseline-$Suffix" -Renderer $renderer -Scale 0.8 -Baseline *> "$here/native-$renderer-baseline.txt"
    python "$here/record-native.py" "stream-acceptance-$renderer-baseline-$Suffix" "$renderer-baseline"
    if ($LASTEXITCODE -ne 0) { throw 'Native baseline evidence or settings check failed' }
}
