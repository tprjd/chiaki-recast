param(
    [Parameter(Mandatory)][string]$Package,
    [Parameter(Mandatory)][string]$QtDirectory
)
$ErrorActionPreference = 'Stop'
$check = Join-Path $PSScriptRoot 'deploy.ps1'
& $check -Package $Package -QtDirectory $QtDirectory -CheckOnly
foreach ($relative in @('QtWebEngineProcess.exe', 'resources/qtwebengine_resources.pak')) {
    $file = Join-Path $Package $relative
    $saved = "$file.package-test"
    Move-Item -LiteralPath $file -Destination $saved
    try {
        $rejected = $false
        try { & $check -Package $Package -QtDirectory $QtDirectory -CheckOnly }
        catch {
            if ($_.Exception.Message -notlike "*Missing package file: *$relative*") { throw }
            $rejected = $true
        }
        if (!$rejected) { throw "The package check accepted missing $relative" }
    } finally { Move-Item -LiteralPath $saved -Destination $file }
}
Write-Output 'PASS: complete package accepted; missing helper and browser assets rejected.'
