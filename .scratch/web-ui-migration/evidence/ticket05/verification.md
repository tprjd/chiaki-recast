# Ticket 05 verification

Console management and manual addresses use compiled Svelte, QWebEngineView,
real WebChannel, SetupBridge, and the native settings services. The page does
not receive backend objects, registration keys, or saved console login PINs.
The browser keeps temporary form input only while the form is open.

## Source and environment

The implementation is `5bf0b24d`. Merge `4cfc374f` includes integration
`691342df`, with ticket 13 renderer settings and its native ProcessLauncher.
The merge retained both sets of bridge methods and state fields.
The shared privacy correction is `8d4725db`.
Merge `aa95d91d` then includes the documentation-only ticket 08 claim from
integration `5555c00a`. Application source is unchanged by that claim merge.
Commit `347b1488` adds the saved-PIN result focus correction and assertion.
Final merge `823cf551` includes ticket 15 and the ticket 14 claim through
integration `24f57a8c`. The final full application and harness build passes.

`identity.json` records the source commits, working-file SHA256 values, and
Git-blob SHA256 values. The blob hashes support checkout line-ending changes. It also records
19 source files, the final application and test executables, compiled JavaScript
and CSS, evidence files, and 27 source comparisons. All comparisons pass.
The comparison script normalizes line endings and reads source as UTF-8.
Two comparisons exclude only the later saved-PIN result focus block and its
added assertion. Their focused red and green reports are recorded separately.
Evidence files disable automatic line-ending conversion to retain their raw
recorded bytes. The original pre-run identity files retain the exact executable SHA256 for
each runtime group before later builds replaced that executable.
Earlier uncommitted sources are identified by their base commit, saved patch,
and hashes for the two then-untracked Svelte files. The passing focus and
manual handoff groups used the same source patch.

The private build directory is
`C:/Users/david/.cache/chiaki-msvc-native/ticket05-build`.
The saved configure and build commands use Qt 6.9.3 MSVC2022 x64,
clang-cl 19.1.7, MSVC 14.44, Node 24.16.0, and npm 11.13.0.
Dependencies use the prepared MSVC environment and the fixed vcpkg install.
The pinned submodules were initialized in this worktree. Only the existing
Windows gf-complete patch was applied locally. That vendor change is excluded
from the commit. Its patched file hash is recorded separately.

The final application and embedded harness build passed.
The Svelte and TypeScript check reports zero errors and zero warnings.
No dependency or production renderer preset was changed.

## Focused results

| Case | Result |
|---|---|
| `manualAddress` first red | Expected missing Add console control, 2 passed and 1 failed, 22220 ms |
| `manualAddress` green | 3 passed, 0 failed, 1132 ms |
| `manualAddressValidation` red | Expected enabled Save for a malformed URL, 2 passed and 1 failed, 22333 ms |
| Validation, management, guards, and affected library group | Validation, guards, and library passed. Management found the missing return focus after non-pointer activation. Total 5 passed, 1 failed, 30663 ms |
| Corrected `consoleManagement` | 3 passed, 0 failed, 2198 ms |
| Windows `manualHandoff` | 3 passed, 0 failed, 2517 ms |
| `consoleManagementPrivacy` red | Expected unmasked hostname alias with different letter case, 2 passed and 1 failed, 644 ms |
| `consoleManagementPrivacy` green | 3 passed, 0 failed, 1419 ms |
| Saved-PIN result focus red | Expected missing Close focus after Save removes its button, 2 passed and 1 failed, 22908 ms |
| Saved-PIN result focus green | 3 passed, 0 failed, 2232 ms |

Every group held the root-coordinated runtime slot. Routine form and library
checks set `QT_QPA_PLATFORM=offscreen` and the test-only Chromium
`--disable-gpu` flag. The separate Windows handoff group used normal window
show behavior. It selects the existing native Fast preset in the isolated test profile.
It did not require or assert desktop foreground ownership.
All group commands and QtTest reports are saved beside this note.

Passing behavior was reused. The management commands, connection command,
native target and session functions, six affected test bodies, management
frontend files, native application controller, stream root, and live PIN
components match the passing implementation. The shared private-name comparison ignores letter case. The focused privacy
case checks that change after the renderer integration merge. The final
saved-PIN result check verifies focus on Close after success. A retryable PIN
error returns focus to the PIN field.

