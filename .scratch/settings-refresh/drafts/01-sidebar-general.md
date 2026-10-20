# 01: Open Settings in the sidebar layout and edit General

Publication: draft; awaiting breakdown approval

**What to build:** Open native Settings from the console library in layout A.
Use the sidebar to reach every category and edit all General settings.

**Blocked by:** None (can start immediately).

- [ ] Introduce only the minimal Settings UI isolation needed before changing the layout. Reuse the existing native controls, settings handlers, and Qt Quick harness.
- [ ] The native Settings entry point shows the application navigation, current profile, category sidebar, selected category, and grouped General form.
- [ ] All nine categories remain reachable. Categories not yet migrated retain their working existing forms and actions.
- [ ] General includes disconnect and suspend actions, audio/video modes, supported Steam Deck controls, streamer mode, log access, and all four stream-menu shortcut choices.
- [ ] Native conditions, defaults, validation, update timing, and storage remain intact. Opening or navigating Settings causes no writes.
- [ ] Back returns to the intended console and restores library focus. Keyboard and controller navigation work; narrow layouts keep categories and controls reachable.
- [ ] Production-QML tests use fake settings and action handlers to verify General writes, conditions, navigation, and no unintended side effects.
- [ ] Build and demonstrate the native General page at normal and narrow sizes. Preserve the existing console-library corrections.

