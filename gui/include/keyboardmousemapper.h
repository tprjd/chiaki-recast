#pragma once

#include "keyboardmousebindings.h"
#include <QSet>
#include <Qt>
#include <deque>
#include <chiaki/controller.h>

struct MouseResponse
{
    // shortcut: these bounds need real mouse and game calibration before release.
    static constexpr double MinimumSensitivity = 0.05;
    static constexpr double MaximumSensitivity = 8.0;
    static constexpr double MaximumCompensation = 0.5;
    static constexpr qint64 ObservationUs = 16000;
    static constexpr double Gain = 1000.0;
    double horizontalSensitivity = 1.0;
    double verticalSensitivity = 1.0;
    double deadZoneCompensation = 0.0;
    bool invertVertical = false;
    bool valid() const;
};

class KeyboardMouseMapper
{
public:
    bool setMouseResponse(const MouseResponse &response);
    void setBindings(const KeyboardMouseBindings &bindings) { if (ValidKeyboardMouseBindings(bindings)) this->bindings = bindings; }
    void reset(const QSet<int> &held = {});
    void key(int key, bool pressed);
    void mouseButton(Qt::MouseButton button, bool pressed);
    void mouseMove(double dx, double dy, qint64 timestampUs);
    bool advanceMouse(qint64 timestampUs);
    ChiakiControllerState state() const;
private:
    MouseResponse response;
    KeyboardMouseBindings bindings = DefaultKeyboardMouseBindings();
    QSet<int> pressed;
    QSet<int> ignored;
    struct MouseSample { double x; double y; qint64 time; };
    std::deque<MouseSample> mouse_samples;
    int16_t right_x = 0;
    int16_t right_y = 0;
};
