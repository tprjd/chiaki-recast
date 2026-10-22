#include "setupbridge.h"
#include "qmlbackend.h"
#include "keyboardmousebindings.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include <QDateTime>
#include <limits>
#include <QGuiApplication>
#include <QClipboard>
#include <cmath>
#include <QHostAddress>
#include <QRegularExpression>

SetupBridge::SetupBridge(QmlBackend *backend, QObject *parent)
    : QObject(parent), backend(backend)
{
    connect(backend, &QmlBackend::hostsChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::hiddenHostsChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::registeredHostsChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::discoveryEnabledChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::connectionAttemptChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::windowChanged, this, &SetupBridge::refresh);
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
    auto *settings = backend->qmlSettings();
    connect(settings, &QmlSettings::currentProfileChanged, this, &SetupBridge::invalidateKeyboardMouse);
    connect(settings, &QmlSettings::mouseCameraEnabledChanged, this, [this] { emit keyboardMouseClosed(); refresh(); });
    connect(settings, &QmlSettings::mouseResponseChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::keyboardMouseBindingsChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::keyboardEnabledChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::mouseTouchEnabledChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::controllerMappingChanged, this, &SetupBridge::refresh);
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
        parts = {"manual", QString::number(host.value("manualId").toInt()), host.value("address").toString(), host.value("mac").toString().toLower()};
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
    QVariantList consoles, registrations, hiddenConsoles;
    const auto opaqueKey = [&](const QString &id) {
        auto key = keys.value(id);
        if (key.isEmpty()) key = QUuid::createUuid().toString(QUuid::WithoutBraces);
        currentKeys.insert(id, key);
        return key;
    };
    const bool privateMode = backend->qmlSettings()->streamerMode();
    const auto safeName = [&](QString name, const QString &address, const QString &identifier) {
        bool sensitive = name.compare(address, Qt::CaseInsensitive) == 0
            || name.compare(identifier, Qt::CaseInsensitive) == 0 || QHostAddress().setAddress(name);
        for (const auto &value : hosts) {
            const auto host = value.toMap();
            for (const auto &field : {"address", "mac", "duid"})
                sensitive = sensitive || (!name.isEmpty() && name.compare(host.value(field).toString(), Qt::CaseInsensitive) == 0);
        }
        if (privateMode && sensitive) name = tr("Console");
        return name.isEmpty() ? tr("Console") : name;
    };
    for (const auto &registration : backend->qmlSettings()->registeredHosts()) {
        const auto host = registration.toMap();
        const auto mac = host.value("mac").toString();
        registrations.append(QVariantMap{{"key", opaqueKey("registration:" + mac)},
            {"name", safeName(host.value("name").toString(), {}, mac)},
            {"ps5", host.value("ps5")}, {"identifier", privateMode ? tr("hidden") : mac}});
    }
    for (const auto &entry : backend->hiddenHosts()) {
        const auto host = entry.toMap();
        const auto mac = host.value("mac").toString();
        hiddenConsoles.append(QVariantMap{{"key", opaqueKey("hidden:" + mac)},
            {"name", safeName(host.value("name").toString(), {}, mac)}, {"identifier", privateMode ? tr("hidden") : mac}});
    }
    for (const auto &value : hosts) {
        const auto host = value.toMap();
        if (!host.value("display").toBool()) continue;
        const auto id = identity(host);
        const bool available = !id.isEmpty() && counts.value(id) == 1;
        QString key;
        if (available) {
            key = opaqueKey(id);
        }
        QString name = host.value("name").toString();
        if (privateMode && name == host.value("address").toString()) name = tr("Manual console");
        name = safeName(name, host.value("address").toString(), host.value("mac").toString());
        QVariantMap console{{"key", key}, {"name", name}, {"available", available},
            {"registered", host.value("registered").toBool()}, {"state", host.value("state").toString()},
            {"route", host.value("discovered").toBool() ? "local" : host.value("manual").toBool() ? "manual" : "psn"}};
        console.insert("address", privateMode ? tr("hidden") : host.value("address").toString());
        console.insert("identifier", privateMode ? tr("hidden") : host.value("mac", host.value("duid")).toString());
        for (const auto &field : {"app", "titleId"}) console.insert(field, host.value(field).toString());
        QStringList actions;
        if (host.value("registered").toBool()) actions.append("pin");
        if (host.value("manual").toBool()) actions.append("deleteManual");
        if (host.value("discovered").toBool() && !host.value("registered").toBool() && !host.value("manual").toBool()) actions.append("hide");
        console.insert("actions", actions);
        if (host.contains("ps5")) console.insert("ps5", host.value("ps5").toBool());
        consoles.append(console);
    }
    keys = currentKeys;
    bool selectedPresent = false;
    for (const auto &console : consoles) if (console.toMap().value("key").toString() == selectedKey && !selectedKey.isEmpty()) selectedPresent = true;
    if (!selectedPresent) {
        selectedKey.clear();
        for (const auto &console : consoles) {
            const auto key = console.toMap().value("key").toString();
            if (!key.isEmpty()) { selectedKey = key; break; }
        }
    }
    QVariantMap strings;
    const char *colorParentTexts[] = {
        QT_TRANSLATE_NOOP("PlaceboSettingsDialog", "Color Mapping Enabled:"),
        QT_TRANSLATE_NOOP("PlaceboSettingsDialog", "Color Mapping"),
        QT_TRANSLATE_NOOP("PlaceboSettingsDialog", "Checked"),
        QT_TRANSLATE_NOOP("PlaceboSettingsDialog", "Color Mapping Preset:"),
        QT_TRANSLATE_NOOP("PlaceboSettingsDialog", "Custom"),
        QT_TRANSLATE_NOOP("PlaceboSettingsDialog", "Default"),
        QT_TRANSLATE_NOOP("PlaceboSettingsDialog", "High Quality"),
        QT_TRANSLATE_NOOP("PlaceboSettingsDialog", "Color Mapping Settings:")};
    for(const auto *text : colorParentTexts) {
        auto key = QString::fromUtf8(text);
        if(key.endsWith(':')) key.chop(1);
        strings.insert(key, QCoreApplication::translate("PlaceboSettingsDialog", text));
    }
    const char *colorDetailTexts[] = {
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Gamut Mapping Function:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Gamut Mapping 1"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Clip"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Perceptual"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Soft Clip"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Relative"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Saturation"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Absolute"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Desaturate"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Darken"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Highlight"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Linear"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Perceptual Deadzone:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Perceptual Strength:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Colorimetric Gamma:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Soft Clip Knee:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Soft Clip Desaturation:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "LUT 3D Size I:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Gamut Mapping 2"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "LUT 3D Size C:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "LUT 3D Size h:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "LUT 3D Tricubic Enabled:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Unchecked"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Gamut Expansion Enabled:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Tone-mapping Function:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Tone-mapping 1"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Spline"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "St2094-40"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "St-2094-10"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Bt2390"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Bt2446a"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Reinhard"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Mobius"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Hable"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Gamma"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Linear Light"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Knee Adaptation:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Knee Minimum:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Knee Maximum:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Knee Default:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Knee Offset:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Slope Tuning:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Tone-mapping 2"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Slope Offset:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Spline Contrast:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Reinhard Contrast:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Linear Knee:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Exposure:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Inverse Tone-mapping Enabled:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Tone-mapping 3"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Tone-mapping Metadata:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Any"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "None"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "HDR10"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "HDR10 Plus"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Cie_y"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Tone LUT Size:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Contrast Recovery:"),
        QT_TRANSLATE_NOOP("PlaceboColorMappingDialog", "Contrast Smoothness:")};
    for(const auto *text : colorDetailTexts) {
        auto key = QString::fromUtf8(text);
        if(key.endsWith(':')) key.chop(1);
        strings.insert(key, QCoreApplication::translate("PlaceboColorMappingDialog", text));
    }
    const char *colorSearchTexts[] = {
        QT_TRANSLATE_NOOP("SettingsSearch", "Choose how out-of-gamut colors are mapped."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set the deadzone. Requires the Perceptual gamut function."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set mapping strength. Requires the Perceptual gamut function."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set gamma. Requires Relative, Absolute or Darken gamut mapping."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set the knee. Requires Perceptual or Soft Clip gamut mapping."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set desaturation. Requires Soft Clip gamut mapping."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set the intensity dimension of the gamut lookup table."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set the chroma dimension of the gamut lookup table."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set the hue dimension of the gamut lookup table."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Use tricubic gamut lookup interpolation."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Allow the output color gamut to expand."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Choose the algorithm for mapping display brightness."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set knee adaptation. Requires Spline or ST 2094 tone mapping."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set the minimum knee. Requires Spline or ST 2094 tone mapping."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set the maximum knee. Requires Spline or ST 2094 tone mapping."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set the default knee."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set knee offset. Requires BT.2390 tone mapping."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Tune the slope. Requires Spline tone mapping."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set slope offset. Requires Spline tone mapping."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set spline contrast. Requires Spline tone mapping."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set contrast. Requires Reinhard tone mapping."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set the linear knee. Requires Mobius or Gamma tone mapping."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set exposure. Requires Linear or Linear Light tone mapping."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Expand SDR brightness to HDR."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Choose the HDR metadata source."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set tone mapping lookup table precision."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Recover contrast after tone mapping."),
        QT_TRANSLATE_NOOP("SettingsSearch", "Set contrast recovery smoothing.")};
    for(const auto *text : colorSearchTexts) strings.insert(QString::fromUtf8(text), QCoreApplication::translate("SettingsSearch", text));
    const char *texts[] = {QT_TR_NOOP("Back to Renderer"), QT_TR_NOOP("Configure color mapping in Renderer"),
        QT_TR_NOOP("Requires the Custom render preset, Color Mapping Enabled, and the Custom color mapping preset in Renderer."), QT_TR_NOOP("Consoles"), QT_TR_NOOP("Search consoles"), QT_TR_NOOP("Discovery"), QT_TR_NOOP("Discovery is on"), QT_TR_NOOP("Discovery is off"),
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
    const char *managementTexts[] = {
        QT_TR_NOOP("Add console"),
        QT_TR_NOOP("Address"),
        QT_TR_NOOP("Address saved"),
        QT_TR_NOOP("App"),
        QT_TR_NOOP("Cancel"),
        QT_TR_NOOP("Choose its registration"),
        QT_TR_NOOP("Console address"),
        QT_TR_NOOP("Console details"),
        QT_TR_NOOP("Console login PIN"),
        QT_TR_NOOP("Console login PIN saved"),
        QT_TR_NOOP("Console unavailable. Close this screen and select the console again."),
        QT_TR_NOOP("Delete manual address"),
        QT_TR_NOOP("Delete registration"),
        QT_TR_NOOP("Delete this console registration? You must pair the console again to connect."),
        QT_TR_NOOP("Delete this saved address? The console registration will be kept."),
        QT_TR_NOOP("Enter an IP address or hostname without a port or URL."),
        QT_TR_NOOP("Find your console"),
        QT_TR_NOOP("Hidden consoles"),
        QT_TR_NOOP("Hide console"),
        QT_TR_NOOP("Hide this console? You can restore it from Manage consoles."),
        QT_TR_NOOP("ID"),
        QT_TR_NOOP("Manage consoles"),
        QT_TR_NOOP("No hidden consoles"),
        QT_TR_NOOP("No registered consoles"),
        QT_TR_NOOP("PlayStation 4"),
        QT_TR_NOOP("PlayStation 5"),
        QT_TR_NOOP("Register on first connection"),
        QT_TR_NOOP("Registered"),
        QT_TR_NOOP("Registered console"),
        QT_TR_NOOP("Registered consoles"),
        QT_TR_NOOP("Registration"),
        QT_TR_NOOP("Save PIN"),
        QT_TR_NOOP("Save address"),
        QT_TR_NOOP("Save an address when discovery does not show your console."),
        QT_TR_NOOP("Save console login PIN"),
        QT_TR_NOOP("Select a console"),
        QT_TR_NOOP("Show PIN"),
        QT_TR_NOOP("Show this console in the library again?"),
        QT_TR_NOOP("Status"),
        QT_TR_NOOP("The address could not be saved. Try again."),
        QT_TR_NOOP("The change could not be saved. Try again."),
        QT_TR_NOOP("The registration is unavailable. Choose a console again."),
        QT_TR_NOOP("The profile changed. Open this form again."),
        QT_TR_NOOP("This is the four-digit PIN for your console user. It is different from the eight-digit Remote Play pairing code."),
        QT_TR_NOOP("Title ID"),
        QT_TR_NOOP("Unhide console"),
        QT_TR_NOOP("Use a manual address"),
        QT_TR_NOOP("Use an existing registration, or register when you first connect. Saving an address does not pair the console or start a connection.")};
    for (const auto *text : managementTexts) strings.insert(QString::fromUtf8(text), tr(text));
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
    QVariantMap keyboardMouse{{"supported", settings->mouseCameraSupported()}, {"windowGeneration", QString::number(keyboardMouseWindowGeneration)},
        {"values", QVariantMap{{"mouseCameraEnabled", settings->mouseCameraEnabled()},
            {"mouseHorizontalSensitivity", settings->mouseHorizontalSensitivity()},
            {"mouseVerticalSensitivity", settings->mouseVerticalSensitivity()},
            {"mouseDeadZoneCompensation", settings->mouseDeadZoneCompensation()},
            {"mouseResponseExponent", settings->mouseResponseExponent()},
            {"mouseInvertVertical", settings->mouseInvertVertical()},
            {"keyboardEnabled", settings->keyboardEnabled()}, {"mouseTouchEnabled", settings->mouseTouchEnabled()}}},
        {"limits", QVariantMap{{"sensitivityMinimum", settings->mouseSensitivityMinimum()},
            {"sensitivityMaximum", settings->mouseSensitivityMaximum()}, {"compensationMaximum", settings->mouseCompensationMaximum()},
            {"exponentMinimum", settings->mouseExponentMinimum()}, {"exponentMaximum", settings->mouseExponentMaximum()}}},
        {"bindings", settings->keyboardMouseBindings()}, {"keys", settings->controllerMapping()}};
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
        {"placeboPeakDetectionAllowDelayedPeak", settings->placeboPeakDetectionAllowDelayedPeak()},
        {"placeboColorMappingEnabled", settings->placeboColorMappingEnabled()},
        {"placeboColorMappingPreset", settings->placeboColorMappingPreset()},
        {"placeboGamutMappingFunction", settings->placeboGamutMappingFunction()},
        {"placeboGamutMappingPerceptualDeadzone", settings->placeboGamutMappingPerceptualDeadzone()},
        {"placeboGamutMappingPerceptualStrength", settings->placeboGamutMappingPerceptualStrength()},
        {"placeboGamutMappingColorimetricGamma", settings->placeboGamutMappingColorimetricGamma()},
        {"placeboGamutMappingSoftClipKnee", settings->placeboGamutMappingSoftClipKnee()},
        {"placeboGamutMappingSoftClipDesat", settings->placeboGamutMappingSoftClipDesat()},
        {"placeboGamutMappingLut3dSizeI", settings->placeboGamutMappingLut3dSizeI()},
        {"placeboGamutMappingLut3dSizeC", settings->placeboGamutMappingLut3dSizeC()},
        {"placeboGamutMappingLut3dSizeH", settings->placeboGamutMappingLut3dSizeH()},
        {"placeboGamutMappingLut3dTricubicEnabled", settings->placeboGamutMappingLut3dTricubicEnabled()},
        {"placeboGamutMappingGamutExpansionEnabled", settings->placeboGamutMappingGamutExpansionEnabled()},
        {"placeboToneMappingFunction", settings->placeboToneMappingFunction()},
        {"placeboToneMappingKneeAdaptation", settings->placeboToneMappingKneeAdaptation()},
        {"placeboToneMappingKneeMinimum", settings->placeboToneMappingKneeMinimum()},
        {"placeboToneMappingKneeMaximum", settings->placeboToneMappingKneeMaximum()},
        {"placeboToneMappingKneeDefault", settings->placeboToneMappingKneeDefault()},
        {"placeboToneMappingKneeOffset", settings->placeboToneMappingKneeOffset()},
        {"placeboToneMappingSlopeTuning", settings->placeboToneMappingSlopeTuning()},
        {"placeboToneMappingSlopeOffset", settings->placeboToneMappingSlopeOffset()},
        {"placeboToneMappingSplineContrast", settings->placeboToneMappingSplineContrast()},
        {"placeboToneMappingReinhardContrast", settings->placeboToneMappingReinhardContrast()},
        {"placeboToneMappingLinearKnee", settings->placeboToneMappingLinearKnee()},
        {"placeboToneMappingExposure", settings->placeboToneMappingExposure()},
        {"placeboToneMappingInverseToneMappingEnabled", settings->placeboToneMappingInverseToneMappingEnabled()},
        {"placeboToneMappingMetadata", settings->placeboToneMappingMetadata()},
        {"placeboToneMappingToneLutSize", settings->placeboToneMappingToneLutSize()},
        {"placeboToneMappingContrastRecovery", settings->placeboToneMappingContrastRecovery()},
        {"placeboToneMappingContrastSmoothness", settings->placeboToneMappingContrastSmoothness()}};
    renderer.insert("targetPeakChoice", settings->displayTargetPeak() == 0 ? 0 : 1);
    renderer.insert("targetContrastChoice", settings->displayTargetContrast() == 0 ? 0 : settings->displayTargetContrast() == -1 ? 1 : 2);
    const auto *window = backend->qmlWindow();
    QVariantMap rendererStatus{{"restartRequired", settings->rendererBackend() != startupRendererBackend
        || (startupRendererBackend == 1 && settings->vSyncEnabled() != startupVSync)}};
    if(window) rendererStatus.insert("runtimeBackend", window->runtimeRendererBackend());
    state = {{"revision", QString::number(++revision)}, {"profileGeneration", QString::number(generation)},
        {"registrations", registrations}, {"hiddenConsoles", hiddenConsoles}, {"consoles", consoles}, {"selectedKey", selectedKey}, {"discoveryEnabled", backend->discoveryEnabled()},
        {"streamerMode", privateMode}, {"connectionBusy", backend->qmlWindow() != nullptr || backend->qmlSession() != nullptr
            || backend->connectionAttempt().value("cleanupPending").toBool()}, {"streamError", streamError}, {"psn", psn}, {"mediaSettings", media}, {"mediaChoices", choices}, {"generalSettings", general}, {"rendererSettings", renderer}, {"rendererStatus", rendererStatus}, {"keyboardMouse", keyboardMouse}, {"capabilities", QVariantMap{{"discovery", true}}}, {"strings", strings}};
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
    if (suspended) invalidateKeyboardMouse();
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
    if (!host.value("registered").toBool() || (!host.value("discovered").toBool() && !host.value("manual").toBool())
            || (host.value("discovered").toBool() && host.value("state") != "ready"))
        return {{"accepted", false}, {"message", tr("Select an online registered console or a registered manual address")}};
    selectedKey = key;
    backend->connectToHost(index, nickname);
    return {{"accepted", true}};
}

