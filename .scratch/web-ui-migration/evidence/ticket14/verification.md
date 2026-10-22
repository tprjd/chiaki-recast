# Ticket 14 verification

Color mapping uses compiled Svelte, a real QWebEngineView, Qt WebChannel,
SetupBridge, QmlSettings, and the native Settings store. The bridge adds explicit
reads and guarded setters for the two parent options and 28 advanced controls.
Native setters retain persistence and placeboChanged notifications.

## Controls and callers

The control metadata keeps the native labels, ranges, steps, choice order,
default hints, and function conditions. The independent audit checks all
28 advanced controls against the retained QML test inventory and source labels.
Knee Default stays visible for every tone mapping function, as in the actual
QML control. Its older search description incorrectly required Spline or ST2094.
The new search description describes the actual control.

The Custom render preset enables the parent Color Mapping Enabled option.
The advanced page requires that option and the Custom color mapping preset.
Each gamut or tone function retains its dependent fields. Search reads public
metadata only. It focuses the first missing prerequisite, then the requested
field when available. Back and Escape return to Renderer and its opener.

Numeric fields now display the precision of their existing step. This fixes
native float 0.57 appearing as 0.5699999928474426. Formatting does not write a
setting. The native value, existing factors, and profile generation guard stay
unchanged.

InitializePlaceboSettings supplies High Quality and contrast recovery 0.30 for
new stores. The getter fallbacks are Custom and 0.0 when those keys are absent.
Both initialization and fallback behavior remain native and unchanged.

StreamRoot still loads PlaceboSettingsDialog and PlaceboColorMappingDialog for
live native settings. Their source and the native renderer remain unchanged.
No setup-only leaf without a caller was found. No color or HDR processing moves
to the browser, and no second settings store was added.

## Build and environment

The private source branch is codex/web-ui-ticket-14. The initial source is
integration 5a61e86a. Source commit 34567f5e implements the settings. Merge
bf4f1263 adds integration documentation through fab6df5c. Commits aa19a4b6 and
b601a8f9 separate layout capture and correct its native fixture. Commit 9f7ba725
registers color descriptions and new navigation text for translation. It reuses
the existing SettingsSearch and native dialog contexts. English values and
setting behavior match the tested source.

Merge e2ddc842 includes ticket 08 integration a5766094. Its PsnRequests and
DiscoveryRequests constructors and remote connection work remain. The later
integration documentation tip a086e0b8 is also merged. The combined target
builds successfully. No runtime test was repeated after this merge.

The private build is C:/Users/david/.cache/chiaki-msvc-native/ticket14-build.
The saved configure and build command files use the prepared environment.cmd,
Qt 6.9.3 MSVC2022 x86_64, clang-cl 19.1.7, and the existing dependencies.
The gf-complete Windows vendor patch remains local and excluded from commits.
The target chiaki-embedded-setup-tests builds successfully. The Svelte and
TypeScript check reports zero errors and zero warnings.

Each runtime group had an exclusive slot from the root agent. All groups used
QT_QPA_PLATFORM=offscreen, QT_QUICK_BACKEND=software, and the test-only Chromium
flag --disable-gpu. PATH included this build's third-party/steam-tools directory.
Windows QtTest results use explicit -o logfile,txt. An initial red launch had
no useful redirected QtTest stdout. The same source and executable were
relaunched with the explicit log option.

## Results

| Check | Result |
|---|---|
| First colorMappingSettings red | Expected missing Color Mapping Enabled field, 2 passed and 1 failed |
| First green group | Native float formatting exposed; 5 passed and 1 failed |
| rendererSettings, rendererLayout, settingsValidation in that group | All passed |
| Corrected colorMappingSettings, videoAudioSettings, keyboardMouseSettings | 5 passed, 0 failed, 18256 ms |
| Separate-process colorMappingRestartRead child | 3 passed, 0 failed, 905 ms |
| First dedicated colorMappingLayout | Fixture omitted Custom color preset, 2 passed and 1 failed |
| Corrected colorMappingLayout | 3 passed, 0 failed, 2835 ms |

The color behavior case checks dependent gamut and tone controls, strict type,
range and enum rejection, rejection when parent options are inactive, obsolete
profile generation rejection, native saved values, native adapter updates,
reload, separate process restart, search prerequisites, Back, and DOM focus.
The directly affected media and keyboard numeric cases pass after the display
precision change. The earlier renderer and general search results are reused.

The first capture attempt preceded WebEngine repaint and did not establish
layout. The separate layout case waits for exact innerWidth, visible heading
and control bounds, two animation frames, and a paint delay. It captures
1280 by 720, 900 by 600, 390 by 720, and 800 by 900 at 200 percent zoom.
All four final images were inspected. The page reflows without horizontal
clipping. The 200 percent image shows the larger text and stacked controls.

The user subsequently limited supported layout acceptance to at least 1024
CSS pixels. The 1280-CSS-pixel image is within that scope. The 900-pixel,
390-pixel, and 200 percent image at 400 CSS pixels are historical development
evidence. They are not current acceptance requirements. No further narrow
work or rerun followed this change. A supported 200 percent viewport of at
least 1024 CSS pixels was not captured in this ticket.

## Evidence identity

Each *-source.json.gz file is the complete pre-launch source inventory. It records
HEAD, physical SHA256 values for every tracked file, complete new text files,
the working diff digest, and executable SHA256. The paired .diff files keep the
actual changes. There were no new production files before these launches.
The source inventories retain the exact red, first-green, corrected behavior,
first-layout, and corrected-layout identities.

The corrected behavior executable SHA256 is
9b7ded78da423a51f7e61959fb5da784a4ac467c80139196200d5b26589f3de4.
The corrected layout executable SHA256 is
cf3e9aca74498c51b4381b53a4ac847a61ec960b3b90f4a903610658681c098a.
The child uses the corrected behavior executable.

The audit script records all 28 independent metadata checks and 28 bounded
source comparisons. Native settings, live QML, window, request constructors,
and discovery files match integration a5766094. The color setter method,
shared SettingField, metadata, and six caller test bodies match the passing
behavior source bf4f1263. Settings.svelte differs from that source only by
the tested Back button class. The color behavior test differs only by moving
capture into the separate layout case. The merged build checks source and
constructor compatibility. Shared CSS matches the tested source after the
unrelated PSN refresh selector is excluded. It does not replace the embedded runtime checks. The final
identity manifest records normalized source and evidence digests for review.

## Limits

These are offscreen form, service, persistence, and layout results. They do not
establish GPU acceleration, physical focus, controller behavior, HDR output,
live console playback, renderer quality, latency, or platform packages.
The native graphics, decoder, and hardware matrices were not repeated for this
settings metadata change. Existing native live QML source is preserved. No new
live playback acceptance is claimed.

The inherited backend can start LAN discovery during construction or cleanup,
even when the saved discovery option is false. These earlier fixtures isolate
settings, but do not establish zero network traffic. The separate account work
adds native control of that discovery boundary. Test logs also retain the
known Qt font-directory warning. Text is visible in the inspected captures.
