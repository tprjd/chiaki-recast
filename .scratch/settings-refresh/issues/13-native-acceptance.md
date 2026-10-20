# 13: Verify the complete native Settings experience

Status: resolved

**What to build:** Deliver one integrated native Settings implementation that
matches layout A, retains the complete settings inventory, and works through
the application's real entry points.

**Blocked by:** 12: Find settings across every category and linked page.

- [x] Reconcile all categories and linked pages against the native control inventory, including read-only data, dynamic lists, actions, conditions, defaults, options, and conversions.
- [x] Remove any obsolete Settings-only transition UI after all replacement routes work. Keep unrelated screens and the earlier console-library corrections intact.
- [x] Run the relevant production-QML screen suites, including existing console-library tests. Resolve Settings integration regressions.
- [x] Build and launch the native Windows app; compare layout A at 1280 by 720 and 900 by 600. The supported minimum width is 900 pixels.
- [x] Check 100%, 150%, and 200% scaling, long translated labels, scrolling, focus, keyboard/controller navigation, popup priority, and reachable actions.
- [x] Verify persistence, profile switching, library return, and native file-flow cancellation using isolated settings and temporary data.
- [x] Check physical controller capture and stream-time display/renderer entry points where hardware is available. Record unavailable cases as untested.
- [x] Confirm no fixtures, fake handlers, prototype controls, sample values, or unsupported-condition review mode ship in the application.
- [x] Record the actual executable, source revision, test results, visual evidence, and remaining limitations. Browser checks and a successful build alone are not acceptance.


## Comments

Native action check found a pre-existing unhide defect: QmlBackend::unhideHost checks strlen of a binary decoded MAC. A valid MAC with a zero byte is rejected. Fix the byte-count validation and rerun the actual native action check (05-06-actions-dpr1).

Visual review: Controllers and Config omit the category heading that General, Video, Stream, Audio/Wifi, Consoles, Keys and Remote show. Check consistency with layout A during final review. Native 900-pixel captures: 07-09-management-dpr1-controllers-900x600.png and config-900x600.png.

Focused-value follow-up: a root production-QML probe proves Video queueDepthLimit leaves its displayed value below the900px viewport (450+19>445). Probe/log in Temp/settings-refresh-value-probes/tst_VideoValueProbe.qml and settings-video-value-probe.txt. Display contrast probe passes at900 with both numeric modes enabled. Shared Settings scroll correction is being assigned separately.

Final verification reminders: include displayed value-label bounds when focusing sliders, not only the slider rectangle. Renderer now has strict native checks for both. Recheck main forms after the search row reduces available height. Use measured application DPR1/1.5/2; record OS-display-setting changes as untested. Exercise long translated labels through a temporary QA translator or equivalent real QML text change, and inspect captures. Keep all QA-only changes out of production build. Native isolation patch does not cover the separate Settings::DeleteProfile QSettings constructor; do not invoke native deletion with that patch. Existing production-QML tests cover deletion restrictions/dispatch.

Final polish decision: add Controllers and Config category headings using the same native Label pattern as the other seven categories. This completes the selected layout's heading consistency. Do this after Search merges, then check both pages at 900 by 600 with the reduced viewport.

Long-label native defect: 13-pre-long-labels-dpr1-renderer4-900x600.png shows the 180-pixel Renderer sidebar eliding expanded Sigmoidization, Peak Detection, and Color Mapping names. Make sidebar names wrap within Settings-owned delegates (main Settings, Renderer, Color as needed), with suitable height and reachable scroll. Do not change the shared controls/Button.qml. Add a focused production-QML regression for complete long sidebar text, then rerun native long-label checks. QA translator must explicitly include PlaceboColorMappingDialog; its name does not contain SettingsDialog.

Layout fixes are now implemented in 17eb2c1 and 8c11da7, merged at a9ff0d5. All three Settings sidebars wrap long names and scroll on focus; Controllers/Config headings and the Color Tricubic checkbox wrap are present. The full production-QML suite passes 521 checks with zero failures. Evidence: ../layout-screen-check.txt. Native final long-label acceptance remains pending after Search integration.

Ticket 12 is resolved at e0e33b8. Final acceptance is assigned to native_final_prep using its prepared acceptance worktree, with the original console files preserved. The full combined screen suite passes 562 checks; focused final corrections pass. The final native lifecycle passes, and the full measured-DPR matrix is running on the fixed final binary.

## Answer

Resolved at `e0e33b830d0f5f7f8eb5101657b16845e1f4d9ba`. The main checkout is on `codex/settings-refresh-integration`. All 12 original console-library files remain byte-identical and unstaged; qml.qrc includes both ConsoleButton.qml and SettingsSearch.js.

The full combined production-QML suite passes 562 checks with no failures, skips, or QML warnings. The final lifecycle and shared-scroll changes pass their focused suites. The final inventory comparison found no changed linked-page choices or numeric limits. The two review axes have no unresolved source findings.

Final native checks pass both supported sizes at measured 100%, 150%, and 200% scale. The full translated matrix passes without elided labels. All 290 final captures agree with their state records. Actual persistence, profile switching, console actions, native file cancellation, and return focus are covered by the bounded native checks.

The normal executable builds and its startup help check passes after removal of test instrumentation. No QA helper, test fixture, prototype, or sample-control mode ships. See [native acceptance and executable](../native-acceptance.md), [production manifest](../production-build.json), [full screen log](../final-combined-screen-check.txt), [final delta logs](../final-delta-checks/), and [review](../final-review.md).

Physical controller and live-stream checks were unavailable. Real HDR, authentication, accepted registration/import/export, native profile deletion, external documentation/clipboard actions, and global OS display-scale changes remain untested. The detailed isolation and launch limits are in the acceptance report.