QVariantMap SetupBridge::refreshAudioDevices()
{
    backend->qmlSettings()->refreshAudioDevices();
    return {{"accepted", true}};
}

bool SetupBridge::keyboardMouseFormValid(const QString &form, const QString &profile) const
{
    return !suspended && keyboardMouseAvailable && !form.isEmpty() && form == keyboardMouseForm && currentProfile(profile);
}

void SetupBridge::invalidateKeyboardMouse()
{
    keyboardMouseForm.clear();
    emit keyboardMouseClosed();
}

void SetupBridge::setKeyboardMouseAvailable(bool available)
{
    if (keyboardMouseAvailable == available) return;
    keyboardMouseAvailable = available;
    ++keyboardMouseWindowGeneration;
    if (!available) invalidateKeyboardMouse();
    refresh();
}

QVariantMap SetupBridge::beginKeyboardMouse(const QString &profile)
{
    if (suspended || !keyboardMouseAvailable || !currentProfile(profile)) return {{"accepted", false}};
    invalidateKeyboardMouse();
    keyboardMouseForm = QUuid::createUuid().toString(QUuid::WithoutBraces);
    return {{"accepted", true}, {"form", keyboardMouseForm}};
}

QVariantMap SetupBridge::endKeyboardMouse(const QString &form)
{
    if (!form.isEmpty() && form == keyboardMouseForm) invalidateKeyboardMouse();
    return {{"accepted", true}};
}

