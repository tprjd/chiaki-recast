#include <QtQuickTest/quicktest.h>
#include <QQmlEngine>
#include <QFont>
#include <QGuiApplication>

#include "qmlsvgprovider.h"

class ConsoleScreenSetup : public QObject
{
    Q_OBJECT

public slots:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
#ifdef Q_OS_WIN
        QGuiApplication::setFont(QFont("Segoe UI"));
#endif
        engine->addImageProvider("svg", new QmlSvgProvider());
    }
};

QUICK_TEST_MAIN_WITH_SETUP(console_screen, ConsoleScreenSetup)

#include "main.moc"
