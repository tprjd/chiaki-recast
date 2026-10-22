# Ticket 15 verification

The setup page uses compiled Svelte, QWebEngineView, Qt WebChannel, SetupBridge,
QmlSettings, and the native Settings store. The tests operate the real page and
native capture dialog. Rejected commands use a second real WebChannel client.
There is no browser bridge fixture or production test interface.

## Behavior

The Keyboard and mouse category contains the native mouse response fields,
22 binding actions with two slots each, and the existing legacy keyboard
controls. The native platform flag controls mouse camera availability.
The response controls use the native bounds and units. They change no input
computation or calibration value.

A native QDialog captures Qt key and mouse events. It uses the existing input
normalization, physical identity, validation, names, defaults, and QmlSettings
mutation. Capture waits for a matching physical release. It preserves reserved
keys, conflict moves, clearing, cancellation, and the explicit Assign Tab action.
Only the native prompt consumes capture events. DOM key names cannot assign a
binding. The setup bridge has no gameplay input method.

Each form has a native identity and profile generation. Closing the form,
changing profiles, reloading the page, renderer termination, hiding the window,
or suspending setup invalidates capture. Window restoration renews the form
without changing its selected tab. Capture returns focus only to its active form.
The native stream input ownership module and gameplay capture code are unchanged.

The web reset confirmation starts with Cancel focused. Its action retains the
form and profile identity from confirmation opening. Cancellation writes no
setting. Confirmation calls the existing complete native reset from either tab.
The reset restores response and both binding slots and turns mouse camera mode
off. It preserves legacy keyboard and mouse touchpad preferences.

Legacy assignment uses the original native helper. The new capture adapter emits
the existing model notification after that helper returns. The old QML capture
callback remains unchanged. The native live settings page reads the same store.

## Source and build

The checked settings merge is `280989cf124919d74f0bb0dde8406d20e6acb376`.
It includes renderer integration `22bb2934` and the native handoff work.
The later integration merges through `5555c00a` change only issue and map
documents. Production source hashes remain identical to the checked merge.
The application and embedded test executable build in the private
`C:/Users/david/.cache/chiaki-msvc-native/ticket15-build` directory.

The environment uses Qt 6.9.3 MSVC2022 x86_64, clang-cl 19.1.7, and the installed
MSVC 14.44 STL. The saved configure command uses the prepared native dependency
environment. Desktop submodules use their pinned commits. The existing
`scripts/windows-vc/gf-complete.patch` is local and excluded from commits.
`identity.json` records source, application, test, compiled asset, and image hashes.
`preserved-source.json` contains 45 passing source comparisons.

The comparisons retain the user's `d0e2f052` binding and mouse algorithms,
native defaults, response persistence, reset, input ownership, and live panes.
They also retain the tested input form commands and final KeyboardMouse component.
The renderer backend and ProcessLauncher sources match `22bb2934` exactly.
Renderer action and file APIs remain in the shared bridge.

## Focused application results

| Case | Built source | Result |
| --- | --- | --- |
| Initial category red | `e8fe9c51` with the first test | Expected missing-category failure |
| Initial category green | implementation before `6c7d5271` | 3 passed, 0 failed |
| Response, persistence, numeric limits, complete reset and cancellation | `7f8f5c2f` | Settings case passed in `ticket15-behavior.txt` |
| Capture restore red | `5ffee502` | Capture cancelled on suspend but could not reopen after resume |
| Capture restore green | `b44a13e6` | 3 passed, 0 failed |
| Guard and legacy lifetime checks | `e40a6a9c` | 4 passed, 0 failed |
| Clean command transport and search/layout | `8e101068` | 4 passed, 0 failed |
| Final capture and presentation | `3eb0f7b8` | 4 passed, 0 failed |
| Shared settings merge, legacy notification and renderer setting actions | `280989cf` | 5 passed, 0 failed |
| Final Svelte and TypeScript check | merged source | 0 errors, 0 warnings |
| Full application and embedded test build | merged source | Passed |
| Current numeric response tests with GNU | `0e2702b7`, equivalent input sources | 10 passed, 0 failed, 196 ms |

Every runtime group held the root-coordinated slot. The Windows runs used
`QT_QPA_PLATFORM=offscreen` and test-only `--disable-gpu`. Final images also used
`QT_QPA_FONTDIR=C:/Windows/Fonts`. Each identity file names its actual executable
hash before the next build. The two earliest category runs predate this hash
recording; their text outcomes are retained as development evidence only.

The first capture group reached the final profile assertion and failed because
the test read the previous Settings instance. The corrected assertion reads
`backend->nativeSettings()`. No production profile change was needed.
The initial guard fixture overlapped its temporary WebChannel client with the
page's form request and produced two callback warnings. Synchronizing the test
removed them. The clean guard result is `ticket15-presentation.txt`.
The merge check retains the renderer test's controlled failing process launcher.
Its expected relaunch warning does not start another application.

The maintained checks cover native capture completion, reserved input rejection,
conflict moves, Enter normalization, mouse buttons, clear, explicit Tab, mismatched
physical releases, focus loss, form close, reload, suspend/resume and profile
cancellation. They cover malformed types and values, inclusive numeric limits,
profile persistence, native notifications, legacy assignment and reset, stale
numeric drafts, and stale reset actions. Both response-tab and binding-tab
confirmation paths pass. Native live settings values match the web results.

The images in `layout/` were inspected. Binding and response controls fit at
390 pixels and at 200 percent zoom. Native capture fits the narrow parent.
Reset remains readable, starts with Cancel focused, and returns focus on cancel.
The native offscreen capture uses a fallback font; its image is a layout check.
These images come from `3eb0f7b8`, before the renderer category merge. The
KeyboardMouse component is unchanged, and the shared settings merge passed.

## Retained native evidence and limits

The original native reset and live settings route records remain under
`.scratch/keyboard-mouse-controls/evidence/`. Reset behavior and native helpers
are unchanged. The original acceptance record explicitly treats these as
earlier-revision evidence. Their physical calibration limits still apply.
The older `response-numeric.log` predates the user's pending response changes
and is not a current numeric result for this migration.

The prepared Linux ticket 03 executable supplies the current numeric result.
Its mapper, header, and eight-case test source match this branch after CRLF
normalization. The relevant native settings and QmlSettings methods also match.
`gnu-response-identity.txt` records the source and pre-run executable hash.
`gnu-response-command.txt` records the exact command, and `gnu-response.txt`
records the passing result. The exit code is 0. No rebuild ran.

This focused QCoreApplication check uses WSL Ubuntu 26.04, GCC 15.2.0, and
Qt 6.10.2. It checks prompt starts, stops and reversals, low-speed precision,
phased report traces, timer jitter, inverse response, stale movement, reset,
persistence, invalid stored values, and numeric validation. Root approved this
non-GUI check alongside another worker's GUI check. It uses isolated settings
and has no shared GPU or window work.

Migration ticket 01's `native-opengl-input.txt` and `native-vulkan-input.txt`
retain the production input ownership and held-input release checks. This ticket
does not replace the GNU wrapped native tests with MSVC tests.

No live console, physical mouse calibration, physical focus, Windows Raw Input,
hardware GPU, package, or new Linux/macOS desktop acceptance is claimed.
The feature's numeric defaults remain provisional. The shared native QML panes
and settings routes still have live stream callers and remain in the resources.
