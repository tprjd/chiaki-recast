# 03: Edit separate local and remote stream quality

Status: resolved

**What to build:** Select PS4 or PS5 in Stream and edit separate Local and Remote
quality settings without changing the other console type or connection route.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [x] Resolution, frame rate, bitrate, and eligible codec controls use the selected sidebar design and complete native option lists.
- [x] All PS4/PS5 and Local/Remote combinations write only their intended settings.
- [x] PS5 codec choices never create unsupported PS4 codec settings.
- [x] Bitrate uses the correct displayed units, bounds, steps, and resolution-based automatic defaults. Opening a page does not create an explicit bitrate override.
- [x] Defaults, current values, and profile changes display correctly. Switching console type or category preserves saved values.
- [x] Production-QML tests cover the four combinations, codec conditions, bitrate conversions, automatic values, and absence of writes during navigation.
- [x] Build and demonstrate native Stream at normal and narrow widths, with every value reachable.



## Answer

Implemented in a5dce34, merged as 57ed9a1. All 35 Stream screen checks pass, including route isolation, PS4/PS5 choices, automatic bitrate and conversions. Native navigation, focus bounds and no-write checks pass at 1280 by 720 and 900 by 600. The 900-pixel native layout was inspected. Evidence: ../native-previews/03-06-input-dpr1-checks.json.
