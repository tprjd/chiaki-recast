#include "setupbridge.h"
#include "qmlbackend.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include <QDateTime>
#include <limits>
#include <QGuiApplication>
#include <QClipboard>
#include <cmath>

SetupBridge::SetupBridge(QmlBackend *backend, QObject *parent)
    : QObject(parent), backend(backend)
{
    connect(backend, &QmlBackend::hostsChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::discoveryEnabledChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::connectionAttemptChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::windowChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::streamerModeChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::currentProfileChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::psnAuthAttemptChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::registrationAttemptChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::accountLookupChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::psnTokenChanged, this, [this] { credentialsRejected = false; refresh(); });
    connect(backend, &QmlBackend::psnCredsExpired, this, [this] { credentialsRejected = true; refresh(); });
    connect(backend->qmlSettings(), &QmlSettings::psnAuthTokenChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::psnRefreshTokenChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::psnAuthTokenExpiryChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::psnAccountIdChanged, this, &SetupBridge::refresh);
    accountExpiry.setSingleShot(true);
    connect(&accountExpiry, &QTimer::timeout, this, &SetupBridge::refresh);
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
    startupRendererBackend = settings->rendererBackend();
    startupVSync = settings->vSyncEnabled();
    connect(settings, &QmlSettings::vSyncEnabledChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::videoPresetChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::queueDepthLimitChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::rendererBackendChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::displayTargetPrimChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::displayTargetTrcChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::displayTargetPeakChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::displayTargetContrastChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::placeboChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::windowChanged, this, &SetupBridge::refresh);
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
        QT_TR_NOOP("Remote connection via PSN"), QT_TR_NOOP("Console"), QT_TR_NOOP("Console unavailable"), QT_TR_NOOP("Selected"), QT_TR_NOOP("Loading consoles"), QT_TR_NOOP("Setup unavailable"), QT_TR_NOOP("Connect"), QT_TR_NOOP("Dismiss"),
        QT_TR_NOOP("PSN account"), QT_TR_NOOP("PSN remote access"), QT_TR_NOOP("Retrieving your Account ID"), QT_TR_NOOP("Completing PSN sign-in"),
        QT_TR_NOOP("PSN remote access is ready"), QT_TR_NOOP("PSN credentials expired"), QT_TR_NOOP("Not signed in"),
        QT_TR_NOOP("Sign-in saves PSN credentials and your Account ID in the current profile. These credentials allow remote access to your consoles."),
        QT_TR_NOOP("You can close this screen. Sign-in can still finish and save credentials in this profile while the app is open."),
        QT_TR_NOOP("Open embedded sign-in"), QT_TR_NOOP("Use external browser"), QT_TR_NOOP("Copy sign-in link"),
        QT_TR_NOOP("After sign-in in an external browser, copy the complete final URL from its address bar. Paste the full URL, including its authorization code, below."),
        QT_TR_NOOP("Sign-in redirect URL"), QT_TR_NOOP("Retry"), QT_TR_NOOP("Complete sign-in"), QT_TR_NOOP("Close"),
        QT_TR_NOOP("Use the same PlayStation account as your console. The Sony sign-in page and your external browser can show account details.")};
    for (const auto *text : texts) strings.insert(QString::fromUtf8(text), tr(text));
    const auto *native = backend->nativeSettings();
    const bool configured = !native->GetPsnAuthToken().isEmpty() && !native->GetPsnRefreshToken().isEmpty()
        && !native->GetPsnAccountId().isEmpty();
    const auto expiry = QDateTime::fromString(native->GetPsnAuthTokenExpiry(), native->GetTimeFormat());
    const auto remaining = QDateTime::currentDateTime().msecsTo(expiry);
    const bool expired = configured && (credentialsRejected || !expiry.isValid() || remaining <= 0);
    accountExpiry.stop();
    if (configured && !expired) accountExpiry.start(int(qMin<qint64>(remaining, std::numeric_limits<int>::max())));
    QVariantMap psn{{"status", configured ? (expired ? "expired" : "ready") : "signedOut"}};
    const auto attempt = backend->psnAuthAttempt();
    // Keep the bridge allowlist explicit: the native attempt also contains an Account ID.
    for (const auto &field : {"id", "phase", "message", "profileValid"})
        if (attempt.contains(field)) psn.insert(field, attempt.value(field));
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
    QVariantMap renderer{{"vSyncEnabled", settings->vSyncEnabled()},
        {"videoPreset", settings->videoPreset()},
        {"queueDepthLimit", settings->queueDepthLimit()},
        {"rendererBackend", settings->rendererBackend()},
        {"displayTargetPrim", settings->displayTargetPrim()},
        {"displayTargetTrc", settings->displayTargetTrc()},
        {"displayTargetPeak", settings->displayTargetPeak()},
        {"displayTargetContrast", settings->displayTargetContrast()},
        {"placeboUpscaler", settings->placeboUpscaler()},
        {"placeboPlaneUpscaler", settings->placeboPlaneUpscaler()},
        {"placeboDownscaler", settings->placeboDownscaler()},
        {"placeboPlaneDownscaler", settings->placeboPlaneDownscaler()},
        {"placeboFrameMixer", settings->placeboFrameMixer()},
        {"placeboAntiringingStrength", settings->placeboAntiringingStrength()},
        {"placeboDebandEnabled", settings->placeboDebandEnabled()},
        {"placeboDebandPreset", settings->placeboDebandPreset()},
        {"placeboDebandIterations", settings->placeboDebandIterations()},
        {"placeboDebandThreshold", settings->placeboDebandThreshold()},
        {"placeboDebandRadius", settings->placeboDebandRadius()},
        {"placeboDebandGrain", settings->placeboDebandGrain()},
        {"placeboSigmoidEnabled", settings->placeboSigmoidEnabled()},
        {"placeboSigmoidPreset", settings->placeboSigmoidPreset()},
        {"placeboSigmoidCenter", settings->placeboSigmoidCenter()},
        {"placeboSigmoidSlope", settings->placeboSigmoidSlope()},
        {"placeboColorAdjustmentEnabled", settings->placeboColorAdjustmentEnabled()},
        {"placeboColorAdjustmentPreset", settings->placeboColorAdjustmentPreset()},
        {"placeboColorAdjustmentBrightness", settings->placeboColorAdjustmentBrightness()},
        {"placeboColorAdjustmentContrast", settings->placeboColorAdjustmentContrast()},
        {"placeboColorAdjustmentSaturation", settings->placeboColorAdjustmentSaturation()},
        {"placeboColorAdjustmentHue", settings->placeboColorAdjustmentHue()},
        {"placeboColorAdjustmentGamma", settings->placeboColorAdjustmentGamma()},
        {"placeboColorAdjustmentTemperature", settings->placeboColorAdjustmentTemperature()},
        {"placeboPeakDetectionEnabled", settings->placeboPeakDetectionEnabled()},
        {"placeboPeakDetectionPreset", settings->placeboPeakDetectionPreset()},
        {"placeboPeakDetectionPeakSmoothingPeriod", settings->placeboPeakDetectionPeakSmoothingPeriod()},
        {"placeboPeakDetectionSceneThresholdLow", settings->placeboPeakDetectionSceneThresholdLow()},
        {"placeboPeakDetectionSceneThresholdHigh", settings->placeboPeakDetectionSceneThresholdHigh()},
        {"placeboPeakDetectionPeakPercentile", settings->placeboPeakDetectionPeakPercentile()},
        {"placeboPeakDetectionBlackCutoff", settings->placeboPeakDetectionBlackCutoff()},
        {"placeboPeakDetectionAllowDelayedPeak", settings->placeboPeakDetectionAllowDelayedPeak()}};
    renderer.insert("targetPeakChoice", settings->displayTargetPeak() == 0 ? 0 : 1);
    renderer.insert("targetContrastChoice", settings->displayTargetContrast() == 0 ? 0 : settings->displayTargetContrast() == -1 ? 1 : 2);
    const auto *window = backend->qmlWindow();
    QVariantMap rendererStatus{{"restartRequired", settings->rendererBackend() != startupRendererBackend
        || (startupRendererBackend == 1 && settings->vSyncEnabled() != startupVSync)}};
    if(window) rendererStatus.insert("runtimeBackend", window->runtimeRendererBackend());
    QVariantMap registration{{"busy", backend->registrationBusy()}, {"savedAccount", !native->GetPsnAccountId().isEmpty()}};
    if (registrationContextId == registrationAttemptContext && !registrationContextId.isEmpty()) {
        const auto attempt = backend->registrationAttempt();
        registration["context"] = registrationContextId;
        for (const auto &field : {"id", "phase", "message", "profileValid"})
            if (attempt.contains(field)) registration[field] = attempt.value(field);
        registration["diagnostics"] = privateMode ? QString() : attempt.value("diagnostics").toString();
        if (attempt.value("phase") == "success" && attempt.value("profileValid").toBool()) {
            for (const auto &value : hosts) {
                const auto host = value.toMap();
                if (host.value("mac") == attempt.value("resultMac") && host.value("address") == attempt.value("resultAddress")
                    && host.value("registered").toBool()) {
                    registration["connectKey"] = keys.value(identity(host));
                    break;
                }
            }
        }
    }
    QVariantMap lookup;
    if (!accountRouteId.isEmpty() && accountSource == "lookup" && registrationAccountCurrent(registrationContextId, accountRouteId)) {
        const auto attempt = backend->accountLookupAttempt();
        lookup = {{"route", accountRouteId}};
        if (attempt.value("id") == accountRequest)
            for (const auto &field : {"id", "phase", "message"})
                if (attempt.contains(field)) lookup[field] = attempt.value(field);
    }
    state = {{"revision", QString::number(++revision)}, {"profileGeneration", QString::number(generation)},
        {"consoles", consoles}, {"selectedKey", selectedKey}, {"discoveryEnabled", backend->discoveryEnabled()},
        {"streamerMode", privateMode}, {"registration", registration}, {"accountLookup", lookup}, {"connectionBusy", backend->qmlWindow() != nullptr || backend->qmlSession() != nullptr
            || backend->connectionAttempt().value("cleanupPending").toBool()}, {"streamError", streamError}, {"psn", psn}, {"mediaSettings", media}, {"mediaChoices", choices}, {"generalSettings", general}, {"rendererSettings", renderer}, {"rendererStatus", rendererStatus}, {"capabilities", QVariantMap{{"discovery", true}}}, {"strings", strings}};
    if (!suspended) emit stateChanged(state);
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

