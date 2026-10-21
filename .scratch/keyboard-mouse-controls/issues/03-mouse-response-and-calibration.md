# 03: Tune and calibrate mouse response

**What to build:** Adjust mouse response during a running stream, then press F8 to use the saved values. Establish measured defaults and useful limits with repeatable input traces and The First Descendant aiming trials.

**Blocked by:** [02: Play with mouse camera control and trigger buttons](02-mouse-camera-and-triggers.md).

Status: claimed

Parent: [Built-in keyboard and mouse controls](../spec.md)

This ticket implements Stage 2. It can proceed independently of ticket 04 using the settings contract from ticket 01.

- [x] Provide the Mouse response tab in the approved layout. Include horizontal sensitivity, vertical sensitivity, game dead-zone compensation, and vertical inversion.
- [x] Pair sliders with numeric entry, clear units, and accessible labels. Keep the mode selector available and make new-mode controls visible but disabled when the mode is off. Ticket 05 adds reset.
- [x] Save valid changes automatically and show the specified save message. Apply the saved values on the next capture without a stream restart or stale held input.
- [x] Validate entered and stored values. Reject non-finite and out-of-range values. Preserve the last valid value or use a validated default when stored data is invalid.
- [x] Apply dead-zone compensation to nonzero movement while retaining an exactly centred idle stick. Keep output bounded and preserve direct response without added acceleration or optional smoothing controls.
- [ ] Measure and record base gain, default values, sensitivity ranges, compensation limits, idle-return timing, and comparison tolerances. Do not adopt mockup numbers as tested defaults.
- [x] Replay equivalent timed movement with different event batching and rendering schedules. Check response against the recorded tolerance, including inversion, small movement, saturation, idle return, and reset.
- [ ] Test fine corrections, tracking, fast turns, aiming while moving, and firing while moving in The First Descendant. Record game controller settings, mouse DPI, and report rate where available.
- [ ] Distinguish conversion faults from the game's response curve and maximum turn speed through repeatable comparisons. Record setting changes between trials.
- [x] Verify persistence across restart and recapture, invalid stored data, settings search, keyboard navigation, and the existing minimum settings window size.
- [ ] Publish calibration evidence tied to the tested source revision and build. Unusable aiming or missing calibration evidence prevents completion.

## Implementation evidence

The shared page is available in main Settings and through the live-stream menu, with inline and native settings windows.
[The evidence record](../mouse-response-evidence.md) identifies the provisional numeric values, synthetic comparisons, and open real mouse and game trials.
Ticket 03 remains claimed because required game calibration is not available in this environment.

The combined-page checks pass after ticket 04 integration. Preserved logs cover
the response and binding snapshots, malformed stored values, numeric validation,
and all 12 synthetic timing comparisons. These comparisons use the recorded
unsaturated tolerance. They do not establish a tolerance for game aiming.
The actual Main settings route passes in OpenGL and Vulkan. The final reset
layout passes at 900 by 600, and its saved image was inspected without a rerun.
See the [integrated acceptance record](../acceptance.md) for preserved evidence.

