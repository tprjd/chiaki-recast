# Settings implementation map

Integration branch: `codex/settings-refresh-integration`
Base commit: `43a839d8c528c574f6e2e2e7ede961c253cf3c3b`
Delivered checkout: `C:/Users/david/Projects/chiaki-ng-ui`

Final commit: `e0e33b830d0f5f7f8eb5101657b16845e1f4d9ba`

Delivery is complete. All 15 managed worktrees are archived. The main checkout is the only remaining Git worktree. Original console-library files remain unchanged and unstaged.

The supported minimum width is 900 pixels. Native acceptance sizes are 1280 by 720 and 900 by 600. Earlier smaller-width captures are historical evidence.

| Ticket | State | Decision and evidence |
| --- | --- | --- |
| [01 General](issues/01-sidebar-general.md) | Resolved | Sidebar and General merged at 82e1258; screen and native checks pass. |
| [02 Video/Display](issues/02-video-display.md) | Resolved | Merged at fee3915; native navigation, focus and no-write checks pass at both sizes. |
| [03 Stream](issues/03-stream-quality.md) | Resolved | Merged at 57ed9a1; route isolation, automatic bitrate and native checks pass. |
| [04 Audio/Wifi](issues/04-audio-wifi.md) | Resolved | Merged at 5dfcf8f; device-list guards, numeric conversions and native checks pass. |
| [05 Consoles](issues/05-console-management.md) | Resolved | Console actions pass against isolated native records. Native Unhide byte-count fix merged at 1434d6a. |
| [06 Keys](issues/06-keyboard-mapping.md) | Resolved | Merged at b172724; all 26 mappings retained. Native capture, cancellation, persistence and reset pass. |
| [07 Controllers](issues/07-controller-settings.md) | Resolved | Merged at f1723ab; screen and native checks pass. Physical controller unavailable. |
| [08 Remote](issues/08-remote-settings.md) | Resolved | Merged at 8b9042b; 31 screen checks and native page checks pass. Real authentication untested. |
| [09 Config/Profiles](issues/09-config-profiles.md) | Resolved | Merged at ba83d9d; screen checks, native profile persistence and file cancellation pass. |
| [10 Renderer](issues/10-renderer-settings.md) | Resolved | All 39 controls/actions retained. Native persistence, file cancellation and focused values pass at 9f857a. |
| [11 Color Mapping](issues/11-color-mapping.md) | Resolved | Merged at 8d40cce; 37 screen checks and native five-group, condition, persistence and return-focus checks pass. |
| [12 Search](issues/12-settings-search.md) | Resolved | Merged at e0e33b8. Full search, privacy, native routes, immediate Escape and corrected keyboard lifecycle pass. |
| [13 Native acceptance](issues/13-native-acceptance.md) | Resolved | Final source e0e33b8. 562 combined screen checks and focused corrections pass; native scale/translation checks and normal build pass. See [acceptance](native-acceptance.md). |

## Preservation and baseline

During implementation, the original checkout stayed on `codex/ui-refresh-integration`. It now uses the completed Settings integration branch. Its 12 original console-library files remain byte-identical and unstaged; its ConsoleButton resource entry is preserved beside SettingsSearch. Those edits remain outside ticket commits. Original console screen tests passed 206 checks with Qt 6.9.3. Log: `C:/Users/david/AppData/Local/Temp/settings-refresh-console-baseline.txt`.

Native evidence is under [native-previews](native-previews/). The action check `05-06-actions-fixed-dpr1-checks.json` verifies the Unhide correction and native key persistence. All native test data is isolated from user settings.
