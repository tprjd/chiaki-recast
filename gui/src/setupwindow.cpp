#include "setupwindow.h"
#include "setupbridge.h"
#include "qmlbackend.h"
#include "psnsigninwindow.h"
#include "keyboardmousebindings.h"
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPointer>
#include <QTimer>
#include <functional>
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

class BindingCapture : public QDialog
{
    QmlSettings *settings;
    QLabel *prompt;
    QLabel *status;
    QPushButton *move;
    int action, slot, pending = 0;
    qint64 pressed = 0;
    Qt::MouseButton mousePressed = Qt::NoButton;
    bool legacy;
    std::function<bool()> current;

    void offer(int input, bool replace = false)
    {
        if (!current()) { reject(); return; }
        if (legacy) {
            settings->changeControllerKey(action, input);
            emit settings->controllerMappingChanged();
            accept();
            return;
        }
        const auto result = settings->changeKeyboardMouseBinding(action, slot, input, replace);
        status->setText(result.value("error").toString());
        if (!status->text().isEmpty()) return;
        if (result.value("conflict").toInt() >= 0) {
            pending = input;
            status->setText(tr("This input is assigned to %1. Move binding or cancel.").arg(result.value("name").toString()));
            prompt->setEnabled(false);
            findChild<QPushButton *>("setupBindingTab")->hide();
            findChild<QPushButton *>("setupBindingClear")->hide();
            move->show();
            move->setFocus(Qt::TabFocusReason);
            return;
        }
        accept();
    }

    bool eventFilter(QObject *object, QEvent *event) override
    {
        if (object != prompt || !prompt->isEnabled()) return QDialog::eventFilter(object, event);
        if (event->type() == QEvent::FocusOut) { pressed = 0; mousePressed = Qt::NoButton; }
        if (event->type() == QEvent::ShortcutOverride) {
            event->accept();
            return true;
        }
        if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
            auto *key = static_cast<QKeyEvent *>(event);
            if (!legacy && (key->key() == Qt::Key_Tab || key->key() == Qt::Key_Backtab)) return false;
            const auto identity = KeyboardMousePhysicalKey(key->key(), key->nativeScanCode());
            if (!key->isAutoRepeat()) {
                if (event->type() == QEvent::KeyPress) pressed = identity;
                else if (pressed && pressed == identity) {
                    pressed = 0;
                    if (key->key() == Qt::Key_Escape) reject();
                    else offer(key->key());
                }
            }
            return true;
        }
        if (!legacy && (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease)) {
            auto *mouse = static_cast<QMouseEvent *>(event);
            if (event->type() == QEvent::MouseButtonPress) { mousePressed = mouse->button(); prompt->setFocus(); }
            else if (mousePressed == mouse->button()) { mousePressed = Qt::NoButton; offer(-int(mouse->button())); }
            return true;
        }
        return QDialog::eventFilter(object, event);
    }

    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::WindowDeactivate) reject();
        return QDialog::event(event);
    }

public:
    BindingCapture(QmlSettings *settings, int action, int slot, bool legacy, std::function<bool()> current, QWidget *parent)
        : QDialog(parent), settings(settings), action(action), slot(slot), legacy(legacy), current(std::move(current))
    {
        setObjectName("setupBindingCapture");
        setWindowTitle(tr("Assign binding"));
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowModality(Qt::WindowModal);
        auto *layout = new QVBoxLayout(this);
        prompt = new QLabel(legacy ? tr("Press a key to assign it. Press Escape or Cancel to cancel.")
            : tr("Press a key, or click this box with a mouse button. Escape cancels. Tab moves focus. Use Assign Tab to bind Tab."), this);
        prompt->setObjectName("setupBindingPrompt");
        prompt->setWordWrap(true);
        prompt->setMargin(16);
        prompt->setFocusPolicy(Qt::StrongFocus);
        prompt->setAccessibleName(prompt->text());
        prompt->installEventFilter(this);
        layout->addWidget(prompt);
        status = new QLabel(this);
        status->setObjectName("setupBindingStatus");
        status->setWordWrap(true);
        layout->addWidget(status);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
        buttons->setOrientation(Qt::Vertical);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
        if (!legacy) {
            auto *tab = buttons->addButton(tr("Assign Tab"), QDialogButtonBox::ActionRole);
            tab->setObjectName("setupBindingTab");
            connect(tab, &QPushButton::clicked, this, [this] { offer(Qt::Key_Tab); });
            auto *clear = buttons->addButton(tr("Clear binding"), QDialogButtonBox::ActionRole);
            clear->setObjectName("setupBindingClear");
            connect(clear, &QPushButton::clicked, this, [this] { offer(0); });
            move = buttons->addButton(tr("Move binding"), QDialogButtonBox::ActionRole);
            move->setObjectName("setupBindingMove");
            move->hide();
            connect(move, &QPushButton::clicked, this, [this] { offer(pending, true); });
        }
        for (auto *button : buttons->findChildren<QPushButton *>()) button->setAutoDefault(false);
        layout->addWidget(buttons);
        resize(qMin(500, parent->width() - 32), sizeHint().height());
        prompt->setFocus(Qt::TabFocusReason);
    }
};

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
    connect(setupBridge, &SetupBridge::keyboardMouseCaptureRequested, this,
        [this](int action, int slot, bool legacy, const QString &form, const QString &profile) {
            if (!isVisible() || !setupBridge->keyboardMouseFormValid(form, profile)) return;
            // A second activation replaces capture, but never replaces the settings presentation.
            emit setupBridge->keyboardMouseClosed();
            auto *capture = new BindingCapture(this->backend->qmlSettings(), action, slot, legacy,
                [this, form, profile, legacy] {
                    const bool cameraEnabled = this->backend->qmlSettings()->mouseCameraEnabled();
                    return setupBridge->keyboardMouseFormValid(form, profile)
                        && (legacy ? !cameraEnabled : cameraEnabled);
                }, this);
            connect(setupBridge, &SetupBridge::keyboardMouseClosed, capture, &QDialog::reject);
            connect(capture, &QDialog::finished, this, [this, form, profile] {
                if (isVisible() && setupBridge->keyboardMouseFormValid(form, profile)) webView->setFocus(Qt::TabFocusReason);
            });
            capture->open();
        });
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
    qApp->installEventFilter(this);
    setMinimumSize(320, 240);
    const auto geometry = backend->nativeSettings()->GetGeometry();
    if (geometry.isEmpty()) resize(1280, 720);
    else setGeometry(geometry);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(webView);
    webProfile = new QWebEngineProfile(this);
    webProfile->setHttpCacheType(QWebEngineProfile::MemoryHttpCache);
    webProfile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    webProfile->setUrlRequestInterceptor(new SetupRequests(webProfile));
    backend->setSetupNavigationTarget(webView);
    webView->setContextMenuPolicy(Qt::NoContextMenu);
    createPage();
}

