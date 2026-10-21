# 06: Verify the complete Windows experience

**What to build:** Deliver a verified Windows keyboard and mouse feature with recorded gameplay results, preserved legacy controls, and clear limits. Review the integrated implementation against the parent specification and correct release-blocking failures.

**Blocked by:** [05: Restore feature defaults safely](05-reset-feature-defaults.md), which transitively requires tickets 01 through 04 and the stream input ownership prerequisite.

Status: claimed

Parent: [Built-in keyboard and mouse controls](../spec.md)

This ticket implements Stage 4. Earlier tickets retain their own tests and completion gates.

- [x] Run the integrated automated checks against production input, ownership, settings, and session behaviour. Review the implementation against all parent-spec requirements and repository standards.
- [ ] Test real Windows capture at screen edges and across the available display arrangement. Check display scaling and mouse removal where practical. Record unavailable hardware checks as limits.
- [ ] With movement and triggers held, exercise F8, Alt+Tab, menus, dialogs, quit, disconnect, reconnect, and session replacement. Verify cursor release, neutral mapped state where deliverable, fresh-press rearming, and no automatic recapture.
- [ ] Verify fullscreen, resize, rapid clicks, side buttons, Ctrl shortcuts, and F11. Check that stale movement and competing controller sources do not enter captured output.
- [ ] Test both supported rendering paths and both stream presentation routes. Verify ownership overlap, focus return, passive capture status, and clearance from menus.
- [x] Check both settings tabs, automatic saving, search, numeric entry, conflicts, reserved keys, reset, keyboard accessibility, disabled states, long labels, and minimum window size.
- [ ] Verify existing physical-controller, keyboard, and mouse-touchpad behaviour with the new mode off. Confirm that enabling, editing, resetting, and restarting preserve legacy configuration.
- [ ] Repeat basic Astro's Playroom controls and the specified The First Descendant aiming scenarios with the integrated build. Stop external mapper capture before tests without modifying its files or installation.
- [x] Record source revision, build, Windows environment, mouse DPI and report rate where available, game settings, scenario results, and unresolved limits in an acceptance record.
- [ ] Correct and retest native capture failures, stuck output, lost settings, and unusable aiming before release. A successful build or preview alone does not pass acceptance.
- [x] Keep claims within the tested scope. Do not claim full Astro's Playroom support, universal native-PC mouse feel, offline-only use of The First Descendant, or protection from Sony detection or account action.

## Answer

The [acceptance record](../acceptance.md) identifies the tested integrated source,
primary build, preserved stage logs, final settings image, and local launcher.
At `992cc170`, the app and maintained native tests build. One selected native
invocation passes keyboard defaults, exclusive output, release and rearm,
mouse buttons, and simultaneous trigger alternatives. No broad suite was repeated.

Actual Main live settings checks already pass in OpenGL and Vulkan. Reset,
numeric validation, snapshots, search, and minimum-size evidence are preserved
from their implementation checks. Both final reviews are complete. All three
code items are fixed in `aaedee02`, with final evidence recorded in `ed4d56d7`.
The affected ownership, notice, session, and binding checks pass. One desktop
Space assertion failed before a bounded diagnostic passed. The original
failure's cause remains unproved. Both logs are preserved.
The final app-only incremental build passes at `ed4d56d7`. Its source revision,
size, and SHA-256 are in the acceptance record. No additional test suite ran.

Ticket 06 remains claimed. Physical foreground input, mouse calibration,
hardware transitions, and both required game trials remain open.
The failed foreground precondition is recorded as unavailable evidence.
No successful physical capture or gameplay result is inferred from a build,
controlled test, or screenshot.
