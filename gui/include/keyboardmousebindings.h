#pragma once

#include <array>
#include <QString>

using KeyboardMouseBindings = std::array<std::array<int, 2>, 22>;
KeyboardMouseBindings DefaultKeyboardMouseBindings();
bool ValidKeyboardMouseBindings(const KeyboardMouseBindings &bindings);
bool ValidKeyboardMouseInput(int input);
QString KeyboardMouseInputName(int input);
int KeyboardMouseNativeKey(int input);
int NormalizeKeyboardMouseKey(int input);