QVariantMap SetupBridge::setKeyboardMouseSetting(const QString &name, const QVariant &value, const QString &form, const QString &profile)
{
    const auto invalid = [this] { return QVariantMap{{"accepted", false}, {"message", tr("Choose a valid setting value.")}}; };
    if (!keyboardMouseFormValid(form, profile)) return invalid();
    auto *settings = backend->qmlSettings();
    if (name == "keyboardEnabled" || name == "mouseTouchEnabled") {
        if (settings->mouseCameraEnabled() || value.metaType().id() != QMetaType::Bool) return invalid();
        if (name == "keyboardEnabled") settings->setKeyboardEnabled(value.toBool());
        else settings->setMouseTouchEnabled(value.toBool());
        return {{"accepted", true}};
    }
    if (!settings->mouseCameraSupported()) return invalid();
    if (name == "mouseCameraEnabled" || name == "mouseInvertVertical") {
        if (value.metaType().id() != QMetaType::Bool || (name != "mouseCameraEnabled" && !settings->mouseCameraEnabled())) return invalid();
        if (name == "mouseCameraEnabled") settings->setMouseCameraEnabled(value.toBool());
        else settings->setMouseInvertVertical(value.toBool());
        return {{"accepted", true}};
    }
    const auto type = value.metaType().id();
    if (!settings->mouseCameraEnabled() || (type != QMetaType::Double && type != QMetaType::Int && type != QMetaType::UInt && type != QMetaType::LongLong)) return invalid();
    const double number = value.toDouble();
    if (!std::isfinite(number)) return invalid();
    if (name == "mouseHorizontalSensitivity" || name == "mouseVerticalSensitivity") {
        if (number < settings->mouseSensitivityMinimum() || number > settings->mouseSensitivityMaximum()) return invalid();
        if (name == "mouseHorizontalSensitivity") settings->setMouseHorizontalSensitivity(number);
        else settings->setMouseVerticalSensitivity(number);
    } else if (name == "mouseDeadZoneCompensation") {
        if (number < 0 || number > settings->mouseCompensationMaximum()) return invalid();
        settings->setMouseDeadZoneCompensation(number);
    } else if (name == "mouseResponseExponent") {
        if (number < settings->mouseExponentMinimum() || number > settings->mouseExponentMaximum()) return invalid();
        settings->setMouseResponseExponent(number);
    } else return invalid();
    return {{"accepted", true}};
}

