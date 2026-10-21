# Mouse response evidence

Recorded: 2026-10-21. This record covers synthetic input and the Windows settings route.
Ticket 03 remains incomplete until real mouse and The First Descendant trials establish usable aiming.

## Source and build

The implementation starts at integration revision `70ecfdd26c1e2053a9297081dd46c1ef6c8cff66`.
Tested standalone response implementation: `9d8fd01d4bd4c9c8fea3eb7793d0060c6fe82438`.
The response implementation commit contains this record and the maintained checks below.
Build directory: `build/keyboard-mouse-native` in the response worktree.
Configuration: Release, GCC 16.2.0 UCRT64, Qt 6.11.2, SDL controller support, Steam shortcuts, and tests enabled.
The build uses the pinned primary checkout dependencies without their Git metadata.
The application, native input tests, numeric response tests, and QML screen runner build successfully.

## Provisional numeric configuration

These are engineering defaults and validation bounds. They are not calibrated game recommendations.
The values retain ticket 02's base response. The sensitivity bounds are independent of the example mockup values.

| Quantity | Value | Basis or limit |
| --- | --- | --- |
| Base gain | 1,000 stick units per count in the observation window | Existing ticket 02 response |
| Observation window | 16,000 microseconds | Includes two 125 Hz reports during steady motion |
| Timer | 4 ms, `Qt::PreciseTimer`, capture-relative monotonic clock | Separate from rendering |
| Horizontal and vertical defaults | 1.00 times | Retains the existing base response |
| Sensitivity bounds | 0.05 to 8.00 times, inclusive | Positive finite engineering bounds, real mouse validation pending |
| Compensation default and bounds | 0%, from 0% to 50% inclusive | Keeps the compensated magnitude bounded and preserves exact zero at idle |
| Inversion default | Off | Preserves the existing vertical direction |
| Idle return | Exactly zero when the final sample reaches 16 ms | The next normal timer observation is less than 20 ms after the last sample, excluding scheduler delays |
| Synthetic integral tolerance | Zero difference for the tested unsaturated integer traces | This does not establish a general tolerance for nonlinear compensation or arbitrary event timing |

At default sensitivity, steady movement has a gain of 16 stick units per count per second.
An axial rate of about 2,048 counts per second reaches the stick limit.
The game can impose its own response curve, dead zone, and maximum turn speed.

Compensation changes the nonzero radial magnitude by `compensation * (32767 - magnitude)` before the final limit.
It does not discard small mouse movements. A zero movement vector stays exactly zero.
The mapper applies axis sensitivity and inversion before radial compensation and the stick limit.

## Recorded checks

`gui/tests/mouseresponse.cpp` uses the production mapper, settings store, and QML settings bridge.
It checks independent axis values, inversion, non-finite and out-of-range rejection, saturation, compensation, and idle return.
It also checks persistence through a new Settings instance and recovery from invalid stored values.
The legacy keyboard preference stays unchanged.

The timed trace supplies 32 horizontal counts across 32 ms.
Event batches are 1, 2, 4, and 8 ms. Independent rendering observations occur every 8, 16, or 33 ms.
All 12 combinations produce an integral of 512,000 stick-unit milliseconds, a peak of 16,000, and final output zero.
These comparisons cover unsaturated default response. They do not prove equal instantaneous waveforms between report rates.

`StreamInputNativeTest::mouseResponseRecapture` observes the real session feedback boundary.
It verifies that a saved change leaves the current snapshot intact, release sends zero, and recapture uses the new sensitivity without old movement.
The test controls the timer boundary. It does not claim a real-time scheduling measurement.

`StreamInputNativeTest::keyboardMouseSettingsRoute` passes with the actual OpenGL and Vulkan application windows.
The shared settings page opens while the session remains present. It blocks game input through the existing ownership module.
Closing the page restores the stream's focus and leaves gameplay capture released.

The focused QML checks cover numeric entry, invalid edit recovery, mode-off disabled controls, search, and a 900 by 600 window.
The stream menu checks cover the inline and native-window routes. The toolbar layout checks cover both routes at the existing test sizes.
Screenshot: `build/mouse-response-minimum.png` in the response worktree.
Test logs use the `kbm-response-` prefix in the system temporary directory.

## Open acceptance work

No real mouse response trial or game trial ran for this record.
Mouse DPI, actual report rate, console settings, and The First Descendant controller settings are not recorded.
Fine corrections, tracking, fast turns, aiming while moving, and firing while moving remain unverified.
The numeric defaults and bounds must remain provisional until those trials pass.

Reset belongs to ticket 05. The shared page mounts ticket 04's separate binding pane when that interface is present.
