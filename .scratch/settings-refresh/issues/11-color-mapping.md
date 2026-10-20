# 11: Configure complete gamut and tone mapping

Status: resolved

**What to build:** Open Color Mapping from Renderer and configure its complete
gamut and tone-mapping options.

**Blocked by:** 10: Configure every custom renderer setting.

- [x] Retain all 28 controls across the two gamut-mapping and three tone-mapping groups.
- [x] Preserve complete function choices, LUT controls, defaults, numeric precision, bounds, steps, and function-dependent conditions.
- [x] Selecting a mapping function updates applicable controls without changing unrelated values or creating unintended writes.
- [x] Back returns to Renderer with focus restored. Existing stream-time entry points retain their behavior.
- [x] Production-QML tests verify all setting destinations, function-dependent fields, numeric boundaries, navigation, and no writes on page construction.
- [x] Build and demonstrate the native page at normal and narrow sizes, including each conditional function group.


## Answer

Implemented in 0fc0a19 and merged at 8d40cce. All 28 controls, conditions, options and numeric limits are retained. The production-QML suite passes 37 checks, including all write destinations, every gamut/tone function condition, no-write construction, profile updates, focus and nested return. Native 11-color-mapping-dpr1-checks.json passes all five groups at 1280 by 720 and 900 by 600, paired value bounds, representative condition changes, no-write browsing, actual numeric persistence and Settings-to-Renderer-to-Color return focus. Actual stream-time entry points remain untested without console hardware.
