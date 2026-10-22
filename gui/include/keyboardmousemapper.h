#pragma once

#include "keyboardmousebindings.h"
#include <QSet>
#include <QHash>
#include <Qt>
#include <deque>
#include <chiaki/controller.h>

struct MouseResponse
{
    // shortcut: these bounds need real mouse and game calibration before release.
    static constexpr double MinimumSensitivity = 0.05;
    static constexpr double MaximumSensitivity = 8.0;
    static constexpr double MaximumCompensation = 0.5;
    static constexpr double MinimumExponent = 0.1;
    static constexpr double MaximumExponent = 3.0;
    static constexpr qint64 UpdateUs = 8000;
    static constexpr double VelocityGain = 16000000.0;
    double horizontalSensitivity = 1.0;
    double verticalSensitivity = 1.0;
    double deadZoneCompensation = 0.0;
    double responseExponent = 1.0;
    bool invertVertical = false;
    bool valid() const;
};

class KeyboardMouseMapper
{
public:
    bool setMouseResponse(const MouseResponse &response);
    void setBindings(const KeyboardMouseBindings &bindings) { if (ValidKeyboardMouseBindings(bindings)) this->bindings = bindings; }
    void reset(const QSet<qint64> &held = {});
    void key(int key, bool pressed, quint32 nativeScanCode = 0);
    void mouseButton(Qt::MouseButton button, bool pressed);
    void mouseMove(double dx, double dy, qint64 timestampUs);
    bool advanceMouse(qint64 timestampUs);
    ChiakiControllerState state() const;
private:
    MouseResponse response;
    KeyboardMouseBindings bindings = DefaultKeyboardMouseBindings();
    QHash<qint64, int> pressed;
    QSet<qint64> ignored;
    struct MouseSample { double x; double y; qint64 time; };
    std::deque<MouseSample> mouse_samples;
    qint64 mouse_timestamp = 0;
    int16_t right_x = 0;
    int16_t right_y = 0;
};
