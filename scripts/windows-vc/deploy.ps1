param(
    [Parameter(Mandatory)][string]$Package,
    [Parameter(Mandatory)][string]$QtDirectory,
    [string]$BuildDirectory,
    [string[]]$DependencyDirectories = @(),
    [string]$NoticeDirectory,
    [switch]$CheckOnly
)
$ErrorActionPreference = 'Stop'
$source = Split-Path (Split-Path $PSScriptRoot)
$qtVersion = (Get-Item "$QtDirectory/bin/Qt6Core.dll").VersionInfo.FileVersion
if ($qtVersion -notlike '6.9.3*') { throw "Use the matching Qt 6.9.3 MSVC x64 SDK, found $qtVersion" }

function Require-File([string]$Relative) {
    $path = Join-Path $Package $Relative
    if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -eq 0) {
        throw "Missing package file: $Relative"
    }
}

function Complete-Dependencies([bool]$Copy) {
    $queue = [System.Collections.Generic.Queue[string]]::new()
    Get-ChildItem -LiteralPath $Package -Recurse -File | Where-Object Extension -in '.dll', '.exe' |
        ForEach-Object { $queue.Enqueue($_.FullName) }
    $seen = @{}
    while ($queue.Count) {
        $binary = $queue.Dequeue()
        if ($seen.ContainsKey($binary)) { continue }
        $seen[$binary] = $true
        $output = & dumpbin.exe /nologo /dependents $binary
        if ($LASTEXITCODE) { throw "Cannot inspect dependencies: $binary" }
        foreach ($line in $output) {
            if ($line -notmatch '^\s+([\w.+-]+\.dll)\s*$') { continue }
            $name = $Matches[1]
            if ($name -match '^(api-ms-|ext-ms-)') { continue }
            $target = Join-Path $Package $name
            if (Test-Path -LiteralPath $target) { continue }
            $candidate = $null
            if ($Copy) {
                foreach ($directory in $DependencyDirectories + @("$QtDirectory/bin")) {
                    $path = Join-Path $directory $name
                    if (Test-Path -LiteralPath $path) { $candidate = $path; break }
                }
            }
            if ($candidate) {
                Copy-Item -LiteralPath $candidate -Destination $target
                $queue.Enqueue($target)
            } elseif ($name -match '^(vcruntime|msvcp|concrt)\d' -or !(Test-Path -LiteralPath "$env:SystemRoot/System32/$name")) {
                throw "Missing dependency $name required by $binary"
            }
        }
    }
}

if (!$CheckOnly) {
    if (!$BuildDirectory -or !$NoticeDirectory) { throw 'BuildDirectory and NoticeDirectory are required for deployment.' }
    if (Test-Path -LiteralPath $Package) { throw 'Use a new, empty package directory to exclude stale runtime files.' }
    New-Item -ItemType Directory -Path $Package | Out-Null
    Copy-Item -LiteralPath "$BuildDirectory/gui/chiaki.exe" -Destination $Package
    Copy-Item -LiteralPath "$BuildDirectory/third-party/steam-tools/cpp-steam-tools.dll" -Destination $Package
    # WebEngine locales are separate from Qt message catalogs. Some SDK archives omit catalogs.json.
    & "$QtDirectory/bin/windeployqt.exe" --release --no-compiler-runtime --no-translations "--qmldir=$source/gui/src/qml" "$Package/chiaki.exe"
    if ($LASTEXITCODE) { throw 'windeployqt failed.' }
    $crt = Get-ChildItem "$env:VCToolsRedistDir/x64/Microsoft.VC*.CRT" -Directory
    if (!$crt) { throw 'The selected MSVC x64 compiler runtime is required. Run in its developer shell.' }
    foreach ($directory in $crt) { Copy-Item "$($directory.FullName)/*.dll" -Destination $Package }
    New-Item -ItemType Directory -Path "$Package/translations" -Force | Out-Null
    Copy-Item -LiteralPath "$QtDirectory/translations/qtwebengine_locales" -Destination "$Package/translations" -Recurse -Force
    Get-ChildItem "$QtDirectory/translations" -Filter '*.qm' -File | Copy-Item -Destination "$Package/translations"
    Copy-Item -LiteralPath "$source/gui/qt.conf" -Destination $Package
    Copy-Item -LiteralPath $NoticeDirectory -Destination "$Package/notices" -Recurse
    Complete-Dependencies $true
}

