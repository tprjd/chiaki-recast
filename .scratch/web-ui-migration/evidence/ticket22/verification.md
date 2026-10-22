# Setup and native session recovery

Ticket 22 is complete for controlled software checks. Production source is
`47de4990`. Integration `41c6134c` is merged. The final correction changes only
the real WebChannel test helper and its identical console-request adapter.

Setup renderer failure now retires the old transport and restores a fresh
restricted page/channel. Hidden failure waits for native cleanup. The same
native bridge retains allowed route/search/category state, opaque selection,
and reminder history. Startup reads snapshots without replaying native work.
DOM focus waits for the first snapshot and DOM update.

Page events invalidate capture, mapping, Steam forms, and web file actions.
Native file actions retain their own lifetime. Local resume keeps its target;
removed registrations cannot become direct connections with old credentials.
Profile changes, new attempts, and accepted cancellation clear obsolete resume
intent. Native media, renderer preparation, input ownership, and PSN policy stay
native. The off-record profile, CSP, allowlist, and separate sign-in remain.

## Focused results

- Native local/PSN recovery: 4 passed, 0 failed, 4748 ms. Real C producers are joined. Production decoding/frame delivery continues across actual owned-renderer termination. Local buffer counts release 63/63 and 12/12; PSN releases 6/6 and 9/9. Geometry, resource preparation, obsolete attempts, removed target, and profile wake pass.
- Final reload/actual renderer failure/route/search/selection/DOM focus plus media caller: 5 passed, 0 failed, 9956 ms.
- Web file invalidation/native file preservation: 4 passed, 0 failed, 1106 ms.
- All eight affected settings, mapping, keyboard, reminder, and Steam cases pass. The same group contains two superseded recovery timing/focus failures.
- Final temporary real-channel/console-adapter media check: 3 passed, 0 failed, 8069 ms. No callback errors remain.
- Full application/harness builds pass. Svelte check has zero errors/warnings.

The native crash fixture checks exact current renderer PID, test-parent
ownership, process image, and queried path before terminating that handle.
Hidden PID 39468 belongs to test 45048; final visible PID 29736 belongs to test
44000. Actual termination is observed. No fake signal or completed DOM state is
injected. Power checks invoke only the owned Windows adapter. No OS sleep or
global broadcast occurs. External native boundaries are controlled before
construction. No real console, authentication, or LAN traffic is used.

Baseline active/frozen/resumed process private memory is
528.27/528.27/409.95 MiB; working set is 329.79/329.93/333.54 MiB.
CPU samples are 62.5/15.625/31.25 ms over 1007/1008/1008 ms. The application and
all owned browser descendants are counted. This includes native Qt overhead
and proves no dedicated GPU budget or long-term trend. No optional page
destruction was justified. Healthy setup remains hidden/frozen.

Initial reds retain lost search, failed renderer replacement, and unwanted
resume after registration removal. Earlier fixture corrections retain the
wrong selected-filename observation, fresh-discovery selection, a non-invokable
reconnect function, and temporary-client reply collisions. Final checks use the
actual owned temporary file path, current discovery, the unique owned timer,
acknowledged native view state, snapshot/DOM focus, and disjoint helper reply IDs.

Exact commands, logs, full source identities/diffs, failures, build output, and
source-equivalence records remain under
`C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket22-*`.
`ticket22-recorded-evidence/verification.md` contains the detailed record.
No binaries, crash dumps, vendor trees, or duplicate inventories are committed.

Application SHA256: `659e86b11ae508c247ea0c01f8fa1356abf984d1ad5627bc882b3af098a08ed4`.
Final harness SHA256: `dafc8c3c20d28d8943de06a9d6de288f9fcd2307c2bb61542ffae3d10efd6479`.
Qt 6.9.3 MSVC x64, clang-cl 19.1.7, FFmpeg 7.1, libplacebo 7.360.1.
Normal Windows/native runs have empty Chromium flags. Offscreen runs use the
test-only GPU-disable flag and make no graphics claim.

Production C++/native QML match native-green identity. Later frontend changes
are covered by final focused checks. File/bridge guards match file-green
identity. Reuse [ticket 04](../ticket04/verification.md) and unchanged later
contracts for cleanup, direct exit, errors, geometry, workers, and input owners.

Physical focus/held devices, real OS power restore, live consoles, Chromium
acceleration under resource measurement, default/other presets, long resource
trends, performance, packages, and platforms remain in ticket 23. HighQuality
remains an independent open failure. Production presets/compute are unchanged.
No full suite or graphics matrix was repeated.
