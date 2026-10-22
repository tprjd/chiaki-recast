# Ticket 07 verification

Pairing and Account ID routes use the compiled Svelte page, real WebEngine,
WebChannel, SetupBridge, and native services. Registration, lookup, and OAuth
outcomes are controlled at their native asynchronous transport boundaries.
No fixture registration, public lookup, or OAuth request reached a live service.

## Source and build

Implementation starts at `3e7a9229`. Merge `0bb38d01` preserves ticket 05 manual
identities and ticket 15 keyboard settings. `b2fa6852` contains the first full
functional checks. `c152f87b` adds the address-correction regression.
`c7a5c7d1` fixes its native context and actual opener focus.
`add8c676` contains the corrected native dismissal check and safe focus fallback.
Merge `b7968026` includes ticket 08 through integration `a5766094`.
The final source also merges the user layout scope through `ade6e7fb`, ticket
14 through `c3f35350`, and the ticket 10 claim through `42d5a5e0`.
Final code merge `878d1af6` has GUI tree
`c3e47fc49ca8c566079614c2eb3d7b773c9b5823`. The later `3d07a8b2` merge changes
only ticket 10 claim documentation and preserves that GUI tree.

The final constructors preserve this order:

```text
PsnRequests, ProcessLauncher, RegistrationRequests, DiscoveryRequests
```

The final ticket 07 fixtures receive controlled discovery before construction.
Their unexpected raw session starts fail closed. The separate native handoff
keeps the real raw session start and uses only `127.0.0.1`.
Earlier functional and handoff runs had inherited LAN discovery broadcasts,
despite saved discovery=false. They had controlled registration, lookup, and
OAuth requests. This limit is retained. Passing results were not repeated only
to remove that background-discovery limit.

The Windows build uses the prepared Qt 6.9.3 MSVC x64 environment, clang-cl
19.1.7, fixed vcpkg libraries, FFmpeg 7.1, and libplacebo 7.360.1. The source
keeps the existing local VC gf-complete patch outside commits.
The final full application and embedded test target both build after the
ticket 14 merge. Svelte check reports zero errors and warnings. The final
reports are `final-app-build.txt`, `final-build.txt`, and
`final-frontend-check.txt`. The native commands are the saved
`ticket07-app-build.cmd` and `ticket07-build.cmd`; the frontend command is
`npm.cmd run check` from `gui/web`. Exact commands and earlier build reports
are saved.

## Focused results

| Report | Result | Meaning |
| --- | --- | --- |
| `ticket07-red-result.txt` | 2 pass, 1 fail | Missing guided registration entry. |
| `ticket07-form-result.txt` | 3 pass, 1 fail | Form passes; native progress was missing. |
| `ticket07-registration-result.txt` | 4 pass, 1 fail | Native progress/failure pass; lookup entry was missing. |
| `focused-result.txt` | 14 pass, 2 fail | Lookup, timeout, sign-in return, native validation, and stale targets pass. Address retry and opener focus failed. |
| `correction-result.txt` | 5 pass, 0 fail | Address retry, form, and native progress pass after correction. |
| `handoff-result.txt` | 2 pass, 1 fail | Registration, reload, native exposure/hide and session cleanup pass. Fixture omitted native presentation dismissal. |
| `handoff-correction-result.txt` | 3 pass, 0 fail | Correct manual association, native handoff, supported dismissal/cancel, window cleanup, and setup restoration pass. |

The native handoff correction follows the existing ticket 05 sequence. The
application controller waits for presentation dismissal before it destroys the
window. No production native lifetime change was needed for this fixture error.
Ticket 04 and 05 cancel-presentation and native cleanup evidence remains valid.

The functional cases cover PS5 and all three supported PS4 firmware choices,
broadcast and entered addresses, Base64 validation, eight-digit pairing codes,
optional four-digit console login PINs, and leading zeroes. Duplicate activation
and completion cannot create a second registration. Failure retains valid
account and address input for correction. Stale manual/discovered targets and
profiles do not save registration. Saved PIN text remains native.

Public lookup checks service explanation before username transmission, HTTP
failure, unreadable and missing results, invalid and noncanonical Account IDs,
the actual native 30-second timeout, retry, replacement, and late replies.
It creates no saved authentication credentials. Lookup and sign-in return to
the same form with valid input and focus. A cancelled or replaced sign-in route
cannot apply its late Account ID. Snapshots and browser storage exclude tokens,
registration keys, saved PINs, temporary codes, and Account ID results.
Streamer mode masks fields and suppresses address results and diagnostics.

The handoff case registers an existing manual entry, keeps its native ID,
reloads the page during registration without replay, and connects after success.
It observes a native session/window, setup hiding, native dismissal/cancellation,
complete cleanup, and setup restoration. It does not contact a console.

## Reuse and limits

`verify.py` makes 37 bounded source comparisons with the tested revisions.
It records 21 source files, four final artifacts, constructor/default checks,
and all evidence hashes in `identity.json`. Run it with `--check` for a read-only
audit. It compares functional test bodies after excluding only transport
arguments and the later viewport/capture policy. The native operations, guarded
account routes, snapshot allowlists, sign-in UI, and pairing request adapters
remain equal. The one focus fallback is isolated from the earlier form source.
The final pairing component equals the passing native handoff source.

Ticket 08 retains its remote connection state, real transport defaults, and
controlled native request seams. The merge changes no ticket 07 business logic.
Ticket 14 adds native color settings in separate bridge and settings code. Its
merge preserves the compared registration operations, projections, request
adapters, and scenarios. The final build verifies the combined native callers.
The passing lookup timeout, OAuth, registration, and stale-target cases are
reused. No broad test rerun was required after this exact source review.

The user later set the supported layout floor to 1024 CSS pixels. The maintained
fixture now uses 1280 and 1024 widths, and 200 percent zoom in a 2560-pixel window.
Those fixture-policy changes compile, but were not rerun as new layout acceptance.
The saved 900/390 and narrow 200 percent images are historical evidence only.
The 1280 capture remains within the supported scope. Existing responsive CSS
and native window minimums are unchanged.

Routine functional runs use offscreen rendering and test-only `--disable-gpu`.
The separate native run uses normal Windows visibility and the existing Fast
preset. No result proves GPU acceleration, actual foreground/focus, a real
console pairing, live public lookup or Sony sign-in, playback quality, physical
controllers, input latency, or performance. Those acceptance limits remain in
ticket 23. Native input and media code is unchanged by this ticket; applicable
04/05/08 evidence is reused. The GNU-only native output targets were not run.
