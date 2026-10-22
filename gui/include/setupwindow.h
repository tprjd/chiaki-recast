#pragma once

#include <QWidget>

class QmlBackend;
class SetupBridge;
class QWebEngineView;
class QCloseEvent;
class PsnSignInWindow;

class SetupWindow : public QWidget
{
    Q_OBJECT
public:
    explicit SetupWindow(QmlBackend *backend, QWidget *parent = nullptr);
    QWebEngineView *view() const { return webView; }
    SetupBridge *bridge() const { return setupBridge; }

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    QmlBackend *backend;
    QWebEngineView *webView;
    SetupBridge *setupBridge;
    PsnSignInWindow *signInWindow = nullptr;
};
