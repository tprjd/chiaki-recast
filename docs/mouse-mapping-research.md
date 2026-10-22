# Mouse movement to controller aim

Research date: 2026-10-21.

The smallest low-delay design converts mouse counts to a velocity at the point
that consumes controller input. It then applies sensitivity, a calibrated dead
zone, and a radial limit. A fixed pulse for each count is a filter. Its shape
changes delay and cannot correct an unknown game response curve.

This document separates inspected source code, vendor statements, and the
mathematical conclusions from those sources. It does not specify a calibrated
profile for The First Descendant.

## What the primary sources establish

GIMX provides an inspectable implementation. `cfg_process_motion` merges mouse
motion for each update and applies a weighted history filter. `mouse2axis1d`
multiplies motion by `frequency_scale` before the power curve. Its output has
the form `gain * sign(motion) * abs(motion)^exponent`, with dead-zone adjustment.
The two-axis path applies the exponent to motion magnitude and keeps the
direction. These are source-code facts, not measured performance claims.
See [GIMX core/config.c](https://github.com/matlo/GIMX/blob/master/core/config.c).

GIMX sets `frequency_scale = DEFAULT_REFRESH_PERIOD / refresh_period`.
Thus the curve uses counts normalized to a reference update period. This is
equivalent to velocity normalization with a fixed scale factor. Its loop also
supports a lower controller frequency when the mouse frequency is low.
See [GIMX core/gimx.c](https://github.com/matlo/GIMX/blob/master/core/gimx.c) and
[GIMX core/mainloop.c](https://github.com/matlo/GIMX/blob/master/core/mainloop.c).

GIMX documents a dead-zone test, a translation test at different speeds, and a
circle test. It warns that a deeper motion buffer increases latency. It also
warns that low mouse resolution can cause jumps after dead-zone compensation.
Its documentation says that a 1:1 result is not possible for every game.
See [GIMX Mouse Calibration](https://gimx.fr/wiki/index.php?title=Mouse_Calibration).

XIM describes Smart Translation as proprietary. Its documentation requires
game settings that agree with the translator. It states that controller aim
has a maximum turn speed, which can depend on aim direction. These statements
support game-specific calibration and a speed limit. They do not expose the
translator algorithm or establish its measured latency. See
[XIM Game Settings](https://guide.xim.tech/Game-Settings/) and
[XIM Aim Settings](https://guide.xim.tech/Aim-Settings/).

reWASD exposes a virtual stick response curve, independent axis sensitivity,
smoothing, and a noise filter. Its guide states that more smoothing reduces
response. The noise filter waits for a selected number of empty mouse reports
before it stops the virtual stick. That wait can prevent gaps during slow
motion, but it also delays the stop. The guide does not publish the complete
conversion algorithm. See
[reWASD Advanced Mouse settings](https://www.help.rewasd.com/basic-functions/advanced-mouse-settings.html).

## Baseline before this change

The inspected files are `gui/include/keyboardmousemapper.h`,
`gui/src/keyboardmousemapper.cpp`, `gui/src/streamsession.cpp`, and
`lib/src/feedbacksender.c`. These observations include local changes present
on the research date.

`KeyboardMouseMapper` stores timestamped relative counts. For each count, it
applies a triangle with a 32 ms duration and a peak at 16 ms. The peak gain is
1000 stick units per count before sensitivity. A 4 ms precise Qt timer calls
`advanceMouse`. The timer does not depend on video frames.

For one count, before compensation, clipping, and integer rounding:

```text
triangle area = 1000 * 0.032 / 2 = 16 stick units * seconds
triangle time centroid = 16 ms
previous 16 ms box area = 1000 * 0.016 = 16 stick units * seconds
previous box time centroid = 8 ms
```

The time centroid is the average age of the count contribution. It is a filter
delay measure, not an end-to-end latency measurement. The triangle keeps the
continuous linear area but adds 8 ms to this measure. It starts at zero and
can affect output for 32 ms. For steady motion followed by an equal reversal,
the ideal triangle crosses zero after 16 ms. The 16 ms box crosses after 8 ms.
Those results assume continuous reports and no clipping.

At 1000 counts per second and sensitivity 1, both filters settle at 16000 stick
units. Their equivalent velocity gain is 16 stick units per count per second.
At sensitivity 1, a straight-axis input reaches the limit near 2048 counts per
second. This threshold is a count rate, not a physical mouse speed. Mouse DPI
changes the physical speed that reaches it.

The feedback sender defines an 8 ms minimum interval, but the inspected code
does not enforce it. The changed-state branch contains
`TODO: FEEDBACK_STATE_TIMEOUT_MIN_MS` and sends after a condition signal.
The 200 ms maximum interval provides a keepalive. Therefore, 8 ms is not a
proven send cadence. A slow sender thread can still combine several state
updates into the latest state.

Qt states that precise timers can run late. On an overrun, Qt emits one timeout
even when several intervals have expired. A requested 4 ms interval is not an
elapsed-time measurement. See
[Qt QTimer accuracy](https://doc.qt.io/qt-6/qtimer.html#accuracy-and-timer-resolution).

## A small velocity mapping design

The following is a recommendation from the source review and the mathematics.
It is not a description of XIM or reWASD internals.

At each controller consumption point, sum all new counts since the preceding
consumption. Measure the elapsed time with the existing monotonic clock.
For a positive elapsed time in seconds, calculate:

```text
vx = sum(dx) / elapsedSeconds
vy = sum(dy) / elapsedSeconds
x = 16 * horizontalSensitivity * vx
y = 16 * verticalSensitivity * vy
```

Apply the existing vertical inversion. If both count sums are zero, output
exact zero. Otherwise, apply the existing optional radial dead-zone
compensation and clamp the magnitude to 32767. Round to controller integers
last. Reset counts, time state, and output when gameplay capture ends.

The sum uses counts, not the number of mouse events. At a fixed consumption
period, 8 counts in one report equal eight reports of 1 count within that
period. Do not divide each event by the time since the previous nonzero event.
That time can include a stationary period and distort the first movement.
Coalesced events can also have the same application timestamp.

An exact per-bin delivery claim requires a shared consumption point with the
sender. An independent
4 ms accumulator that the sender samples less often can lose motion. For
example, one count in a 4 ms bin produces 4000 stick units. An 8 ms sampler
can miss that bin or hold it for 8 ms. The sent area is then 0 or 32 stick
units times seconds, although the intended area is 16.

The current sender has no fixed consumption callback. A small local compromise
uses an 8 ms mapper interval and consumes each count once. This interval matches
the nominal report period of a 125 Hz mouse. Faster mouse reports add into the
same bin. This change removes the 32 ms filter memory without a sender change.
It provides a clear mapper-level rule, but it cannot guarantee delivery of every
bin under sender delay. A mapper-only check must state this limit. Reducing
pulse duration without a send trace can hide lost motion.

Start with no added smoothing. With regular consumption, the bin already
averages movement over one output interval. A low-rate mouse or irregular
report delivery can still produce alternating empty and large bins. No causal
filter can know whether an empty interval means a stop or a delayed report.
A short hold or a short filter trades stop delay for less variation. Add that
trade only after a real trace shows a need.

Normalize by actual elapsed time when timer delivery is late. This prevents a
nominal-period velocity error. It does not guarantee exact sent area under
jitter. The next state holds for an interval that is not yet known. Exact
retrospective count area, immediate stops, and arbitrary future send times
cannot all be guaranteed by this simple rule. Do not retain large saturated
motion as a queue. That queue causes continued movement after the mouse stops.

## Dead zone and inverse response

A mouse reports displacement. A controller requests a turn rate. Let the game
map stick magnitude `r` to turn rate `F(r)`. The required conversion for a
target rate `w` is `r = inverseF(w)`, within the game's maximum turn rate.
Sensitivity alone supplies the correct relationship only when `F` is linear
over the useful range.

For a calibrated radial dead zone `d`, a useful model is:

```text
F(r) = 0                                      when r <= d
F(r) = maximumRate * ((r - d) / (1 - d))^p     when r > d
r = d + (1 - d) * (w / maximumRate)^(1 / p)   when w > 0
r = 0                                       when w = 0
```

Here, `r` and `d` are normalized to the range 0 to 1. This model is a proposed
fit, not an established model of The First Descendant. The existing
compensation `d + (1 - d) * magnitude` fits the special case `p = 1`.
A guessed exponent can add acceleration instead of removing it. A rectangular
dead zone, different axis speeds, angle-dependent limits, or time-dependent
acceleration can require a different model. Aim assist adds another variable.

No numeric game-calibrated curve for The First Descendant was established in
the reviewed sources. Keep exponent 1 until a console test measures otherwise.
Keep the in-game settings fixed during that test. Measure the minimum moving
stick value, turn rate at several magnitudes, horizontal and vertical motion,
and both hip aim and aim down sight. GIMX's speed-return and circle tests give
useful checks, but they do not replace a measured response curve.

An optional inverse-response exponent can support manual calibration without a
game preset. Apply `pow(clamp(speed / 32767, 0, 1), exponent)` to magnitude,
then apply dead-zone compensation. Preserve direction and the exact zero case.
The default exponent is 1. An exponent of 0.5 cancels an ideal quadratic game
model under the stated assumptions. It does not establish that a real game is
quadratic. A small exponent raises slow movement sharply. For example, 0.1 can
turn a one-count bin into a large stick movement. Explain that effect in the
control help instead of presenting the exponent as an automatic correction.

Preserved stick area does not prove preserved camera rotation. Dead-zone
compensation, clipping, and a nonlinear `F` break that implication. Clipped
counts cannot produce extra turn speed. A long tail can recover distance only
by adding delayed movement, which is a poor default for low-delay aim.

## Verification that distinguishes the designs

Use synthetic timestamped traces for repeatable checks, then inspect sent
controller states. The necessary cases are:

- One count, a steady count rate, a stop, and an equal-speed reversal. Measure
  first output, time centroid, zero-crossing time, and final zero.
- Mouse periods of 1, 2, 4, and 8 ms, plus irregular batches and slow one-count
  motion. Vary the mouse phase relative to the mapper and sender.
- Mapper periods of 4, 8, and 16 ms, with late intervals and a long event-loop
  stall. Include repeated timestamps and capture reset.
- The actual event-driven sender, plus 8 ms and 16 ms sampling models. Integrate
  each sent state over its actual hold duration. Do not sum nominal timer ticks.
- Zero compensation first, then nonzero compensation, integer rounding, radial
  limits, diagonal movement, and saturation. Check direction as well as area.
- Video intervals of 8, 16, and 33 ms. The controller trace must be unchanged
  when only video timing changes.

For regular unsaturated bins, verify an area of `16 * sensitivity * totalCounts`
on each axis, with exponent 1 and compensation 0. Add a separate test with an
analytic quadratic game and an inverse exponent of 0.5. For jitter and
coalescing, report the error instead of assuming
the same identity. Compare the triangle, the previous box, and the proposed
conversion with the same reports and send times. A 4 ms mapper integral alone
does not prove that the console receives the same motion.

The analytic triangle area and centroid were checked with a numerical integral
at 10 microsecond steps. The result was 16 stick units times seconds and 16 ms.
The 4 ms bin alias example was also checked. No implementation change, console
measurement, or mouse hardware test was part of this research.