void SetupBridge::setSuspended(bool value)
{
    suspended = value;
    if (!suspended) refresh();
}

void SetupBridge::reportStreamError(const QString &title, const QString &text)
{
    streamError = {{"id", QString::number(++streamErrorGeneration)}, {"title", title}, {"text", text}};
    refresh();
}

QVariantMap SetupBridge::dismissStreamError(const QString &id)
{
    if (id.isEmpty() || id != streamError.value("id").toString()) return {{"accepted", false}};
    streamError.clear();
    refresh();
    return {{"accepted", true}};
}

QVariantMap SetupBridge::connectConsole(const QString &key)
{
    int index;
    QString nickname;
    if (state.value("connectionBusy").toBool())
        return {{"accepted", false}, {"message", tr("A connection is already active")}};
    if (!resolveConsole(key, index, nickname))
        return {{"accepted", false}, {"message", tr("Console unavailable")}};
    const auto host = backend->hosts().at(index).toMap();
    if (!host.value("registered").toBool() || !host.value("discovered").toBool() || host.value("state") != "ready")
        return {{"accepted", false}, {"message", tr("Select an online registered local console")}};
    selectedKey = key;
    backend->connectToHost(index, nickname);
    return {{"accepted", true}};
}

QVariantMap SetupBridge::openRegistration(const QString &key, const QString &profile)
{
    if (!currentProfile(profile)) return {{"accepted", false}, {"message", tr("The profile changed. Open registration again.")}};
    if (backend->registrationBusy()) {
        if (registrationContextId.isEmpty() || registrationContext.value("profileGeneration").toULongLong() != backend->profileGeneration())
            return {{"accepted", false}, {"message", tr("Wait for the current registration to finish before starting another request.")}};
    } else {
        int index = -1;
        QString nickname;
        if (!key.isEmpty() && !resolveConsole(key, index, nickname))
            return {{"accepted", false}, {"message", tr("Console unavailable")}};
        backend->cancelAccountLookup(accountRequest);
        accountRouteId.clear();
        accountRequest.clear();
        registrationContext = backend->createRegistrationContext(index);
        if (registrationContext.isEmpty()) return {{"accepted", false}, {"message", tr("Select a local console or enter its address.")}};
        registrationContextId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        registrationAddress = registrationContext.value("address", "255.255.255.255").toString();
        refresh();
    }
    return {{"accepted", true}, {"context", registrationContextId}, {"address", registrationAddress},
        {"ps5", registrationContext.value("ps5", true)}};
}

