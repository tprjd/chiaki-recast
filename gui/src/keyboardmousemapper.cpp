#include "keyboardmousemapper.h"
#include <Qt>

void KeyboardMouseMapper::reset(const QSet<int> &held)
{
    pressed.clear();
    ignored = held;
}

void KeyboardMouseMapper::key(int key, bool down)
{
    if (!down) {
        pressed.remove(key);
        ignored.remove(key);
    } else if (!ignored.contains(key)) pressed.insert(key);
}

ChiakiControllerState KeyboardMouseMapper::state() const
{
    ChiakiControllerState result;
    chiaki_controller_state_set_idle(&result);
    const struct { int key; uint32_t button; } buttons[] = {
        {Qt::Key_Space, CHIAKI_CONTROLLER_BUTTON_CROSS},
        {Qt::Key_Control, CHIAKI_CONTROLLER_BUTTON_MOON},
        {Qt::Key_R, CHIAKI_CONTROLLER_BUTTON_BOX}, {Qt::Key_F, CHIAKI_CONTROLLER_BUTTON_PYRAMID},
        {Qt::Key_Q, CHIAKI_CONTROLLER_BUTTON_L1}, {Qt::Key_E, CHIAKI_CONTROLLER_BUTTON_R1},
        {Qt::Key_Shift, CHIAKI_CONTROLLER_BUTTON_L3}, {Qt::Key_C, CHIAKI_CONTROLLER_BUTTON_R3},
        {Qt::Key_Up, CHIAKI_CONTROLLER_BUTTON_DPAD_UP}, {Qt::Key_Down, CHIAKI_CONTROLLER_BUTTON_DPAD_DOWN},
        {Qt::Key_Left, CHIAKI_CONTROLLER_BUTTON_DPAD_LEFT}, {Qt::Key_Right, CHIAKI_CONTROLLER_BUTTON_DPAD_RIGHT},
        {Qt::Key_Return, CHIAKI_CONTROLLER_BUTTON_OPTIONS},
        {Qt::Key_Backspace, CHIAKI_CONTROLLER_BUTTON_SHARE}, {Qt::Key_Tab, CHIAKI_CONTROLLER_BUTTON_TOUCHPAD},
        {Qt::Key_F1, CHIAKI_CONTROLLER_BUTTON_PS}
    };
    for (const auto &binding : buttons)
        if (pressed.contains(binding.key)) result.buttons |= binding.button;
    result.l2_state = pressed.contains(Qt::Key_Z) ? 255 : 0;
    result.r2_state = pressed.contains(Qt::Key_X) ? 255 : 0;
    const int x = int(pressed.contains(Qt::Key_D)) - int(pressed.contains(Qt::Key_A));
    const int y = int(pressed.contains(Qt::Key_S)) - int(pressed.contains(Qt::Key_W));
    const int scale = x && y ? 23169 : 32767;
    result.left_x = x * scale;
    result.left_y = y * scale;
    return result;
}
