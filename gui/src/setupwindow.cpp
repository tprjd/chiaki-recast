#include "setupwindow.h"
#include "setupbridge.h"
#include "qmlbackend.h"
#include "psnsigninwindow.h"
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineUrlRequestInfo>
#include <QWebEngineView>

namespace {
const QUrl setupUrl(QStringLiteral("qrc:/setup/index.html"));

class SetupRequests : public QWebEngineUrlRequestInterceptor
{
public:
    using QWebEngineUrlRequestInterceptor::QWebEngineUrlRequestInterceptor;
    void interceptRequest(QWebEngineUrlRequestInfo &request) override
    {
        const auto url = request.requestUrl();
        const auto path = url.path();
        request.block(url.scheme() != "qrc" || !url.host().isEmpty() || url.hasQuery()
            || !(path == "/setup/index.html" || path == "/setup/app.js" || path == "/setup/app.css"
                || path == "/qtwebchannel/qwebchannel.js" || path == "/icons/console-ps4.svg"
                || path == "/icons/console-ps5.svg" || path == "/icons/chiaking-logo-white.svg"));
    }
};

class SetupPage : public QWebEnginePage
{
public:
    using QWebEnginePage::QWebEnginePage;
protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType, bool mainFrame) override
    {
        return mainFrame && url == setupUrl;
    }
    QWebEnginePage *createWindow(WebWindowType) override { return nullptr; }
};
}

SetupWindow::SetupWindow(QmlBackend *backend, QWidget *parent)
    : QWidget(parent), backend(backend), webView(new QWebEngineView(this)), setupBridge(new SetupBridge(backend, this))
{
    const auto updateTitle = [this] {
        const auto profile = this->backend->nativeSettings()->GetCurrentProfile();
        setWindowTitle(this->backend->qmlSettings()->streamerMode() || profile.isEmpty()
            ? tr("Chiaki Recast") : QStringLiteral("Chiaki Recast: %1").arg(profile));
    };
    connect(backend->qmlSettings(), &QmlSettings::streamerModeChanged, this, updateTitle);
    connect(backend->qmlSettings(), &QmlSettings::currentProfileChanged, this, updateTitle);
    updateTitle();
    connect(setupBridge, &SetupBridge::psnSignInRequested, this, [this] {
        if (!signInWindow) {
            signInWindow = new PsnSignInWindow(this->backend, this);
            signInWindow->setAttribute(Qt::WA_DeleteOnClose);
            connect(signInWindow, &QObject::destroyed, this, [this] { signInWindow = nullptr; });
            signInWindow->start();
        } else {
            signInWindow->start();
        }
    });
    setMinimumSize(320, 240);
    const auto geometry = backend->nativeSettings()->GetGeometry();
    if (geometry.isEmpty()) resize(1280, 720);
    else setGeometry(geometry);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(webView);
    auto *profile = new QWebEngineProfile(this);
    profile->setHttpCacheType(QWebEngineProfile::MemoryHttpCache);
    profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    profile->setUrlRequestInterceptor(new SetupRequests(profile));
    auto *page = new SetupPage(profile, webView);
    webView->setPage(page);
    page->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    page->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    page->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    page->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    page->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, false);
    auto *channel = new QWebChannel(page);
    channel->registerObject(QStringLiteral("setup"), setupBridge);
    page->setWebChannel(channel);
    webView->setContextMenuPolicy(Qt::NoContextMenu);
    webView->load(setupUrl);
}

void SetupWindow::closeEvent(QCloseEvent *event)
{
    if (signInWindow) signInWindow->close();
    backend->nativeSettings()->SetGeometry(geometry());
    QWidget::closeEvent(event);
}
