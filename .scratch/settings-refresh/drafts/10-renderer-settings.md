# 10: Configure every custom renderer setting

Publication: draft; awaiting breakdown approval

**What to build:** Open Custom Renderer from Video and edit all renderer groups
in the selected form style while preserving native preset behavior.

**Blocked by:** 02: Edit Video and linked Display settings.

- [ ] Retain all 39 visible controls and actions across Config, Scaling, Debanding, Sigmoidization, Color, Peak Detection, and Color Mapping.
- [ ] Keep complete option lists, defaults, bounds, precision, steps, preset effects, and conditional visibility.
- [ ] Preserve renderer documentation, URL-copy actions, and renderer import/export. Keep the permanently hidden URL field hidden.
- [ ] Color Mapping opens its existing working page until ticket 11 migrates that page.
- [ ] Back returns to the correct caller with focus restored. Existing stream-time entry points and separate-window behavior continue to work.
- [ ] Production-QML tests verify every write or action destination, presets and conditions, numeric boundaries, cancellation, and no writes on page construction.
- [ ] Build and demonstrate the native renderer page at normal and narrow sizes. Record real stream-time checks or hardware limitations.