QVariantMap SetupBridge::captureKeyboardMouse(const QVariant &action, const QVariant &slot, bool legacy, const QString &form, const QString &profile)
{
    if (!keyboardMouseFormValid(form, profile)) return {{"accepted", false}};
    const auto integer = [](const QVariant &value) {
        const auto type = value.metaType().id();
        return (type == QMetaType::Double || type == QMetaType::Int || type == QMetaType::UInt || type == QMetaType::LongLong)
            && std::isfinite(value.toDouble()) && value.toDouble() == std::floor(value.toDouble());
    };
    if (!integer(action) || !integer(slot) || action.toDouble() < 0 || action.toDouble() > std::numeric_limits<int>::max()) return {{"accepted", false}};
    auto *settings = backend->qmlSettings();
    if (legacy) {
        if (settings->mouseCameraEnabled() || slot.toDouble() != 0 || !backend->nativeSettings()->GetControllerMapping().contains(action.toInt())) return {{"accepted", false}};
    } else if (!settings->mouseCameraSupported() || !settings->mouseCameraEnabled()
        || action.toDouble() < 0 || action.toDouble() >= KeyboardMouseAction::Count || slot.toDouble() < 0 || slot.toDouble() > 1) return {{"accepted", false}};
    emit keyboardMouseCaptureRequested(action.toInt(), slot.toInt(), legacy, form, profile);
    return {{"accepted", true}};
}

