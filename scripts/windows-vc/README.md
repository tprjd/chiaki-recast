# Build the Windows package

The supported Windows package uses MSVC x64, LLVM 19.1.7, MSVC toolset 14.44,
Qt 6.9.3, and Vulkan SDK 1.4.357.0. MinGW and Windows ARM64 do not have a
compatible selected WebEngine SDK. Their old workflows report that limit.
There is no QML setup fallback.

Release, weekly, pull request, and manual x64 builds call
`build-windows-x86.yml`. The GUI CMake target builds the locked frontend once
with Node 24.16.0 and npm 11.13.0. Qt resources contain the generated page,
script, and style. The runtime package does not need Node or source assets.

After the application build, run the scripts in the selected MSVC developer
shell. Use a new package directory. Supply the native DLL search directories
as a PowerShell array.

```powershell
	./scripts/windows-vc/notices.ps1 -OutputDirectory package-notices -BuildDirectory build `
	    -QtDirectory $env:QT_ROOT_DIR -VcpkgDirectory "$env:VCPKG_INSTALLED_DIR/x64-windows" `
	    -FFmpegDirectory deps -LibplaceboDirectory libplacebo
	./scripts/windows-vc/deploy.ps1 -Package "$PWD/chiaki-ng-Win" `
	    -QtDirectory $env:QT_ROOT_DIR -BuildDirectory build -NoticeDirectory package-notices `
	    -DependencyDirectories @('deps/bin', "$env:VCPKG_INSTALLED_DIR/x64-windows/bin", "$env:VULKAN_SDK/Bin")
	./scripts/windows-vc/test-package.ps1 -Package "$PWD/chiaki-ng-Win" -QtDirectory $env:QT_ROOT_DIR
```

Deployment scans PE imports, runs the matching `windeployqt`, copies the selected
MSVC runtime DLLs, and checks the result. It copies every WebEngine locale PAK
and every available Qt message catalog. Some Qt SDK archives omit
`translations/catalogs.json` and the Qt message catalogs. `windeployqt` then
prints a catalog warning even with `--no-translations`. The script copies the
WebEngine locale PAKs directly and verifies their hashes. The current setup
text is English. This warning does not establish translated Qt dialogs.

`gui/qt.conf` keeps plugin, QML, helper, resource, and locale paths inside the
package. `--check-package` reads the actual embedded page references, script,
style, and WebChannel resource with the development PATH removed. This command
exits before audio, settings, discovery, and Chromium start. Missing required
runtime files fail deployment. The failure checks remove and restore one helper
and one browser resource file in the owned package directory.

The package includes dependency notices and a SHA256 file manifest. Qt notices
come from the matching SDK SPDX files, including Chromium license texts.
Native notices come from the selected dependencies. Vite generates the frontend
notice from dependency code present in the browser bundle. CMake also installs
it at `share/licenses/chiaki-ng/frontend/THIRD-PARTY-NOTICES.txt`. The notice collection step downloads pinned license
texts during the build. Runtime operation is offline.

The existing Inno Setup script includes the complete staged directory. It
accepts native x64 Windows only. Package file checks do not prove normal setup
startup, Chromium sandbox operation, GPU acceleration, native playback, physical
input, microphone operation, or window handoff. Run those checks against the
final package and record the actual loaded module paths. Keep release remote
debugging disabled and the Chromium sandbox enabled.