QVariantMap SetupBridge::submitRegistration(const QString &context, const QVariantMap &fields)
{
    if (context.isEmpty() || context != registrationContextId || backend->registrationBusy()
        || registrationContext.value("profileGeneration").toULongLong() != backend->profileGeneration())
        return {{"accepted", false}, {"message", tr("The registration context changed. Close this screen and open registration again.")}};
    for (const auto &field : {"address", "account", "pin", "consolePin"})
        if (fields.value(field).metaType().id() != QMetaType::QString) return {{"accepted", false}, {"message", tr("Enter text for the address, account, and pairing codes.")}};
    const auto target = fields.value("target");
    const double targetNumber = target.toDouble();
    if (target.metaType().id() == QMetaType::QString || !std::isfinite(targetNumber) || std::floor(targetNumber) != targetNumber)
        return {{"accepted", false}, {"message", tr("Select a supported console and firmware.")}};
    const auto address = fields.value("address").toString().trimmed();
    const auto nativeContext = address == registrationAddress ? registrationContext : backend->createRegistrationContext();
    const auto previous = backend->registrationAttempt().value("id");
    registrationAttemptContext = context;
    const bool accepted = backend->registerHost(address, fields.value("account").toString().trimmed(),
        fields.value("pin").toString(), fields.value("consolePin").toString(), address == "255.255.255.255",
        fields.value("target").toInt(), nativeContext);
    if (backend->registrationAttempt().value("id") == previous) {
        registrationAttemptContext.clear();
        refresh();
        return {{"accepted", false}, {"message", tr("Check the console address, Account ID, and pairing codes. The console or profile can have changed.")}};
    }
    registrationAddress = address;
    refresh();
    return {{"accepted", accepted}, {"id", backend->registrationAttempt().value("id")}};
}