QVariantMap SetupBridge::resetKeyboardMouse(const QString &form, const QString &profile)
{
    if (!keyboardMouseFormValid(form, profile) || !backend->qmlSettings()->mouseCameraSupported()) return {{"accepted", false}};
    backend->qmlSettings()->resetKeyboardMouseDefaults();
    return {{"accepted", true}};
}

QVariantMap SetupBridge::resetKeyboardKeys(const QString &form, const QString &profile)
{
    if (!keyboardMouseFormValid(form, profile) || backend->qmlSettings()->mouseCameraEnabled()) return {{"accepted", false}};
    backend->qmlSettings()->clearKeyMapping();
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

QVariantMap SetupBridge::saveManualAddress(const QString &value, const QString &registrationKey, const QString &profile)
{
    const auto reject = [this](const char *text) { return QVariantMap{{"accepted", false}, {"message", tr(text)}}; };
    if (!currentProfile(profile)) return reject(QT_TR_NOOP("The profile changed. Open this form again."));
    if (state.value("connectionBusy").toBool()) return reject(QT_TR_NOOP("A connection is already active"));
    const auto address = value.trimmed();
    static const QRegularExpression hostname(QStringLiteral("^(?=.{1,253}$)(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)*[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.?$"));
    static const QRegularExpression numeric(QStringLiteral("^[0-9.]+$"));
    QHostAddress ip;
    if (!ip.setAddress(address) && (!hostname.match(address).hasMatch() || numeric.match(address).hasMatch()))
        return reject(QT_TR_NOOP("Enter an IP address or hostname without a port or URL."));
    int index = -1;
    if (registrationKey != "later") {
        const auto id = keys.key(registrationKey);
        const auto hosts = backend->qmlSettings()->registeredHosts();
        for (int i = 0; i < hosts.size(); ++i) {
            if (id != "registration:" + hosts.at(i).toMap().value("mac").toString()) continue;
            if (index >= 0) return reject(QT_TR_NOOP("The registration is unavailable. Choose a console again."));
            index = i;
        }
        if (index < 0) return reject(QT_TR_NOOP("The registration is unavailable. Choose a console again."));
    }
    for (const auto &host : backend->nativeSettings()->GetManualHosts())
        if (host.GetHost() == address) return reject(QT_TR_NOOP("This address is already saved."));
    const auto before = backend->nativeSettings()->GetManualHosts().size();
    backend->addManualHost(index, address);
    return {{"accepted", backend->nativeSettings()->GetManualHosts().size() == before + 1}};
}

QVariantMap SetupBridge::manageConsole(const QString &action, const QString &key, const QString &pin)
{
    const auto unavailable = [this] { return QVariantMap{{"accepted", false}, {"message", tr("Console unavailable. Close this screen and select the console again.")}}; };
    if (generation != backend->profileGeneration() || key.isEmpty() || state.value("connectionBusy").toBool()) return unavailable();
    const auto id = keys.key(key);
    if (id.isEmpty()) return unavailable();
    if (action == "deleteRegistration") {
        const auto hosts = backend->qmlSettings()->registeredHosts();
        int found = -1;
        for (int i = 0; i < hosts.size(); ++i) {
            if (id != "registration:" + hosts.at(i).toMap().value("mac").toString()) continue;
            if (found >= 0) return unavailable();
            found = i;
        }
        if (found < 0) return unavailable();
        backend->qmlSettings()->deleteRegisteredHost(found);
        return {{"accepted", true}};
    }
    if (action == "unhide") {
        const auto hosts = backend->hiddenHosts();
        QString mac;
        for (const auto &value : hosts) {
            const auto current = value.toMap().value("mac").toString();
            if (id != "hidden:" + current) continue;
            if (!mac.isEmpty()) return unavailable();
            mac = current;
        }
        if (mac.isEmpty()) return unavailable();
        backend->unhideHost(mac);
        return {{"accepted", true}};
    }
    int index;
    QString nickname;
    if (!resolveConsole(key, index, nickname)) return unavailable();
    const auto host = backend->hosts().at(index).toMap();
    if (action == "pin" && host.value("registered").toBool())
        return {{"accepted", backend->setConsolePin(index, pin)}};
    if (action == "deleteManual" && host.value("manual").toBool()) {
        backend->deleteHost(index);
        return {{"accepted", true}};
    }
    if (action == "hide" && host.value("discovered").toBool() && !host.value("registered").toBool() && !host.value("manual").toBool()) {
        backend->hideHost(host.value("mac").toString(), host.value("name").toString());
        return {{"accepted", true}};
    }
    return unavailable();
}

QVariantMap SetupBridge::setRendererSetting(const QString &name, const QVariant &value, const QString &profileGeneration)
{
    const auto reject = [this] { return QVariantMap{{"accepted", false}, {"message", tr("Choose a valid renderer setting value.")}}; };
    if(!currentProfile(profileGeneration)) return {{"accepted", false}, {"message", tr("The profile changed. Try again in the current profile.")}};
    auto *settings = backend->qmlSettings();
    const auto type = value.metaType().id();
    const bool numeric = type == QMetaType::Double || type == QMetaType::Int || type == QMetaType::UInt || type == QMetaType::LongLong;
    const auto number = value.toDouble();
    if(name == "placeboColorMappingEnabled") {
        if(!(settings->videoPreset() == 5)) return reject();
        if(type != QMetaType::Bool) return reject();
        settings->setPlaceboColorMappingEnabled(value.toBool());
        return {{"accepted", true}};
    }
    if(name == "placeboColorMappingPreset") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 2 || std::floor(number) != number) return reject();
        settings->setPlaceboColorMappingPreset(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboGamutMappingFunction") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 9 || std::floor(number) != number) return reject();
        settings->setPlaceboGamutMappingFunction(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboGamutMappingPerceptualDeadzone") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboGamutMappingFunction() == 1))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 1.0) return reject();
        settings->setPlaceboGamutMappingPerceptualDeadzone(number);
        return {{"accepted", true}};
    }
    if(name == "placeboGamutMappingPerceptualStrength") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboGamutMappingFunction() == 1))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 1.0) return reject();
        settings->setPlaceboGamutMappingPerceptualStrength(number);
        return {{"accepted", true}};
    }
    if(name == "placeboGamutMappingColorimetricGamma") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboGamutMappingFunction() == 3 || settings->placeboGamutMappingFunction() == 5 || settings->placeboGamutMappingFunction() == 7))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 10.0) return reject();
        settings->setPlaceboGamutMappingColorimetricGamma(number);
        return {{"accepted", true}};
    }
    if(name == "placeboGamutMappingSoftClipKnee") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboGamutMappingFunction() == 1 || settings->placeboGamutMappingFunction() == 2))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 1.0) return reject();
        settings->setPlaceboGamutMappingSoftClipKnee(number);
        return {{"accepted", true}};
    }
    if(name == "placeboGamutMappingSoftClipDesat") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboGamutMappingFunction() == 2))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 1.0) return reject();
        settings->setPlaceboGamutMappingSoftClipDesat(number);
        return {{"accepted", true}};
    }
    if(name == "placeboGamutMappingLut3dSizeI") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 1024.0 || std::floor(number) != number) return reject();
        settings->setPlaceboGamutMappingLut3dSizeI(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboGamutMappingLut3dSizeC") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 1024.0 || std::floor(number) != number) return reject();
        settings->setPlaceboGamutMappingLut3dSizeC(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboGamutMappingLut3dSizeH") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 1024.0 || std::floor(number) != number) return reject();
        settings->setPlaceboGamutMappingLut3dSizeH(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboGamutMappingLut3dTricubicEnabled") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0)) return reject();
        if(type != QMetaType::Bool) return reject();
        settings->setPlaceboGamutMappingLut3dTricubicEnabled(value.toBool());
        return {{"accepted", true}};
    }
    if(name == "placeboGamutMappingGamutExpansionEnabled") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0)) return reject();
        if(type != QMetaType::Bool) return reject();
        settings->setPlaceboGamutMappingGamutExpansionEnabled(value.toBool());
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingFunction") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 11 || std::floor(number) != number) return reject();
        settings->setPlaceboToneMappingFunction(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingKneeAdaptation") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboToneMappingFunction() == 1 || settings->placeboToneMappingFunction() == 2 || settings->placeboToneMappingFunction() == 3))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 1.0) return reject();
        settings->setPlaceboToneMappingKneeAdaptation(number);
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingKneeMinimum") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboToneMappingFunction() == 1 || settings->placeboToneMappingFunction() == 2 || settings->placeboToneMappingFunction() == 3))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 0.5) return reject();
        settings->setPlaceboToneMappingKneeMinimum(number);
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingKneeMaximum") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboToneMappingFunction() == 1 || settings->placeboToneMappingFunction() == 2 || settings->placeboToneMappingFunction() == 3))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.5 || number > 1.0) return reject();
        settings->setPlaceboToneMappingKneeMaximum(number);
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingKneeDefault") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 1.0) return reject();
        settings->setPlaceboToneMappingKneeDefault(number);
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingKneeOffset") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboToneMappingFunction() == 4))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.5 || number > 2.0) return reject();
        settings->setPlaceboToneMappingKneeOffset(number);
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingSlopeTuning") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboToneMappingFunction() == 1))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 10.0) return reject();
        settings->setPlaceboToneMappingSlopeTuning(number);
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingSlopeOffset") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboToneMappingFunction() == 1))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 1.0) return reject();
        settings->setPlaceboToneMappingSlopeOffset(number);
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingSplineContrast") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboToneMappingFunction() == 1))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 1.5) return reject();
        settings->setPlaceboToneMappingSplineContrast(number);
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingReinhardContrast") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboToneMappingFunction() == 6))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 1.0) return reject();
        settings->setPlaceboToneMappingReinhardContrast(number);
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingLinearKnee") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboToneMappingFunction() == 7 || settings->placeboToneMappingFunction() == 9))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 1.0) return reject();
        settings->setPlaceboToneMappingLinearKnee(number);
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingExposure") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0 && (settings->placeboToneMappingFunction() == 10 || settings->placeboToneMappingFunction() == 11))) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 10.0) return reject();
        settings->setPlaceboToneMappingExposure(number);
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingInverseToneMappingEnabled") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0)) return reject();
        if(type != QMetaType::Bool) return reject();
        settings->setPlaceboToneMappingInverseToneMappingEnabled(value.toBool());
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingMetadata") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0 || number > 4 || std::floor(number) != number) return reject();
        settings->setPlaceboToneMappingMetadata(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingToneLutSize") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 4096.0 || std::floor(number) != number) return reject();
        settings->setPlaceboToneMappingToneLutSize(int(number));
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingContrastRecovery") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 0.0 || number > 2.0) return reject();
        settings->setPlaceboToneMappingContrastRecovery(number);
        return {{"accepted", true}};
    }
    if(name == "placeboToneMappingContrastSmoothness") {
        if(!(settings->videoPreset() == 5 && settings->placeboColorMappingEnabled() == true && settings->placeboColorMappingPreset() == 0)) return reject();
        if(!numeric || !std::isfinite(number) || number < 1.0 || number > 32.0) return reject();
        settings->setPlaceboToneMappingContrastSmoothness(number);
        return {{"accepted", true}};
    }
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
