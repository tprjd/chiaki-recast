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
    decoders = settings->availableDecoders();
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
    connect(settings, &QmlSettings::windowTypeChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::customResolutionWidthChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::customResolutionHeightChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::audioBufferSizeChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::audioVolumeChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::wifiDroppedNotifChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::packetLossReportedMaxChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::resolutionLocalPS4Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::fpsLocalPS4Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::bitrateLocalPS4Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::resolutionRemotePS4Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::fpsRemotePS4Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::bitrateRemotePS4Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::resolutionLocalPS5Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::fpsLocalPS5Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::bitrateLocalPS5Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::codecLocalPS5Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::resolutionRemotePS5Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::fpsRemotePS5Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::bitrateRemotePS5Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::codecRemotePS5Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::useZeroCopyChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::fullscreenDoubleClickChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::hideCursorChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::startMicUnmutedChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::iDROnFECFailureEnabledChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::showStreamStatsChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::decoderChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::audioInDeviceChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::audioOutDeviceChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::audioDevicesChanged, this, &SetupBridge::refresh);
#if CHIAKI_GUI_ENABLE_SPEEX
    connect(settings, &QmlSettings::speechProcessingChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::noiseSuppressLevelChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::echoSuppressLevelChanged, this, &SetupBridge::refresh);
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
    QVariantMap media{{"windowType", settings->windowType()},
        {"customResolutionWidth", settings->customResolutionWidth()},
        {"customResolutionHeight", settings->customResolutionHeight()},
        {"audioBufferSize", settings->audioBufferSize()},
        {"audioVolume", settings->audioVolume()},
        {"wifiDroppedNotif", settings->wifiDroppedNotif()},
        {"packetLossReportedMax", settings->packetLossReportedMax()},
        {"resolutionLocalPS4", settings->resolutionLocalPS4()},
        {"fpsLocalPS4", settings->fpsLocalPS4()},
        {"bitrateLocalPS4", settings->bitrateLocalPS4()},
        {"resolutionRemotePS4", settings->resolutionRemotePS4()},
        {"fpsRemotePS4", settings->fpsRemotePS4()},
        {"bitrateRemotePS4", settings->bitrateRemotePS4()},
        {"resolutionLocalPS5", settings->resolutionLocalPS5()},
        {"fpsLocalPS5", settings->fpsLocalPS5()},
        {"bitrateLocalPS5", settings->bitrateLocalPS5()},
        {"codecLocalPS5", settings->codecLocalPS5()},
        {"resolutionRemotePS5", settings->resolutionRemotePS5()},
        {"fpsRemotePS5", settings->fpsRemotePS5()},
        {"bitrateRemotePS5", settings->bitrateRemotePS5()},
        {"codecRemotePS5", settings->codecRemotePS5()},
        {"useZeroCopy", settings->useZeroCopy()},
        {"fullscreenDoubleClick", settings->fullscreenDoubleClick()},
        {"hideCursor", settings->hideCursor()},
        {"startMicUnmuted", settings->startMicUnmuted()},
        {"iDROnFECFailureEnabled", settings->iDROnFECFailureEnabled()},
        {"showStreamStats", settings->showStreamStats()},
        {"decoder", settings->decoder().isEmpty() ? QStringLiteral("none") : settings->decoder()},
        {"audioInDevice", settings->audioInDevice()},
        {"audioOutDevice", settings->audioOutDevice()}};
#if CHIAKI_GUI_ENABLE_SPEEX
    media.insert("speechProcessing", settings->speechProcessing());
    media.insert("noiseSuppressLevel", settings->noiseSuppressLevel());
    media.insert("echoSuppressLevel", settings->echoSuppressLevel());
#endif
    QVariantMap choices;
    const auto addChoices = [&choices](const QString &key, const QStringList &available, const QString &saved, bool audio) {
        QVariantList list;
        if (audio) list.append(QVariantMap{{"value", ""}, {"label", tr("Auto")}});
        for (const auto &name : available) list.append(QVariantMap{{"value", name}, {"label", name}});
        if (!saved.isEmpty() && !available.contains(saved))
            list.append(QVariantMap{{"value", saved}, {"label", saved + tr(" (unavailable)")}});
        choices.insert(key, list);
    };
    addChoices("decoder", decoders, settings->decoder(), false);
    addChoices("audioInDevice", settings->availableAudioInDevices(), settings->audioInDevice(), true);
    addChoices("audioOutDevice", settings->availableAudioOutDevices(), settings->audioOutDevice(), true);
    state = {{"revision", QString::number(++revision)}, {"profileGeneration", QString::number(generation)},
        {"consoles", consoles}, {"selectedKey", selectedKey}, {"discoveryEnabled", backend->discoveryEnabled()},
        {"streamerMode", privateMode}, {"mediaSettings", media}, {"mediaChoices", choices}, {"generalSettings", general}, {"capabilities", QVariantMap{{"discovery", true}}}, {"strings", strings}};
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

