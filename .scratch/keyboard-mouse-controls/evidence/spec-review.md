Reviewed HEAD `992cc170e834c6f92132cc72487a071ec7dd4a0d` against `0730007cff122cb5a17fe1dd2749f6bd7233c8c4`.

- **[P2] Keep an action held until all physical keys for that input are released.** In `gui/src/streamsession.cpp:826`, only `event->key()` reaches the mapper. In `gui/src/keyboardmousemapper.cpp:34`, any release removes that logical key from `pressed`. Both Shift keys produce `Qt::Key_Shift`, and both Ctrl keys produce `Qt::Key_Control`. Hold left Shift, hold right Shift, then release left Shift: L3 becomes released while right Shift remains held. The same sequence releases Circle for Ctrl. [Qt's Windows key mapper](https://github.com/qt/qtbase/blob/6.11/src/plugins/platforms/windows/qwindowskeymapper.cpp) delivers these modifier releases separately. Preserve physical-key state or check the combined modifier state before clearing the action. Spec `.scratch/keyboard-mouse-controls/spec.md:150`: "Track held physical inputs. An action remains active while either assigned input is held."

Missing or partial requirements: calibration and native/game acceptance remain open, as already recorded. Spec line 165 says, "Calibration is required implementation work, not an optional later enhancement." The provisional constants in `gui/include/keyboardmousemapper.h:12` therefore do not establish Stage 2 completion. This is an existing acceptance limit, not a new code defect.

Unrequested scope: none identified.

No tests or repository changes were made. Physical input, console behaviour, and game calibration were not verified in this review.

