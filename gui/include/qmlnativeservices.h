#pragma once

#include "qmlbackend.h"
#include <QQmlEngine>

class QmlNativeServices
{
public:
    explicit QmlNativeServices(Settings *settings, PsnRequests requests = {}, QmlSettings::ProcessLauncher launcher = {}, RegistrationRequests registration = {}, DiscoveryRequests discovery = {});
    QmlBackend *backend() { return &backend_; }
    QQmlEngine *engine() { return &engine_; }

private:
    // The engine must release its roots and providers before the backend.
    QmlBackend backend_;
    QQmlEngine engine_;
};
