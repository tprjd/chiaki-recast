# Shared stream interface baseline

The supplied-state test boundary is the complete production `StreamView`.
`gui/tests/README.md` describes its properties, observations, and entry points.
The old menu presentation is retained. The toolbar design belongs to ticket 02.

## Results

- `screen-summary.json` records eight passing production QML suites on Qt 6.11.2.
  The stream suite has 37 passing checks. It has no QML warnings.
- `native-vulkan.json` records 327 passing controlled native checks.
- `native-opengl.json` records 347 passing controlled native checks.
- Each native route opens the real menu, checks its initial focus and native
  session input capture, and uses Return to enter the existing disconnect path.
  Both logical sizes, 900 by 600 and 1280 by 720, cover Cancel, Escape, native
  close, Disconnect, and Disconnect and sleep. Actual native stop and sleep
  handlers are intercepted before transport. The preference stays unchanged.
- The measured native DPR is 1.25. Native capture records refer to dialog render
  targets, not desktop composition or the menu's final visual acceptance.
- `normal-final-build.txt` records the normal production `chiaki` build.
  `normal-checks.json` verifies all staged inputs against this worktree and
  excludes test fixtures, prototype resources, and native QA hooks.
  The normal executable also exits successfully with `--help` before settings
  construction. `normal-source.json` records its source hashes.
- `native-source.json` records the native QA source hashes. The first Vulkan
  attempt selected a hidden window's control in the observer. The observer now
  excludes hidden separate menu windows. The subsequent native runs passed.
- `stream-red*.txt` retain failing TDD slices for the supplied-state boundary,
  native close, dialog focus, session removal, closure repeat, and repeated
  configured disconnect input. The final screen logs contain the passing state.

## Replay

Use the native cache only when no other task uses it. Build the standalone
screen runner as described in `gui/tests/README.md`. Run `run-screens.ps1` from
this directory. It uses that worktree's runner and production QML files.

For native checks, track new build inputs before staging. Use a new name for
both preparations and each run. `stage-native.py <stream-interface-prep-name>`
backs up and stages build inputs. `prepare-native.py <native-prep-name>` reuses
PSN ticket 06's INI isolation, session transport interception, and GPU readback.
It changes only the cached QA copy and adds the real stream menu entry.
Build `chiaki` with runtime output `stream-menu-qa-bin`. Run `run-native.ps1`
with `-Run <new-run-name>` and `-Renderer vulkan` or `-Renderer opengl`.

Restore the nested native preparation with PSN ticket 06's `restore-native.py`.
Refresh staged production inputs with `stage-native.py --refresh <name>`.
Build `chiaki` with runtime output `stream-interface-normal-bin`.
Restore the outer preparation with `stage-native.py --restore <name>`.
Reconfigure with the original output directory and rebuild `chiaki` and
`chiaki-console-screen-tests`. Retain `cache-restoration.json` as the hash check.

No real console, user settings, account service, or package output is used.
Physical controllers, live video and audio, real network loss, desktop
composition, fullscreen, display-scale changes, and final overlay layout
remain outside this baseline. Ticket 05 owns those acceptance checks.
