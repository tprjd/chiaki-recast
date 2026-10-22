$ErrorActionPreference = 'Stop'
$project = Split-Path $PSScriptRoot -Parent
$package = Join-Path $project 'release/Chiaki-Recast-Windows.zip'
$install = [IO.Path]::GetFullPath((Join-Path $project 'build/recast-launch'))

try {
    if (!(Test-Path -LiteralPath $package)) {
        throw "The app package is missing: $package. Keep the ZIP with the launcher."
    }
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $stream = [IO.File]::OpenRead($package)
    $sha256 = [Security.Cryptography.SHA256]::Create()
    try { $hash = [BitConverter]::ToString($sha256.ComputeHash($stream)).Replace('-', '') }
    finally { $stream.Dispose(); $sha256.Dispose() }
    $stamp = Join-Path $install '.package-sha256'
    $update = !(Test-Path -LiteralPath $stamp) -or (Get-Content -LiteralPath $stamp -Raw).Trim() -ne $hash
    $zip = [IO.Compression.ZipFile]::OpenRead($package)
    $preparing = $false
    try {
        foreach ($entry in $zip.Entries) {
            $target = [IO.Path]::GetFullPath((Join-Path $install $entry.FullName))
            if (!$target.StartsWith($install + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
                throw "Invalid app package path: $($entry.FullName)"
            }
            if (!$entry.Name) { continue }
            $file = Get-Item -LiteralPath $target -ErrorAction SilentlyContinue
            if ($update -or !$file -or $file.Length -ne $entry.Length) {
                if (!$preparing) { Write-Host 'Preparing Recast...'; $preparing = $true }
                [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($target)) | Out-Null
                [IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $target, $true)
            }
            if ((Get-Item -LiteralPath $target).Length -ne $entry.Length) {
                throw "App file restoration failed: $target"
            }
        }
    } finally {
        $zip.Dispose()
    }
    Set-Content -LiteralPath $stamp -Value $hash -NoNewline
    $env:QT_QPA_PLATFORM = 'windows'
    $env:QT_PLUGIN_PATH = $install
    $env:QML2_IMPORT_PATH = Join-Path $install 'qml'
    Start-Process -FilePath (Join-Path $install 'chiaki.exe') -WorkingDirectory $install -WindowStyle Normal
} catch {
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}
