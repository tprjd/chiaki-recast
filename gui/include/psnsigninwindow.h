#pragma once

#include <QWidget>

class QmlBackend;
class QWebEngineView;
class QWebEngineProfile;
class QCloseEvent;

class PsnSignInWindow : public QWidget
{
    Q_OBJECT
public:
    explicit PsnSignInWindow(QmlBackend *backend, QWidget *parent = nullptr);
    ~PsnSignInWindow() override;
    void start();
    QWebEngineView *view() const { return browser; }
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    QmlBackend *backend;
    QWebEngineView *browser;
    QWebEngineProfile *profile;
    quint64 generation;
    bool clearOnClose;
    bool cleaned = false;
    void cleanup();
};
