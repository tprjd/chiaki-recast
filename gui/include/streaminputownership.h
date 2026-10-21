#pragma once

#include <QObject>
#include <QHash>
#include <QSet>
#include <QPointer>
#include <QQuickItem>
#include <QQuickWindow>

class StreamInputOwnership : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool blocked READ blocked NOTIFY blockedChanged)
public:
    StreamInputOwnership(QWindow *host, QQuickWindow *content, QObject *parent = nullptr);
    bool blocked() const;
    Q_INVOKABLE QString present(QObject *owner, const QString &role, QQuickItem *focusTarget = nullptr);
    Q_INVOKABLE void dismiss(const QString &presentation);
    Q_INVOKABLE void settled(const QString &presentation);
    void sessionChanged(bool active, bool pendingPin, bool psn);
signals:
    void blockedChanged();
    void captureChanged(bool blocked);
private:
    friend class QmlMainWindow;
    quint64 captureGeneration = 0;
    struct Presentation {
        QPointer<QObject> owner;
        QString role;
        QPointer<QQuickItem> returnFocus;
        bool active = true;
        bool captured = true;
        bool settled = false;
        quint64 epoch = 0;
    };
    QPointer<QWindow> host;
    QPointer<QQuickWindow> content;
    QHash<QString, Presentation> presentations;
    QHash<QObject *, QString> current;
    QHash<QObject *, QPointer<QQuickItem>> noticeFocus;
    QSet<QObject *> watched;
    quint64 serial = 0;
    quint64 epoch = 0;
    quint64 focusGeneration = 0;
    bool sessionActive = false;
    void changed(bool previous);
    void retire(QObject *owner);
    void forget(const QString &id);
    bool returnEligible(const QString &id) const;
    void restore(const QString &id, bool deferred);
    void restoreNow(const QString &id);
    void focusNotice(const QString &id, QQuickItem *target);
};