The final CSS merge retains both library and keyboard/mouse rules. Ticket 15
uses `aria-labelledby="reset-keyboard-title"` on its reset dialog. Its three
previous global dialog rules now use that existing selector. The reset dialog
retains the same rules, and the console dialogs retain their passing styles.
Source and selector review plus the successful final build support reuse.
Native capture and form lifecycle code is unchanged from integrated ticket 15.

## Verified behavior

The manual form rejects malformed URLs, ports, paths, spaces, invalid hostname
labels, and invalid IPv4 values. Native commands reject the same malformed
requests through real WebChannel. IPv4, IPv6, and hostname saves pass.
The native service trims the address and stores it in the selected profile.
The library refreshes from native state and retains the address after reload.
A fresh Settings reader also reads the saved value from the native store.

Existing registrations have opaque keys. Adding another registration while
the form is open does not change its selected console. Saving associates the
intended MAC. Removed and replaced registrations cannot reuse an old key.
Profile changes reject the old form. Valid non-secret address input remains
after a rejected selection. Repeated activation saves one manual entry.
Unregistered entries show Not registered and do not start a native session.
Ticket 07 supplies pairing and the unregistered connection entry points.

Console details show native type, state, registration, address, identifier,
and applicable actions. Untrusted console names are rendered as text.
The actions distinguish hiding a discovered console, restoring a hidden
console, deleting a manual address, and deleting registration credentials.
Each action resolves its current native target immediately before dispatch.
The tests cancel every destructive action before they confirm its result.
Manual deletion keeps the registration. Registration deletion keeps the
manual address and changes its library registration status.

Saved console login PIN edits accept exactly four ASCII digits. Leading
zeroes reach the native saved value. Invalid native submissions fail.
The form is initially empty and clears transient PIN text after submission.
A real transport snapshot contains neither the saved PIN nor registration
key data. PIN reveal is disabled in streamer mode.

A reordered discovery list preserves the intended hide target. Removed,
ambiguous, or edited targets reject stale actions. An old profile cannot
save or delete from the current profile. Private snapshots mask identifiers,
manual addresses, address aliases with different letter case, and hidden
address names. Details, confirmations, registration choices, PIN controls,
and address errors retain that masking. Errors do not echo the submitted
private address.

Dialogs set initial focus and return focus to their action. A deleted action
returns focus to the selected remaining card or Add console. Live discovery
retains library selection. The real embedded page passes narrow reflow and
900 by 600 at 200 percent zoom. The affected library case also passes its
1280 by 720, narrow, and zoom checks. These are browser layout and focus
checks. They do not establish physical controller, screen-reader, touch,
display scaling, or Windows foreground acceptance.

`embedded-manual.png` shows the inspected narrow manual form with masked
input. `embedded-details.png` shows inspected masked details and actions.
`embedded-library.png` shows the inspected selected library design and
native console artwork. Captures that retained an earlier rendered
presentation were discarded. No physical screen claim uses these captures.

The registered manual handoff starts with the actual web address form.
It saves the selected native registration, starts Connect from the library,
creates the exposed native stream window and native StreamSession, hides and
freezes setup, waits for native cancellation cleanup, destroys native session
and window resources, and restores the active setup page. The connection is
to loopback. It does not establish live console playback or first-frame time.

## Retired setup code

The old ManualHostDialog and saved ConsolePinDialog, their callback routes,
and their dedicated QML fixtures and tests are removed. Native
ConsolePinContent, LiveConsolePinContent, LiveConsolePinHost, and StreamRoot
remain unchanged. The shared legacy MainView, library, and settings resource
chains still support tests for later reminder, Steam, and settings work.
Ticket 23 retains their final cleanup once those flows are replaced.

## Reused evidence and limits

Ticket 04 native lifetime, decoded buffer cleanup, initialization recovery,
and sign-in closure evidence is reused. Ticket 03 accelerated Chromium and
native renderer coexistence evidence is reused. This ticket does not change
native media, input ownership, the decoder, or native window lifetime.
No full graphics or decoder matrix was repeated.

There is no real console, PSN, physical controller, microphone, HDR, input
latency, sustained playback, or release-package result in this evidence.
The Windows native handoff case does not prove GPU acceleration or actual
foreground focus. The recorded native HighQuality processing limit remains
open. Linux and macOS runtime behavior is not checked here. Tickets 19 through
23 retain package, platform, hardware, accessibility, and performance acceptance.
