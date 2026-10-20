# 03: Edit separate local and remote stream quality

Publication: draft; awaiting breakdown approval

**What to build:** Select PS4 or PS5 in Stream and edit separate Local and Remote
quality settings without changing the other console type or connection route.

**Blocked by:** 01: Open Settings in the sidebar layout and edit General.

- [ ] Resolution, frame rate, bitrate, and eligible codec controls use the selected sidebar design and complete native option lists.
- [ ] All PS4/PS5 and Local/Remote combinations write only their intended settings.
- [ ] PS5 codec choices never create unsupported PS4 codec settings.
- [ ] Bitrate uses the correct displayed units, bounds, steps, and resolution-based automatic defaults. Opening a page does not create an explicit bitrate override.
- [ ] Defaults, current values, and profile changes display correctly. Switching console type or category preserves saved values.
- [ ] Production-QML tests cover the four combinations, codec conditions, bitrate conversions, automatic values, and absence of writes during navigation.
- [ ] Build and demonstrate native Stream at normal and narrow widths, with every value reachable.

