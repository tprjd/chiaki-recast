# Ticket 14 integration audit

The merger checked ticket 14 separately on 2026-10-22. No build or runtime
check was run during integration. The recorded passing checks are reused.

- Prior integration: `ade6e7fbb20853756a961c1ed80857c80b5c4794`.
- Worker source: `5189d42ca7d23e855131750567d87f4768c55b2c`.
- Merge: `80844b4cd72bb25fe9a188fa2c3e970dadd56209`.
- The merge had no conflicts. Its complete tree equals the worker tree.
- The local gf-complete patch is excluded. The integration worktree was clean.

The merger read the ticket Answer, verification report, identity manifest,
recorded audit, commands, and actual focused logs. All 24 normalized source
hashes, 46 evidence hashes, three current build artifact hashes, and five
prelaunch inventory identities match. Each inventory also matches its saved
diff and executable digest. The final behavior and corrected layout inventory
hashes bind the six changed GUI files to their recorded source commits.

The merger ran `ticket14-audit.py` as a source comparison only. All 28 controls
match the retained independent QML inventory. All 28 bounded comparisons
reproduce the recorded audit hashes. No runtime test was started by this script.

Only six GUI files differ from prior integration. All prior embedded test
bodies remain byte-identical. Seventeen complete files for native settings,
window lifetime, console and account services, native requests and discovery,
live QML, shared browser state, and keyboard capture remain byte-identical.
SetupBridge equals prior integration after removal of only the added color
metadata, translations, snapshot values, and guarded setters. Existing renderer
actions and file safety remain. No registration constructor was added.

The actual final behavior log reports 5 passed and 0 failed for color mapping,
Video and Audio, and Keyboard and mouse. The child persistence log reports
3 passed and 0 failed. The corrected layout log reports 3 passed and 0 failed.
The earlier group passed renderer settings, renderer layout, and validation,
but failed color mapping because the native float showed excess digits.
The final behavior log covers the numeric display fix. The frontend report
has zero errors and zero warnings. Existing search evidence is reused.

The merger inspected only `color-1280.png`, SHA256
`ac49dd52441f58dc235155577e1c7e460a66c87c31b3bd24a77739346887366f`.
Its visible labels, values, select controls, navigation, and Back button are
readable and have no horizontal clipping. Narrow captures remain historical.
No new review or acceptance work below 1024 CSS pixels was done. Supported
200 percent zoom acceptance remains in ticket 23. No native window minimum
was added.

These checks establish source preservation and reuse of recorded offscreen
results. They do not establish live playback, physical focus, GPU acceleration,
HDR output, latency, zero LAN traffic, or package acceptance.