void SetupWindow::invalidatePage()
{
    setupBridge->invalidateControllerMapping();
    setupBridge->invalidateKeyboardMouse();
    setupBridge->invalidateSteamShortcut();
    if (signInWindow) signInWindow->close();
}

void SetupWindow::resetChannel(QWebEnginePage *page)
{
    auto *old = page->webChannel();
    page->setWebChannel(nullptr);
    if (old) {
        old->deregisterObject(setupBridge);
        old->deleteLater();
    }
    auto *channel = new QWebChannel(page);
    channel->registerObject(QStringLiteral("setup"), setupBridge);
    page->setWebChannel(channel);
}

void SetupWindow::createPage()
{
    QPointer<QWebEnginePage> old = webView->page();
    auto *page = new SetupPage(webProfile, webView);
    webView->setPage(page);
    if (old) old->deleteLater();
    pageFailed = false;
    connect(page, &QWebEnginePage::loadStarted, this, [this, page] {
        if (webView->page() != page) return;
        invalidatePage();
        resetChannel(page);
    });
    connect(page, &QWebEnginePage::renderProcessTerminated, this, [this, page] {
        if (webView->page() != page) return;
        invalidatePage();
        auto *channel = page->webChannel();
        page->setWebChannel(nullptr);
        if (channel) { channel->deregisterObject(setupBridge); channel->deleteLater(); }
        pageFailed = true;
        // Hidden setup stays inactive until native playback returns to setup.
        if (isVisible()) QTimer::singleShot(0, this, [this, page] {
            if (pageFailed && webView->page() == page && isVisible()) createPage();
        });
    });
    connect(page, &QWebEnginePage::recommendedStateChanged, this, [this, page](QWebEnginePage::LifecycleState state) {
        if (!isVisible() && state != QWebEnginePage::LifecycleState::Active)
            page->setLifecycleState(QWebEnginePage::LifecycleState::Frozen);
    });
    page->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    page->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    page->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    page->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    page->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, false);
    resetChannel(page);
    webView->load(setupUrl);
}

bool SetupWindow::eventFilter(QObject *object, QEvent *event)
{
    if ((object == webView || object == webView->focusProxy()) && isVisible() && isActiveWindow()
        && (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)) {
        auto *key = static_cast<QKeyEvent *>(event);
        const auto state = setupBridge->snapshot();
        const auto reminder = state.value("reminder").toMap().value("id").toString();
        if (key->key() == Qt::Key_Yes && !reminder.isEmpty()) {
            if (event->type() == QEvent::KeyPress && !key->isAutoRepeat())
                setupBridge->reminderAction(reminder, "later", state.value("profileGeneration").toString());
            return true;
        }
    }
    return QWidget::eventFilter(object, event);
}

void SetupWindow::suspend()
{
    if(signInWindow)
        signInWindow->close();
    setupBridge->setSuspended(true);
    hide();
    auto *page = webView->page();
    if (page->recommendedState() != QWebEnginePage::LifecycleState::Active)
        page->setLifecycleState(QWebEnginePage::LifecycleState::Frozen);
}

void SetupWindow::resume()
{
    if (pageFailed) createPage();
    webView->page()->setLifecycleState(QWebEnginePage::LifecycleState::Active);
    setupBridge->setSuspended(false);
    show();
    raise();
    activateWindow();
    webView->setFocus();
}

void SetupWindow::closeEvent(QCloseEvent *event)
{
    invalidatePage();
    backend->nativeSettings()->SetGeometry(geometry());
    QWidget::closeEvent(event);
    if (event->isAccepted()) emit closed();
}

void SetupWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    setupBridge->setKeyboardMouseAvailable(true);
}

void SetupWindow::hideEvent(QHideEvent *event)
{
    setupBridge->setKeyboardMouseAvailable(false);
    QWidget::hideEvent(event);
}