foreach ($relative in @(
    'chiaki.exe', 'Qt6Core.dll', 'Qt6Gui.dll', 'Qt6Widgets.dll', 'Qt6Quick.dll',
    'Qt6WebEngineWidgets.dll', 'Qt6WebEngineCore.dll', 'Qt6WebChannel.dll',
    'QtWebEngineProcess.exe', 'platforms/qwindows.dll', 'qml/QtQuick/qmldir',
    'qml/QtQuick/Controls/qmldir', 'qt.conf', 'vcruntime140.dll', 'msvcp140.dll',
    'resources/qtwebengine_resources.pak', 'resources/qtwebengine_resources_100p.pak',
    'resources/qtwebengine_resources_200p.pak', 'resources/qtwebengine_devtools_resources.pak',
    'resources/icudtl.dat', 'resources/v8_context_snapshot.bin',
    'translations/qtwebengine_locales/en-US.pak', 'notices/COPYING',
    'notices/Qt/qtwebengine-chromium-webengine-6.9.3.spdx.json', 'notices/frontend/svelte-LICENSE.md',
    'notices/Qt/LGPL-3.0-only.txt', 'notices/Qt/GPL-3.0-only.txt', 'notices/Qt/GPL-2.0-only.txt',
    'notices/native/FFmpeg-LICENSE.txt', 'notices/native/libplacebo-LICENSE',
    'notices/native/shaderc-LICENSE.txt', 'notices/native/SPIRV-Cross-LICENSE.txt',
    'notices/native/SPIRV-Tools-LICENSE.txt', 'notices/native/SPIRV-Headers-LICENSE.txt',
    'notices/native/glslang-LICENSE.txt', 'notices/native/vcpkg-openssl.txt',
    'notices/native/vcpkg-sdl2.txt', 'notices/native/vcpkg-opus.txt', 'notices/native/vcpkg-speexdsp.txt',
    'notices/native/vcpkg-lcms.txt', 'notices/native/vcpkg-json-c.txt', 'notices/native/vcpkg-miniupnpc.txt',
    'notices/native/vcpkg-libevent.txt', 'notices/native/cpp-steam-tools-LICENSE'
)) { Require-File $relative }
foreach ($locale in Get-ChildItem "$QtDirectory/translations/qtwebengine_locales/*.pak") {
    $relative = "translations/qtwebengine_locales/$($locale.Name)"
    Require-File $relative
    if ((Get-FileHash "$Package/$relative").Hash -ne (Get-FileHash $locale.FullName).Hash) {
        throw "Wrong WebEngine locale: $relative"
    }
}
foreach ($relative in @('QtWebEngineProcess.exe', 'Qt6WebEngineCore.dll', 'Qt6WebEngineWidgets.dll', 'Qt6WebChannel.dll')) {
    if ((Get-FileHash "$Package/$relative").Hash -ne (Get-FileHash "$QtDirectory/bin/$relative").Hash) {
        throw "WebEngine files must come from the selected SDK: $relative"
    }
}
if ((Get-Item "$Package/Qt6Core.dll").VersionInfo.FileVersion -ne $qtVersion) { throw 'QtCore version does not match the selected SDK.' }
if ((Get-Content "$Package/qt.conf" -Raw) -ne (Get-Content "$source/gui/qt.conf" -Raw)) { throw 'The package must use the relative Qt paths in gui/qt.conf.' }
Complete-Dependencies $false
if (!$CheckOnly) {
    Get-ChildItem -LiteralPath $Package -File -Recurse | ForEach-Object {
        [ordered]@{ path = $_.FullName.Substring((Get-Item $Package).FullName.Length + 1).Replace('\', '/');
            sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant(); size = $_.Length }
    } | ConvertTo-Json -Depth 3 | Set-Content "$Package/package-manifest.json" -Encoding utf8
    $names = @('PATH', 'QT_PLUGIN_PATH', 'QML_IMPORT_PATH', 'QML2_IMPORT_PATH', 'QTWEBENGINEPROCESS_PATH',
        'QTWEBENGINE_RESOURCES_PATH', 'QTWEBENGINE_LOCALES_PATH', 'QTWEBENGINE_CHROMIUM_FLAGS',
        'QTWEBENGINE_DISABLE_SANDBOX', 'QTWEBENGINE_REMOTE_DEBUGGING')
    $saved = @{}
    try {
        foreach ($name in $names) { $saved[$name] = [Environment]::GetEnvironmentVariable($name); [Environment]::SetEnvironmentVariable($name, $null) }
        $env:PATH = "$Package;$env:SystemRoot/System32;$env:SystemRoot"
        $process = Start-Process -FilePath "$Package/chiaki.exe" -ArgumentList '--check-package' -Wait -PassThru -WindowStyle Hidden
        if ($process.ExitCode -ne 0) { throw "Embedded setup assets or WebChannel are missing. Exit $($process.ExitCode)." }
    } finally {
        foreach ($name in $names) { [Environment]::SetEnvironmentVariable($name, $saved[$name]) }
    }
}
Write-Output 'PASS: Windows package dependencies, matching WebEngine files, locales, and notices.'
