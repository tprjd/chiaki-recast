#pragma once

#include "keyboardmousebindings.h"
#include <QSet>
#include <Qt>
#include <deque>
#include <chiaki/controller.h>

class KeyboardMouseMapper
{
public:
    void setBindings(const KeyboardMouseBindings &bindings) { if (ValidKeyboardMouseBindings(bindings)) this->bindings = bindings; }
    void reset(const QSet<int> &held = {});
    void key(int key, bool pressed);
    void mouseButton(Qt::MouseButton button, bool pressed);
    void mouseMove(double dx, double dy, qint64 timestampUs);
    bool advanceMouse(qint64 timestampUs);
    ChiakiControllerState state() const;
private:
    KeyboardMouseBindings bindings = DefaultKeyboardMouseBindings();
    QSet<int> pressed;
    QSet<int> ignored;
    struct MouseSample { double x; double y; qint64 time; };
    std::deque<MouseSample> mouse_samples;
    int16_t right_x = 0;
    int16_t right_y = 0;
};
