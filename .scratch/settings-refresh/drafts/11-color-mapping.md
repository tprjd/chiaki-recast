# 11: Configure complete gamut and tone mapping

Publication: draft; awaiting breakdown approval

**What to build:** Open Color Mapping from Renderer and configure its complete
gamut and tone-mapping options.

**Blocked by:** 10: Configure every custom renderer setting.

- [ ] Retain all 28 controls across the two gamut-mapping and three tone-mapping groups.
- [ ] Preserve complete function choices, LUT controls, defaults, numeric precision, bounds, steps, and function-dependent conditions.
- [ ] Selecting a mapping function updates applicable controls without changing unrelated values or creating unintended writes.
- [ ] Back returns to Renderer with focus restored. Existing stream-time entry points retain their behavior.
- [ ] Production-QML tests verify all setting destinations, function-dependent fields, numeric boundaries, navigation, and no writes on page construction.
- [ ] Build and demonstrate the native page at normal and narrow sizes, including each conditional function group.

