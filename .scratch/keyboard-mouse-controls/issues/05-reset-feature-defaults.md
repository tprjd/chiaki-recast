# 05: Restore feature defaults safely

**What to build:** Restore the feature's default mouse response and bindings from either settings tab after confirmation. Turn the mode off and preserve every unrelated setting.

**Blocked by:** [03: Tune and calibrate mouse response](03-mouse-response-and-calibration.md) and [04: Edit keyboard and mouse bindings](04-edit-bindings.md).

Status: claimed

Parent: [Built-in keyboard and mouse controls](../spec.md)

This ticket completes Stage 3. It depends on calibrated defaults and the complete binding configuration.

- [x] Make Reset defaults available on both tabs with the approved layout and accessible keyboard operation.
- [x] Require confirmation before replacing feature settings. Cancellation changes no values and returns focus correctly.
- [ ] After confirmation, restore calibrated mouse defaults and all accepted primary and alternative bindings. Turn the new mode off and save the complete result.
- [x] Clear mapped state and pending movement through the established capture lifecycle. Reset must not capture input or replay held commands.
- [x] Preserve legacy keyboard and mouse-touchpad settings, physical-controller settings, console configuration, unrelated profiles, and external emulator files.
- [x] Update both tabs and the mode selector consistently. The result survives restart. Re-enabling the feature and pressing F8 uses the restored defaults.
- [x] Test cancellation, confirmation from both tabs, persistence, and subsequent controller output. Compare unrelated settings before and after reset to verify that they remain intact.

## Implementation evidence

The shared page has one Reset defaults button for both tabs. The existing
confirmation dialog starts with Cancel focused and returns focus to the reset
button. A settings replacement, profile change, or hidden page cancels the
pending reset. Settings search finds reset without opening confirmation and
omits it on unsupported platforms.

`Settings::ResetKeyboardMouseDefaults` turns the mode off through its existing
setter, restores `MouseResponse{}` and `DefaultKeyboardMouseBindings()`, then
saves the feature values. It writes no legacy or unrelated keys. No external
mapper file is read or written.

The focused QML suite passed all nine checks. The native reset check passed
through the production settings bridge, main window, and session output path.
It checks cleared buttons, sticks, and pending mouse movement, saved defaults,
unchanged unrelated keys and another profile, held-key rearm, restored controller
output, and no automatic capture. The minimum-size mouse response check passed,
and its 900 by 600 image was inspected.

The numeric mouse defaults are provisional. Ticket 05 remains claimed until
ticket 03 supplies the required physical calibration evidence. No physical
mouse calibration or PS5 gameplay acceptance ran for this ticket.

