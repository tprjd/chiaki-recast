# Shared setup button review cleanup

Date: 2026-10-20
Production source revision: `f939013851443607b7d03b736d8f94b03f374fd3`
Integration base: `3b3bd4457ffc2b494218b3002e55e234cc927da0`

The P3 duplicate-button finding is fixed. `SetupButton.qml` now holds the
wrapping label, vertical padding, and content height once. It uses the existing
`ConsoleButton`. Manual entry keeps its default AutoText format and its five
existing width policies. Registration uses a four-line local specialization
for PlainText and the parent width limit across all ten buttons. The component
is in the production QML resource list. Control names, IDs, actions, focus,
keys, and the surrounding layout are unchanged.

The code change is four production files, 32 insertions and 38 deletions.
All fifteen button instances were checked. No caller, fixture, test, export,
or build configuration needs another change. Existing tests already observe
the production screens at the agreed public boundary. A new test that checks
only this extraction would duplicate their assertions.

## Focused verification

The documented MSVC Qt 6.9.3 runner used this checkout's test input files and
production QML. It used offscreen/software rendering, the production Controls
configuration, and `QT_QPA_FONTDIR=C:\Windows\Fonts`.

- [Manual entry](evidence/review-ManualHost.txt): 23 passed, 0 failed.
- [Registration form](evidence/review-RegistrationForm.txt): 22 passed, 0 failed.
- [Registration results](evidence/review-RegistrationResults.txt): 19 passed, 0 failed.

The manual and results suites each render six cases at 900 by 600 and 1280 by
720. They read the resulting image dimensions and pixels. The manual form
also checks the blue primary button. The result cases expand action labels
and verify wrapping and reachability in progress, success, and failure.
Other checks cover account routes, key input, focus, validation, selection,
privacy, and button actions. All 64 focused checks passed without QML warnings.
`git diff --check` passed.

## Final normal build

Only the four changed production files were copied into the existing staged
source. All staged production GUI files match this source revision. The
[source manifest](evidence/review-production-source-manifest.json) records each
file hash and the exact binary hash. The [resource list](evidence/review-production-resources.txt)
contains the shared button. The normal source and complete build inputs have
no QA helper, fixture, or prototype. The [final link command](evidence/review-native-link-inputs.txt)
and the [normal build log](evidence/review-native-build.txt) record the build.
The [configure log](evidence/review-native-configure.txt) keeps the existing
native dependency configuration and uses a separate output directory.

Executable: `C:/Users/david/.cache/chiaki-ui-native/console-setup-final-bin/chiaki.exe`
Launcher: `C:/Users/david/.cache/chiaki-ui-native/console-setup-final-bin/Launch-chiaki.cmd`
SHA256: `f1c30bac465ce6920a2c542d77f169831627008cb6d63ad605f985dc148d76d6`

The final executable was not launched. The user's open application was not
controlled, closed, or replaced.

The [native acceptance record](native-acceptance.md) remains historical evidence
for revision `a19f26f1`: 627 full-suite checks and 111 native framebuffer captures.
Those checks were not repeated for this button extraction. The focused render
checks and final production resource build apply to the cleanup revision.
Real authentication, pairing, and physical controller input remain unverified.
Qt WebEngineQuick remains absent from the cached native toolchain.
