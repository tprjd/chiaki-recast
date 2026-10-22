#include "setupbridge.h"
#include "qmlbackend.h"
#include "keyboardmousebindings.h"
#include "qmlcontroller.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include <QDateTime>
#include <limits>
#include <QGuiApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <cmath>
#include <QHostAddress>
#include <QRegularExpression>

SetupBridge::SetupBridge(QmlBackend *backend, QObject *parent)
    : QObject(parent), backend(backend)
{
    connect(backend, &QmlBackend::steamShortcutMessage, this,
        [this](const QString &operation, const QString &message, bool ok, bool done) {
            if (operation != steamForm || steamPhase != "progress" || steamProfile != this->backend->profileGeneration()) return;
            steamDiagnostics += message + "\n";
            if (done) {
                steamPhase = ok ? "success" : "failure";
                if (ok) this->backend->qmlSettings()->setAddSteamShortcutAsk(false);
            }
            refresh();
        });
    connect(backend, &QmlBackend::autoConnectChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::hostsChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::hiddenHostsChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::registeredHostsChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::discoveryEnabledChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::connectionAttemptChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::autoConnectMacChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::windowChanged, this, [this] {
        if (this->backend->qmlWindow()) { invalidateControllerMapping(); invalidateSteamShortcut(); }
        refresh();
    });
    connect(backend->qmlSettings(), &QmlSettings::streamerModeChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::currentProfileChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::psnAuthAttemptChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::registrationAttemptChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::accountLookupChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::psnRefreshChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::psnTokenChanged, this, [this] { credentialsRejected = false; refresh(); });
    connect(backend, &QmlBackend::psnCredsExpired, this, [this] { credentialsRejected = true; refresh(); });
    connect(backend->qmlSettings(), &QmlSettings::psnAuthTokenChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::psnRefreshTokenChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::psnAuthTokenExpiryChanged, this, &SetupBridge::refresh);
    connect(backend->qmlSettings(), &QmlSettings::psnAccountIdChanged, this, &SetupBridge::refresh);
    accountExpiry.setSingleShot(true);
    connect(&accountExpiry, &QTimer::timeout, this, &SetupBridge::refresh);
    auto *settings = backend->qmlSettings();
    connect(settings, &QmlSettings::profilesChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::currentProfileChanged, this, [this] {
        invalidateControllerMapping(tr("The profile changed. Select the controller again."));
    });
    connect(backend, &QmlBackend::controllersChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::currentControllerMappingChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::currentControllerTypeChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::controllerMappingAlteredChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::enableAnalogStickMappingChanged, this, &SetupBridge::refresh);
    connect(backend, &QmlBackend::controllerMappingInProgressChanged, this, [this] {
        if (controllerOperation.isEmpty()) return;
        if (!this->backend->controllerMappingInProgress()) {
            invalidateControllerMapping(tr("The controller was disconnected. Connect it and try again."));
        } else if (controllerReset) {
            controllerOperation.clear();
            this->backend->controllerMappingReset();
            controllerPhase.clear();
            controllerMessage = tr("Controller mapping reset.");
            controllerNavigation(true);
            refresh();
        } else {
            controllerPhase = "edit";
            controllerNavigation(true);
            refresh();
        }
    });
    connect(backend, &QmlBackend::controllerMappingSteamControllerSelected, this, [this] {
        invalidateControllerMapping(tr("This controller is managed by Steam. Use Steam to map it, or disable Steam Input before mapping here."));
    });
    connect(backend, &QmlBackend::controllerMappingButtonSelected, this, [this](const QString &input) {
        if (controllerOperation.isEmpty() || controllerPhase != "capture" || !this->backend->controllerMappingInProgress()) return;
        this->backend->controllerMappingButtonQuit();
        controllerPhase = "edit";
        this->backend->updateButton(controllerButton, input, controllerSlot);
        controllerNavigation(true);
        refresh();
    });
    connect(settings, &QmlSettings::allowJoystickBackgroundEventsChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::buttonsByPositionChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::dpadTouchEnabledChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::dpadTouchIncrementChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::dpadTouchShortcut1Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::dpadTouchShortcut2Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::dpadTouchShortcut3Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::dpadTouchShortcut4Changed, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::rumbleHapticsIntensityChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::hapticOverrideChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::currentProfileChanged, this, &SetupBridge::invalidateKeyboardMouse);
    connect(settings, &QmlSettings::mouseCameraEnabledChanged, this, [this] { emit keyboardMouseClosed(); refresh(); });
    connect(settings, &QmlSettings::mouseResponseChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::keyboardMouseBindingsChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::keyboardEnabledChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::mouseTouchEnabledChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::controllerMappingChanged, this, &SetupBridge::refresh);
    decoders = settings->availableDecoders();
    connect(settings, &QmlSettings::logSanitizeChanged, this, &SetupBridge::refresh);
    connect(settings, &QmlSettings::logVerboseChanged, this, &SetupBridge::refresh);
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

SetupBridge::~SetupBridge()
{
    const bool mapping = !controllerOperation.isEmpty();
    controllerOperation.clear();
    if (mapping) backend->controllerMappingQuit();
    backend->setControllerNavigationEnabled(true);
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
        profileKeys.clear();
        selectedKey.clear();
        credentialsRejected = false;
        reminderKind.clear();
        reminderId.clear();
        steamForm.clear();
        steamPhase.clear();
        steamPath.clear();
        steamName.clear();
        steamOptions.clear();
        steamDiagnostics.clear();
        steamFromReminder = false;
    }
    const auto hosts = backend->hosts();
    QHash<QString, int> counts;
    for (const auto &value : hosts) {
        const auto host = value.toMap();
        if (host.value("display").toBool()) ++counts[identity(host)];
    }
    QHash<QString, QString> currentKeys;
    QVariantList consoles, registrations, hiddenConsoles;
    QString autoConnectKey;
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
        const auto key = opaqueKey("registration:" + mac);
        if (mac == backend->qmlSettings()->autoConnectMac()) autoConnectKey = key;
        registrations.append(QVariantMap{{"key", key},
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
        if (host.value("discovered").toBool() && !host.value("registered").toBool() && !host.value("duid").toString().isEmpty()) actions.append("autoRegister");
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
    const char *texts[] = {QT_TR_NOOP("Steam shortcut"),
        QT_TR_NOOP("PSN sign-in"),
        QT_TR_NOOP("Set up now"),
        QT_TR_NOOP("Don't ask again"),
        QT_TR_NOOP("Later"), QT_TR_NOOP("Done"),
        QT_TR_NOOP("Add to Steam"),
        QT_TR_NOOP("Create or update a non-Steam game for the selected profile."),
        QT_TR_NOOP("Steam game name"),
        QT_TR_NOOP("Enter a name for the Steam game."),
        QT_TR_NOOP("Launch options (optional)"),
        QT_TR_NOOP("Current profile default"),
        QT_TR_NOOP("The default launch options select the current profile. You can edit them before you create the shortcut."),
        QT_TR_NOOP("Choose Steam folder"),
        QT_TR_NOOP("Create shortcut"),
        QT_TR_NOOP("Wait for Steam files to finish writing. This operation cannot be cancelled."),
        QT_TR_NOOP("Steam shortcut ready"),
        QT_TR_NOOP("Steam shortcut was not completed"),
        QT_TR_NOOP("Restart Steam yourself to see the shortcut in your library."),
        QT_TR_NOOP("Check the details and Steam folder, then try again. Some files may already have changed."),
        QT_TR_NOOP("Technical details"),
        QT_TR_NOOP("A shortcut with the same executable and launch options is updated. Artwork and the Steam Deck controller configuration are also applied."),
        QT_TR_NOOP("Steam setup help"),
        QT_TR_NOOP("Create or update a non-Steam game for Chiaki Recast with the official artwork and controller layout. You can also set this up later from the console library."),
        QT_TR_NOOP("Sign in to PSN to register consoles automatically and use Remote Play outside your home network. Sign-in saves your account credentials in Chiaki Recast. You can also set this up later from PSN account."),
        QT_TR_NOOP("Back to Renderer"), QT_TR_NOOP("Configure color mapping in Renderer"),
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
        QT_TR_NOOP("Use the same PlayStation account as your console. The Sony sign-in page and your external browser can show account details."),
        QT_TR_NOOP("Refresh remote consoles"), QT_TR_NOOP("Refreshing PSN credentials"), QT_TR_NOOP("Finding remote consoles"),
        QT_TR_NOOP("Connecting through PSN"), QT_TR_NOOP("Connection cancelled"), QT_TR_NOOP("Waiting for connection cleanup"),
        QT_TR_NOOP("Could not start Remote Play"), QT_TR_NOOP("Cancel"), QT_TR_NOOP("Back to consoles"),
        QT_TR_NOOP("Auto-connect at startup"), QT_TR_NOOP("Off"), QT_TR_NOOP("Wake console"), QT_TR_NOOP("Wake and connect"),
        QT_TR_NOOP("Waiting for the console to wake"), QT_TR_NOOP("Console is awake"), QT_TR_NOOP("Connecting to console"),
        QT_TR_NOOP("Auto-connect"), QT_TR_NOOP("Waiting for the selected console"), QT_TR_NOOP("Stop auto-connect"),
        QT_TR_NOOP("Automatic registration"), QT_TR_NOOP("Registering through PSN"), QT_TR_NOOP("Console registered"),
        QT_TR_NOOP("Waiting for internet"), QT_TR_NOOP("Starting PSN connection"), QT_TR_NOOP("Registering console"),
        QT_TR_NOOP("Automatic registration requires your main PS4 console registered to your PSN account."),
        QT_TR_NOOP("Could not start Remote Play"), QT_TR_NOOP("Cancel"), QT_TR_NOOP("Back to consoles")};
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
    const char *registrationTexts[] = {
        QT_TR_NOOP("Register console"),
        QT_TR_NOOP("View registration progress"),
        QT_TR_NOOP("Register selected console"),
        QT_TR_NOOP("Console and firmware"),
        QT_TR_NOOP("PS4 firmware before 7.0"),
        QT_TR_NOOP("PS4 firmware 7.0 to before 8.0"),
        QT_TR_NOOP("PS4 firmware 8.0 or later"),
        QT_TR_NOOP("Console address"),
        QT_TR_NOOP("Use an IP address, hostname, or 255.255.255.255 for local broadcast."),
        QT_TR_NOOP("PSN Online ID"),
        QT_TR_NOOP("PSN Account ID in Base64"),
        QT_TR_NOOP("Use your Account ID encoded as Base64. This is different from your PSN username."),
        QT_TR_NOOP("Use saved Account ID"),
        QT_TR_NOOP("Public lookup"),
        QT_TR_NOOP("PSN sign-in"),
        QT_TR_NOOP("Remote Play pairing code"),
        QT_TR_NOOP("Console login PIN, optional"),
        QT_TR_NOOP("Get your pairing code"),
        QT_TR_NOOP("Use the same PlayStation Network account as the user signed in on your console."),
        QT_TR_NOOP("Open Settings, System, then Remote Play. Enable Remote Play and select Link Device."),
        QT_TR_NOOP("Open Settings, then Remote Play Connection Settings. Enable Remote Play and select Add Device."),
        QT_TR_NOOP("Keep your console on and connected to your network. Enter the eight-digit code before it expires."),
        QT_TR_NOOP("The Remote Play pairing code pairs the console. The optional console login PIN signs in to your console user."),
        QT_TR_NOOP("Registering console"),
        QT_TR_NOOP("Console registered"),
        QT_TR_NOOP("Registration failed"),
        QT_TR_NOOP("Address hidden"),
        QT_TR_NOOP("Waiting for the console. Keep it on and connected to your network."),
        QT_TR_NOOP("Closing this screen does not cancel registration. The operation can continue in the background."),
        QT_TR_NOOP("Correct details and retry"),
        QT_TR_NOOP("Diagnostics are hidden in streamer mode."),
        QT_TR_NOOP("Hide diagnostics"),
        QT_TR_NOOP("Show diagnostics"),
        QT_TR_NOOP("Close"),
        QT_TR_NOOP("Find a public Account ID"),
        QT_TR_NOOP("This sends your PSN username to the third-party service psn.flipscreen.games. Your PSN privacy settings must allow anyone to find you in search. Public lookup retrieves your Account ID. It does not sign you in or save authentication credentials."),
        QT_TR_NOOP("PSN username"),
        QT_TR_NOOP("Retry lookup"),
        QT_TR_NOOP("Find Account ID"),
        QT_TR_NOOP("Cancel"),
        QT_TR_NOOP("The profile changed. Close this screen and open registration again."),
        QT_TR_NOOP("Registration did not start. Check your details and try again."),
        QT_TR_NOOP("The registration context changed. Open registration again."),
        QT_TR_NOOP("No valid saved Account ID is available.")};
    for (const auto *text : registrationTexts) strings.insert(QString::fromUtf8(text), tr(text));
    const auto *native = backend->nativeSettings();
    const bool configured = !native->GetPsnAuthToken().isEmpty() && !native->GetPsnRefreshToken().isEmpty()
        && !native->GetPsnAccountId().isEmpty();
    const auto expiry = QDateTime::fromString(native->GetPsnAuthTokenExpiry(), native->GetTimeFormat());
    const auto remaining = QDateTime::currentDateTime().msecsTo(expiry);
    const bool expired = configured && (credentialsRejected || !expiry.isValid() || remaining <= 0);
    accountExpiry.stop();
    if (configured && !expired) accountExpiry.start(int(qMin<qint64>(remaining, std::numeric_limits<int>::max())));
    QVariantMap psn{{"status", configured ? (expired ? "expired" : "ready") : "signedOut"}};
    psn.insert("refresh", backend->psnRefresh());
    const auto attempt = backend->psnAuthAttempt();
    // Keep the bridge allowlist explicit: the native attempt also contains an Account ID.
    for (const auto &field : {"id", "phase", "message", "profileValid"})
        if (attempt.contains(field)) psn.insert(field, attempt.value(field));
    auto *settings = backend->qmlSettings();
    QVariantMap general{{"disconnectAction", settings->disconnectAction()}, {"suspendAction", settings->suspendAction()},
        {"audioVideoDisabled", settings->audioVideoDisabled()}, {"streamerMode", privateMode},
        {"logSanitize", settings->logSanitize()}, {"logVerbose", settings->logVerbose()},
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
    QVariantList devices;
    for (const auto *controller : backend->qmlControllers())
        devices.append(QVariantMap{{"name", controller->name()}, {"type", controller->type().isEmpty() ? tr("Controller") : controller->type()},
            {"playStation", controller->isPS()}, {"steamVirtual", controller->isSteamVirtual()}});
    QVariantList controllerTargets;
    for (int index = 0; index < 23; ++index) {
        const int value = 1 << (index < 18 ? index : index + 8);
        controllerTargets.append(QVariantMap{{"buttonValue", value}, {"buttonName", Settings::GetChiakiControllerButtonName(value)}});
    }
    QVariantMap controllerValues{{"allowJoystickBackgroundEvents", settings->allowJoystickBackgroundEvents()}, {"buttonsByPosition", settings->buttonsByPosition()},
        {"dpadTouchEnabled", settings->dpadTouchEnabled()}, {"dpadTouchIncrement", settings->dpadTouchIncrement()},
        {"dpadTouchShortcut1", settings->dpadTouchShortcut1()}, {"dpadTouchShortcut2", settings->dpadTouchShortcut2()},
        {"dpadTouchShortcut3", settings->dpadTouchShortcut3()}, {"dpadTouchShortcut4", settings->dpadTouchShortcut4()},
        {"rumbleHapticsIntensity", settings->rumbleHapticsIntensity()}, {"hapticOverride", settings->hapticOverride()}};
    QVariantMap controllerSetup{{"values", controllerValues}, {"devices", devices}, {"targets", controllerTargets},
        {"operation", controllerOperation}, {"phase", controllerPhase}, {"message", controllerMessage},
        {"type", backend->currentControllerType()}, {"mappings", backend->currentControllerMapping()},
        {"altered", backend->controllerMappingAltered()}, {"analog", backend->enableAnalogStickMapping()},
        {"button", controllerButton}, {"slot", controllerSlot}};
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
    const auto nativeAttempt = backend->connectionAttempt();
    QVariantMap connection;
    if ((!backend->qmlWindow() || nativeAttempt.value("registrationOnly").toBool()) && nativeAttempt.value("presented").toBool()) {
        for (const auto &field : {"id", "phase", "route", "context", "waitingForWake", "cancelable", "cleanupPending", "targetValid", "registrationOnly"})
            connection.insert(field, nativeAttempt.value(field));
        if (nativeAttempt.value("registrationOnly").toBool() && backend->qmlWindow()) connection["cleanupPending"] = true;
        connection.insert("errorTitle", privateMode ? tr("Could not start Remote Play") : nativeAttempt.value("errorTitle"));
        connection.insert("errorText", privateMode ? QString() : nativeAttempt.value("errorText"));
    }
    const auto phase = nativeAttempt.value("phase").toString();
    const bool pending = nativeAttempt.value("presented").toBool() && !phase.isEmpty()
        && phase != "failure" && phase != "cancelled" && phase != "registered" && phase != "woken";
    QVariantMap reminder{{"id", reminderId}, {"kind", reminderKind}, {"eligible", !initialAsk && !backend->autoConnect()
        && !backend->qmlWindow() && !backend->qmlSession() && !pending && !backend->registrationBusy()}};
    QVariantMap steam{{"supported", false}, {"form", steamForm}, {"phase", steamPhase}};
#if CHIAKI_GUI_ENABLE_STEAM_SHORTCUT
    steam["supported"] = true;
#endif
    if (!steamForm.isEmpty()) {
        steam["name"] = privateMode ? QString() : steamName;
        steam["options"] = privateMode ? QString() : steamOptions;
        steam["path"] = steamPath.isEmpty() ? tr("Use the default Steam folder")
            : privateMode ? tr("Folder hidden") : QDir::toNativeSeparators(steamPath);
        steam["diagnostics"] = privateMode ? tr("Technical details hidden in streamer mode.") : steamDiagnostics;
    }
    QVariantList profiles;
    QHash<QString, QString> updatedProfileKeys;
    auto names = backend->nativeSettings()->GetProfiles();
    names.prepend(QString());
    int profileIndex = 0;
    for(const auto &name : names) {
        auto key = profileKeys.value(name);
        if(key.isEmpty()) key = QUuid::createUuid().toString(QUuid::WithoutBraces);
        updatedProfileKeys.insert(name, key);
        const bool active = name == backend->qmlSettings()->currentProfile();
        profiles.append(QVariantMap{{"key", key}, {"label", name.isEmpty() ? tr("default") : privateMode ? tr("Profile %1").arg(++profileIndex) : name},
            {"active", active}, {"deletable", !name.isEmpty() && !active}});
    }
    profileKeys = updatedProfileKeys;
    const QVariantMap profileState{{"items", profiles}, {"current", privateMode ? tr("Hidden") : backend->qmlSettings()->currentProfile().isEmpty() ? tr("default") : backend->qmlSettings()->currentProfile()},
        {"logDirectory", privateMode ? tr("Log directory hidden") : backend->qmlSettings()->logDirectory()}};
    state = {{"steam", steam}, {"reminder", reminder}, {"revision", QString::number(++revision)}, {"profileGeneration", QString::number(generation)},
        {"profiles", profileState}, {"controllerSetup", controllerSetup}, {"registrations", registrations}, {"autoConnectKey", autoConnectKey}, {"hiddenConsoles", hiddenConsoles}, {"consoles", consoles}, {"selectedKey", selectedKey}, {"discoveryEnabled", backend->discoveryEnabled()},
        {"streamerMode", privateMode}, {"registration", registration}, {"accountLookup", lookup}, {"connectionBusy", backend->registrationBusy() || backend->qmlWindow() != nullptr || backend->qmlSession() != nullptr
            || nativeAttempt.value("cleanupPending").toBool() || pending}, {"connection", connection}, {"streamError", streamError}, {"psn", psn}, {"mediaSettings", media}, {"mediaChoices", choices}, {"generalSettings", general}, {"rendererSettings", renderer}, {"rendererStatus", rendererStatus}, {"keyboardMouse", keyboardMouse}, {"capabilities", QVariantMap{{"discovery", true}}}, {"strings", strings}};
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
    const bool boolean = name == "streamerMode" || name == "streamMenuEnabled" || name == "logSanitize" || name == "logVerbose"
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
    else if (name == "logSanitize") settings->setLogSanitize(value.toBool());
    else if (name == "logVerbose") settings->setLogVerbose(value.toBool());
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
    if (suspended) invalidateControllerMapping();
    if (suspended) invalidateKeyboardMouse();
    if (suspended) invalidateSteamShortcut();
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
    if (!controllerOperation.isEmpty()) return {{"accepted", false}};
    int index;
    QString nickname;
    if (state.value("connectionBusy").toBool())
        return {{"accepted", false}, {"message", tr("A connection is already active")}};
    if (!resolveConsole(key, index, nickname))
        return {{"accepted", false}, {"message", tr("Console unavailable")}};
    const auto host = backend->hosts().at(index).toMap();
    if (!host.value("registered").toBool() || (!host.value("discovered").toBool() && !host.value("manual").toBool() && host.value("duid").toString().isEmpty())
            || (host.value("discovered").toBool() && host.value("state") != "ready" && host.value("state") != "standby"))
        return {{"accepted", false}, {"message", tr("Select an online registered console, a registered manual address, or a remote console")}};
    selectedKey = key;
    backend->connectToHost(index, nickname);
    return {{"accepted", true}};
}

QVariantMap SetupBridge::wakeConsole(const QString &key)
{
    int index;
    QString nickname;
    if (state.value("connectionBusy").toBool() || !resolveConsole(key, index, nickname)) return {{"accepted", false}};
    return {{"accepted", backend->wakeUpHost(index, nickname)}};
}

QVariantMap SetupBridge::setAutoConnectTarget(const QString &key, const QString &profile)
{
    if (!currentProfile(profile) || state.value("connectionBusy").toBool()) return {{"accepted", false}};
    QString mac;
    if (!key.isEmpty()) {
        const auto id = keys.key(key);
        int matches = 0;
        for (const auto &host : backend->nativeSettings()->GetRegisteredHosts())
            if (id == "registration:" + host.GetServerMAC().ToString()
                    && !backend->nativeSettings()->GetHiddenHostHidden(host.GetServerMAC())) {
                mac = host.GetServerMAC().ToString();
                ++matches;
            }
        if (matches != 1) return {{"accepted", false}, {"message", tr("Console unavailable")}};
    }
    backend->qmlSettings()->setAutoConnectMac(mac);
    return {{"accepted", true}};
}

QVariantMap SetupBridge::stopAutoConnect(const QString &attempt)
{
    return {{"accepted", !attempt.isEmpty() && backend->stopAutoConnect(attempt)}};
}

QVariantMap SetupBridge::autoRegisterConsole(const QString &key, const QString &profile)
{
    int index;
    QString nickname;
    if (!currentProfile(profile) || state.value("connectionBusy").toBool() || !resolveConsole(key, index, nickname))
        return {{"accepted", false}, {"message", tr("Console unavailable")}};
    selectedKey = key;
    const bool accepted = backend->autoRegisterHost(index);
    return {{"accepted", accepted}, {"message", accepted ? QString() : tr("Automatic registration is unavailable for this console.")}};
}

QVariantMap SetupBridge::openRegistration(const QString &key, const QString &profile)
{
    if (!currentProfile(profile)) return {{"accepted", false}, {"message", tr("The profile changed. Open registration again.")}};
    if (state.value("connectionBusy").toBool() && !backend->registrationBusy()) return {{"accepted", false}, {"message", tr("A connection is already active")}};
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
    registrationContext = nativeContext;
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

QVariantMap SetupBridge::refreshPsnConsoles(const QString &profile)
{
    const auto phase = backend->psnRefresh().value("phase").toString();
    if (!currentProfile(profile) || phase == "refreshing" || phase == "listing") return {{"accepted", false}};
    backend->refreshPsnToken();
    return {{"accepted", true}};
}

QVariantMap SetupBridge::cancelConnection(const QString &attempt)
{
    return {{"accepted", backend->cancelConnection(attempt)}};
}

QVariantMap SetupBridge::retryConnection(const QString &attempt)
{
    if (!controllerOperation.isEmpty()) return {{"accepted", false}};
    return {{"accepted", backend->retryConnection(attempt)}};
}

QVariantMap SetupBridge::dismissConnection(const QString &attempt)
{
    return {{"accepted", backend->dismissConnection(attempt)}};
}

QVariantMap SetupBridge::refreshAudioDevices()
{
    backend->qmlSettings()->refreshAudioDevices();
    return {{"accepted", true}};
}

void SetupBridge::controllerNavigation(bool enabled)
{
    const auto generation = ++controllerNavigationGeneration;
    if (!enabled) backend->setControllerNavigationEnabled(false);
    else QTimer::singleShot(0, this, [this, generation] {
        // Drain the selecting/captured SDL event before navigation can resume.
        if (generation == controllerNavigationGeneration) backend->setControllerNavigationEnabled(true);
    });
}

void SetupBridge::invalidateControllerMapping(const QString &message)
{
    if (controllerOperation.isEmpty()) return;
    controllerOperation.clear();
    controllerPhase.clear();
    controllerMessage = message;
    backend->controllerMappingQuit();
    controllerNavigation(true);
    refresh();
}

bool SetupBridge::controllerOperationValid(const QString &operation, const QString &profile) const
{
    return !suspended && keyboardMouseAvailable && !operation.isEmpty() && operation == controllerOperation
        && currentProfile(profile) && !backend->qmlWindow() && !backend->qmlSession();
}

QVariantMap SetupBridge::beginControllerMapping(bool reset, const QString &profile)
{
    if (suspended || !keyboardMouseAvailable || !currentProfile(profile) || backend->qmlWindow() || backend->qmlSession()
            || state.value("connectionBusy").toBool() || !controllerOperation.isEmpty() || backend->qmlControllers().isEmpty()) return {{"accepted", false}};
    controllerOperation = QUuid::createUuid().toString(QUuid::WithoutBraces);
    controllerPhase = "select";
    controllerReset = reset;
    controllerMessage.clear();
    controllerNavigation(false);
    backend->beginControllerMapping(reset);
    refresh();
    return {{"accepted", true}};
}

QVariantMap SetupBridge::controllerMappingAction(const QString &action, const QString &operation, const QString &profile)
{
    if (!controllerOperationValid(operation, profile)) return {{"accepted", false}};
    if (action == "cancel") invalidateControllerMapping();
    else if (action == "cancelCapture" && controllerPhase == "capture") {
        backend->controllerMappingButtonQuit();
        controllerPhase = "edit";
        controllerNavigation(true);
        refresh();
    } else if (controllerPhase != "edit" || !backend->controllerMappingInProgress()) return {{"accepted", false}};
    else if (action == "apply" && backend->controllerMappingAltered()) {
        backend->controllerMappingApply();
        invalidateControllerMapping(tr("Controller mapping saved."));
    } else if (action == "analogOn" || action == "analogOff") backend->setEnableAnalogStickMapping(action == "analogOn");
    else return {{"accepted", false}};
    return {{"accepted", true}};
}

QVariantMap SetupBridge::captureControllerMapping(const QVariant &button, const QVariant &slot, const QString &operation, const QString &profile)
{
    const auto integer = [](const QVariant &value) {
        const auto type = value.metaType().id();
        return (type == QMetaType::Int || type == QMetaType::UInt || type == QMetaType::Double || type == QMetaType::LongLong)
            && std::isfinite(value.toDouble()) && value.toDouble() == std::floor(value.toDouble());
    };
    if (!controllerOperationValid(operation, profile) || controllerPhase != "edit" || !integer(button) || !integer(slot)
        || (slot.toDouble() != 0 && slot.toDouble() != 1)) return {{"accepted", false}};
    bool found = false;
    for (const auto &row : backend->currentControllerMapping())
        if (row.toMap().value("buttonValue").toDouble() == button.toDouble()) found = true;
    if (!found) return {{"accepted", false}};
    controllerButton = button.toInt();
    controllerSlot = slot.toInt();
    controllerPhase = "capture";
    controllerNavigation(false);
    backend->controllerMappingSelectButton();
    refresh();
    return {{"accepted", true}};
}

QVariantMap SetupBridge::setControllerSetting(const QString &name, const QVariant &value, const QString &profile)
{
    const auto invalid = [this] { return QVariantMap{{"accepted", false}, {"message", tr("Choose a valid setting value.")}}; };
    if (suspended || !keyboardMouseAvailable || !currentProfile(profile)) return invalid();
    auto *settings = backend->qmlSettings();
    if (name == "allowJoystickBackgroundEvents" || name == "buttonsByPosition" || name == "dpadTouchEnabled") {
        if (value.metaType().id() != QMetaType::Bool) return invalid();
        if (name == "allowJoystickBackgroundEvents") settings->setAllowJoystickBackgroundEvents(value.toBool());
        else if (name == "buttonsByPosition") settings->setButtonsByPosition(value.toBool());
        else settings->setDpadTouchEnabled(value.toBool());
        return {{"accepted", true}};
    }
    const auto type = value.metaType().id();
    if (type != QMetaType::Int && type != QMetaType::UInt && type != QMetaType::Double && type != QMetaType::LongLong) return invalid();
    const double number = value.toDouble();
    if (!std::isfinite(number)) return invalid();
    if (name == "hapticOverride") {
        if (number < 0 || number > 2) return invalid();
        settings->setHapticOverride(number);
    } else {
        if (number != std::floor(number)) return invalid();
        if (name == "rumbleHapticsIntensity" && number >= 0 && number <= 5) settings->setRumbleHapticsIntensity(number);
        else if (name == "dpadTouchIncrement" && settings->dpadTouchEnabled() && number >= 1 && number <= 1079) settings->setDpadTouchIncrement(number);
        else if (number >= 0 && number <= 16 && settings->dpadTouchEnabled()) {
            if (name == "dpadTouchShortcut1") settings->setDpadTouchShortcut1(number);
            else if (name == "dpadTouchShortcut2") settings->setDpadTouchShortcut2(number);
            else if (name == "dpadTouchShortcut3") settings->setDpadTouchShortcut3(number);
            else if (name == "dpadTouchShortcut4") settings->setDpadTouchShortcut4(number);
            else return invalid();
        } else return invalid();
    }
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
    if (!available) { invalidateKeyboardMouse(); invalidateControllerMapping(); invalidateSteamShortcut(); }
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

void SetupBridge::offerPsnReminder()
{
    auto *settings = backend->qmlSettings();
    if (!settings->remotePlayAsk()) return;
    if (!settings->psnRefreshToken().isEmpty() && !settings->psnAuthToken().isEmpty()
        && !settings->psnAuthTokenExpiry().isEmpty() && !settings->psnAccountId().isEmpty()) {
        settings->setRemotePlayAsk(false);
        return;
    }
    reminderKind = "psn";
    reminderId = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QVariantMap SetupBridge::startReminders(const QString &profile)
{
    if (initialAsk || suspended || !currentProfile(profile) || backend->autoConnect()
        || backend->qmlWindow() || backend->qmlSession() || state.value("connectionBusy").toBool()) return {{"accepted", false}};
    initialAsk = true;
#if CHIAKI_GUI_ENABLE_STEAM_SHORTCUT
    if (backend->qmlSettings()->addSteamShortcutAsk()) {
        reminderKind = "steam";
        reminderId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    } else
#endif
        offerPsnReminder();
    refresh();
    return {{"accepted", true}};
}

QVariantMap SetupBridge::reminderAction(const QString &id, const QString &action, const QString &profile)
{
    if (suspended || !currentProfile(profile) || id.isEmpty() || id != reminderId
        || (action != "now" && action != "never" && action != "later")) return {{"accepted", false}};
    const QString kind = reminderKind;
    reminderId.clear();
    reminderKind.clear();
    if (action == "never") {
        if (kind == "steam") backend->qmlSettings()->setAddSteamShortcutAsk(false);
        else backend->qmlSettings()->setRemotePlayAsk(false);
    }
    if (kind == "steam") {
        if (action == "now") {
            const auto reply = beginSteamShortcut(profile);
            if (!reply.value("accepted").toBool()) { refresh(); return reply; }
            steamFromReminder = true;
        } else offerPsnReminder();
    }
    refresh();
    return {{"accepted", true}, {"route", action == "now" ? kind : QString()}};
}

bool SetupBridge::steamFormValid(const QString &form, const QString &profile) const
{
    return !suspended && keyboardMouseAvailable && !form.isEmpty() && form == steamForm
        && currentProfile(profile) && steamProfile == backend->profileGeneration()
        && !backend->qmlWindow() && !backend->qmlSession();
}

QVariantMap SetupBridge::beginSteamShortcut(const QString &profile)
{
#if CHIAKI_GUI_ENABLE_STEAM_SHORTCUT
    if (suspended || !keyboardMouseAvailable || !currentProfile(profile) || !steamForm.isEmpty()
        || backend->qmlWindow() || backend->qmlSession() || state.value("connectionBusy").toBool()) return {{"accepted", false}};
    const auto name = backend->nativeSettings()->GetCurrentProfile();
    steamName = name.isEmpty() ? tr("Chiaki Recast") : tr("Chiaki Recast %1").arg(name);
    steamOptions = name.isEmpty() ? QString() : "--profile=" + name;
    steamForm = QUuid::createUuid().toString(QUuid::WithoutBraces);
    steamProfile = backend->profileGeneration();
    steamPhase = "form";
    steamPath.clear();
    steamDiagnostics.clear();
    steamFromReminder = false;
    refresh();
    return {{"accepted", true}, {"form", steamForm}};
#else
    return {{"accepted", false}};
#endif
}

QVariantMap SetupBridge::chooseSteamFolder(const QString &form, const QString &profile)
{
    if (!steamFormValid(form, profile) || (steamPhase != "form" && steamPhase != "failure")) return {{"accepted", false}};
    steamPhase = "choosing";
    refresh();
    const auto path = backend->qmlSettings()->chooseSteamBasePath();
    if (!steamFormValid(form, profile) || steamPhase != "choosing") return {{"accepted", false}};
    if (!path.isEmpty()) steamPath = path;
    steamPhase = "form";
    refresh();
    return {{"accepted", true}, {"cancelled", path.isEmpty()}};
}

QVariantMap SetupBridge::submitSteamShortcut(const QString &form, const QVariantMap &fields, const QString &profile)
{
#if CHIAKI_GUI_ENABLE_STEAM_SHORTCUT
    if (!steamFormValid(form, profile) || (steamPhase != "form" && steamPhase != "failure")
        || state.value("connectionBusy").toBool()) return {{"accepted", false}};
    for (auto it = fields.begin(); it != fields.end(); ++it)
        if ((it.key() != "name" && it.key() != "options") || it.value().metaType().id() != QMetaType::QString)
            return {{"accepted", false}};
    const auto name = fields.value("name", steamName).toString().trimmed();
    const auto options = fields.value("options", steamOptions).toString().trimmed();
    if (name.isEmpty() || name.size() > 256 || options.size() > 4096 || name.contains(QChar::Null) || options.contains(QChar::Null))
        return {{"accepted", false}, {"message", tr("Enter a name for the Steam game.")}};
    steamName = name;
    steamOptions = options;
    steamDiagnostics.clear();
    steamPhase = "progress";
    refresh();
    QTimer::singleShot(50, this, [this, form, profile] {
        if (!steamFormValid(form, profile) || steamPhase != "progress") return;
        backend->createSteamShortcut(form, steamName, steamOptions, steamPath);
    });
    return {{"accepted", true}};
#else
    return {{"accepted", false}};
#endif
}

QVariantMap SetupBridge::endSteamShortcut(const QString &form, const QString &profile)
{
    if (!steamFormValid(form, profile) || steamPhase == "progress" || steamPhase == "choosing") return {{"accepted", false}};
    const bool next = steamFromReminder;
    steamFromReminder = false;
    invalidateSteamShortcut();
    if (next) offerPsnReminder();
    refresh();
    return {{"accepted", true}};
}

void SetupBridge::invalidateSteamShortcut()
{
    steamForm.clear();
    steamPhase.clear();
    steamPath.clear();
    steamName.clear();
    steamOptions.clear();
    steamDiagnostics.clear();
    steamFromReminder = false;
    refresh();
}

QVariantMap SetupBridge::openSetupHelp(const QString &topic)
{
    static const QMap<QString, QUrl> links{
        {"steam", QUrl("https://streetpea.github.io/chiaki-ng/setup/steaming/")},
        {"psn", QUrl("https://streetpea.github.io/chiaki-ng/setup/remoteconnection/")}};
    const auto link = links.constFind(topic);
    if (suspended || link == links.constEnd()) return {{"accepted", false}};
    return {{"accepted", QDesktopServices::openUrl(*link)}};
}

QVariantMap SetupBridge::profileAction(const QString &action, const QString &key, const QString &name, bool confirmed, const QString &profile)
{
    if(!currentProfile(profile)) return {{"accepted", false}, {"message", tr("The profile changed. Select the profile again.")}};
    if(suspended || state.value("connectionBusy").toBool()) return {{"accepted", false}, {"message", tr("Finish the current connection before changing profiles.")}};
    QString target;
    if(action == "create") {
        target = name.trimmed();
        if(target.isEmpty()) return {{"accepted", false}, {"message", tr("Enter a profile name.")}};
    } else {
        const auto it = std::find_if(profileKeys.cbegin(), profileKeys.cend(), [&](const QString &value) { return value == key; });
        if(it == profileKeys.cend()) return {{"accepted", false}, {"message", tr("Select an available profile.")}};
        target = it.key();
    }
    if(action == "delete") {
        if(!confirmed || target.isEmpty() || target == backend->qmlSettings()->currentProfile())
            return {{"accepted", false}, {"message", tr("The default and current profiles cannot be deleted. Confirm deletion of another profile.")}};
        const bool deleted = backend->nativeSettings()->DeleteProfile(target);
        refresh();
        return {{"accepted", deleted}, {"message", deleted ? tr("Profile deleted.") : tr("The profile could not be deleted.")}};
    }
    if(action != "create" && action != "switch") return {{"accepted", false}};
    const bool saved = backend->nativeSettings()->SetCurrentProfile(target);
    refresh();
    return {{"accepted", saved}, {"profileGeneration", QString::number(backend->profileGeneration())}, {"message", saved ? tr("Profile selected.") : tr("The profile could not be selected.")}};
}

QVariantMap SetupBridge::settingsFileAction(const QString &action, const QString &profile)
{
    if(!currentProfile(profile)) return {{"accepted", false}, {"message", tr("The profile changed. Try again in the current profile.")}};
    if(action == "exportSettings") return backend->qmlSettings()->exportSettings();
    if(action == "importSettings") {
        if(suspended || state.value("connectionBusy").toBool()) return {{"accepted", false}, {"message", tr("Finish the current connection before importing settings.")}};
        auto reply = backend->qmlSettings()->importSettings();
        reply.insert("profileGeneration", QString::number(backend->profileGeneration()));
        refresh();
        return reply;
    }
    if(action == "openLogDirectory") {
        const bool opened = QDesktopServices::openUrl(QUrl::fromLocalFile(backend->qmlSettings()->logDirectory()));
        return {{"accepted", opened}, {"message", opened ? tr("Log directory opened.") : tr("The log directory could not be opened.")}};
    }
    return {{"accepted", false}};
}
