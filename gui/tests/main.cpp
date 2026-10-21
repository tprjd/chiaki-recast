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
        QGuiApplication::setApplicationName(QStringLiteral("Chiaki"));
        QGuiApplication::setApplicationVersion(QStringLiteral("0.0.0-screen-test"));
#ifdef Q_OS_WIN
        QGuiApplication::setFont(QFont("Segoe UI"));
#endif
        engine->addImportPath(QStringLiteral(QUICK_TEST_SOURCE_DIR "/../imports"));
        engine->addImageProvider("svg", new QmlSvgProvider());
    }
};

QUICK_TEST_MAIN_WITH_SETUP(console_screen, ConsoleScreenSetup)

#include "main.moc"
