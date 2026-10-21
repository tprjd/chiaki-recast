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
    key = NormalizeKeyboardMouseKey(key);
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
    const auto active = [this](int action) {
        return (bindings[action][0] && pressed.contains(bindings[action][0]))
            || (bindings[action][1] && pressed.contains(bindings[action][1]));
    };
    const uint32_t buttons[] = {CHIAKI_CONTROLLER_BUTTON_CROSS, CHIAKI_CONTROLLER_BUTTON_MOON,
        CHIAKI_CONTROLLER_BUTTON_BOX, CHIAKI_CONTROLLER_BUTTON_PYRAMID,
        CHIAKI_CONTROLLER_BUTTON_L1, CHIAKI_CONTROLLER_BUTTON_R1, 0, 0,
        CHIAKI_CONTROLLER_BUTTON_L3, CHIAKI_CONTROLLER_BUTTON_R3,
        CHIAKI_CONTROLLER_BUTTON_DPAD_UP, CHIAKI_CONTROLLER_BUTTON_DPAD_DOWN,
        CHIAKI_CONTROLLER_BUTTON_DPAD_LEFT, CHIAKI_CONTROLLER_BUTTON_DPAD_RIGHT,
        CHIAKI_CONTROLLER_BUTTON_OPTIONS, CHIAKI_CONTROLLER_BUTTON_SHARE,
        CHIAKI_CONTROLLER_BUTTON_TOUCHPAD, CHIAKI_CONTROLLER_BUTTON_PS};
    for (int i = 0; i < 18; ++i)
        if (active(i + 4)) result.buttons |= buttons[i];
    result.l2_state = active(10) ? 255 : 0;
    result.r2_state = active(11) ? 255 : 0;
    const int x = int(active(3)) - int(active(2));
    const int y = int(active(1)) - int(active(0));
    const int scale = x && y ? 23169 : 32767;
    result.left_x = x * scale;
    result.left_y = y * scale;
    result.right_x = right_x;
    result.right_y = right_y;
    return result;
}
