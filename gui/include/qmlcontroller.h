#pragma once

#include "controllermanager.h"
#include <QPointer>

class QTimer;

class QmlController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool dualSense READ isDualSense CONSTANT)
    Q_PROPERTY(bool handheld READ isHandheld CONSTANT)
    Q_PROPERTY(bool steamVirtual READ isSteamVirtual CONSTANT)
    Q_PROPERTY(bool dualSenseEdge READ isDualSenseEdge CONSTANT)
    Q_PROPERTY(bool playStation READ isPS CONSTANT)

public:
    QmlController(Controller *controller, uint32_t shortcut, QObject *target, QObject *parent = nullptr);
    ~QmlController();

    bool isDualSense() const;
    bool isHandheld() const;
    bool isSteamVirtual() const;
    bool isDualSenseEdge() const;
    bool isPS() const;
    void setEscapeShortcut(uint32_t shortcut) { escape_shortcut = shortcut; };
    void setTarget(QObject *target);
    void setNavigationEnabled(bool enabled);
    QString GetGUID() const;
    QString GetVIDPID() const;
    QString name() const { return controller->GetName(); }
    QString type() const { return controller->GetType(); }

private:
    void sendKey(Qt::Key key, Qt::KeyboardModifiers modifiers = Qt::NoModifier);

    QPointer<QObject> target;
    uint32_t escape_shortcut = 0;
    uint32_t old_buttons = 0;
    bool navigation_enabled = true;
    bool wait_for_release = false;
    Controller *controller = {};
    QTimer *repeat_timer = {};
    int repeat_running = 0;
    Qt::Key pressed_key = Qt::Key_unknown;
};
