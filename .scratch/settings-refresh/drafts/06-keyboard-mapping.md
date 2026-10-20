# 06: Edit the complete keyboard and mouse controls

Publication: draft; awaiting breakdown approval

**What to build:** Use Keys to enable keyboard and mouse input, capture all native
key assignments, and restore the default key mapping.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [ ] Include all 26 native mappings, keyboard-as-controller, mouse touchpad, and Reset All Keys.
- [ ] Reuse native key capture and persistence. The prototype's editable key-name text fields do not replace capture.
- [ ] Capture, cancel, close, and reset affect only the intended mapping state and restore usable focus.
- [ ] Capture keystrokes do not also navigate, close Settings, or trigger the console library.
- [ ] Profile changes refresh assignments; opening or browsing the mapping page causes no writes.
- [ ] Production-QML tests verify complete mapping coverage, target and key dispatch, cancellation, reset, and input isolation.
- [ ] Build and demonstrate the complete native mapping list with keyboard navigation and scrolling at narrow sizes.

