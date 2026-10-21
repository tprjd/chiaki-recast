#include "keyboardmousemapper.h"
#include <Qt>
#include <cmath>
#include <limits>

void KeyboardMouseMapper::reset(const QSet<int> &held)
{
    pressed.clear();
    ignored = held;
    mouse_samples.clear();
    right_x = right_y = 0;
}

void KeyboardMouseMapper::key(int key, bool down)
{
    if (!down) {
        pressed.remove(key);
        ignored.remove(key);
    } else if (!ignored.contains(key)) pressed.insert(key);
}

void KeyboardMouseMapper::mouseButton(Qt::MouseButton button, bool down)
{
    if (button == Qt::LeftButton || button == Qt::RightButton || button == Qt::MiddleButton
            || button == Qt::BackButton || button == Qt::ForwardButton)
        key(-int(button), down);
}

void KeyboardMouseMapper::mouseMove(double dx, double dy, qint64 timestampUs)
{
    if (std::isfinite(dx) && std::isfinite(dy) && std::abs(dx) <= std::numeric_limits<int32_t>::max()
            && std::abs(dy) <= std::numeric_limits<int32_t>::max() && (dx || dy))
        mouse_samples.push_back({dx, dy, timestampUs});
}

bool KeyboardMouseMapper::advanceMouse(qint64 timestampUs)
{
    // shortcut: provisional 16 ms observation and gain, calibrate with real mice in ticket 03.
    constexpr qint64 observationUs = 16000;
    while (!mouse_samples.empty() && timestampUs - mouse_samples.front().time >= observationUs)
        mouse_samples.pop_front();
    double x = 0, y = 0;
    for (const auto &sample : mouse_samples) {
        x += sample.x * 1000.0;
        y += sample.y * 1000.0;
    }
    const double magnitude = std::hypot(x, y);
    if (magnitude > 32767) {
        x *= 32767 / magnitude;
        y *= 32767 / magnitude;
    }
    const auto nextX = int16_t(std::lround(x));
    const auto nextY = int16_t(std::lround(y));
    const bool changed = nextX != right_x || nextY != right_y;
    right_x = nextX;
    right_y = nextY;
    return changed;
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
    result.l2_state = pressed.contains(Qt::Key_Z) || pressed.contains(-int(Qt::RightButton)) ? 255 : 0;
    result.r2_state = pressed.contains(Qt::Key_X) || pressed.contains(-int(Qt::LeftButton)) ? 255 : 0;
    const int x = int(pressed.contains(Qt::Key_D)) - int(pressed.contains(Qt::Key_A));
    const int y = int(pressed.contains(Qt::Key_S)) - int(pressed.contains(Qt::Key_W));
    const int scale = x && y ? 23169 : 32767;
    result.left_x = x * scale;
    result.left_y = y * scale;
    result.right_x = right_x;
    result.right_y = right_y;
    return result;
}
