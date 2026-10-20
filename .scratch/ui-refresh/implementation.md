# Card library implementation

Date: 2026-10-20
Integration branch: `codex/ui-refresh-integration`
Base commit: `0c4a45d`
Specification: [Main console screen: card library](spec.md)

## Ticket dependencies

`01 -> 02 -> 03 -> 04`

`03 -> 05 -> 06`

`04 + 06 -> 07 -> 08`

Each ticket uses an isolated branch and worktree. A merger agent integrates
each completed ticket. Search and details can run in parallel after ticket 03.

## Existing behavior

- `MainView.qml` displays `Chiaki.hosts` as rows. Hidden entries have zero height.
- `connectToHost` receives the backend list index and the discovered nickname.
  The backend handles registration, standby wake-and-connect, manual connections,
  and remote PSN connections.
- Manual rows offer Delete. Unregistered discovered rows offer Hide.
  Registered rows offer PIN entry. Registered manual rows offer explicit Wake.
- The screen provides Settings, Add Manual Host, discovery, conditional PSN
  refresh, conditional Steam shortcut creation, the version, and Quit.
- Activation runs the existing Steam and PSN reminders. Controller shortcuts
  dispatch through the main screen.
- The existing PIN dialog stores a list index until acceptance. The new flow
  must resolve its target again when the user submits the PIN.

## Verification boundary

The spec approves the production QML card-library screen with supplied host
records and observable action signals. Use Qt Quick Test at this boundary.
Do not replace native verification with browser prototype results.

The full native build and real console checks remain pending. Record unavailable
hardware or account checks as untested.

## Completed work

- [01: Native card baseline](issues/01-native-card-baseline.md), merged as
  `9afeb20`. The production card and focused C++ Qt Quick Test harness are
  available. Three behavior checks pass. Full-app and hardware checks remain
  pending.
- [02: Host states and primary actions](issues/02-host-states-and-primary-actions.md),
  merged as `32a9711`. Current-identity actions, profile invalidation, host
  labels, and PSN backend indices are implemented. The Qt screen checks pass.
- [03: Card library and navigation](issues/03-card-library-and-navigation.md),
  merged as `d34a667`. The old list is replaced. The production QML captures
  were inspected at both requested sizes, and 61 screen checks pass.
- [04: Search, counts, and live selection](issues/04-search-counts-and-live-selection.md),
  merged as `02d5c62`. The single-card stretch is fixed. Search, empty states,
  source targets, and live selection pass the screen checks. The integration
  agent inspected the desktop result and narrow empty-state captures.
- [05: Console details and privacy](issues/05-console-details-and-privacy.md),
  merged as `689d56b`. Live details, masking, plain host text, long-content
  scrolling, and focus recovery are implemented. The combined screen suite
  reports 106 passing checks. The integration agent repeated CTest using
  the source in the integration checkout.

## Native baseline evidence

The complete Windows application built at `9afeb20` in a task-cache checkout.
The QML screen and existing core CTest targets passed. An isolated native
OpenGL launch rendered the baseline main screen. The integration agent
inspected [the native capture](native-previews/baseline-01-native-1280x720.png).
It has 1280 by 720 logical pixels and 1600 by 900 physical pixels at the current
125% desktop scale. No account or private console data appears in the image.

Vulkan capture returned a blank image and is not visual acceptance evidence.
Native build and isolation commands are recorded outside the repo in
`C:\Users\david\.codex\ui-refresh-native-toolchain.md`.

## Native card-screen checks

The complete application and both CTest targets passed at `d34a667`.
The integration agent inspected the full-app captures at 1280 by 720 and
800 by 600 logical pixels, with device pixel ratio 1.25.

[The native action record](native-previews/03-native-checks.json) confirms
Settings, Add console, and registration entry through real mouse events,
followed by Escape return. Registration targeted the disposable RFC 5737
manual entry exactly once. No session started. The entry was removed after
the check. Test settings remained in the cache.

The single visible card stretched across the grid. Ticket 04 will fix this
for filtered results as well. Existing Settings and registration forms clip
at narrow sizes; their contents are outside this screen change.

## Native search and details checks

The complete application and both CTest targets passed at `689d56b`.
[The native action record](native-previews/05-native-checks.json) confirms
search, Enter isolation, counts, empty-state recovery, details masking,
focus recovery, and details closure after target removal. Existing dialog
entry and return checks also passed. No session started, and cleanup passed.