QVariantMap SetupBridge::refreshAudioDevices()
{
    backend->qmlSettings()->refreshAudioDevices();
    return {{"accepted", true}};
}

QVariantMap SetupBridge::setMediaSetting(const QString &name, const QVariant &value, const QString &profileGeneration)
{
    const auto reject = [this](const char *message) { return QVariantMap{{"accepted", false}, {"message", tr(message)}}; };
    if (profileGeneration != QString::number(backend->profileGeneration()))
        return reject(QT_TR_NOOP("The profile changed. Try again in the current profile."));
    auto *settings = backend->qmlSettings();
    const auto invalid = [&] { return reject(QT_TR_NOOP("Choose a valid setting value.")); };
    // Each accepted name has an explicit native setter. Private QObject properties are never writable here.
    if (name == "decoder") {
        if (value.metaType().id() != QMetaType::QString) return invalid();
        const auto selected = value.toString();
        if (!decoders.contains(selected)) return invalid();
        settings->setDecoder(selected == "none" ? QString() : selected);
        return {{"accepted", true}};
    }
    if (name == "audioInDevice") {
        if (value.metaType().id() != QMetaType::QString) return invalid();
        const auto selected = value.toString();
        if (!selected.isEmpty() && !settings->availableAudioInDevices().contains(selected)) return invalid();
        settings->setAudioInDevice(selected);
        return {{"accepted", true}};
    }
    if (name == "audioOutDevice") {
        if (value.metaType().id() != QMetaType::QString) return invalid();
        const auto selected = value.toString();
        if (!selected.isEmpty() && !settings->availableAudioOutDevices().contains(selected)) return invalid();
        settings->setAudioOutDevice(selected);
        return {{"accepted", true}};
    }
    const bool boolean = name == "useZeroCopy" || name == "fullscreenDoubleClick" || name == "hideCursor" || name == "startMicUnmuted" || name == "iDROnFECFailureEnabled" || name == "showStreamStats"
#if CHIAKI_GUI_ENABLE_SPEEX
        || name == "speechProcessing"
#endif
        ;
    if (boolean) {
        if (value.metaType().id() != QMetaType::Bool) return invalid();
        if (name == "useZeroCopy") settings->setUseZeroCopy(value.toBool());
        else if (name == "fullscreenDoubleClick") settings->setFullscreenDoubleClick(value.toBool());
        else if (name == "hideCursor") settings->setHideCursor(value.toBool());
        else if (name == "startMicUnmuted") settings->setStartMicUnmuted(value.toBool());
        else if (name == "iDROnFECFailureEnabled") settings->setIDROnFECFailureEnabled(value.toBool());
        else if (name == "showStreamStats") settings->setShowStreamStats(value.toBool());
#if CHIAKI_GUI_ENABLE_SPEEX
        else if (name == "speechProcessing") settings->setSpeechProcessing(value.toBool());
#endif
        return {{"accepted", true}};
    }
    const auto type = value.metaType().id();
    if (type != QMetaType::Double && type != QMetaType::Int && type != QMetaType::UInt && type != QMetaType::LongLong) return invalid();
    const double number = value.toDouble();
    if (!std::isfinite(number) || std::floor(number) != number) return invalid();
    if (name == "windowType") {
        if (number < 0 || number > 5) return invalid();
        settings->setWindowType(int(number));
        return {{"accepted", true}};
    }
    if (name == "customResolutionWidth") {
        if (number < 0 || number > 9999) return invalid();
        settings->setCustomResolutionWidth(int(number));
        return {{"accepted", true}};
    }
    if (name == "customResolutionHeight") {
        if (number < 0 || number > 9999) return invalid();
        settings->setCustomResolutionHeight(int(number));
        return {{"accepted", true}};
    }
    if (name == "audioBufferSize") {
        if (number < 1920 || number > 19200 || int(number) % 1920 != 0) return invalid();
        settings->setAudioBufferSize(int(number));
        return {{"accepted", true}};
    }
    if (name == "audioVolume") {
        if (number < 0 || number > 128) return invalid();
        settings->setAudioVolume(int(number));
        return {{"accepted", true}};
    }
    if (name == "wifiDroppedNotif") {
        if (number < 0 || number > 100) return invalid();
        settings->setWifiDroppedNotif(int(number));
        return {{"accepted", true}};
    }
    if (name == "packetLossReportedMax") {
        if (number < 0 || number > 100) return invalid();
        settings->setPacketLossReportedMax(int(number));
        return {{"accepted", true}};
    }
    if (name == "resolutionLocalPS4") {
        if (number < 1 || number > 4) return invalid();
        settings->setResolutionLocalPS4(int(number));
        settings->setBitrateLocalPS4(0);
        return {{"accepted", true}};
    }
    if (name == "fpsLocalPS4") {
        if (number < 30 || number > 60 || (number != 30 && number != 60)) return invalid();
        settings->setFpsLocalPS4(int(number));
        return {{"accepted", true}};
    }
    if (name == "bitrateLocalPS4") {
        if (number < 0 || number > 100000 || (number != 0 && (number < 2000 || int(number) % 1000 != 0))) return invalid();
        settings->setBitrateLocalPS4(int(number));
        return {{"accepted", true}};
    }
    if (name == "resolutionRemotePS4") {
        if (number < 1 || number > 4) return invalid();
        settings->setResolutionRemotePS4(int(number));
        settings->setBitrateRemotePS4(0);
        return {{"accepted", true}};
    }
    if (name == "fpsRemotePS4") {
        if (number < 30 || number > 60 || (number != 30 && number != 60)) return invalid();
        settings->setFpsRemotePS4(int(number));
        return {{"accepted", true}};
    }
    if (name == "bitrateRemotePS4") {
        if (number < 0 || number > 100000 || (number != 0 && (number < 2000 || int(number) % 1000 != 0))) return invalid();
        settings->setBitrateRemotePS4(int(number));
        return {{"accepted", true}};
    }
    if (name == "resolutionLocalPS5") {
        if (number < 1 || number > 4) return invalid();
        settings->setResolutionLocalPS5(int(number));
        settings->setBitrateLocalPS5(0);
        return {{"accepted", true}};
    }
    if (name == "fpsLocalPS5") {
        if (number < 30 || number > 60 || (number != 30 && number != 60)) return invalid();
        settings->setFpsLocalPS5(int(number));
        return {{"accepted", true}};
    }
    if (name == "bitrateLocalPS5") {
        if (number < 0 || number > 100000 || (number != 0 && (number < 2000 || int(number) % 1000 != 0))) return invalid();
        settings->setBitrateLocalPS5(int(number));
        return {{"accepted", true}};
    }
    if (name == "codecLocalPS5") {
        if (number < 0 || number > 2) return invalid();
        settings->setCodecLocalPS5(int(number));
        return {{"accepted", true}};
    }
    if (name == "resolutionRemotePS5") {
        if (number < 1 || number > 4) return invalid();
        settings->setResolutionRemotePS5(int(number));
        settings->setBitrateRemotePS5(0);
        return {{"accepted", true}};
    }
    if (name == "fpsRemotePS5") {
        if (number < 30 || number > 60 || (number != 30 && number != 60)) return invalid();
        settings->setFpsRemotePS5(int(number));
        return {{"accepted", true}};
    }
    if (name == "bitrateRemotePS5") {
        if (number < 0 || number > 100000 || (number != 0 && (number < 2000 || int(number) % 1000 != 0))) return invalid();
        settings->setBitrateRemotePS5(int(number));
        return {{"accepted", true}};
    }
    if (name == "codecRemotePS5") {
        if (number < 0 || number > 2) return invalid();
        settings->setCodecRemotePS5(int(number));
        return {{"accepted", true}};
    }
#if CHIAKI_GUI_ENABLE_SPEEX
    if (name == "noiseSuppressLevel" || name == "echoSuppressLevel") {
        if (!settings->speechProcessing() || number < 0 || number > 60) return invalid();
        if (name == "noiseSuppressLevel") settings->setNoiseSuppressLevel(int(number));
        else settings->setEchoSuppressLevel(int(number));
        return {{"accepted", true}};
    }
#endif
    return reject(QT_TR_NOOP("This setting is not available."));
}