bool SetupBridge::registrationAccountCurrent(const QString &context, const QString &route) const
{
    return !context.isEmpty() && context == registrationContextId && !route.isEmpty() && route == accountRouteId
        && registrationContext.value("profileGeneration").toULongLong() == backend->profileGeneration()
        && backend->registrationContextCurrent(registrationContext, registrationAddress);
}

QVariantMap SetupBridge::beginRegistrationAccount(const QString &context, const QString &source)
{
    if (context.isEmpty() || context != registrationContextId || backend->registrationBusy()
        || registrationContext.value("profileGeneration").toULongLong() != backend->profileGeneration()
        || (source != "lookup" && source != "signIn" && source != "saved")) return {{"accepted", false}};
    backend->cancelAccountLookup(accountRequest);
    accountRequest.clear();
    accountSource = source;
    accountRouteId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    accountAuthBaseline = backend->psnAuthAttempt().value("id").toString();
    refresh();
    return {{"accepted", true}, {"route", accountRouteId}};
}

QVariantMap SetupBridge::lookupRegistrationAccount(const QString &context, const QString &route, const QString &username)
{
    if (!registrationAccountCurrent(context, route) || accountSource != "lookup"
        || (!accountRequest.isEmpty() && backend->accountLookupAttempt().value("phase") == "pending")) return {{"accepted", false}};
    accountRequest = backend->startAccountLookup(username);
    refresh();
    return {{"accepted", true}, {"id", accountRequest}};
}

