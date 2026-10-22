#pragma once

#include <QObject>
#include <QHash>
#include <QVariantMap>
#include <QTimer>

class QmlBackend;

class SetupBridge : public QObject
{
    Q_OBJECT
public:
    explicit SetupBridge(QmlBackend *backend, QObject *parent = nullptr);
    Q_INVOKABLE QVariantMap snapshot() const { return state; }
    Q_INVOKABLE QVariantMap setDiscoveryEnabled(bool enabled);
    Q_INVOKABLE QVariantMap selectConsole(const QString &key);
    Q_INVOKABLE QVariantMap connectConsole(const QString &key);
    Q_INVOKABLE QVariantMap openRegistration(const QString &key, const QString &profile);
    Q_INVOKABLE QVariantMap submitRegistration(const QString &context, const QVariantMap &fields);
    Q_INVOKABLE QVariantMap connectRegistration(const QString &attempt);
    Q_INVOKABLE QVariantMap beginRegistrationAccount(const QString &context, const QString &source);
    Q_INVOKABLE QVariantMap lookupRegistrationAccount(const QString &context, const QString &route, const QString &username);
    Q_INVOKABLE QVariantMap readRegistrationAccount(const QString &context, const QString &route, const QString &request);
    Q_INVOKABLE QVariantMap endRegistrationAccount(const QString &context, const QString &route);
    void setSuspended(bool suspended);
    void reportStreamError(const QString &title, const QString &text);
    Q_INVOKABLE QVariantMap dismissStreamError(const QString &id);
    Q_INVOKABLE QVariantMap openPsnSignIn(const QString &profile);
    Q_INVOKABLE QVariantMap openPsnExternal(const QString &profile);
    Q_INVOKABLE QVariantMap copyPsnSignInLink(const QString &profile);
    Q_INVOKABLE QVariantMap submitPsnRedirect(const QString &profile, const QString &url);
    Q_INVOKABLE QVariantMap setGeneralSetting(const QString &name, const QVariant &value, const QString &profileGeneration);
    Q_INVOKABLE QVariantMap setMediaSetting(const QString &name, const QVariant &value, const QString &profileGeneration);
    Q_INVOKABLE QVariantMap refreshAudioDevices();
    Q_INVOKABLE QVariantMap setRendererSetting(const QString &name, const QVariant &value, const QString &profileGeneration);
    Q_INVOKABLE QVariantMap rendererAction(const QString &action, const QString &profileGeneration);
    // Native callers resolve immediately before dispatch; no row index crosses WebChannel.
    bool resolveConsole(const QString &key, int &index, QString &nickname) const;

signals:
    void stateChanged(const QVariantMap &state);
    void psnSignInRequested();

private:
    void refresh();
    QString identity(const QVariantMap &host) const;
    QmlBackend *backend;
    QHash<QString, QString> keys;
    quint64 generation = 0;
    quint64 revision = 0;
    QString selectedKey;
    QVariantMap state;
    bool suspended = false;
    QVariantMap streamError;
    quint64 streamErrorGeneration = 0;
    QStringList decoders;
    QTimer accountExpiry;
    bool credentialsRejected = false;
    int startupRendererBackend = 0;
    bool startupVSync = true;
    bool currentProfile(const QString &profile) const;
    QVariantMap registrationContext;
    QString registrationContextId;
    QString registrationAttemptContext;
    QString registrationAddress;
    QString accountRouteId;
    QString accountSource;
    QString accountRequest;
    QString accountAuthBaseline;
    bool registrationAccountCurrent(const QString &context, const QString &route) const;
};
