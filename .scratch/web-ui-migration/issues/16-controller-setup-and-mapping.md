# 16: Configure controllers and their mappings

**What to build:** Users can inspect available controllers, change supported controller options, and complete the existing native mapping flow from web setup.

**Blocked by:** 11: Open settings and change general session options.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 50, 51, 52, 59, 63.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] Controller availability and device type update through native services, including connection and removal during setup.
- [x] Existing controller options and primary and alternate mappings retain their native values and supported conditions.
- [x] Native capture, selection, reset, cancel, apply, and mapping conflict behavior are preserved.
- [x] Mapping input is routed once and does not also activate setup controls or reach a streaming console.
- [x] Profile changes and device loss invalidate only the relevant pending mapping operation and show a recoverable outcome.
- [x] Embedded tests verify native mapping results, reset and cancel, device removal, and controller navigation after the mapping flow.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Comments

- Claimed after general settings. Prepare the isolated worktree while pairing, PSN connection, and color controls are implemented. Reuse the native controller service and mapping flow; preserve stream input ownership.

## Answer

The Controllers category now shows connected native devices and all existing
controller options. The mapping dialog shows all 23 primary and alternate
mapping targets. Native services still own selection, capture, normalization,
conflict movement, apply, reset, and cancel. The page receives display data and
restricted commands with an opaque operation and profile generation.

Native controller Options, Return, and D-pad input operate the real embedded
page. Capture input does not also activate setup controls. Profile changes,
page reload, setup hide, selected-device removal, and native window attachment
end the operation. Removal of another controller preserves it. Old commands
cannot apply a newer operation. Connect and retry are refused during mapping.

The compiled Svelte, WebEngine, WebChannel, production bridge, native services,
and SDL virtual-device checks pass. Settings and mapping pass 4/0. The final
mapping case passes 3/0. Three affected QML cases pass 5/0. These counts include
initialization and cleanup. The frontend check reports no errors or warnings.
The setup-only mapping QML dialog, resource, routes, adapters, and replaced tests
are removed. Shared live stream QML and native input algorithms remain.

[Verification](../evidence/ticket16/verification.md) records source and executable
hashes, red failures, corrections, commands, and limits. The inspected normal
1280-pixel capture is usable. The 200 percent DOM check retains 1280 CSS pixels,
but its widget grab has stale backing pixels and is invalid visual evidence.
Ticket 23 retains zoom visual, physical controller, haptics, focus, live-console,
GPU, platform, and latency acceptance.
