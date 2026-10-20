# 07: Configure controllers and edit their mappings

Status: resolved

**What to build:** Use Controllers to configure input and haptics, select the
intended controller, and edit or reset its complete mapping.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [x] Include mapping change/reset, background events, button-position interpretation, D-pad emulation and increment, all four shortcut choices, rumble, and true-haptics intensity.
- [x] The linked mapping page retains all 23 targets, primary/alternate inputs, analog-stick and L2/R2 eligibility, apply, cancel, and reset behavior.
- [x] Controller selection and device changes cannot redirect a mapping change to another controller. Preserve Steam-managed-controller handling.
- [x] Preserve conditions, units, bounds, steps, defaults, and native apply timing.
- [x] Capture input does not also navigate or trigger background actions; closing capture or the child page restores focus.
- [x] Production-QML tests verify property writes, complete mapping coverage, target selection, conditions, cancellation, reset, apply, and input isolation.
- [x] Build and demonstrate the native pages. Use a physical controller where available and record unavailable hardware checks.



## Answer

Implemented in 0685cb0 and merged at f1723ab. Controllers and Mapping suites pass, including all 23 targets and both input slots, late-event cancellation, device/profile changes, analog/apply/reset behavior and real StackView focus return. Native Controllers navigation, selection cancellation, focus bounds and no-write checks pass at 1280 by 720 and 900 by 600. The native 900-pixel page was inspected. Evidence: ../native-previews/07-09-management-dpr1-checks.json. No physical controllers were available; actual hardware mapping capture remains untested.
