# 11: Open settings and change general session options

**What to build:** Users can open the settings sidebar, search implemented controls, and save general and session-behavior settings through native services.

**Blocked by:** 03: Show live consoles in the GPU-accelerated web UI.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 48, 50, 51, 52.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] The selected settings sidebar and shared field components display native values, labels, units, defaults, and capability conditions.
- [x] General and session-behavior settings retain their validated ranges, enums, native side effects, and restart requirements. This ticket owns the shared settings shell, search, and general session controls. Video profiles, audio, display, renderer, color mapping, input, profiles, and file actions belong to their respective tickets.
- [x] Search routes to the implemented category or control and preserves appropriate focus. Later settings groups can join the same search behavior.
- [x] Native state remains authoritative. Rejected writes do not appear saved, and changes survive page reload and application restart.
- [x] Platform-specific controls and privacy masking retain their existing conditions.
- [x] Embedded tests change settings through the real bridge, inspect native persisted values, and verify invalid input and reload behavior.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Answer

Added the settings entry and category sidebar with reusable enum and boolean
fields. Search uses public metadata and restores focus to the control, or to
the required enable control when a shortcut field is hidden. The shared
`data-setup-panel` marker isolates panel keys from console navigation.

The native bridge publishes only the allowed general values. Its command
checks the profile generation, property name, type, and enum range before it
calls the existing QmlSettings setter. The page waits for native values and
shows rejected writes without a false saved value. Deck fields retain their
native compile conditions. Streamer mode retains native masking.

Focused embedded checks passed save/reload 3/0 and validation/restart 4/0.
The restart check reads the setting in a new executable process. Frontend
checks passed with zero errors and warnings. See the [verification record](../evidence/ticket11/verification.md)
for source hashes, commands, results, reused evidence, and hardware limits.

No QML resource was removed because SettingsDialog still has real callers.
Native live settings remain native. Other settings categories remain assigned
to tickets 12 through 17. The current integration tip was already merged.

Integration merge `322c486b` contains worker commit `ceae3eb7` without conflicts.
The merged tree matches the worker tree. All 14 recorded source, lock,
executable, JavaScript, and CSS hashes match. The shared panel marker and the
settings route guard remain in place. Passing results were reused without
builds or test reruns. Source-only whitespace checks pass.

This resolves the settings shell and general session control milestone.
The offscreen checks used a test-only GPU-disable flag. They establish behavior
and persistence, but do not establish physical focus, visual layout, Deck
hardware, live session effects, other platform packages, or GPU acceptance.
Ticket 03 graphics evidence remains unchanged. These limits remain in the
relevant hardware and final acceptance tickets.
