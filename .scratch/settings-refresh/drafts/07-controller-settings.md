# 07: Configure controllers and edit their mappings

Publication: draft; awaiting breakdown approval

**What to build:** Use Controllers to configure input and haptics, select the
intended controller, and edit or reset its complete mapping.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [ ] Include mapping change/reset, background events, button-position interpretation, D-pad emulation and increment, all four shortcut choices, rumble, and true-haptics intensity.
- [ ] The linked mapping page retains all 23 targets, primary/alternate inputs, analog-stick and L2/R2 eligibility, apply, cancel, and reset behavior.
- [ ] Controller selection and device changes cannot redirect a mapping change to another controller. Preserve Steam-managed-controller handling.
- [ ] Preserve conditions, units, bounds, steps, defaults, and native apply timing.
- [ ] Capture input does not also navigate or trigger background actions; closing capture or the child page restores focus.
- [ ] Production-QML tests verify property writes, complete mapping coverage, target selection, conditions, cancellation, reset, apply, and input isolation.
- [ ] Build and demonstrate the native pages. Use a physical controller where available and record unavailable hardware checks.

