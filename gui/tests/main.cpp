#include <QtQuickTest/quicktest.h>
#include <QQmlEngine>
#include <QFont>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QTranslator>

#include "qmlsvgprovider.h"
#include "streaminputownership.h"
#include <QSet>

class StreamInputFixture : public StreamInputOwnership
{
    Q_OBJECT
    Q_PROPERTY(int grabs MEMBER grabs NOTIFY countsChanged)
    Q_PROPERTY(int releases MEMBER releases NOTIFY countsChanged)
public:
    StreamInputFixture(QObject *parent = nullptr) : StreamInputOwnership(nullptr, nullptr, parent) {}
    Q_INVOKABLE QString present(QObject *owner, const QString &role, QQuickItem *target = nullptr)
    {
        const auto id = StreamInputOwnership::present(owner, role, target);
        if (!id.isEmpty() && !active.contains(id)) {
            active.insert(id);
            ++grabs;
            emit countsChanged();
            connect(owner, &QObject::destroyed, this, [this, id] { released(id); });
        }
        return id;
    }
    Q_INVOKABLE void dismiss(const QString &id)
    {
        StreamInputOwnership::dismiss(id);
        released(id);
    }
    int grabs = 0;
    int releases = 0;
signals:
    void countsChanged();
private:
    QSet<QString> active;
    void released(const QString &id) { if (active.remove(id)) { ++releases; emit countsChanged(); } }
};

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
        qmlRegisterType<StreamInputFixture>("Chiaki.Test", 1, 0, "StreamInputFixture");
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
