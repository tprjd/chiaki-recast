# 02: Play with mouse camera control and trigger buttons

**What to build:** Move the mouse to control the camera without screen-edge limits. Use left click for R2 and right click for L2, with the accepted keyboard alternatives. Play basic Astro's Playroom actions without a physical controller or external mapper.

**Blocked by:** [01: Enable gameplay capture with default keyboard controls](01-gameplay-capture-and-keyboard.md).

Status: ready-for-agent

Parent: [Built-in keyboard and mouse controls](../spec.md)

This ticket completes Stage 1. Numeric calibration and editable response controls belong to ticket 03.

- [ ] Use relative mouse input through the existing Windows, Qt, and SDL integration as appropriate. Capture permits continuous turning and release restores desktop cursor control.
- [x] Convert movement and elapsed time into bounded right-stick output. Keep conversion independent of rendering frames and return the mapped stick exactly to centre when movement stops.
- [x] Use the production conversion logic for deterministic timed-input tests. Do not add acceleration presets, optional smoothing controls, or a general input-plugin framework.
- [x] Map left click to R2 and right click to L2. Preserve X and Z as their respective alternatives. An action remains held until all of its held bindings are released.
- [x] Recognize the five specified mouse buttons. Middle and side buttons start unassigned. Do not map the wheel or let captured side-button events trigger navigation.
- [x] Apply every release and rearming rule from ticket 01 to mouse buttons and deltas. Discard pending movement across focus, owner, session, and capture transitions.
- [ ] Prevent artificial camera movement during cursor repositioning, resize, and fullscreen changes. Release capture if a transition loses focus.
- [ ] Disable double-click fullscreen during gameplay capture while keeping F11 available. Restore the existing double-click behaviour after release.
- [ ] Verify emitted controller state for movement, idle centring, saturation, alternative triggers, reset, and exclusive input. Verify actual cursor capture and release on Windows.
- [ ] Demonstrate movement, jumping, camera control, standard buttons, and menu actions in Astro's Playroom. Stop the external mapper's capture before testing without changing its configuration.
- [ ] Record the tested build, Windows environment, mouse settings, and gameplay results. Identify initial response constants as provisional until ticket 03 measures them.
- [ ] Resolve capture failures and stuck input before completion. Do not defer these faults to final acceptance or claim full Astro's Playroom compatibility.

## Implementation evidence, 2026-10-21

Software implementation is on `codex/kbm-mouse`, based on ticket 01 commit
`9257ca1b50e458b6216f29fcba4985154b2ff834`. The ticket remains open for native
mouse and gameplay acceptance.

- Built `chiaki` and `chiaki-stream-input-native-tests` with GNU UCRT64 16.2.0,
  Qt 6.11.2, SDL 2.32.10, and libplacebo 7.360.1. The app version is 1.10.0.
- Host was Windows 11 Pro, version 10.0.26100. The native test used OpenGL on
  NVIDIA GeForce RTX 5090, reported OpenGL driver 617.42.
- TDD trigger case first failed with R2 at zero for left click. It passed after
  mouse buttons used the held-input mapper. The test covers both alternatives,
  all five buttons, release, and held-button rearming at emitted feedback.
- TDD motion case first failed with a centred right stick. Production conversion
  then passed timed displacement, idle centre, radial saturation, and reset.
- TDD desktop case first failed because the prior Raw Input registration still
  owned the mouse. It now checks actual registration, cursor confinement,
  resize, restoration, newer-owner protection, owner and focus release, cursor
  visibility, double-click firing, and stale Qt mouse events.
- Production capture requires both Qt activation and the actual Windows
  foreground HWND. Deterministic native tests supply controlled foreground
  eligibility through a narrow test subclass. They exercise production native
  capture and release operations but do not prove real foreground eligibility.
- The existing keyboard output and platform-routing cases passed with the
  mouse adapter in the capture lifecycle. The final affected checks are
  `mouseMotion`, `mouseTriggers`, `mouseDesktop`, `keyboardCapture`, and
  `platformKeyboard`, each native window case in a separate process.
- Initial constants are provisional. The observation window is 16 ms. The
  session timer is 4 ms. Gain is 1,000 stick units per count within that window,
  equivalent to 16 stick units per count per second. Radial limit is 32,767.
  Simulated traces used 125 Hz and 1,000 Hz. Physical DPI and polling rate were
  not measured. Ticket 03 must calibrate these values.
- The optional `spontaneousMouse` test failed its Windows foreground-window
  precondition before `SendInput`. No native movement was injected. This is
  not evidence of successful Raw Input delivery or a product capture failure.
- No physical mouse, display scaling, fullscreen transition, forced API
  failure, external mapper, or live console acceptance ran. No Astro's Playroom
  actions are recorded as passed. No claim of full-game compatibility is made.

The API, test commands, and release behavior are in
[`gui/tests/stream-input.md`](../../../gui/tests/stream-input.md).
Microsoft documents process-wide registration in
[RegisterRawInputDevices](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerrawinputdevices),
relative device counts in
[RAWMOUSE](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawmouse),
and foreground packet cleanup in
[WM_INPUT](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-input).

