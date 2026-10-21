#include "keyboardmousemapper.h"
#include <Qt>
#include <cmath>
#include <limits>

bool MouseResponse::valid() const
{
    return std::isfinite(horizontalSensitivity) && horizontalSensitivity >= MinimumSensitivity
        && horizontalSensitivity <= MaximumSensitivity && std::isfinite(verticalSensitivity)
        && verticalSensitivity >= MinimumSensitivity && verticalSensitivity <= MaximumSensitivity
        && std::isfinite(deadZoneCompensation) && deadZoneCompensation >= 0
        && deadZoneCompensation <= MaximumCompensation;
}

bool KeyboardMouseMapper::setMouseResponse(const MouseResponse &value)
{
    if (!value.valid()) return false;
    response = value;
    return true;
}

void KeyboardMouseMapper::reset(const QSet<qint64> &held)
{
    pressed.clear();
    ignored = held;
    mouse_samples.clear();
    right_x = right_y = 0;
}

void KeyboardMouseMapper::key(int key, bool down, quint32 nativeScanCode)
{
    key = NormalizeKeyboardMouseKey(key);
    const auto physical = KeyboardMousePhysicalKey(key, nativeScanCode);
    if (!down) {
        pressed.remove(physical);
        ignored.remove(physical);
    } else if (!ignored.contains(physical)) pressed.insert(physical, key);
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
    while (!mouse_samples.empty() && timestampUs - mouse_samples.front().time >= MouseResponse::ObservationUs)
        mouse_samples.pop_front();
    double x = 0, y = 0;
    for (const auto &sample : mouse_samples) {
        x += sample.x * MouseResponse::Gain * response.horizontalSensitivity;
        y += sample.y * MouseResponse::Gain * response.verticalSensitivity;
    }
    if (response.invertVertical) y = -y;
    const double magnitude = std::hypot(x, y);
    if (magnitude > 0) {
        const double output = std::min(32767.0, magnitude + response.deadZoneCompensation * (32767 - magnitude));
        x = x / magnitude * output;
        y = y / magnitude * output;
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
    using namespace KeyboardMouseAction;
    ChiakiControllerState result;
    chiaki_controller_state_set_idle(&result);
    const auto active = [this](int action) {
        for (int input : bindings[action])
            if (input && pressed.key(input, 0)) return true;
        return false;
    };
    const struct { Id action; uint32_t button; } buttons[] = {
        {Cross, CHIAKI_CONTROLLER_BUTTON_CROSS}, {Circle, CHIAKI_CONTROLLER_BUTTON_MOON},
        {Square, CHIAKI_CONTROLLER_BUTTON_BOX}, {Triangle, CHIAKI_CONTROLLER_BUTTON_PYRAMID},
        {L1, CHIAKI_CONTROLLER_BUTTON_L1}, {R1, CHIAKI_CONTROLLER_BUTTON_R1},
        {L3, CHIAKI_CONTROLLER_BUTTON_L3}, {R3, CHIAKI_CONTROLLER_BUTTON_R3},
        {DpadUp, CHIAKI_CONTROLLER_BUTTON_DPAD_UP}, {DpadDown, CHIAKI_CONTROLLER_BUTTON_DPAD_DOWN},
        {DpadLeft, CHIAKI_CONTROLLER_BUTTON_DPAD_LEFT}, {DpadRight, CHIAKI_CONTROLLER_BUTTON_DPAD_RIGHT},
        {Options, CHIAKI_CONTROLLER_BUTTON_OPTIONS}, {Create, CHIAKI_CONTROLLER_BUTTON_SHARE},
        {Touchpad, CHIAKI_CONTROLLER_BUTTON_TOUCHPAD}, {PS, CHIAKI_CONTROLLER_BUTTON_PS}};
    for (const auto &binding : buttons)
        if (active(binding.action)) result.buttons |= binding.button;
    result.l2_state = active(L2) ? 255 : 0;
    result.r2_state = active(R2) ? 255 : 0;
    const int x = int(active(LeftStickRight)) - int(active(LeftStickLeft));
    const int y = int(active(LeftStickDown)) - int(active(LeftStickUp));
    const int scale = x && y ? 23169 : 32767;
    result.left_x = x * scale;
    result.left_y = y * scale;
    result.right_x = right_x;
    result.right_y = right_y;
    return result;
}
