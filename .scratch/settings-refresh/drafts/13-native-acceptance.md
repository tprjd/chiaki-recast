# 13: Verify the complete native Settings experience

Publication: draft; awaiting breakdown approval

**What to build:** Deliver one integrated native Settings implementation that
matches layout A, retains the complete settings inventory, and works through
the application's real entry points.

**Blocked by:** 12: Find settings across every category and linked page.

- [ ] Reconcile all categories and linked pages against the native control inventory, including read-only data, dynamic lists, actions, conditions, defaults, options, and conversions.
- [ ] Remove any obsolete Settings-only transition UI after all replacement routes work. Keep unrelated screens and the earlier console-library corrections intact.
- [ ] Run the relevant production-QML screen suites, including existing console-library tests. Resolve Settings integration regressions.
- [ ] Build and launch the native Windows app; compare layout A at 1280 by 720, 800 by 600, and the supported minimum size.
- [ ] Check 100%, 150%, and 200% scaling, long translated labels, scrolling, focus, keyboard/controller navigation, popup priority, and reachable actions.
- [ ] Verify persistence, profile switching, library return, and native file-flow cancellation using isolated settings and temporary data.
- [ ] Check physical controller capture and stream-time display/renderer entry points where hardware is available. Record unavailable cases as untested.
- [ ] Confirm no fixtures, fake handlers, prototype controls, sample values, or unsupported-condition review mode ship in the application.
- [ ] Record the actual executable, source revision, test results, visual evidence, and remaining limitations. Browser checks and a successful build alone are not acceptance.
