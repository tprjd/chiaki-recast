# 10: Configure every custom renderer setting

Status: resolved

**What to build:** Open Custom Renderer from Video and edit all renderer groups
in the selected form style while preserving native preset behavior.

**Blocked by:** 02: Edit Video and linked Display settings.

- [x] Retain all 39 visible controls and actions across Config, Scaling, Debanding, Sigmoidization, Color, Peak Detection, and Color Mapping.
- [x] Keep complete option lists, defaults, bounds, precision, steps, preset effects, and conditional visibility.
- [x] Preserve renderer documentation, URL-copy actions, and renderer import/export. Keep the permanently hidden URL field hidden.
- [x] Color Mapping opens its existing working page until ticket 11 migrates that page.
- [x] Back returns to the correct caller with focus restored. Existing stream-time entry points and separate-window behavior continue to work.
- [x] Production-QML tests verify every write or action destination, presets and conditions, numeric boundaries, cancellation, and no writes on page construction.
- [x] Build and demonstrate the native renderer page at normal and narrow sizes. Record real stream-time checks or hardware limitations.


## Answer

Implemented in 9838411, with native Debanding storage-key correction a396a6a and focused-value scrolling fix 58323ff. Integration tip 9f857a includes all changes. All 39 controls/actions are retained; Renderer screen tests pass, including a red-to-green regression for focused slider value visibility. Exact option lists and numeric limits match the original inventory. Native checks pass for all seven groups at 1280 by 720 and 900 by 600, including control and value bounds, no-write browsing, nested Color Mapping return, file-dialog cancellation, preset behavior and numeric persistence. Native Debanding disable now changes only its own stored value. Evidence: ../native-previews/10-renderer-values-dpr1-checks.json and ../native-previews/10-renderer-deband-fixed-checks.json. Live stream entry points remain untested without console hardware.
