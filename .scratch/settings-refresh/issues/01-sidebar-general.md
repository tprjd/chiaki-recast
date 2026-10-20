# 01: Open Settings in the sidebar layout and edit General

Status: resolved

**What to build:** Open native Settings from the console library in layout A.
Use the sidebar to reach every category and edit all General settings.

**Blocked by:** None (can start immediately).

- [x] Introduce only the minimal Settings UI isolation needed before changing the layout. Reuse the existing native controls, settings handlers, and Qt Quick harness.
- [x] The native Settings entry point shows the application navigation, current profile, category sidebar, selected category, and grouped General form.
- [x] All nine categories remain reachable. Categories not yet migrated retain their working existing forms and actions.
- [x] General includes disconnect and suspend actions, audio/video modes, supported Steam Deck controls, streamer mode, log access, and all four stream-menu shortcut choices.
- [x] Native conditions, defaults, validation, update timing, and storage remain intact. Opening or navigating Settings causes no writes.
- [x] Back returns to the intended console and restores library focus. Keyboard and controller navigation work; narrow layouts keep categories and controls reachable.
- [x] Production-QML tests use fake settings and action handlers to verify General writes, conditions, navigation, and no unintended side effects.
- [x] Build and demonstrate the native General page at normal and narrow sizes. Preserve the existing console-library corrections.



## Answer

Implemented in 783fa90 and merged as 82e1258. The production-QML suite passes 207 checks, including 17 General checks. The isolated native Windows application builds and passes General navigation, no-write, Back, and capture checks at 1280 by 720, 800 by 600, and 390 by 600 at measured DPR 1. Screenshots were inspected. Evidence: ../native-previews/01-general-return-dpr1-checks.json. Physical controller input remains untested; controller key navigation is covered by the screen tests. Native return also preserves the synthetic selected console and restores the Settings opener focus. Complete scaling checks remain in ticket 13.


