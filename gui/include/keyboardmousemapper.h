#pragma once

#include <QSet>
#include <chiaki/controller.h>

class KeyboardMouseMapper
{
public:
    void reset(const QSet<int> &held = {});
    void key(int key, bool pressed);
    ChiakiControllerState state() const;
private:
    QSet<int> pressed;
    QSet<int> ignored;
};
