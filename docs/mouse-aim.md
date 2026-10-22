# Tune mouse aim

Open Settings, then Keyboard and mouse. Enable mouse camera control. Changes
apply the next time you press F8 to enter gameplay capture.

The mapper now uses new mouse counts and the measured update time. It requests
an update every 8 ms. It does not use the previous 32 ms smoothing pulse. Empty
updates stop the right stick, and direction changes use the next update.
Windows scheduling, the network, the console, and video decoding add separate
delays. The game still sets the maximum turn speed.

## Set a repeatable baseline

1. Keep your mouse DPI fixed. The reported setup was 1600 DPI.
2. Keep the game's sensitivity high enough for the turn speed you need. The
   reported First Descendant setting was 100. Do not change it during a test.
3. Set the game's dead zone to its lowest usable value. Keep app dead-zone
   compensation at zero unless small movements produce no camera movement.
4. Start Game response correction at 1.00. This applies no curve correction.
5. Adjust the app's horizontal and vertical sensitivity to set aim speed below
   the game's turn limit. Higher app sensitivity reaches that limit sooner.

## Correct small movements

Use a stationary scene without a nearby aim-assist target. Move the mouse a
fixed distance slowly, then repeat the same distance faster. Stay below the
game's maximum turn rate.

If fast movements turn farther than slow movements, reduce Game response
correction a little and repeat. A value below 1 increases small movements.
Very low values can make aim jump. If slow movements turn farther than fast
movements, increase the value. Keep horizontal and vertical sensitivity fixed
while you compare the curve. Then adjust sensitivity again for a useful speed.

A correction of 0.50 cancels an ideal squared game response in the analytic
test. This does not establish a suitable value for The First Descendant or
GTA V. Neither game has a measured profile in this build. Test hip aim and
aim down sight separately because their responses can differ.

If fast motion reaches full stick, extra mouse travel cannot increase camera
speed. Lower app sensitivity can give more usable movement before this limit.
It cannot raise the limit. The mapper discards excess movement rather than
turning the camera after you stop.

For the research behind this approach, see
[mouse mapping research](mouse-mapping-research.md). Exact native mouse behavior
cannot be promised without game support or a measured game-specific response.
