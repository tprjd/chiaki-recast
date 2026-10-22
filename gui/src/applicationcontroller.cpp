#include "applicationcontroller.h"
#include "qmlnativeservices.h"
#include "qmlmainwindow.h"
#include "setupbridge.h"
#include <QApplication>
#include <QEvent>
#include <QPointer>
#include <QTimer>

ApplicationController::ApplicationController(QmlNativeServices *services, bool exitOnStreamEnd)
    : services(services), setup(services->backend()), exitOnStreamEnd(exitOnStreamEnd)
{
    auto *backend = services->backend();
    connect(backend, &QmlBackend::streamWindowRequested, this, &ApplicationController::prepareStreamWindow);
    connect(backend, &QmlBackend::connectionAttemptChanged, this, &ApplicationController::returnToSetup);
    connect(backend, &QmlBackend::sessionChanged, this, &ApplicationController::returnToSetup);
    connect(&setup, &SetupWindow::closed, qApp, &QApplication::quit);
}

ApplicationController::~ApplicationController() = default;

void ApplicationController::prepareStreamWindow()
{
    if (stream) return;
    returnRequested = false;
    stream = std::make_unique<QmlMainWindow>(services->backend(), services->engine(), exitOnStreamEnd);
    if (!stream->initializationError().isEmpty()) {
        setup.bridge()->reportStreamError(tr("Stream window is unavailable"), stream->initializationError());
        returnRequested = true;
        returnToSetup();
        return;
    }
    stream->installEventFilter(this);
    const QPointer<QmlMainWindow> window(stream.get());
    connect(window, &QmlMainWindow::streamWindowReady, this, [this, window] {
        if (!window || stream.get() != window || returnRequested || !setup.isVisible()) return;
        setup.suspend();
        stream->raise();
        stream->requestActivate();
    });
    connect(window, &QmlMainWindow::initializationFailed, this, [this, window](const QString &reason) {
        if (!window || stream.get() != window) return;
        setup.bridge()->reportStreamError(tr("Stream window is unavailable"), reason);
        returnRequested = true;
        services->backend()->stopSession(false);
        returnToSetup();
    });
    connect(window, &QmlMainWindow::streamViewDismissed, this, [this, window] {
        if (!window || stream.get() != window) return;
        returnRequested = true;
        returnToSetup();
    });
    stream->show();
}

bool ApplicationController::eventFilter(QObject *object, QEvent *event)
{
    if (object == stream.get() && event->type() == QEvent::Hide) {
        returnRequested = true;
        returnToSetup();
    }
    return QObject::eventFilter(object, event);
}

void ApplicationController::returnToSetup()
{
    auto *backend = services->backend();
    if (!stream || !returnRequested || returnQueued || backend->qmlSession()
            || backend->connectionAttempt().value("cleanupPending").toBool()) return;
    returnQueued = true;
    // The QML callback and native completion signals must return before their window is deleted.
    QTimer::singleShot(0, this, [this] {
        returnQueued = false;
        if (!stream || !returnRequested || services->backend()->qmlSession()
                || services->backend()->connectionAttempt().value("cleanupPending").toBool()) return;
        stream.reset();
        returnRequested = false;
        setup.resume();
    });
}
