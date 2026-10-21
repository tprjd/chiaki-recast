#include <QtQuickTest/quicktest.h>
#include <QQmlEngine>
#include <QFont>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QTranslator>

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
        if (qEnvironmentVariableIsSet("CHIAKI_SCREEN_LONG_TRANSLATIONS")) {
            class LongLabels : public QTranslator {
                QString translate(const char *context, const char *source, const char *, int) const override
                {
                    if (QByteArrayView(context) != "StreamMenuContent") return {};
                    return QString::fromUtf8(source) + QStringLiteral(" extended translated label");
                }
            };
            auto *translator = new LongLabels;
            translator->setParent(engine);
            QGuiApplication::installTranslator(translator);
            engine->retranslate();
        }
    }
};

QUICK_TEST_MAIN_WITH_SETUP(console_screen, ConsoleScreenSetup)

#include "main.moc"
