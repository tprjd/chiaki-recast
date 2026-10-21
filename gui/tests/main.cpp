#include <QtQuickTest/quicktest.h>
#include <QQmlEngine>
#include <QFont>
#include <QGuiApplication>
#include <QQuickWindow>

#include "qmlsvgprovider.h"

class StreamTestWindow : public QQuickWindow
{
    Q_OBJECT
    Q_PROPERTY(double ZoomFactor MEMBER zoomFactor NOTIFY zoomFactorChanged)
public:
    using QQuickWindow::QQuickWindow;
    double zoomFactor = 0;
signals:
    void zoomFactorChanged();
};

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
        qmlRegisterType<StreamTestWindow>("Chiaki.Test", 1, 0, "StreamTestWindow");
        engine->addImportPath(QStringLiteral(QUICK_TEST_SOURCE_DIR "/../imports"));
        engine->addImageProvider("svg", new QmlSvgProvider());
    }
};

QUICK_TEST_MAIN_WITH_SETUP(console_screen, ConsoleScreenSetup)

#include "main.moc"