The integration agent inspected the native library, no-match, and masked
details captures. The images come from the actual OpenGL QML framebuffer,
because desktop window capture was unreliable when the test window was
occluded. The production renderer and UI behavior were unchanged by that
cache-only capture hook. Single-card width is now 321 logical pixels.

- [06: Console management](issues/06-console-management-actions.md), merged as 04d6ec8. Guarded acceptance and existing backend handlers cover PIN, Wake, Delete, and Hide. All 149 screen checks pass. The integration checkout repeated CTest successfully in 17.77 seconds. Full native lifecycle evidence follows separately. Ticket 07 is claimed in its own worktree.

## Native management checks

At 04d6ec8 the clean full application built, and both native CTest targets passed in 27.46 seconds. The integration agent inspected the real PIN and failure captures.

The native checks proved that Escape and Back preserve PIN 1111, Set stores the exact value 0042 on disk, and a second console keeps PIN 2222. Delete removes the intended disposable entry. No cancellation preserves the later entry, and a stale acceptance cannot delete a console that reused its index. Fixture cleanup passed, with no session and discovery disabled.

The record native-previews/06-native-checks.json has passed:false. A real focus defect remains in this revision: details can finish closing after Delete confirmation opens, steal focus behind that modal, and let Escape open Quit. The failing capture and JSON remain as evidence. Ticket 07 owns the fix and native replay. All hardware Wake checks remain untested.

## Input completion

[07: Keyboard and controller navigation](issues/07-keyboard-and-controller-navigation.md) is merged as 70f22b3. Screen and popup rendered focus, Tab order, grid isolation, focus scrolling, controller hints, opener restoration, and the native details-close race are covered. The integration agent inspected full-screen primary, last-card, and popup focus captures and repeated CTest against integration successfully in 64.22 seconds. Native Escape replay follows separately. Ticket 08 is claimed for final layout, scaling acceptance, and test documentation.

## Native input replay

The full application at 70f22b3 built all three targets, and both native CTest targets passed in 68.82 seconds. The native replay record native-previews/07-native-checks.json has passed:true. The integration agent read that record and inspected the real Delete-cancel capture.

Repeated Details, Delete, Escape now closes Delete without opening Quit, preserves the intended entry, and restores its ellipsis opener. No and PIN return to their opener as well. PIN 0042 persists on disk, its peer stays 2222, stale Delete cannot act on a reused index, and all fixtures were removed. No session started. The 06 failing record is retained as regression evidence; the defect is fixed at 07.

## Final code and review

[08: Native acceptance and polish](issues/08-native-acceptance-and-polish.md) is merged as 43a839d. Desktop search is compact, optional PSN refresh stacks in narrow windows, and the heading fits 390 by 600. Full application wheel events now reach QML in UI mode under the existing stream input gate. The integration screen suite passes CTest in 64.20 seconds. All 190 screen checks pass. Deliberately dark PS4/PS5 artwork and an absent primary background each fail the strengthened contrast guards, followed by restored source and a passing suite.

The integration agent inspected the final B desktop/narrow and long-details captures, checked production resource lists, and verified that the spec and prototype hashes remain unchanged. Production contains no fixture, capture, prototype route, or QuickTest link. Test installation remains disabled.

The independent [Standards and Spec review](code-review.md) reports zero findings on each axis at 43a839d. The focused-host reorder runtime probe also passes. No review fix is required. Final measured-DPR native acceptance is still being completed.

## Acceptance complete

All eight tickets are resolved at 43a839d. Final native DPR 1.0, 1.5, and 2.0 records pass at all three sizes. The integration agent checked all nine actual image dimensions and inspected the native 150-percent library and 200-percent narrow details. Startup, auto-connect cancellation, profile/error/Steam gates pass. No review fixes remain. [Final acceptance and limits](acceptance.md) contains the direct records and reproduction pointers. Ticket worktrees are being archived; the root integration branch remains available.

## Cleanup complete

Only the main integration checkout remains in `git worktree list`. Seven ticket worktrees were archived. The app re-attached the clean baseline checkout after archive, so it was removed with Git after verifying its absolute target, clean state, and merged ancestry. All ticket branches remain available. Root has only its original untracked paths and no tracked modifications.

Native cache source is clean at 43a839d. The normal production binary is rebuilt with CMAKE_PROJECT_INCLUDE empty. Its actual SHA256 matches the final native summary, and no final QA process remains. Full evidence is linked from acceptance.md. No source change followed the two-axis review.
