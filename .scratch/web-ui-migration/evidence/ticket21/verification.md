# Ticket 21 package implementation evidence

Date: 2026-10-22. Base: `41c6134c0c42227b7776f8df95a3fbbc821bd452`.
Host: Windows PC with WSL. No Mac SDK, Apple signing identity, account session,
notarization environment, Mac GPU, or Mac runtime is available.

## Implemented paths

All ten macOS jobs in six maintained workflows call the same composite action.
It selects Node 24.16.0, npm 11.13.0, and official universal Qt 6.9.3 with required
Widgets, WebEngineWidgets, WebChannel, and GuiPrivate components. It builds and
installs the current CMake target. It retains native Qt Quick, SDL, MoltenVK,
helper symlink, and microphone support. It adds automatic graphics switching.
No native renderer, overlay, input, saved setting, or default changes.

Official SDK metadata inspected over HTTPS:

- [Base Qt metadata](https://download.qt.io/online/qtsdkrepository/mac_x64/desktop/qt6_693/qt6_693/Updates.xml).
- [WebEngine extension metadata](https://download.qt.io/online/qtsdkrepository/mac_x64/extensions/qtwebengine/693/clang_64/Updates.xml).
- The extension package is `extensions.qtwebengine.693.clang_64`, version
  `6.9.3-0-202509261208`, described as Universal x64/ARM64. Its archive SHA256
  from the official archive sidecar is
  `95a1fc974e50936dfe15f74007748c11de030397f419ad5b4f09766374d1648d`.
- [Qt deployment requirements](https://doc.qt.io/archives/qt-6.9/qtwebengine-deploying.html)
  require the matching SDK helper entitlements and microphone permission.
- [Qt 6.9.3 resource paths](https://github.com/qt/qtwebengine/blob/v6.9.3/src/core/api/CMakeLists.txt)
  specify framework Resources, qtwebengine_locales, and architecture-specific V8 snapshots.

The implementation checks these package files, all deployed Mach-O architecture
slices and library paths, matching SDK locales, actual signed helper/app
entitlements, and the common embedded-resource preflight from ticket 19. It
copies installed common frontend notices without a second frontend collector.
The Mac notice collector copies the actual SDK SBOM, matching source licenses,
actual shipped Homebrew keg notices and version provenance. Missing required
notices fail closed. Real SDK/keg output remains untested.

Release/weekly signing uses a complete conditional secret set. Empty configuration
uses explicitly reported ad-hoc signing. Partial configuration fails before
keychain or signing commands. The configured path signs contained code before
enclosing bundles, then creates the DMG, signs it, requires accepted notarization,
staples it and checks Gatekeeper. Bundle changes finish before final signing.
Temporary certificate/keychain cleanup runs on failure. No secrets were accessed
and no signing, notarization, publishing, or account operation was run here.

## Focused local checks

- Red: the new helper-missing test failed because the package checker did not exist.
- Green: `python3 scripts/tests/test_check_macos_bundle.py` in WSL. Four fixture
  checks cover both architecture layouts, unsupported architecture, missing
  helper/resource/locale/snapshot, helper execute permission, external helper
  symlink, and missing required native notice text.
- `bash -n scripts/deploy-macos.sh scripts/sign-macos.sh`.
- YAML parse of six affected workflows and the composite action. Each of the ten
  Mac jobs has exactly one action call and an explicit supported architecture.
  PR action calls contain no signing secrets.
- A real invocation of `sign-macos.sh` with a partial signing environment rejects
  the missing certificate before native tools run.
- `git diff --check`.

The fixture files are placeholders. They are not Mach-O or signed applications.
No package, native dependency, signature, SDK installation, notarization,
Gatekeeper, GPU, or runtime pass is claimed. The first actual Mac build can
expose SDK archive/layout, native dependency, notice, deployment or signing
issues. Ticket 23 retains both architecture package builds and clean-machine
normal/direct startup, actual Chromium GPU/sandbox evidence, native playback,
scaling, keyboard/controller input, focus, microphone, and setup restoration.

## Final source milestone

Merged integration `1d769ee9ec3bfd267137d8ca738255feed9c7f90`, including ticket
19's common QCore `--check-package` implementation and frontend notice install.
The merge had no conflicts. It retained Windows and Linux workflow sections.
Moved the single preflight from deployment to after final app signing and
entitlement verification, before DMG creation. It uses `env -i` and a
system-only PATH, so inherited Qt, DYLD, Node and SDK overrides cannot supply
package dependencies. The preflight remains read-only and starts no GUI.

User direction supersedes the remaining test plan: skip the rest of the build
and performance tests and focus on bringing the UI to prototype standards.
No new build, package, signing, notarization, runtime, performance, or repeated
fixture-matrix check was run for this final source correction. Previous
completed fixture/static results above remain recorded. No actual Mac package
or hardware acceptance is claimed. Ticket 21 is resolved as the source
milestone under this user-directed scope.
