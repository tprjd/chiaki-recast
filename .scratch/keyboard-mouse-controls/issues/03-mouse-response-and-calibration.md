# 03: Tune and calibrate mouse response

**What to build:** Adjust mouse response during a running stream, then press F8 to use the saved values. Establish measured defaults and useful limits with repeatable input traces and The First Descendant aiming trials.

**Blocked by:** [02: Play with mouse camera control and trigger buttons](02-mouse-camera-and-triggers.md).

Status: ready-for-agent

Parent: [Built-in keyboard and mouse controls](../spec.md)

This ticket implements Stage 2. It can proceed independently of ticket 04 using the settings contract from ticket 01.

- [ ] Provide the Mouse response tab in the approved layout. Include horizontal sensitivity, vertical sensitivity, game dead-zone compensation, and vertical inversion.
- [ ] Pair sliders with numeric entry, clear units, and accessible labels. Keep the mode selector available and make new-mode controls visible but disabled when the mode is off. Ticket 05 adds reset.
- [ ] Save valid changes automatically and show the specified save message. Apply the saved values on the next capture without a stream restart or stale held input.
- [ ] Validate entered and stored values. Reject non-finite and out-of-range values. Preserve the last valid value or use a validated default when stored data is invalid.
- [ ] Apply dead-zone compensation to nonzero movement while retaining an exactly centred idle stick. Keep output bounded and preserve direct response without added acceleration or optional smoothing controls.
- [ ] Measure and record base gain, default values, sensitivity ranges, compensation limits, idle-return timing, and comparison tolerances. Do not adopt mockup numbers as tested defaults.
- [ ] Replay equivalent timed movement with different event batching and rendering schedules. Check response against the recorded tolerance, including inversion, small movement, saturation, idle return, and reset.
- [ ] Test fine corrections, tracking, fast turns, aiming while moving, and firing while moving in The First Descendant. Record game controller settings, mouse DPI, and report rate where available.
- [ ] Distinguish conversion faults from the game's response curve and maximum turn speed through repeatable comparisons. Record setting changes between trials.
- [ ] Verify persistence across restart and recapture, invalid stored data, settings search, keyboard navigation, and the existing minimum settings window size.
- [ ] Publish calibration evidence tied to the tested source revision and build. Unusable aiming or missing calibration evidence prevents completion.

