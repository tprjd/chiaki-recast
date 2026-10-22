#pragma once

#include <QWidget>

class QmlBackend;
class SetupBridge;
class QWebEngineView;
class QWebEnginePage;
class QWebEngineProfile;
class QCloseEvent;
class PsnSignInWindow;

class SetupWindow : public QWidget
{
    Q_OBJECT
public:
    explicit SetupWindow(QmlBackend *backend, QWidget *parent = nullptr);
    QWebEngineView *view() const { return webView; }
    SetupBridge *bridge() const { return setupBridge; }
    void suspend();
    void resume();

signals:
    void closed();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void createPage();
    void resetChannel(QWebEnginePage *page);
    void invalidatePage();
    QmlBackend *backend;
    QWebEngineView *webView;
    SetupBridge *setupBridge;
    QWebEngineProfile *webProfile = nullptr;
    bool pageFailed = false;
    PsnSignInWindow *signInWindow = nullptr;
};
