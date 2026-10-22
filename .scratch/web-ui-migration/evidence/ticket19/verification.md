# Windows package verification

The Windows package milestone passes. Source `584d6471d81092a4a7c63eb78ab220539f198702`
includes audited tickets 18 and 22 and integration `0c982b32`. `final-source.json`
records the clean source, executable hashes, and excluded existing gf-complete
Windows patch. Production defaults are unchanged.

## Final artifact

- Stage: `C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket19-package-final`.
- ZIP: `C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/chiaki-recast-windows-x64-web-native-584d6471.zip`.
- ZIP SHA256: `94ff670d03f1a9eea6cb4227336e3f0a18d170cb7fdcc98e44b4cdd25077f62a`.
- Application SHA256: `6e268c734f0c3d9b5b48a84740323af35106042b4a238dafd6e9530384f0bc0a`.
- Harness SHA256: `15b62ffe223a29313ded0bc0310755643b363b346097e07cd571112948824744`.
- Production manifest SHA256: `33a727071260a778f31a2b349be50f0f882076976171671cd6d379968bc6cecc`.

The ZIP has 1,541 files and is 190,837,567 bytes. Extracted contents total
520,315,789 bytes. Every ZIP member SHA256 equals the prelaunch production
manifest, including the manifest file. ZIP integrity passes. QA executables
and QtTest were removed. No Node runtime, server, or external frontend source
is needed. See `final-artifact.json`, `final-package-manifest.json.gz`, and
`zip-test.txt`.

## Deployment and build

The final native build, deployment, and clean-PATH RCC preflight pass. Release,
weekly, PR, and manual x64 routes share LLVM 19.1.7, MSVC 14.44, Qt 6.9.3,
Node 24.16.0, and Vulkan SDK 1.4.357.0. The actual generated SDK metadata passes.
All nine changed workflows pass actionlint, with the existing `macos-15-intel`
runner label supplied to its older inventory. Non-Windows job objects are
unchanged. Hosted CI execution is unverified.

The stage includes matching widget, Quick, WebChannel, WebEngine, helper,
native, and app-local CRT files. Actual PE imports are checked. Resources and
all 53 locale PAKs match the SDK. Relative `qt.conf` paths keep Qt inside the
stage. QtCore's patched hash is recorded separately. The SDK lacks Qt message
catalogs and `catalogs.json`; the log retains that warning. Chromium locales
are complete; translated Qt dialogs are unverified.

The initial helper and browser-PAK removal cases fail the release check.
Their required-file checks are unchanged. Initial records remain available.
The common Vite runtime notice builds and installs through CMake under
`share/licenses/chiaki-ng/frontend`. It contains Svelte and Vite's browser
preload helper licenses. The existing common notice install
pass is reused. Windows packages it with matching Qt/Chromium SDK SPDX and
native notices.

## Actual packaged runtime

Normal production `RunMain` displayed the actual Consoles page at 1280 pixels
and exited 0. A fresh profile had no consoles or PSN credentials. The full native
settings tree was privately saved and restored. Ordinary LAN discovery was
allowed. No real registration, authentication, console connection, or Steam
write was requested. Both observations restored the original full state hash.
An independent read-only check confirmed the current state still matches.
Private settings backups remain outside Git.

The focused group passed **5/0 in 7,789 ms**: `startupNativeConnection:local-ready`,
`setupReload:reload`, and `graphicsCoexistence`, plus QtTest setup/cleanup.
External boundaries are controlled before fixture services start. Native
rendering remains real. Deeper source-equivalent ticket 22 lifetime results
are reused. No GPU group was repeated.

Chromium reports hardware compositing, WebGL, and rasterization on RTX 5090,
ANGLE D3D11, and Skia 130. Fast native OpenGL, Vulkan, recreation, and fallback
rendered real frame/overlay pixels and restored setup. Saved GL/Vulkan images
were inspected. Desktop names were both `Default`; no protected dialog was
present. Win32 foreground remained Codex during the fixture, so physical focus
and input are unverified. Actual app, harness, and renderer observations found
all relevant Qt/native/CRT modules inside the stage, with no observation errors.
Commands have a stage-and-Windows-only PATH and empty Qt overrides.
See `module-validation.json`, compressed module records, `package-qt.txt.gz`,
`chromium-gpu.txt.gz`, command records, and saved images.

## Renderer sandbox

A bounded read-only observation found the exact staged renderer at integrity
RID 0, restricted SID `S-1-0-0`, and `IsTokenRestricted=true`. Its parent had
medium integrity RID 8192 and an unrestricted token. `TokenIsAppContainer=false`
for both. No elevation, debugging, or sandbox-disable flag was used.
`renderer-token.json` records the actual result.

Qt 6.9.3 [adds the in-process GPU switch](https://github.com/qt/qtwebengine/blob/v6.9.3/src/core/web_engine_context.cpp#L961).
The GPU report's `Sandboxed=false` field describes that application-process GPU,
not the restricted renderer. Versioned [platform notes](https://github.com/qt/qtwebengine/blob/v6.9.3/src/core/doc/src/qtwebengine-platform-notes.qdoc.in)
describe default renderer sandbox support. The Windows helper
[initializes sandbox information](https://github.com/qt/qtwebengine/blob/v6.9.3/src/core/process_main.cpp#L24).
Source hashes are in `qt-sandbox-source-refs.json`.
Computer Use stopped after this observation. The owned helper reached its
existing timeout, closed the app, and restored native settings. Actual process
completion and restoration were checked. The first startup had closed normally.

## Remaining limits

Physical controller, keyboard/mouse output, microphone, live-console/direct CLI
streaming, full scaling/accessibility, matched performance, and default
HighQuality acceptance remain in ticket 23. The independent native HighQuality
failure is unchanged. Fast results do not certify other presets.

MinGW and Windows ARM64 workflows explicitly report the selected SDK support
limit, with no QML setup fallback. Only this Windows PC was available. The x64
installer includes the complete stage, but local compilation and installation
remain unverified. Automatic approval review rejected the Inno Setup tool
installation with reason "blocked by policy". It was not retried. The verified
tool download does not replace an installer result. The portable ZIP is complete;
the existing CI installer path remains available.
User priority update: remaining build, test, runtime, package, and performance checks were skipped at the user's request. The completed source, ZIP, and evidence are preserved. No further verification ran after that instruction.