QVariantMap SetupBridge::readRegistrationAccount(const QString &context, const QString &route, const QString &request)
{
    if (!registrationAccountCurrent(context, route)) return {{"accepted", false}};
    QString account;
    if (accountSource == "saved") account = backend->nativeSettings()->GetPsnAccountId();
    else {
        const auto attempt = accountSource == "lookup" ? backend->accountLookupAttempt() : backend->psnAuthAttempt();
        if (request.isEmpty() || attempt.value("id") != request || attempt.value("phase") != "success"
            || (accountSource == "lookup" ? request != accountRequest : request == accountAuthBaseline || !attempt.value("profileValid").toBool()))
            return {{"accepted", false}};
        account = attempt.value("accountId").toString();
    }
    const auto bytes = account.toLatin1();
    const auto decoded = QByteArray::fromBase64Encoding(bytes, QByteArray::AbortOnBase64DecodingErrors);
    if (!decoded || decoded.decoded.size() != CHIAKI_PSN_ACCOUNT_ID_SIZE || decoded.decoded.toBase64() != bytes)
        return {{"accepted", false}, {"message", tr("No valid saved Account ID is available. Use public lookup or PSN sign-in.")}};
    return {{"accepted", true}, {"accountId", account}};
}

QVariantMap SetupBridge::endRegistrationAccount(const QString &context, const QString &route)
{
    if (!registrationAccountCurrent(context, route)) return {{"accepted", false}};
    accountRouteId.clear();
    backend->cancelAccountLookup(accountRequest);
    accountRequest.clear();
    refresh();
    return {{"accepted", true}};
}

