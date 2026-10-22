#include "setupbridge.h"
#include "qmlbackend.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include <cmath>

SetupBridge::SetupBridge(QmlBackend *backend, QObject *parent)
    : QObject(parent), backend(backend)
{
    connect(backend, &QmlBackend::hostsChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::discoveryEnabledChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::streamerModeChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::currentProfileChanged, this, &SetupBridge::refresh);
    auto *settings = backend->qmlSettings();
    connect(settings, &QmlSettings::disconnectActionChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::suspendActionChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::audioVideoDisabledChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::streamMenuEnabledChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::streamMenuShortcut1Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::streamMenuShortcut2Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::streamMenuShortcut3Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::streamMenuShortcut4Changed, this, &SetupBridge::refresh);
#ifdef CHIAKI_GUI_ENABLE_STEAMDECK_NATIVE
    connect(settings, &QmlSettings::steamDeckHapticsChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::verticalDeckChanged, this, &SetupBridge::refresh);
#endif
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
        QT_TR_NOOP("Remote connection via PSN"), QT_TR_NOOP("Console"), QT_TR_NOOP("Console unavailable"), QT_TR_NOOP("Selected"), QT_TR_NOOP("Loading consoles"), QT_TR_NOOP("Setup unavailable")};
    for (const auto *text : texts) strings.insert(QString::fromUtf8(text), tr(text));
    auto *settings = backend->qmlSettings();
    QVariantMap general{{"disconnectAction", settings->disconnectAction()}, {"suspendAction", settings->suspendAction()},
        {"audioVideoDisabled", settings->audioVideoDisabled()}, {"streamerMode", privateMode},
        {"streamMenuEnabled", settings->streamMenuEnabled()}, {"streamMenuShortcut1", settings->streamMenuShortcut1()},
        {"streamMenuShortcut2", settings->streamMenuShortcut2()}, {"streamMenuShortcut3", settings->streamMenuShortcut3()},
        {"streamMenuShortcut4", settings->streamMenuShortcut4()}};
#ifdef CHIAKI_GUI_ENABLE_STEAMDECK_NATIVE
    general.insert("steamDeckHaptics", settings->steamDeckHaptics());
    general.insert("verticalDeck", settings->verticalDeck());
#endif
    state = {{"revision", QString::number(++revision)}, {"profileGeneration", QString::number(generation)},
        {"consoles", consoles}, {"selectedKey", selectedKey}, {"discoveryEnabled", backend->discoveryEnabled()},
        {"streamerMode", privateMode}, {"generalSettings", general}, {"capabilities", QVariantMap{{"discovery", true}}}, {"strings", strings}};
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

QVariantMap SetupBridge::setGeneralSetting(const QString &name, const QVariant &value, const QString &profileGeneration)
{
    const auto reject = [this](const char *message) { return QVariantMap{{"accepted", false}, {"message", tr(message)}}; };
    if (profileGeneration != QString::number(backend->profileGeneration()))
        return reject(QT_TR_NOOP("The profile changed. Try again in the current profile."));
    auto *settings = backend->qmlSettings();
    const bool boolean = name == "streamerMode" || name == "streamMenuEnabled"
#ifdef CHIAKI_GUI_ENABLE_STEAMDECK_NATIVE
        || name == "steamDeckHaptics" || name == "verticalDeck"
#endif
        ;
    int maximum = -1;
    if (name == "disconnectAction") maximum = 2;
    else if (name == "suspendAction") maximum = 1;
    else if (name == "audioVideoDisabled") maximum = 3;
    else if (name == "streamMenuShortcut1" || name == "streamMenuShortcut2" || name == "streamMenuShortcut3" || name == "streamMenuShortcut4") maximum = 16;
    if (!boolean && maximum < 0) return reject(QT_TR_NOOP("This setting is not available."));
    if (boolean) {
        if (value.metaType().id() != QMetaType::Bool) return reject(QT_TR_NOOP("Choose a valid setting value."));
    } else {
        const auto type = value.metaType().id();
        if (type != QMetaType::Double && type != QMetaType::Int && type != QMetaType::UInt && type != QMetaType::LongLong)
            return reject(QT_TR_NOOP("Choose a valid setting value."));
        const double number = value.toDouble();
        if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > maximum)
            return reject(QT_TR_NOOP("Choose a valid setting value."));
    }
    if (name == "disconnectAction") settings->setDisconnectAction(value.toInt());
    else if (name == "suspendAction") settings->setSuspendAction(value.toInt());
    else if (name == "audioVideoDisabled") settings->setAudioVideoDisabled(value.toInt());
    else if (name == "streamerMode") settings->setStreamerMode(value.toBool());
    else if (name == "streamMenuEnabled") settings->setStreamMenuEnabled(value.toBool());
    else if (name == "streamMenuShortcut1") settings->setStreamMenuShortcut1(value.toUInt());
    else if (name == "streamMenuShortcut2") settings->setStreamMenuShortcut2(value.toUInt());
    else if (name == "streamMenuShortcut3") settings->setStreamMenuShortcut3(value.toUInt());
    else if (name == "streamMenuShortcut4") settings->setStreamMenuShortcut4(value.toUInt());
#ifdef CHIAKI_GUI_ENABLE_STEAMDECK_NATIVE
    else if (name == "steamDeckHaptics") settings->setSteamDeckHaptics(value.toBool());
    else if (name == "verticalDeck") settings->setVerticalDeck(value.toBool());
#endif
    return {{"accepted", true}};
}
