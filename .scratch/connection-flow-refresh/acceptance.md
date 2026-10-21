# Integrated connection and PIN acceptance

Date: 2026-10-21. Production source: `125e9284`.
Integration base: `5b40ba0c`. Review base: `a0f9dbf0`.

The integrated native app passed controlled acceptance. The checks used production Main, connection views, PIN controls, and fully constructed StreamSession objects. Native event handlers controlled progress and cleanup. The checks disabled discovery and intercepted transport, token refresh, wake, and browser-open calls. Each process used a fresh explicit INI directory with synthetic consoles. No real console, account, or user settings were used.

| Check | Actual result |
| --- | --- |
| Full Qt Quick runner after sidebar fix | 711 passed, 0 failed, 0 skipped |
| Focused Qt Quick runner after review fixes | Local 12, PSN 11, wake 13, saved PIN 24, live PIN 23. All 83 passed |
| Final native PSN contracts | 64 assertions passed |
| Final native wake contracts | 40 assertions passed |
| Final native live PIN contracts | 89 assertions passed with visible separate windows |
| Visible OpenGL, measured DPR 1 | 623 assertions passed |
| Visible OpenGL, measured DPR 1.5 | 388 assertions passed |
| Visible OpenGL, measured DPR 2 | 388 assertions passed |
| Actual Vulkan inline PIN | Renderer 0, 78 assertions passed |
| Saved-PIN failure and automatic-registration result | 90 assertions passed |
| Authentication cleanup regression | Red reproduced, green 8 assertions passed |
| Explicit-credential CLI retry regression | Red reproduced, green 14 assertions passed |

The full Qt Quick result precedes the review fixes. The focused Qt Quick and native results include those fixes. The normal build contains the final reviewed sources.

The native checks cover local connecting, video waiting, readiness, failure, retry, cancellation, and cleanup. PSN checks cover internet, initiating, linking, registration, data, video waiting, errors, and readiness with video disabled. Wake checks cover startup waiting, manual wake, timeout, discovery handoff, actual PSN fallback dispatch, and suppression of old events. Saved PIN checks cover form, confirmed save, and invalidated-target failure. Live PIN checks cover saved-PIN rejection, correction, submission waiting, acceptance, cancellation, and cancelled results.

Cross-route checks include pending PIN entry during PSN-to-StreamView handoff, exact field continuity, balanced input capture, duplicate actions, reordered or removed targets, profile changes, old token and session events, and cleanup-gated retry. Cancellation preserves the auto-connect preference and sends no sleep request. The active stream menu opens after PIN acceptance.

Both 900 by 600 and 1280 by 720 logical client sizes passed at measured DPR 1, 1.5, and 2. Expanded translated labels, long synthetic names, long error details, scroll access, and focused actions passed. The separate PIN client is 720 by 560, centered on its host. Host movement, resize, and movement to the second screen preserve the request, masked text, focus, and input ownership. Both screens have the same measured DPR within each process. A live move between different DPR values remains unverified.

The retained [capture manifest](evidence/ticket06-capture-manifest.json) lists 30 representative images, requested and measured geometry, DPR, focus bounds, window ownership, phases, capture methods, and hashes. Main OpenGL images use its real render-control framebuffer. Separate PIN images use QQuickWindow.grabWindow. Vulkan images use the composed desktop region of the active QA client. The native check verifies a pixel change between driven PIN states. Cached Vulkan window grabs and the obscured first registration result are excluded from visual proof. Corrected result and Vulkan captures show the actual states.

Native mouse and keyboard events reach each control's real owner window. Checks cover Tab, Shift+Tab, Enter, Escape, Show, immediate streamer-mode masking, window close, host close, and valid return focus. Production settings, display, placebo, color, and menu commands cannot act through the pending PIN prompt. Root's separate OS check confirms masked typing, Tab navigation, Escape, the cancelled result, and mouse Back to the Library. The [OS input record](evidence/ticket06-os-input.json) describes its exact scope.

Post-exit checks read the actual INI files. Only the selected synthetic record receives the saved PIN. Live entry leaves stored PINs unchanged. Peer records, manual records, auto-connect choice, and other profiles remain unchanged. Runtime logs and JSON evidence contain no PIN values. Captures use empty or masked fields. The PIN submission spy remains in memory. Raw INI files are not published as evidence.

Acceptance found two presentation defects. The saved-PIN sidebar now uses the existing wrapping button. The color-settings route now applies the existing live-PIN guard. Review also found two recovery defects. Authentication remains open after token Finished, and explicit CLI credentials can retry without a saved console. Originally stored targets and profile ownership still receive validation. Both reconnect timers now use one private handler with the existing cleanup and scoped logging.

Root completed independent Standards and Spec reviews of `125e9284`. Both reviews report zero unresolved findings. The initial duplicated timer handler and both recovery findings are resolved.

The [normal production manifest](evidence/ticket06-normal-production-manifest.json) records all 178 source hashes, resource inputs, link hash, and executable hash. All production hashes match the native acceptance source manifest. The normal resources and executable contain no QA hooks, synthetic data, fixture, prototype, or capture files. Root independently confirmed the source hashes and parsed the 38 QML resource entries.

Normal executable: `C:/Users/david/.cache/chiaki-ui-native/connection-flow-final-bin/chiaki.exe`.

SHA256: `f63cb93305bbe58972e0deaa59cb1ab306c3e1a0641fcb6192244817f232219f`.

`Launch-chiaki.cmd` supplies the existing runtime paths. The normal app was not launched. The [cache restoration record](evidence/ticket06-cache-restoration.json) confirms all 178 original hashes, the original output configuration, and normal app and screen-test target rebuilds. Cache ownership is released.

The complete QA source, preparation and launch scripts, post-exit checker, restoration scripts, patch, and original backups remain in `C:/Users/david/.cache/chiaki-ui-native/connection-acceptance-prep01/`. The combined helper has no dependency on an implementation worktree. Run records remain in the adjacent `connection-acceptance-*` directories. A copy of the native assertion source is retained with the evidence.

Real PSN authentication, physical console wake-up, hardware PIN acceptance, decoded video or stream transport, a physical controller, and a live move between different monitor DPR values remain unverified. Controlled native events and a normal build do not prove those hardware paths.
