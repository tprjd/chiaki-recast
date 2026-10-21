# 02: Play with mouse camera control and trigger buttons

**What to build:** Move the mouse to control the camera without screen-edge limits. Use left click for R2 and right click for L2, with the accepted keyboard alternatives. Play basic Astro's Playroom actions without a physical controller or external mapper.

**Blocked by:** [01: Enable gameplay capture with default keyboard controls](01-gameplay-capture-and-keyboard.md).

Status: ready-for-agent

Parent: [Built-in keyboard and mouse controls](../spec.md)

This ticket completes Stage 1. Numeric calibration and editable response controls belong to ticket 03.

- [ ] Use relative mouse input through the existing Windows, Qt, and SDL integration as appropriate. Capture permits continuous turning and release restores desktop cursor control.
- [ ] Convert movement and elapsed time into bounded right-stick output. Keep conversion independent of rendering frames and return the mapped stick exactly to centre when movement stops.
- [ ] Use the production conversion logic for deterministic timed-input tests. Do not add acceleration presets, optional smoothing controls, or a general input-plugin framework.
- [ ] Map left click to R2 and right click to L2. Preserve X and Z as their respective alternatives. An action remains held until all of its held bindings are released.
- [ ] Recognize the five specified mouse buttons. Middle and side buttons start unassigned. Do not map the wheel or let captured side-button events trigger navigation.
- [ ] Apply every release and rearming rule from ticket 01 to mouse buttons and deltas. Discard pending movement across focus, owner, session, and capture transitions.
- [ ] Prevent artificial camera movement during cursor repositioning, resize, and fullscreen changes. Release capture if a transition loses focus.
- [ ] Disable double-click fullscreen during gameplay capture while keeping F11 available. Restore the existing double-click behaviour after release.
- [ ] Verify emitted controller state for movement, idle centring, saturation, alternative triggers, reset, and exclusive input. Verify actual cursor capture and release on Windows.
- [ ] Demonstrate movement, jumping, camera control, standard buttons, and menu actions in Astro's Playroom. Stop the external mapper's capture before testing without changing its configuration.
- [ ] Record the tested build, Windows environment, mouse settings, and gameplay results. Identify initial response constants as provisional until ticket 03 measures them.
- [ ] Resolve capture failures and stuck input before completion. Do not defer these faults to final acceptance or claim full Astro's Playroom compatibility.

