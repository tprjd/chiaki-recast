# 06: Edit the complete keyboard and mouse controls

Status: resolved

**What to build:** Use Keys to enable keyboard and mouse input, capture all native
key assignments, and restore the default key mapping.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [x] Include all 26 native mappings, keyboard-as-controller, mouse touchpad, and Reset All Keys.
- [x] Reuse native key capture and persistence. The prototype's editable key-name text fields do not replace capture.
- [x] Capture, cancel, close, and reset affect only the intended mapping state and restore usable focus.
- [x] Capture keystrokes do not also navigate, close Settings, or trigger the console library.
- [x] Profile changes refresh assignments; opening or browsing the mapping page causes no writes.
- [x] Production-QML tests verify complete mapping coverage, target and key dispatch, cancellation, reset, and input isolation.
- [x] Build and demonstrate the complete native mapping list with keyboard navigation and scrolling at narrow sizes.



## Answer

Implemented in d2083e1, merged as b172724. All 42 Keys screen checks pass. The native 26-mapping list, scrolling and focus bounds pass at 1280 by 720 and 900 by 600. The final focused mapping was inspected in the actual native framebuffer. Native Escape cancellation, F7 assignment, persisted key value, reset and focus restoration pass using disposable settings. Evidence: ../native-previews/03-06-input-dpr1-checks.json and ../native-previews/05-06-actions-fixed-dpr1-checks.json.
