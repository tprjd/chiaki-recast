# 12: Find settings across every category and linked page

Publication: draft; awaiting breakdown approval

**What to build:** Search setting names and descriptions across the complete
Settings UI and reach the correct control with its native rules intact.

**Blocked by:** 03: Edit separate local and remote stream quality; 04: Configure
audio, microphone, and connection feedback; 05: Manage registered and hidden
consoles from Settings; 06: Edit the complete keyboard and mouse controls;
07: Configure controllers and edit their mappings; 08: Manage PSN access and
remote connection options; 09: Manage profiles, settings files, and diagnostics;
11: Configure complete gamut and tone mapping.

- [ ] Search includes every migrated category and linked settings page, with category/group context.
- [ ] Matching ignores case and surrounding whitespace. Clear, empty-query, and no-match behavior are usable.
- [ ] Selecting a category clears search and opens that category. Linked results open the intended page and restore usable focus on return.
- [ ] Search does not bypass platform, device, feature, preset, or dependent-setting conditions.
- [ ] Edits and actions reached through search retain their validation, target, conversions, and commit behavior. Searching alone causes no writes.
- [ ] Search remains transient and excludes credentials, arbitrary settings values, and log contents. Retain streamer-mode masking and accessible names.
- [ ] Production-QML tests cover results throughout the completed inventory, condition changes, profile updates, action routing, privacy, keyboard navigation, and empty states.
- [ ] Build and demonstrate search in the native application. Use existing page controls or routes rather than a second settings implementation.

