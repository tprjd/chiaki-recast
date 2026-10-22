#include "setupbridge.h"
#include "qmlbackend.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include <QDateTime>
#include <limits>
#include <QGuiApplication>
#include <QClipboard>

SetupBridge::SetupBridge(QmlBackend *backend, QObject *parent)
    : QObject(parent), backend(backend)
{
    connect(backend, &QmlBackend::hostsChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::discoveryEnabledChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::streamerModeChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::currentProfileChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::psnAuthAttemptChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::psnTokenChanged, this, [this] { credentialsRejected = false; refresh(); });
    connect(backend, &QmlBackend::psnCredsExpired, this, [this] { credentialsRejected = true; refresh(); });
    connect(backend->qmlSettings(), &QmlSettings::psnAuthTokenChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::psnRefreshTokenChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::psnAuthTokenExpiryChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::psnAccountIdChanged, this, &SetupBridge::refresh);
    accountExpiry.setSingleShot(true);
    connect(&accountExpiry, &QTimer::timeout, this, &SetupBridge::refresh);
    refresh();
}

QString SetupBridge::identity(const QVariantMap &host) const
{
    QJsonArray parts;
    if (host.value("discovered").toBool())
        parts = {"local", host.value("mac").toString().toLower()};
    else if (!host.value("duid").toString().isEmpty())
        parts = {"psn", host.value("duid").toString()};
    else if (host.value("manual").toBool())
        parts = {"manual", host.value("address").toString(), host.value("mac").toString().toLower()};
    if (parts.size() < 2 || parts.at(1).toString().isEmpty()) return {};
    return QString::fromUtf8(QJsonDocument(parts).toJson(QJsonDocument::Compact));
}

void SetupBridge::refresh()
{
    if (generation != backend->profileGeneration()) {
        generation = backend->profileGeneration();
        keys.clear();
        selectedKey.clear();
        credentialsRejected = false;
    }
    const auto hosts = backend->hosts();
    QHash<QString, int> counts;
    for (const auto &value : hosts) {
        const auto host = value.toMap();
        if (host.value("display").toBool()) ++counts[identity(host)];
    }
    QHash<QString, QString> currentKeys;
    QVariantList consoles;
    const bool privateMode = backend->qmlSettings()->streamerMode();
    for (const auto &value : hosts) {
        const auto host = value.toMap();
        if (!host.value("display").toBool()) continue;
        const auto id = identity(host);
        const bool available = !id.isEmpty() && counts.value(id) == 1;
        QString key;
        if (available) {
            key = keys.value(id);
            if (key.isEmpty()) key = QUuid::createUuid().toString(QUuid::WithoutBraces);
            currentKeys.insert(id, key);
        }
        QString name = host.value("name").toString();
        if (privateMode && name == host.value("address").toString()) name = tr("Manual console");
        if (name.isEmpty()) name = tr("Console");
        QVariantMap console{{"key", key}, {"name", name}, {"available", available},
            {"registered", host.value("registered").toBool()}, {"state", host.value("state").toString()},
            {"route", host.value("discovered").toBool() ? "local" : host.value("manual").toBool() ? "manual" : "psn"}};
        if (host.contains("ps5")) console.insert("ps5", host.value("ps5").toBool());
        consoles.append(console);
    }
    keys = currentKeys;
    if (!keys.values().contains(selectedKey)) {
        selectedKey.clear();
        for (const auto &console : consoles) {
            const auto key = console.toMap().value("key").toString();
            if (!key.isEmpty()) { selectedKey = key; break; }
        }
    }
    QVariantMap strings;
    const char *texts[] = {QT_TR_NOOP("Consoles"), QT_TR_NOOP("Search consoles"), QT_TR_NOOP("Discovery"), QT_TR_NOOP("Discovery is on"), QT_TR_NOOP("Discovery is off"),
        QT_TR_NOOP("No consoles found"), QT_TR_NOOP("No matching consoles"), QT_TR_NOOP("Turn on discovery to find consoles on your local network."),
        QT_TR_NOOP("Online"), QT_TR_NOOP("In standby"), QT_TR_NOOP("Status unknown"), QT_TR_NOOP("Not registered"), QT_TR_NOOP("Local network"), QT_TR_NOOP("Manual address"),
        QT_TR_NOOP("Remote connection via PSN"), QT_TR_NOOP("Console"), QT_TR_NOOP("Console unavailable"), QT_TR_NOOP("Selected"), QT_TR_NOOP("Loading consoles"), QT_TR_NOOP("Setup unavailable"),
        QT_TR_NOOP("PSN account"), QT_TR_NOOP("PSN remote access"), QT_TR_NOOP("Retrieving your Account ID"), QT_TR_NOOP("Completing PSN sign-in"),
        QT_TR_NOOP("PSN remote access is ready"), QT_TR_NOOP("PSN credentials expired"), QT_TR_NOOP("Not signed in"),
        QT_TR_NOOP("Sign-in saves PSN credentials and your Account ID in the current profile. These credentials allow remote access to your consoles."),
        QT_TR_NOOP("You can close this screen. Sign-in can still finish and save credentials in this profile while the app is open."),
        QT_TR_NOOP("Open embedded sign-in"), QT_TR_NOOP("Use external browser"), QT_TR_NOOP("Copy sign-in link"),
        QT_TR_NOOP("After sign-in in an external browser, copy the complete final URL from its address bar. Paste the full URL, including its authorization code, below."),
        QT_TR_NOOP("Sign-in redirect URL"), QT_TR_NOOP("Retry"), QT_TR_NOOP("Complete sign-in"), QT_TR_NOOP("Close"),
        QT_TR_NOOP("Use the same PlayStation account as your console. The Sony sign-in page and your external browser can show account details.")};
    for (const auto *text : texts) strings.insert(QString::fromUtf8(text), tr(text));
    const auto *settings = backend->nativeSettings();
    const bool configured = !settings->GetPsnAuthToken().isEmpty() && !settings->GetPsnRefreshToken().isEmpty()
        && !settings->GetPsnAccountId().isEmpty();
    const auto expiry = QDateTime::fromString(settings->GetPsnAuthTokenExpiry(), settings->GetTimeFormat());
    const auto remaining = QDateTime::currentDateTime().msecsTo(expiry);
    const bool expired = configured && (credentialsRejected || !expiry.isValid() || remaining <= 0);
    accountExpiry.stop();
    if (configured && !expired) accountExpiry.start(int(qMin<qint64>(remaining, std::numeric_limits<int>::max())));
    QVariantMap psn{{"status", configured ? (expired ? "expired" : "ready") : "signedOut"}};
    const auto attempt = backend->psnAuthAttempt();
    // Keep the bridge allowlist explicit: the native attempt also contains an Account ID.
    for (const auto &field : {"id", "phase", "message", "profileValid"})
        if (attempt.contains(field)) psn.insert(field, attempt.value(field));
    state = {{"revision", QString::number(++revision)}, {"profileGeneration", QString::number(generation)},
        {"consoles", consoles}, {"selectedKey", selectedKey}, {"discoveryEnabled", backend->discoveryEnabled()},
        {"streamerMode", privateMode}, {"psn", psn}, {"capabilities", QVariantMap{{"discovery", true}}}, {"strings", strings}};
    emit stateChanged(state);
}

