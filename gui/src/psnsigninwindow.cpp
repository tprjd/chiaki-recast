#include "psnsigninwindow.h"
#include "qmlbackend.h"
#include <QCloseEvent>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWebEngineCookieStore>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

namespace {
class SignInPage : public QWebEnginePage
{
    QmlBackend *backend;
    quint64 generation;
public:
    SignInPage(QWebEngineProfile *profile, QmlBackend *backend, quint64 generation, QObject *parent)
        : QWebEnginePage(profile, parent), backend(backend), generation(generation) {}
protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType, bool mainFrame) override
    {
        if (generation != backend->profileGeneration()) return false;
        if (mainFrame && backend->checkPsnRedirectURL(url)) {
            backend->startPsnAuth(url);
            return false;
        }
        return url.scheme() == "https" && url.userInfo().isEmpty();
    }
    QWebEnginePage *createWindow(WebWindowType) override { return nullptr; }
};
}

PsnSignInWindow::PsnSignInWindow(QmlBackend *backend, QWidget *parent)
    : QWidget(parent, Qt::Window), backend(backend), browser(new QWebEngineView(this)),
      profile(new QWebEngineProfile("psn-token", this)), generation(backend->profileGeneration()),
      clearOnClose(backend->nativeSettings()->GetRemotePlayAsk())
{
    setObjectName("psnSignInWindow");
    setWindowTitle(tr("PlayStation Network sign-in"));
    resize(920, 760);
    setMinimumSize(320, 300);
    auto *layout = new QVBoxLayout(this);
    auto *status = new QLabel(tr("Sign in to PlayStation Network. You can use an external browser if this page does not load."), this);
    status->setWordWrap(true);
    layout->addWidget(status);
    // This persistent sign-in profile deliberately has no setup WebChannel.
    auto *page = new SignInPage(profile, backend, generation, browser);
    browser->setPage(page);
    page->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    page->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    page->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    backend->setWebEngineHints(profile);
    layout->addWidget(browser, 1);
    auto *reload = new QPushButton(tr("Reload and clear cookies"), this);
    auto *external = new QPushButton(tr("Use external browser"), this);
    auto *closeButton = new QPushButton(tr("Close"), this);
    layout->addWidget(reload);
    layout->addWidget(external);
    layout->addWidget(closeButton);
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    connect(external, &QPushButton::clicked, this, [this] {
        if (generation != this->backend->profileGeneration()) return;
        browser->stop();
        this->backend->openPsnLink();
        close();
    });
    connect(reload, &QPushButton::clicked, this, [this] {
        profile->cookieStore()->deleteAllCookies();
        profile->clearHttpCache();
    });
    connect(profile, &QWebEngineProfile::clearHttpCacheCompleted, this, [this] {
        if (!cleaned && isVisible() && generation == this->backend->profileGeneration()) browser->reload();
    });
    connect(browser, &QWebEngineView::loadFinished, this, [this, status](bool ok) {
        if (!ok && this->backend->psnAuthAttempt().value("phase").toString().isEmpty())
            status->setText(tr("The sign-in page did not load. You can retry or use an external browser."));
    });
    connect(backend->qmlSettings(), &QmlSettings::currentProfileChanged, this, &QWidget::close);
    connect(backend, &QmlBackend::psnAuthAttemptChanged, this, [this, status, reload, external] {
        if (generation != this->backend->profileGeneration()) return;
        const auto attempt = this->backend->psnAuthAttempt();
        const auto phase = attempt.value("phase").toString();
        const bool busy = phase == "exchanging" || phase == "account";
        reload->setEnabled(!busy);
        external->setEnabled(!busy);
        if (busy) {
            browser->stop();
            status->setText(phase == "account" ? tr("Retrieving your Account ID") : tr("Completing PSN sign-in"));
        } else if (phase == "success") {
            clearOnClose = false;
            close();
        } else if (phase == "failure") status->setText(attempt.value("message").toString());
    });
}

void PsnSignInWindow::start()
{
    if (cleaned || generation != backend->profileGeneration()) return;
    browser->load(backend->psnLoginUrl());
    show();
    raise();
    activateWindow();
}

void PsnSignInWindow::closeEvent(QCloseEvent *event)
{
    cleanup();
    QWidget::closeEvent(event);
}

PsnSignInWindow::~PsnSignInWindow() { cleanup(); }

void PsnSignInWindow::cleanup()
{
    if (cleaned) return;
    cleaned = true;
    browser->stop();
    if (clearOnClose) profile->cookieStore()->deleteAllCookies();
    profile->clearHttpCache();
}
