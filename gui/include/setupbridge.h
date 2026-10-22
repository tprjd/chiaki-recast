#pragma once

#include <QObject>
#include <QHash>
#include <QVariantMap>

class QmlBackend;

class SetupBridge : public QObject
{
    Q_OBJECT
public:
    explicit SetupBridge(QmlBackend *backend, QObject *parent = nullptr);
    Q_INVOKABLE QVariantMap snapshot() const { return state; }
    Q_INVOKABLE QVariantMap setDiscoveryEnabled(bool enabled);
    Q_INVOKABLE QVariantMap selectConsole(const QString &key);
    // Native callers resolve immediately before dispatch; no row index crosses WebChannel.
    bool resolveConsole(const QString &key, int &index, QString &nickname) const;

signals:
    void stateChanged(const QVariantMap &state);

private:
    void refresh();
    QString identity(const QVariantMap &host) const;
    QmlBackend *backend;
    QHash<QString, QString> keys;
    quint64 generation = 0;
    quint64 revision = 0;
    QString selectedKey;
    QVariantMap state;
};