QVariantMap SetupBridge::connectRegistration(const QString &attempt)
{
    const auto registration = state.value("registration").toMap();
    if (attempt.isEmpty() || registration.value("id") != attempt || registration.value("phase") != "success")
        return {{"accepted", false}};
    return connectConsole(registration.value("connectKey").toString());
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

QVariantMap SetupBridge::setRendererSetting(const QString &name, const QVariant &value, const QString &profileGeneration)
{
    const auto reject = [this] { return QVariantMap{{"accepted", false}, {"message", tr("Choose a valid renderer setting value.")}}; };
    if(!currentProfile(profileGeneration)) return {{"accepted", false}, {"message", tr("The profile changed. Try again in the current profile.")}};
    auto *settings = backend->qmlSettings();
    const auto type = value.metaType().id();
    const bool numeric = type == QMetaType::Double || type == QMetaType::Int || type == QMetaType::UInt || type == QMetaType::LongLong;
    const auto number = value.toDouble();
    if(name == "targetPeakChoice") {
        if(!numeric || (number != 0 && number != 1)) return reject();
        settings->setDisplayTargetPeak(number == 0 ? 0 : 1000);
        return {{"accepted", true}};
    }
    if(name == "targetContrastChoice") {
        if(!numeric || (number != 0 && number != 1 && number != 2)) return reject();
        settings->setDisplayTargetContrast(number == 0 ? 0 : number == 1 ? -1 : 1000);
        return {{"accepted", true}};
    }
    if(name == "vSyncEnabled") {
        if(type != QMetaType::Bool) return reject();
        const bool changed = settings->vSyncEnabled() != value.toBool();
        const auto *window = backend->qmlWindow();
        const int runtimeBackend = window ? window->runtimeRendererBackend() : startupRendererBackend;
        settings->setVSyncEnabled(value.toBool());
        if(changed && runtimeBackend == static_cast<int>(RenderBackend::OpenGL)) return settings->restartApplication();
        return {{"accepted", true}};
    }
    if(name == "videoPreset") {
        if(!numeric || !std::isfinite(number) || number < 0 || number > 5 || std::floor(number) != number) return reject();
        settings->setVideoPreset(int(number));
        if(auto *window = backend->qmlWindow()) window->setVideoPreset(static_cast<QmlMainWindow::VideoPreset>(int(number)));
        return {{"accepted", true}};
    }
    if(name == "queueDepthLimit") {
        if(!numeric || !std::isfinite(number) || number < 2 || number > 5 || std::floor(number) != number) return reject();
        settings->setQueueDepthLimit(int(number));
        return {{"accepted", true}};
    }
    if(name == "rendererBackend") {
        if(!numeric || !std::isfinite(number) || number < 0 || number > 1 || std::floor(number) != number) return reject();
        const bool changed = settings->rendererBackend() != int(number);
        settings->setRendererBackend(int(number));
        if(changed) return settings->restartApplication();
        return {{"accepted", true}};
    }
    if(name == "displayTargetPrim") {
        if(!numeric || !std::isfinite(number) || number < 0 || number > 17 || std::floor(number) != number) return reject();
        settings->setDisplayTargetPrim(int(number));
        return {{"accepted", true}};
    }
    if(name == "displayTargetTrc") {
        if(!numeric || !std::isfinite(number) || number < 0 || number > 16 || std::floor(number) != number) return reject();
        settings->setDisplayTargetTrc(int(number));
        return {{"accepted", true}};
    }
    if(name == "displayTargetPeak") {
        if(!(settings->displayTargetPeak() != 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 10 || number > 10000 || std::floor(number) != number) return reject();
        settings->setDisplayTargetPeak(int(number));
        return {{"accepted", true}};
    }
    if(name == "displayTargetContrast") {
        if(!(settings->displayTargetContrast() > 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 10 || number > 1000000 || std::floor(number) != number) return reject();
        settings->setDisplayTargetContrast(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboUpscaler") {
        if(settings->videoPreset() != 5) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 13 || std::floor(number) != number) return reject();
        settings->setPlaceboUpscaler(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboPlaneUpscaler") {
        if(settings->videoPreset() != 5) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 10 || std::floor(number) != number) return reject();
        settings->setPlaceboPlaneUpscaler(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboDownscaler") {
        if(settings->videoPreset() != 5) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 8 || std::floor(number) != number) return reject();
        settings->setPlaceboDownscaler(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboPlaneDownscaler") {
        if(settings->videoPreset() != 5) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 8 || std::floor(number) != number) return reject();
        settings->setPlaceboPlaneDownscaler(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboFrameMixer") {
        if(settings->videoPreset() != 5) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 4 || std::floor(number) != number) return reject();
        settings->setPlaceboFrameMixer(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboAntiringingStrength") {
        if(settings->videoPreset() != 5) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 1) return reject();
        settings->setPlaceboAntiringingStrength(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboDebandEnabled") {
        if(settings->videoPreset() != 5) return reject();
        if(type != QMetaType::Bool) return reject();
        settings->setPlaceboDebandEnabled(value.toBool());
        return {{"accepted", true}};
    }
    if(name == "placeboDebandPreset") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboDebandEnabled())) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 1 || std::floor(number) != number) return reject();
        settings->setPlaceboDebandPreset(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboDebandIterations") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboDebandEnabled() && settings->placeboDebandPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 16 || std::floor(number) != number) return reject();
        settings->setPlaceboDebandIterations(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboDebandThreshold") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboDebandEnabled() && settings->placeboDebandPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 1000) return reject();
        settings->setPlaceboDebandThreshold(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboDebandRadius") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboDebandEnabled() && settings->placeboDebandPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 1000) return reject();
        settings->setPlaceboDebandRadius(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboDebandGrain") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboDebandEnabled() && settings->placeboDebandPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 1000) return reject();
        settings->setPlaceboDebandGrain(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboSigmoidEnabled") {
        if(settings->videoPreset() != 5) return reject();
        if(type != QMetaType::Bool) return reject();
        settings->setPlaceboSigmoidEnabled(value.toBool());
        return {{"accepted", true}};
    }
    if(name == "placeboSigmoidPreset") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboSigmoidEnabled())) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 1 || std::floor(number) != number) return reject();
        settings->setPlaceboSigmoidPreset(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboSigmoidCenter") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboSigmoidEnabled() && settings->placeboSigmoidPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 1) return reject();
        settings->setPlaceboSigmoidCenter(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboSigmoidSlope") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboSigmoidEnabled() && settings->placeboSigmoidPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 1 || number > 20) return reject();
        settings->setPlaceboSigmoidSlope(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboColorAdjustmentEnabled") {
        if(settings->videoPreset() != 5) return reject();
        if(type != QMetaType::Bool) return reject();
        settings->setPlaceboColorAdjustmentEnabled(value.toBool());
        return {{"accepted", true}};
    }
    if(name == "placeboColorAdjustmentPreset") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboColorAdjustmentEnabled())) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 1 || std::floor(number) != number) return reject();
        settings->setPlaceboColorAdjustmentPreset(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboColorAdjustmentBrightness") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboColorAdjustmentEnabled() && settings->placeboColorAdjustmentPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < -1 || number > 1) return reject();
        settings->setPlaceboColorAdjustmentBrightness(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboColorAdjustmentContrast") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboColorAdjustmentEnabled() && settings->placeboColorAdjustmentPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 100) return reject();
        settings->setPlaceboColorAdjustmentContrast(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboColorAdjustmentSaturation") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboColorAdjustmentEnabled() && settings->placeboColorAdjustmentPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 100) return reject();
        settings->setPlaceboColorAdjustmentSaturation(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboColorAdjustmentHue") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboColorAdjustmentEnabled() && settings->placeboColorAdjustmentPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 6.28) return reject();
        settings->setPlaceboColorAdjustmentHue(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboColorAdjustmentGamma") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboColorAdjustmentEnabled() && settings->placeboColorAdjustmentPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 100) return reject();
        settings->setPlaceboColorAdjustmentGamma(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboColorAdjustmentTemperature") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboColorAdjustmentEnabled() && settings->placeboColorAdjustmentPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < -1.143 || number > 5.286) return reject();
        settings->setPlaceboColorAdjustmentTemperature(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboPeakDetectionEnabled") {
        if(settings->videoPreset() != 5) return reject();
        if(type != QMetaType::Bool) return reject();
        settings->setPlaceboPeakDetectionEnabled(value.toBool());
        return {{"accepted", true}};
    }
    if(name == "placeboPeakDetectionPreset") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboPeakDetectionEnabled())) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 2 || std::floor(number) != number) return reject();
        settings->setPlaceboPeakDetectionPreset(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboPeakDetectionPeakSmoothingPeriod") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboPeakDetectionEnabled() && settings->placeboPeakDetectionPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 1000) return reject();
        settings->setPlaceboPeakDetectionPeakSmoothingPeriod(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboPeakDetectionSceneThresholdLow") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboPeakDetectionEnabled() && settings->placeboPeakDetectionPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 100) return reject();
        settings->setPlaceboPeakDetectionSceneThresholdLow(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboPeakDetectionSceneThresholdHigh") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboPeakDetectionEnabled() && settings->placeboPeakDetectionPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 100) return reject();
        settings->setPlaceboPeakDetectionSceneThresholdHigh(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboPeakDetectionPeakPercentile") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboPeakDetectionEnabled() && settings->placeboPeakDetectionPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 100) return reject();
        settings->setPlaceboPeakDetectionPeakPercentile(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboPeakDetectionBlackCutoff") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboPeakDetectionEnabled() && settings->placeboPeakDetectionPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 100) return reject();
        settings->setPlaceboPeakDetectionBlackCutoff(float(number));
        return {{"accepted", true}};
    }
    if(name == "placeboPeakDetectionAllowDelayedPeak") {
        if(settings->videoPreset() != 5) return reject();
        if(!(settings->placeboPeakDetectionEnabled() && settings->placeboPeakDetectionPreset() == 0)) return reject();
        if(type != QMetaType::Bool) return reject();
        settings->setPlaceboPeakDetectionAllowDelayedPeak(value.toBool());
        return {{"accepted", true}};
    }
    return reject();
}

QVariantMap SetupBridge::rendererAction(const QString &action, const QString &profileGeneration)
{
    if(!currentProfile(profileGeneration)) return {{"accepted", false}, {"message", tr("The profile changed. Try again in the current profile.")}};
    auto *settings = backend->qmlSettings();
    if(action == "importPlaceboSettings") return settings->importPlaceboSettings();
    if(action == "exportPlaceboSettings") return settings->exportPlaceboSettings();
    if(action == "openRendererDocumentation") {
        backend->openPlaceboOptionsLink();
        return {{"accepted", true}, {"message", tr("Renderer documentation opened.")}};
    }
    if(action == "copyRendererDocumentationUrl") {
        QGuiApplication::clipboard()->setText(QStringLiteral("https://libplacebo.org/options/"));
        return {{"accepted", true}, {"message", tr("Renderer documentation link copied.")}};
    }
    if(action == "restartRenderer") return settings->restartApplication();
    return {{"accepted", false}};
}
