# 05: Restore feature defaults safely

**What to build:** Restore the feature's default mouse response and bindings from either settings tab after confirmation. Turn the mode off and preserve every unrelated setting.

**Blocked by:** [03: Tune and calibrate mouse response](03-mouse-response-and-calibration.md) and [04: Edit keyboard and mouse bindings](04-edit-bindings.md).

Status: ready-for-agent

Parent: [Built-in keyboard and mouse controls](../spec.md)

This ticket completes Stage 3. It depends on calibrated defaults and the complete binding configuration.

- [ ] Make Reset defaults available on both tabs with the approved layout and accessible keyboard operation.
- [ ] Require confirmation before replacing feature settings. Cancellation changes no values and returns focus correctly.
- [ ] After confirmation, restore calibrated mouse defaults and all accepted primary and alternative bindings. Turn the new mode off and save the complete result.
- [ ] Clear mapped state and pending movement through the established capture lifecycle. Reset must not capture input or replay held commands.
- [ ] Preserve legacy keyboard and mouse-touchpad settings, physical-controller settings, console configuration, unrelated profiles, and external emulator files.
- [ ] Update both tabs and the mode selector consistently. The result survives restart. Re-enabling the feature and pressing F8 uses the restored defaults.
- [ ] Test cancellation, confirmation from both tabs, persistence, and subsequent controller output. Compare unrelated settings before and after reset to verify that they remain intact.

