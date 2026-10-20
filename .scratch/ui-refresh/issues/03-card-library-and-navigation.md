# 03: Make B the native main console screen

**What to build:** The responsive card library and navigation from B, using live host data and preserving the application's existing main-screen commands.

**Blocked by:** 02: Preserve connection behavior for every host type.

**Status:** resolved

**Spec stories:** 1, 2, 9, 13, 14, 15, 21, 22.

- [ ] Show B's sidebar, Consoles heading, Add console control, artwork, names, status, connection routes, and per-card primary actions with the approved functional text.
- [ ] Arrange visible cards in a responsive grid with vertical scrolling and no holes for hidden entries. Keep long names readable or expose the full name accessibly.
- [ ] Keep Settings and Add console accessible at normal and narrow widths. Open their existing native screens and dialogs.
- [ ] Retain automatic local discovery and its toggle. Show Refresh PSN consoles only under the existing authentication condition, using the current PSN refresh handler.
- [ ] Keep supported Steam shortcut creation, version information, and the quit path accessible. Preserve startup reminders, automatic connection, and PSN/stream navigation.
- [ ] Keep existing PIN, wake, hide, and delete access until tickets 05 and 06 provide their replacement. Remove the old console list only when every action remains reachable.
- [ ] Demonstrate the real main screen at 1280 by 720 and 800 by 600 logical pixels. Essential controls remain visible or reachable through scrolling.
- [ ] Screen tests verify conditional PSN/Steam controls, Settings, Add console, and discovery dispatch. Basic keyboard focus and existing controller access remain usable.

**Scope limit:** No sample-state switcher, browser code, new theme framework, or Settings content redesign ships with this screen. Search and final empty states follow in ticket 04.

**Specification:** [Main console screen: card library](../spec.md).

**Design reference:** B after text cleanup, preserved at `codex/prototype-console-b`, commit `73051300680f89e5c0f6dc3db71c7b581b8176e0`.

## Answer

Implemented in `eacccdc`, merged as `d34a667`. The production ConsoleLibrary
now provides B's full sidebar and responsive card screen. MainView keeps
startup behavior and connects screen commands to the existing handlers.
Temporary management access remains available for later details work.

The Qt 6.9.3 screen suite reports 61 checks passed. CTest passed again after
the merge. The integration agent inspected real QML captures at 1280 by 720
and 800 by 600. Artwork contrast and primary backgrounds were corrected.
The current screen and card API are in `gui/tests/README.md`.

Full-app verification follows in the native cache checkout. Search and final
empty states are ticket 04. Details are ticket 05. PIN submission still needs
ticket 06's delayed target check; full input checks remain in ticket 07.