bool SetupBridge::resolveConsole(const QString &key, int &index, QString &nickname) const
{
    if (key.isEmpty() || generation != backend->profileGeneration()) return false;
    const auto id = keys.key(key);
    if (id.isEmpty()) return false;
    const auto hosts = backend->hosts();
    int found = -1;
    for (int row = 0; row < hosts.size(); ++row) {
        const auto host = hosts.at(row).toMap();
        if (!host.value("display").toBool() || identity(host) != id) continue;
        if (found >= 0) return false;
        found = row;
        nickname = host.value("discovered").toBool() ? host.value("name").toString() : QString();
    }
    if (found < 0) return false;
    index = found;
    return true;
}

QVariantMap SetupBridge::selectConsole(const QString &key)
{
    int index;
    QString nickname;
    if (!resolveConsole(key, index, nickname)) return {{"accepted", false}, {"message", tr("Console unavailable")}};
    selectedKey = key;
    refresh();
    return {{"accepted", true}};
}

QVariantMap SetupBridge::setDiscoveryEnabled(bool enabled)
{
    backend->setDiscoveryEnabled(enabled);
    return {{"accepted", backend->discoveryEnabled() == enabled}};
}

bool SetupBridge::currentProfile(const QString &profile) const
{
    return profile == QString::number(backend->profileGeneration());
}

QVariantMap SetupBridge::openPsnSignIn(const QString &profile)
{
    if (!currentProfile(profile)) return {{"accepted", false}};
    const auto phase = backend->psnAuthAttempt().value("phase").toString();
    if (phase == "exchanging" || phase == "account") return {{"accepted", false}};
    emit psnSignInRequested();
    return {{"accepted", true}};
}

QVariantMap SetupBridge::openPsnExternal(const QString &profile)
{
    if (!currentProfile(profile)) return {{"accepted", false}};
    backend->openPsnLink();
    return {{"accepted", true}};
}

QVariantMap SetupBridge::copyPsnSignInLink(const QString &profile)
{
    if (!currentProfile(profile)) return {{"accepted", false}};
    QGuiApplication::clipboard()->setText(QString::fromUtf8(backend->psnLoginUrl().toEncoded()));
    return {{"accepted", true}};
}

QVariantMap SetupBridge::submitPsnRedirect(const QString &profile, const QString &url)
{
    if (!currentProfile(profile)) return {{"accepted", false}};
    const auto id = backend->startPsnAuth(QUrl(url));
    return {{"accepted", !id.isEmpty()}};
}
