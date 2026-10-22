# 03: Show live consoles in the GPU-accelerated web UI

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Normal startup opens a packaged Svelte setup window that uses GPU acceleration and displays real discovered consoles through Qt WebChannel.

**Blocked by:** 01: Preserve native playback with application-owned services.

**Status:** needs-info

**Spec stories:** 1, 2, 5, 6, 7, 8, 9, 11, 35, 36, 62, 63, 64, 65, 66, 67, 68, 69, 70, 90, 91.

- [ ] Normal startup loads packaged Svelte 5 and TypeScript output in QWebEngineView. The GUI build includes WebEngineWidgets, WebChannel, and the frontend build dependency.
- [ ] The unconditional startup override that disables WebEngine GPU acceleration is removed from the web setup path. Actual Chromium acceleration is verified in the embedded application.
- [ ] One restricted production SetupBridge supplies current console state, capabilities, and discovery control through real WebChannel transport. Backend QObjects and secrets are not published.
- [ ] Console keys are native-issued and profile-scoped. Live updates preserve selection, and missing or ambiguous targets are rejected.
- [ ] The selected card library design uses shared CSS values and real artwork. Search, empty states, long names, and discovery changes display correctly.
- [ ] Packaged assets, scripts, styles, and fonts load offline without a runtime Node server. Toolchain versions and the package lock are pinned.
- [ ] The trusted setup page blocks external navigation and unexpected resources, applies its content policy, and treats console names as text. Streamer mode masks private values.
- [ ] Keyboard, native controller navigation, desktop touch, visible focus, labels, reduced motion, scaling, and the specified window sizes work. Translation support is established for migrated text.
- [ ] An embedded application test observes real native discovery updates and a discovery command. Normal startup selects the web UI without adding a QML setup fallback.
