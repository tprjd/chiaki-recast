#pragma once

#include "setupwindow.h"
#include <QObject>
#include <memory>

class QmlNativeServices;
class QmlMainWindow;

class ApplicationController : public QObject
{
    Q_OBJECT
public:
    ApplicationController(QmlNativeServices *services, bool exitOnStreamEnd = false);
    ~ApplicationController();
    SetupWindow *setupWindow() { return &setup; }

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void prepareStreamWindow();
    void returnToSetup();
    QmlNativeServices *services;
    SetupWindow setup;
    std::unique_ptr<QmlMainWindow> stream;
    bool exitOnStreamEnd;
    bool returnRequested = false;
    bool returnQueued = false;
};
