# Ticket 09 verification

The web setup can start automatic PSN registration for the current opaque local
console. The native backend keeps the target MAC, DUID, console type, profile,
and attempt identity. It uses the existing registration-only connection and
completion path. Native completion saves the registration once.

The application controller prepares the real native renderer without showing
the stream window or loading StreamRoot. Setup stays visible. Registration does
not change stream geometry. Discovery stays active for this operation so that
the current-target guard can detect a removed or replaced console. Normal PSN
playback retains its existing discovery policy.

The shared context now permits manual pairing of a discovered local console
with a DUID. Remote-only rows stay rejected. Manual registration, automatic
registration, and playback cannot start over active registration work. Native
cleanup completes before retry or later playback. The obsolete QML registration
method dialog, signal route, resource entry, and screen test were removed after
the caller search. Native stream QML remains.

## Source and build

- Production implementation: `6faf8ff3ebded10c6de035d5b2cc5a4a78efa227`.
- Latest integration merged: `25ece857`, including controller source `56acff77`
  and final controller audit `df307813`.
- Audited worker source: `6a18262dcefe4e4ce72011c5b21713bf98e1b2b5`.
- Built GUI tree: `962d140dd463ad143e9f7b165b1a9e76708d92e5`.
- Full application and embedded target builds pass after the controller merge.
  Svelte check reports zero errors and zero warnings.
- Windows Qt 6.9.3 MSVC x64, clang-cl 19.1.7, libplacebo 7.360.1, FFmpeg 7.1.
  Each launcher first calls the prepared environment.cmd, then adds this
  worker's SteamTools directory. Shared dependencies were not rebuilt.
- The known local gf-complete VC patch is excluded from the commit.

`final-artifacts.txt` records the application, embedded executable, compiled
assets, and lockfile hashes. `verify.py` checks 24 bounded source comparisons.
The only conflict kept both controllerSetup and the registration busy guard in
the setup snapshot. The native controller, setup widget, web navigation, and
canonical native constructor order match the controller integration. The
connectConsole comparison allows only its added controller-operation refusal;
the registration fixtures have no controller mapping operation.

## Runtime results

The maintained check runs compiled Svelte in QWebEngineView, the real Qt
WebChannel transport, the production bridge, and native services. Per-instance
HTTP, device list, discovery, PSN worker, registration callback, and raw session
start boundaries are controlled before construction. A successful fixture start
creates a real chiaki_thread that delivers CHIAKI_EVENT_REGIST and QUIT through
the raw session callback. Native cleanup joins that thread. Unexpected playback
starts return an error before console traffic.

Every runtime group had a root slot and a source, full diff, untracked-file
inventory, and executable hash recorded before launch. Empty early untracked
inventories produced no file; their source status records contain no untracked
source entries. QtTest writes explicit text logs. Normal Windows runs use only
test-process Chromium `--disable-gpu`; they make no graphics or physical-focus
claim. New checks use a 1200 by 800 setup window.

| Record | Result |
| --- | --- |
| red | 2 pass, 1 expected failure at the missing automatic action. |
| green01 | Offscreen cannot create a native OpenGL context. No worker starts. |
| green01-windows | Native resources and thread work run. The completion check exposes discovery clearing its current target. |
| green02 | 3 pass, 0 fail after keeping discovery active for registration-only work. Duplicate completion saves once, settings reopen, resources are released, setup stays visible, and stream geometry is unchanged. |
| guards-red02 | 2 pass, 2 expected failures. Changed DUID and wrong returned MAC each save a registration before the native guards are fixed. |
| final01 | 12 pass, 2 fail. Success, retry, cancel, profile change, removed target, changed DUID, wrong MAC, request exclusion, and affected manual form/progress pass. Success also starts and dismisses a controlled normal native playback attempt using the same services and controller after hidden cleanup. |
| final02 | 4 pass, 0 fail for only the two corrected fixture cases. PS4 nickname rejection passes. The local-DUID manual context rejects a changed address and duplicate rows, completes the real native manual callback, and rejects remote-only rows and an old profile. |

The final01 PS4 fixture accidentally gave a PS5 and PS4 the same nickname. The
existing native lookup takes the nickname match first. The corrected fixture
gives that PS5 a distinct name and preserves native PS4 behavior. The context
fixture expected the wrong refusal text; the native address guard already
rejected the request. Only these two cases were repeated. All ten automatic
data rows and both affected manual cases have a passing result at equivalent
production source. The final01 invalid test-name invocation is retained; it ran
no test body and was corrected to registrationProgress.

No runtime was repeated after the controller merge. The compared registration
functions, hidden-window controller, and test cases are preserved. The merged
application and harness were rebuilt and the frontend was checked.

## Limits

These checks do not prove live PSN or console registration, physical controller
input, actual focus, GPU coexistence, decoder performance, or package behavior.
The unchanged native same-nickname PS4/PS5 selection rule is not certified by
the distinct-console fixture. Real console checks and the combined method
chooser remain in ticket 23. No new layout work or capture below 1024 CSS
pixels was added. The renderer/input algorithms and graphics matrices were not
repeated.
