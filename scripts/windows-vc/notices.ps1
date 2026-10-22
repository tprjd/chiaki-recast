param(
    [Parameter(Mandatory)][string]$OutputDirectory,
    [Parameter(Mandatory)][string]$QtDirectory,
    [Parameter(Mandatory)][string]$VcpkgDirectory,
    [Parameter(Mandatory)][string]$FFmpegDirectory,
    [Parameter(Mandatory)][string]$LibplaceboDirectory
)
$ErrorActionPreference = 'Stop'
$source = Split-Path (Split-Path $PSScriptRoot)
New-Item -ItemType Directory -Path "$OutputDirectory/Qt", "$OutputDirectory/native", "$OutputDirectory/frontend" -Force | Out-Null
Copy-Item -LiteralPath "$source/COPYING", "$source/LICENSES/AGPL-3.0-only-OpenSSL.txt" -Destination $OutputDirectory
# The SDK SBOM includes the license texts and copyright records for its actual Chromium build.
Copy-Item "$QtDirectory/sbom/*" -Destination "$OutputDirectory/Qt"
foreach ($license in @('LGPL-3.0-only', 'GPL-3.0-only', 'GPL-2.0-only', 'GFDL-1.3-no-invariants-only')) {
    Invoke-WebRequest "https://raw.githubusercontent.com/qt/qtbase/v6.9.3/LICENSES/$license.txt" -OutFile "$OutputDirectory/Qt/$license.txt"
}
foreach ($port in Get-ChildItem "$VcpkgDirectory/share" -Directory) {
    if (Test-Path -LiteralPath "$($port.FullName)/copyright") {
        Copy-Item -LiteralPath "$($port.FullName)/copyright" -Destination "$OutputDirectory/native/vcpkg-$($port.Name).txt"
    }
}
Copy-Item -LiteralPath "$FFmpegDirectory/LICENSE.txt" -Destination "$OutputDirectory/native/FFmpeg-LICENSE.txt"
Copy-Item -LiteralPath "$LibplaceboDirectory/LICENSE" -Destination "$OutputDirectory/native/libplacebo-LICENSE"
foreach ($dependency in @('cpp-steam-tools', 'curl', 'gf-complete', 'jerasure', 'nanopb')) {
    Get-ChildItem "$source/third-party/$dependency" -File | Where-Object Name -match '^(LICENSE|COPYING)' |
        ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination "$OutputDirectory/native/$dependency-$($_.Name)" }
}
$licenses = @{
    'shaderc' = 'google/shaderc/v2026.3/LICENSE'
    'SPIRV-Cross' = 'KhronosGroup/SPIRV-Cross/vulkan-sdk-1.4.357.0/LICENSE'
    'SPIRV-Tools' = 'KhronosGroup/SPIRV-Tools/vulkan-sdk-1.4.357.0/LICENSE'
    'SPIRV-Headers' = 'KhronosGroup/SPIRV-Headers/vulkan-sdk-1.4.357.0/LICENSE'
    'glslang' = 'KhronosGroup/glslang/vulkan-sdk-1.4.357.0/LICENSE.txt'
    'sdl2-compat' = 'libsdl-org/sdl2-compat/release-2.32.64/LICENSE.txt'
    'SDL3' = 'libsdl-org/SDL/release-3.2.0/LICENSE.txt'
}
foreach ($name in $licenses.Keys) {
    Invoke-WebRequest "https://raw.githubusercontent.com/$($licenses[$name])" -OutFile "$OutputDirectory/native/$name-LICENSE.txt"
}
Get-ChildItem "$source/gui/web/node_modules" -Recurse -File | Where-Object Name -match '^(LICENSE|COPYING)(\..*)?$' |
    ForEach-Object {
        $relative = $_.FullName.Substring((Get-Item "$source/gui/web/node_modules").FullName.Length + 1).Replace('\', '-')
        Copy-Item -LiteralPath $_.FullName -Destination "$OutputDirectory/frontend/$relative"
    }
@'
Qt 6.9.3 and Chromium notices are in Qt/, including the SDK's SPDX records and extracted third-party license texts.
Qt source: https://download.qt.io/archive/qt/6.9/6.9.3/submodules/
Native dependency notices are in native/. Vulkan SDK shaderc 2026.3 and SPIRV components use SDK 1.4.357.0.
SDL3 and sdl2-compat use the zlib license. The SDL2 vcpkg notice also applies when that build uses native SDL2.
Frontend license files come from the locked npm install. Node and npm are build tools and are not required to run this package.
Application and bundled dependency source: https://github.com/streetpea/chiaki-ng
FFmpeg source and build configuration: https://github.com/streetpea/FFmpeg-Builds
libplacebo source: https://github.com/haasn/libplacebo/tree/v7.360.1
'@ | Set-Content "$OutputDirectory/README.txt" -Encoding utf8
