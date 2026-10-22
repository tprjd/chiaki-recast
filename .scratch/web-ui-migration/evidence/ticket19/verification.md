# Windows package verification

Status: deployment code and initial package checks pass. Final source and
runtime acceptance are pending ticket 22 integration.

## Package implementation

The x64 release, weekly, pull request, and manual routes use the same MSVC
workflow. It selects LLVM 19.1.7, MSVC 14.44, Qt 6.9.3, Node 24.16.0,
and Vulkan SDK 1.4.357.0. An STL compile check rejects an incompatible compiler.
The old shaderc badge download returns 404. The workflow now uses the selected
Vulkan SDK shaderc and SPIRV-Cross binaries and pkg-config metadata.

The GUI target owns the locked frontend build and RCC compilation. Deployment
scans actual PE imports, stages matching Qt widget, Quick, WebChannel and
WebEngine files, copies app-local MSVC runtime DLLs, and includes dependency
notices. It copies all 53 selected WebEngine locale PAKs. The current SDK lacks
Qt message catalogs and `catalogs.json`. The deployment log retains that warning.
The package has English setup text and the complete Chromium locale set. This
does not prove translated Qt dialogs.

The package uses the relative paths in `gui/qt.conf`. Its resource preflight
opens the compiled index, script, style, and actual WebChannel resource. It also
checks the index references. This preflight runs before SDL, settings, native
services, QApplication, and Chromium.

MinGW and Windows ARM64 workflows report the selected SDK support limit. They
do not publish a package or select a QML setup fallback. The installer accepts
native x64 Windows only.

## Initial checks

The initial application and embedded test program build passed against
integration `478a116d` plus the ticket 19 resource preflight. This was before
the ticket 18 and ticket 22 source merges. It is package-boundary evidence, not
the final application runtime result.

- `initial-before-preflight.json.gz` records the source commit, source diff,
  new file hashes, and application executable hash before the preflight.
- `initial-deploy.txt` records successful deployment and the resource preflight
  with the development PATH and Qt overrides removed.
- `initial-package-test.txt` records the complete package pass and rejection
  after removal of `QtWebEngineProcess.exe` and
  `resources/qtwebengine_resources.pak`. Each file was restored.
- `initial-package-manifest.json.gz` has 1,582 files and 520,097,130 bytes.
  The uncompressed manifest SHA256 is
  `a49e16b3825aecca35eff20c00fe1ecb26646ac11eb1a2234aa5fd3626c02f9a`.
- `initial-runtime-identities.json` records staged Qt, helper, native, and CRT
  file versions and hashes. `windeployqt` patches QtCore's prefix. Its staged
  hash is recorded separately from the SDK hash.

All nine changed workflows pass actionlint 1.7.7. Its older runner inventory
needs the existing `macos-15-intel` label in an external configuration. A
structural YAML comparison confirms that the non-Windows jobs in release,
weekly, and pull request workflows are unchanged. Each workflow PowerShell
step parses. The actual changed SDK metadata step ran in an external directory.
`pkgconf` resolved versions `2026.3` and `0.68.0` and the expected SDK libraries.
These checks do not establish a hosted CI run.

## Installer limit

The official Inno Setup 6.5.4 tool download matches the GitHub release SHA256
recorded in `inno-tool-source.txt`. Authenticode reports a valid signature from
Pyrsys B.V. No installer or OS change ran. Automatic approval review rejected
the proposed tool installation with the stated reason "blocked by policy".
There is no existing local ISCC compiler. The ZIP can be checked locally;
installer compilation and installation remain unverified. The existing CI
Inno Setup path includes the complete staged directory.

## Acceptance limits

Only this Windows PC is available. No Linux or macOS GPU result is inferred.
Package file checks do not establish a GPU, sandbox, native playback, physical
input, microphone, foreground, or performance result. The existing hardware
graphics fixture uses the Fast preset. The independent native HighQuality
control failure remains in ticket 23. Production defaults are unchanged.
