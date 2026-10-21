#pragma once
#include <QJSValue>
#include <QString>
class QmlBackend {
public:
    void createSteamShortcut(QString shortcutName, QString launchOptions, const QJSValue &callback, QString steamDir);
};
