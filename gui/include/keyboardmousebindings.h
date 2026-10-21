#pragma once

#include <array>
#include <QString>

namespace KeyboardMouseAction {
// Values are the persisted binding order.
enum Id {
    LeftStickUp = 0, LeftStickDown = 1, LeftStickLeft = 2, LeftStickRight = 3,
    Cross = 4, Circle = 5, Square = 6, Triangle = 7, L1 = 8, R1 = 9, L2 = 10, R2 = 11, L3 = 12, R3 = 13,
    DpadUp = 14, DpadDown = 15, DpadLeft = 16, DpadRight = 17, Options = 18, Create = 19, Touchpad = 20, PS = 21, Count = 22
};
}

using KeyboardMouseBindings = std::array<std::array<int, 2>, KeyboardMouseAction::Count>;
KeyboardMouseBindings DefaultKeyboardMouseBindings();
bool ValidKeyboardMouseBindings(const KeyboardMouseBindings &bindings);
bool ValidKeyboardMouseInput(int input);
QString KeyboardMouseInputName(int input);
int KeyboardMouseNativeKey(int input);
int NormalizeKeyboardMouseKey(int input);
qint64 KeyboardMousePhysicalKey(int input, quint32 nativeScanCode = 0);
