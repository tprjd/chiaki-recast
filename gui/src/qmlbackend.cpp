#include "qmlbackend.h"
#include "qmlsettings.h"
#include "qmlmainwindow.h"
#include "streamsession.h"
#include "controllermanager.h"
#include "psnaccountid.h"
#include "psntoken.h"
#include "systemdinhibit.h"
#include "chiaki/remote/holepunch.h"
#ifdef Q_OS_MACOS
#include "macWakeSleep.h"
#elif defined(Q_OS_WINDOWS)
#include "windowsWakeSleep.h"
#endif
#if CHIAKI_GUI_ENABLE_STEAM_SHORTCUT
#include "steamtools.h"
#endif

#ifdef CHIAKI_HAVE_WEBENGINE
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include <QWebEngineClientHints>
#endif
#include <QWebEngineCookieStore>
#endif
#include <QUrlQuery>
#include <QMetaObject>
#include <QPointer>
#include <QtGlobal>
#include <QGuiApplication>
#include <QPixmap>
#include <QImageReader>
#include <QProcessEnvironment>
#include <QDesktopServices>
#include <QtConcurrent>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
#include <libavutil/pixfmt.h>
}

static inline bool frame_has_planes(const AVFrame *frame)
{
    if (!frame)
        return false;
    if (frame->hw_frames_ctx)
        return true; // zero copy frames still map via hw context
    const int planes = av_pix_fmt_count_planes(static_cast<AVPixelFormat>(frame->format));
    return frame->data[0] && planes > 0;
}

static bool frame_can_use_direct_render(const AVFrame *frame, bool use_opengl_renderer)
{
    if (!frame || !frame->hw_frames_ctx)
        return true;

    switch (frame->format) {
    case AV_PIX_FMT_VULKAN:
        return true;
#ifdef Q_OS_LINUX
    case AV_PIX_FMT_VAAPI:
        return !use_opengl_renderer;
#endif
    default:
        return false;
    }
}

#define PSN_DEVICES_TRIES 2
#define MAX_PSN_RECONNECT_TRIES 6
#define PSN_INTERNET_WAIT_SECONDS 5
#define WAKEUP_PSN_IGNORE_SECONDS 10
#define WAKEUP_WAIT_SECONDS 25
static QMutex chiaki_log_mutex;
static ChiakiLog *chiaki_log_ctx = nullptr;
static QtMessageHandler qt_msg_handler = nullptr;

static void msg_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker lock(&chiaki_log_mutex);
    if (!chiaki_log_ctx) {
        qt_msg_handler(type, context, msg);
        return;
    }
    ChiakiLogLevel chiaki_level;
    switch (type) {
    case QtDebugMsg:
        chiaki_level = CHIAKI_LOG_DEBUG;
        break;
    case QtInfoMsg:
        chiaki_level = CHIAKI_LOG_INFO;
        break;
    case QtWarningMsg:
        chiaki_level = CHIAKI_LOG_WARNING;
        break;
    case QtCriticalMsg:
        chiaki_level = CHIAKI_LOG_ERROR;
        break;
    case QtFatalMsg:
        chiaki_level = CHIAKI_LOG_ERROR;
        break;
    }
    chiaki_log(chiaki_log_ctx, chiaki_level, "%s", qPrintable(msg));
}

QmlRegist::QmlRegist(uint32_t log_mask, QObject *parent)
    : QObject(parent)
{
    chiaki_log_init(&chiaki_log, log_mask, &QmlRegist::log_cb, this);
}

ChiakiErrorCode QmlRegist::start(const ChiakiRegistInfo &regist_info)
{
    return chiaki_regist_start(&chiaki_regist, &chiaki_log, &regist_info, &QmlRegist::regist_cb, this);
}

void QmlRegist::log_cb(ChiakiLogLevel level, const char *msg, void *user)
{
    chiaki_log_cb_print(level, msg, nullptr);
    auto r = static_cast<QmlRegist*>(user);
    QMetaObject::invokeMethod(r, std::bind(&QmlRegist::log, r, level, QString::fromUtf8(msg)), Qt::QueuedConnection);
}

void QmlRegist::regist_cb(ChiakiRegistEvent *event, void *user)
{
    auto r = static_cast<QmlRegist*>(user);
    switch (event->type) {
    case CHIAKI_REGIST_EVENT_TYPE_FINISHED_SUCCESS:
        QMetaObject::invokeMethod(r, std::bind(&QmlRegist::success, r, *event->registered_host), Qt::QueuedConnection);
        QMetaObject::invokeMethod(r, &QObject::deleteLater, Qt::QueuedConnection);
        break;
    case CHIAKI_REGIST_EVENT_TYPE_FINISHED_FAILED:
        QMetaObject::invokeMethod(r, &QmlRegist::failed, Qt::QueuedConnection);
        QMetaObject::invokeMethod(r, &QObject::deleteLater, Qt::QueuedConnection);
        break;
    default:
        break;
    }
}

QmlBackend::QmlBackend(Settings *settings, QmlMainWindow *window)
    : QObject(window)
    , settings(settings)
    , settings_qml(new QmlSettings(settings, this))
    , window(window)
{
    qt_msg_handler = qInstallMessageHandler(msg_handler);

    const char *uri = "org.streetpea.chiaking";
    qmlRegisterSingletonInstance(uri, 1, 0, "Chiaki", this);
    qmlRegisterUncreatableType<QmlMainWindow>(uri, 1, 0, "ChiakiWindow", {});
    qmlRegisterUncreatableType<QmlSettings>(uri, 1, 0, "ChiakiSettings", {});
    qmlRegisterUncreatableType<StreamSession>(uri, 1, 0, "ChiakiSession", {});

    QObject *frame_obj = new QObject();
    frame_thread = new QThread(frame_obj);
    frame_thread->setObjectName("frame");
    frame_thread->start();
    frame_obj->moveToThread(frame_thread);

    PsnConnectionWorker *worker = new PsnConnectionWorker;
    worker->moveToThread(&psn_connection_thread);
    connect(&psn_connection_thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &QmlBackend::psnConnect, worker, &PsnConnectionWorker::ConnectPsnConnection);
    connect(worker, &PsnConnectionWorker::resultReady, this, [this](StreamSession *owned_session, quint64 attempt_id, const ChiakiErrorCode &err) {
        if (owned_session != session || connection_generation != attempt_id || !psn_worker_pending)
            return;
        psn_worker_pending = false;
        if (connection_cancelled || connection_dismissed || connection_profile_generation != profile_generation) {
            finishPsnCleanup();
            return;
        }
        checkPsnConnection(err);
    });
    connect(&psn_hosts_watcher, &QFutureWatcher<void>::finished, [this] {
        updating_psn_hosts = false;
        if (psn_hosts_refresh_pending) {
            psn_hosts_refresh_pending = false;
            updatePsnHosts();
        }
    });
    psn_connection_thread.start();

    setConnectState(PsnConnectState::NotStarted);
    connect(settings_qml, &QmlSettings::audioVolumeChanged, this, &QmlBackend::updateAudioVolume);
    connect(settings_qml, &QmlSettings::placeboChanged, window, &QmlMainWindow::updatePlacebo);
    connect(settings_qml, &QmlSettings::vSyncEnabledChanged, window, &QmlMainWindow::updateVSync);
    connect(settings_qml, &QmlSettings::streamMenuEnabledChanged, this, &QmlBackend::updateStreamShortcut);
    connect(settings_qml, &QmlSettings::streamMenuShortcut1Changed, this, &QmlBackend::updateStreamShortcut);
    connect(settings_qml, &QmlSettings::streamMenuShortcut2Changed, this, &QmlBackend::updateStreamShortcut);
    connect(settings_qml, &QmlSettings::streamMenuShortcut3Changed, this, &QmlBackend::updateStreamShortcut);
    connect(settings_qml, &QmlSettings::streamMenuShortcut4Changed, this, &QmlBackend::updateStreamShortcut);
    connect(settings, &Settings::RegisteredHostsUpdated, this, &QmlBackend::hostsChanged);
    connect(settings, &Settings::HiddenHostsUpdated, this, &QmlBackend::hiddenHostsChanged);
    connect(settings, &Settings::ManualHostsUpdated, this, &QmlBackend::hostsChanged);
    connect(settings, &Settings::CurrentProfileChanged, this, &QmlBackend::profileChanged);
    connect(this, &QmlBackend::hostsChanged, this, [this] {
        if (connection_target.value("kind") == "auto" && connection_attempt.value("phase") == "waiting"
                && !connection_cancelled && !connection_dismissed && !connectionTargetValid()) {
            clearWakeWait();
            failConnection(tr("Console is unavailable"), tr("Select the console again from the current profile."));
        }
        emit connectionAttemptChanged();
    });
    connect(window, &QmlMainWindow::hasVideoChanged, this, [this] {
        if (session && session->IsConnected() && this->window->hasVideo() && !connection_cancelled && !connection_dismissed
                && connection_profile_generation == profile_generation
                && (connection_attempt.value("phase") == "connecting" || connection_attempt.value("phase") == "video"))
            setConnectionPhase(QStringLiteral("ready"));
    });
    connect(&discovery_manager, &DiscoveryManager::HostsUpdated, this, &QmlBackend::updateDiscoveryHosts);
    discovery_manager.SetSettings(settings);
    setDiscoveryEnabled(true);
    connect(ControllerManager::GetInstance(), &ControllerManager::AvailableControllersUpdated, this, &QmlBackend::updateControllers);
    connect(settings_qml, &QmlSettings::allowJoystickBackgroundEventsChanged, this, &QmlBackend::setAllowJoystickBackgroundEvents);
    connect(window, &QmlMainWindow::activeChanged, this, &QmlBackend::setIsAppActive);
    setAllowJoystickBackgroundEvents();
    setIsAppActive();
    ControllerManager::GetInstance()->SetIsAppActive(window->isActive());
    updateControllers();
    updateControllerMappings();
    connect(settings, &Settings::ControllerMappingsUpdated, this, &QmlBackend::updateControllerMappings);
    connect(this, &QmlBackend::controllersChanged, this, &QmlBackend::updateControllerMappings);
    auto_connect_mac = settings->GetAutoConnectHost().GetServerMAC();
    auto_connect_nickname = settings->GetAutoConnectHost().GetServerNickname();
    psn_auto_connect_timer = new QTimer(this);
    psn_auto_connect_timer->setSingleShot(true);
    psn_reconnect_tries = 0;
    psn_reconnect_timer = new QTimer(this);
    wakeup_start_timer = new QTimer(this);
    wakeup_start_timer->setSingleShot(true);
    connect(psn_auto_connect_timer, &QTimer::timeout, this, &QmlBackend::autoConnectPsn);
    if (autoConnect())
        beginAutoConnect(auto_connect_mac);
    connect(psn_reconnect_timer, &QTimer::timeout, this, &QmlBackend::reconnectPsn);
    connect(wakeup_start_timer, &QTimer::timeout, this, [this]
    {
        if (!wakeup_start || connection_cancelled || connection_dismissed
                || connection_profile_generation != profile_generation || connection_attempt.value("phase") != "waking")
            return;
        clearWakeWait();
        failConnection(tr("Console did not respond"), tr("Timed out waiting for the console to wake. Check its power and network settings."));
        emit wakeupStartFailed();
    });
    sleep_inhibit = new SystemdInhibit(QGuiApplication::applicationName(), tr("Remote Play session"), "sleep", "delay", this);
    connect(sleep_inhibit, &SystemdInhibit::sleep, this, &QmlBackend::goToSleep);
    connect(sleep_inhibit, &SystemdInhibit::resume, this, &QmlBackend::resumeFromSleep);
    connect(ControllerManager::GetInstance(), &ControllerManager::ControllerMoved, sleep_inhibit, &SystemdInhibit::simulateUserActivity);
#ifdef Q_OS_MACOS
    mac_wake_sleep = new MacWakeSleep(this);
    connect(mac_wake_sleep, &MacWakeSleep::wokeUp, this, &QmlBackend::resumeFromSleep);
    connect(ControllerManager::GetInstance(), &ControllerManager::ControllerMoved, mac_wake_sleep, &MacWakeSleep::simulateUserActivity);
#elif defined(Q_OS_WINDOWS)
    windows_wake_sleep = new WindowsWakeSleep(this);
    connect(windows_wake_sleep, &WindowsWakeSleep::wokeUp, this, &QmlBackend::resumeFromSleep);
    connect(windows_wake_sleep, &WindowsWakeSleep::sleeping, this, &QmlBackend::goToSleep);
#endif
    refreshPsnToken();
}

bool QmlBackend::prepareFrameForPresentation(ChiakiFfmpegFrame &frame, bool use_opengl_renderer)
{
    if (!frame.frame)
        return false;

    if (frame.frame->format == AV_PIX_FMT_NONE)
        return false;

    if (!frame.frame->hw_frames_ctx)
        return frame_has_planes(frame.frame);

    const bool direct_render = !disable_zero_copy && frame_can_use_direct_render(frame.frame, use_opengl_renderer);
    if (direct_render)
        return frame_has_planes(frame.frame);

    const int format = frame.frame->format;
    {
        QMutexLocker locker(&hw_transfer_state_mutex);
        if (!logged_hw_transfer_formats.contains(format)) {
            logged_hw_transfer_formats.insert(format);
            const char *format_name = av_get_pix_fmt_name(static_cast<AVPixelFormat>(format));
            qCInfo(chiakiGui) << "Transferring hardware-decoded frames to software for presentation:"
                              << (format_name ? format_name : "unknown");
        }
    }

    AVFrame *sw_frame = av_frame_alloc();
    if (!sw_frame)
        return false;

    if (av_hwframe_transfer_data(sw_frame, frame.frame, 0) < 0) {
        bool should_log_failure = false;
        {
            QMutexLocker locker(&hw_transfer_state_mutex);
            if (!logged_hw_transfer_failures.contains(format)) {
                logged_hw_transfer_failures.insert(format);
                should_log_failure = true;
            }
        }
        if (should_log_failure) {
            const char *format_name = av_get_pix_fmt_name(static_cast<AVPixelFormat>(format));
            qCWarning(chiakiGui) << "Failed to transfer hardware frame for presentation:"
                                 << (format_name ? format_name : "unknown");
        }
        av_frame_free(&sw_frame);
        return false;
    }

    av_frame_copy_props(sw_frame, frame.frame);
    av_frame_free(&frame.frame);
    frame.frame = sw_frame;
    return frame_has_planes(frame.frame);
}

QmlBackend::~QmlBackend()
{
    psn_hosts_future.waitForFinished();
    if (session && psn_worker_pending)
        session->CancelPsnConnection(true);
    if(session)
    {
        chiaki_log_mutex.lock();
        chiaki_log_ctx = nullptr;
        chiaki_log_mutex.unlock();
        session->deleteLater();
        session = nullptr;
    }
#ifdef CHIAKI_HAVE_WEBENGINE
    if(request_interceptor)
        request_interceptor->deleteLater();
#endif
    frame_thread->quit();
    frame_thread->wait();
    frame_thread->parent()->deleteLater();
    psn_connection_thread.quit();
    psn_connection_thread.wait();
}

QmlMainWindow *QmlBackend::qmlWindow() const
{
    return window;
}

QmlSettings *QmlBackend::qmlSettings() const
{
    return settings_qml;
}

StreamSession *QmlBackend::qmlSession() const
{
    return session;
}

void QmlBackend::updateAudioVolume()
{
    if(session)
        session->SetAudioVolume(settings->GetAudioVolume());
}

void QmlBackend::goToSleep()
{
    qCInfo(chiakiGui) << "About to sleep";
    if (session) {
        if (this->settings->GetSuspendAction() == SuspendAction::Sleep)
            session->GoToBed();
        session->Stop();
        if(!session_info.duid.isEmpty())
            psnCancel(true);
        resume_session = true;
    }
}
void QmlBackend::resumeFromSleep()
{
#ifdef Q_OS_WINDOWS
    if(windows_wake_sleep->getWakeState() == WindowsWakeState::AboutToSleep)
    {
        windows_wake_sleep->setWakeState(WindowsWakeState::Awake);
        return;
    }
    windows_wake_sleep->setWakeState(WindowsWakeState::Awake);
#endif
    if (resume_session) {
        qCInfo(chiakiGui) << "Resuming session...";
        resume_session = false;
        if(session_info.duid.isEmpty())
        {
            createSession({
                session_info.settings,
                session_info.target,
                session_info.host,
                session_info.nickname,
                session_info.regist_key,
                session_info.morning,
                session_info.initial_login_pin,
                session_info.duid,
                session_info.auto_regist,
                session_info.fullscreen,
                session_info.zoom,
                session_info.stretch,
            });
        }
        else
        {
            const auto target = connection_target;
            beginConnection(session_info, target);
            connection_attempt["cleanupPending"] = false;
            emit showPsnView();
            setConnectState(PsnConnectState::WaitingForInternet);
            psn_reconnect_timer->start(PSN_INTERNET_WAIT_SECONDS * 1000);
        }
    }
}

QList<QmlController*> QmlBackend::qmlControllers() const
{
    return controllers.values();
}

void QmlBackend::reconnectPsn()
{
    if (session || connection_token || connection_cancelled || connection_dismissed
            || connection_profile_generation != profile_generation || connection_attempt.value("phase") == "failure")
        return;
    const auto attempt_id = connection_generation;
    const auto generation = profile_generation;
    const auto owned = [this, attempt_id, generation] {
        return connection_generation == attempt_id && profile_generation == generation
            && !connection_cancelled && !connection_dismissed;
    };
    QString refresh = this->settings->GetPsnRefreshToken();
    if(refresh.isEmpty())
    {
        qCWarning(chiakiGui) << "No refresh token found, can't refresh PSN token to use PSN remote connection";
        psn_reconnect_tries = 0;
        resume_session = false;
        psn_reconnect_timer->stop();
        setConnectState(PsnConnectState::ConnectFailed);
        return;
    }
    PSNToken *psnToken = new PSNToken(this->settings, this);
    watchConnectionToken(psnToken);
    connect(psnToken, &PSNToken::PSNTokenError, this, [this, owned](const QString &error) {
        if (!owned()) return;
        qCWarning(chiakiGui) << "Internet is currently down...waiting 5 seconds" << error;
        psn_reconnect_tries++;
        if(psn_reconnect_tries < MAX_PSN_RECONNECT_TRIES)
            return;
        else
        {
            resume_session = false;
            psn_reconnect_tries = 0;
            psn_reconnect_timer->stop();
            setConnectState(PsnConnectState::ConnectFailed);
        }
    });
    connect(psnToken, &PSNToken::UnauthorizedError, this, [this, owned] { if (owned()) emit psnCredsExpired(); });
    connect(psnToken, &PSNToken::PSNTokenSuccess, this, [this, owned]() {
        if (!owned()) return;
        qCWarning(chiakiGui) << "PSN Remote Connection Tokens Refreshed. Internet is back up";
        resume_session = false;
        psn_reconnect_tries = 0;
        psn_reconnect_timer->stop();
        session_info.psn_token = this->settings->GetPsnAuthToken();
        createSession(session_info, connection_target, true);
    });
    QString refresh_token = this->settings->GetPsnRefreshToken();
    psnToken->RefreshPsnToken(std::move(refresh_token));
}

void QmlBackend::profileChanged()
{
    ++profile_generation;
    for (auto *request : findChildren<PSNAccountID *>())
        request->Invalidate();
    if (!psn_auth_attempt.isEmpty()) {
        psn_auth_pending = false;
        psn_auth_attempt["phase"] = "failure";
        psn_auth_attempt["profileValid"] = false;
        psn_auth_attempt["message"] = tr("The profile changed. Sign in again in the current profile.");
        emit psnAuthAttemptChanged();
    }
    // Stop replies before the old profile's Settings object can be removed.
    for (auto *token : findChildren<PSNToken *>())
        delete token;
    psn_hosts.clear();
    psn_nickname_hosts.clear();
    waking_sleeping_nicknames.clear();
    if (!connection_attempt.isEmpty()) {
        if (connection_attempt.value("phase") != "ready") {
            connection_attempt["cancelable"] = true;
            cancelConnection(connection_attempt.value("id").toString());
            connection_cancelled = false;
            failConnection(tr("Profile changed"), tr("Select the console again from the current profile."));
        }
        emit connectionAttemptChanged();
    }
    session_pin_request.clear();
    emit sessionPinRequestChanged();
    QString profile = settings->GetCurrentProfile();
    Settings *settings_copy = new Settings(profile);
    if(settings_allocd)
        settings->deleteLater();
    settings_allocd = true;
    settings = settings_copy;
    emit hostsChanged();
    updateControllerMappings();
    connect(settings, &Settings::RegisteredHostsUpdated, this, &QmlBackend::hostsChanged);
    connect(settings, &Settings::HiddenHostsUpdated, this, &QmlBackend::hiddenHostsChanged);
    connect(settings, &Settings::ManualHostsUpdated, this, &QmlBackend::hostsChanged);
    connect(settings, &Settings::CurrentProfileChanged, this, &QmlBackend::profileChanged);
    connect(settings, &Settings::ControllerMappingsUpdated, this, &QmlBackend::updateControllerMappings);
    settings_qml->setSettings(settings);
    discovery_manager.SetSettings(settings);
    window->setSettings(settings);
    setDiscoveryEnabled(true);

    auto_connect_mac = settings->GetAutoConnectHost().GetServerMAC();
    auto_connect_nickname = settings->GetAutoConnectHost().GetServerNickname();
    psn_reconnect_timer->deleteLater();
    psn_auto_connect_timer->deleteLater();
    psn_auto_connect_timer = new QTimer(this);
    psn_auto_connect_timer->setSingleShot(true);
    psn_reconnect_tries = 0;
    psn_reconnect_timer = new QTimer(this);
    connect(psn_auto_connect_timer, &QTimer::timeout, this, &QmlBackend::autoConnectPsn);
    if (autoConnect())
        beginAutoConnect(auto_connect_mac);
    connect(psn_reconnect_timer, &QTimer::timeout, this, &QmlBackend::reconnectPsn);
    sleep_inhibit->deleteLater();
    sleep_inhibit = new SystemdInhibit(QGuiApplication::applicationName(), tr("Remote Play session"), "sleep", "delay", this);
    connect(sleep_inhibit, &SystemdInhibit::sleep, this, &QmlBackend::goToSleep);
    connect(sleep_inhibit, &SystemdInhibit::resume, this, &QmlBackend::resumeFromSleep);
    connect(ControllerManager::GetInstance(), &ControllerManager::ControllerMoved, sleep_inhibit, &SystemdInhibit::simulateUserActivity);
#ifdef Q_OS_MACOS
    mac_wake_sleep->deleteLater();
    mac_wake_sleep = new MacWakeSleep(this);
    connect(mac_wake_sleep, &MacWakeSleep::wokeUp, this, &QmlBackend::resumeFromSleep);
    connect(ControllerManager::GetInstance(), &ControllerManager::ControllerMoved, mac_wake_sleep, &MacWakeSleep::simulateUserActivity);
#elif defined(Q_OS_WINDOWS)
    windows_wake_sleep->deleteLater();
    windows_wake_sleep = new WindowsWakeSleep(this);
    connect(windows_wake_sleep, &WindowsWakeSleep::wokeUp, this, &QmlBackend::resumeFromSleep);
    connect(windows_wake_sleep, &WindowsWakeSleep::sleeping, this, &QmlBackend::goToSleep);
#endif
    refreshPsnToken();
    emit hostsChanged();
    emit hiddenHostsChanged();
}

bool QmlBackend::discoveryEnabled() const
{
    return discovery_manager.GetActive();
}

void QmlBackend::setDiscoveryEnabled(bool enabled)
{
    discovery_manager.SetActive(enabled);
    emit discoveryEnabledChanged();
}

QmlBackend::PsnConnectState QmlBackend::connectState() const
{
    return psn_connect_state;
}

void QmlBackend::checkNickname(QString nickname)
{
    if(!session)
        return;
    if(!settings->GetNicknameRegisteredHostRegistered(nickname))
    {
        emit error(tr("PS4 Console Unregistered"), tr("Can't proceed...please register your PS4 console locally"));
        session->Stop();
    }
}

void QmlBackend::setConnectState(PsnConnectState connect_state)
{
    psn_connect_state = connect_state;
    if (!connection_attempt.isEmpty() && connection_attempt.value("route") == "psn"
            && !connection_cancelled && !connection_dismissed && connection_profile_generation == profile_generation
            && connection_attempt.value("phase") != "failure" && connection_attempt.value("phase") != "registered"
            && connection_attempt.value("phase") != "ready") {
        auto completed = connection_attempt.value("completedStages").toStringList();
        auto complete = [&completed](const QString &stage) { if (!completed.contains(stage)) completed.append(stage); };
        QString phase;
        bool cancelable = true;
        switch (connect_state) {
        case PsnConnectState::WaitingForInternet: phase = "internet"; break;
        case PsnConnectState::InitiatingConnection: phase = "initiating"; break;
        case PsnConnectState::LinkingConsole:
            complete("psn");
            phase = connection_attempt.value("registrationOnly").toBool() ? "registering" : "linking";
            cancelable = false;
            break;
        case PsnConnectState::RegisteringConsole: phase = "registering"; cancelable = false; break;
        case PsnConnectState::RegistrationFinished: complete("registration"); phase = "registered"; break;
        case PsnConnectState::DataConnectionStart: complete("linking"); phase = "data"; break;
        case PsnConnectState::DataConnectionFinished: complete("data"); phase = "video"; cancelable = session && session->IsConnected(); break;
        case PsnConnectState::ConnectFailedStart:
            failConnection(tr("PSN connection did not start"), tr("PlayStation Network could not establish a connection with your console. Try again."));
            break;
        case PsnConnectState::ConnectFailedConsoleUnreachable:
            failConnection(tr("Console is unreachable"), tr("The console could not be contacted through the established connection. Its network may not support Remote Play."));
            break;
        case PsnConnectState::ConnectFailed:
            failConnection(tr("Could not connect over PSN"), tr("The remote connection failed. Check your connection and try again."));
            break;
        default: break;
        }
        connection_attempt["completedStages"] = completed;
        connection_attempt["cancelable"] = cancelable;
        if (!phase.isEmpty()) connection_attempt["phase"] = phase;
        emit connectionAttemptChanged();
    }
    if(connect_state == PsnConnectState::ConnectFailed)
    {
        if(!this->settings->GetGeometry().isEmpty())
        {
            window->setGeometry(this->settings->GetGeometry());
            window->normalTime();
        }
        else
            window->showMaximized();
    }
    emit connectStateChanged();
}

QVariantList QmlBackend::hosts() const
{
    QVariantList out;
    QList<QString> discovered_nicknames;
    QList<ManualHost> discovered_manual_hosts;
    size_t registered_discovered_ps4s = 0;
    auto manual_hosts = settings->GetManualHosts();
    for (const auto &host : discovery_manager.GetHosts()) {
        QVariantMap m;
        HostMAC host_mac = host.GetHostMAC();
        bool registered = settings->GetRegisteredHostRegistered(host_mac);
        bool hidden = settings->GetHiddenHostHidden(host_mac);
        if(registered && hidden)
        {
            settings->RemoveHiddenHost(host_mac);
            bool hidden = false;
        }
        // Update hidden host nickname if it's changed
        if(hidden)
        {
            auto hidden_host = settings->GetHiddenHost(host_mac);
            if(hidden_host.GetNickname() != host.host_name)
            {
                hidden_host.SetNickname(host.host_name);
                settings->RemoveHiddenHost(host_mac);
                settings->AddHiddenHost(hidden_host);
            }
        }
        m["discovered"] = true;
        bool manual = false;
        for(int i = 0; i < manual_hosts.length(); i++)
        {
            const auto &manual_host = manual_hosts.at(i);
            if(manual_host.GetRegistered() && manual_host.GetMAC() == host_mac && manual_host.GetHost() == host.host_addr)
            {
                manual = true;
                discovered_manual_hosts.append(manual_host);
            }
        }
        m["manual"] = manual;
        m["name"] = host.host_name;
        QString duid = "";
        if(!registered)
        {
            if(psn_nickname_hosts.contains(host.host_name))
                duid = psn_nickname_hosts.value(host.host_name).GetDuid();
            else if(!host.ps5)
                duid =  psn_nickname_hosts.value(QString("Main PS4 Console")).GetDuid();
        }
        m["duid"] = duid;
        m["address"] = host.host_addr;
        m["ps5"] = host.ps5;
        m["mac"] = host_mac.ToString();
        m["state"] = chiaki_discovery_host_state_string(host.state);
        m["app"] = host.running_app_name;
        m["titleId"] = host.running_app_titleid;
        m["registered"] = registered;
        m["display"] = hidden ? false : true;
        discovered_nicknames.append(host.host_name);
        out.append(m);
        if(!host.ps5 && registered)
            registered_discovered_ps4s++;
    }
    for (const auto &host : settings->GetManualHosts()) {
        QVariantMap m;
        m["discovered"] = false;
        m["manual"] = true;
        m["name"] = host.GetHost();
        m["duid"] = "";
        m["address"] = host.GetHost();
        m["state"] = "unknown";
        m["registered"] = false;
        m["display"] = discovered_manual_hosts.contains(host) ? false : true;
        if (host.GetRegistered() && settings->GetRegisteredHostRegistered(host.GetMAC())) {
            auto registered = settings->GetRegisteredHost(host.GetMAC());
            m["registered"] = true;
            m["name"] = registered.GetServerNickname();
            m["ps5"] = chiaki_target_is_ps5(registered.GetTarget());
            m["mac"] = registered.GetServerMAC().ToString();
        }
        out.append(m);
    }
    if(registered_discovered_ps4s >= settings->GetPS4RegisteredHostsRegistered())
        discovered_nicknames.append(QString("Main PS4 Console"));
    for (const auto &host : psn_hosts) {
        int registrations = 0;
        for (const auto &registered : settings->GetRegisteredHosts()) {
            if (settings->GetHiddenHostHidden(registered.GetServerMAC())) continue;
            if (host.IsPS5() ? chiaki_target_is_ps5(registered.GetTarget()) && registered.GetServerNickname() == host.GetName()
                             : !chiaki_target_is_ps5(registered.GetTarget()))
                ++registrations;
        }
        if (host.IsPS5() ? registrations != 1 : registrations == 0)
            continue;
        QVariantMap m;
        // Only list PSN remote hosts that aren't discovered locally
        bool discovered = false;
        for (int i = 0; i < discovered_nicknames.size(); ++i)
        {
            if (discovered_nicknames.at(i) == host.GetName())
                discovered = true;
        }
        for (int i = 0; i < waking_sleeping_nicknames.size(); ++i)
        {
            if (waking_sleeping_nicknames.at(i) == host.GetName())
                discovered = true;
        }
        if(discovered)
            continue;
        m["discovered"] = false;
        m["manual"] = false;
        m["display"] = true;
        m["name"] = host.GetName();
        m["duid"] = host.GetDuid();
        m["address"] = "";
        m["registered"] = true;
        m["ps5"] = host.IsPS5();
        out.append(m);
    }
    return out;
}

bool QmlBackend::autoConnect() const
{
    return auto_connect_mac.GetValue();
}

void QmlBackend::psnCancel(bool stop_thread)
{
    if (!stop_thread && !connection_attempt.isEmpty()) {
        cancelConnection(connection_attempt.value("id").toString());
        return;
    }
    if (session)
        session->CancelPsnConnection(stop_thread);
}

void QmlBackend::finishPsnCleanup()
{
    if (psn_worker_pending)
        return;
    chiaki_log_mutex.lock();
    chiaki_log_ctx = nullptr;
    chiaki_log_mutex.unlock();
    if (session)
        session->deleteLater();
    session = nullptr;
    psn_session_started = false;
    connection_attempt["cleanupPending"] = false;
    if (connection_cancelled)
        connection_attempt["phase"] = "cancelled";
    emit connectionAttemptChanged();
    emit sessionChanged(nullptr);
    sleep_inhibit->release();
    setDiscoveryEnabled(true);
}

void QmlBackend::checkPsnConnection(const ChiakiErrorCode &err)
{
    if (!session)
        return;
    if (err == CHIAKI_ERR_SUCCESS) {
        setConnectState(PsnConnectState::LinkingConsole);
        psnSessionStart();
        return;
    }
    setConnectState(err == CHIAKI_ERR_HOST_DOWN ? PsnConnectState::ConnectFailedStart
        : err == CHIAKI_ERR_HOST_UNREACH ? PsnConnectState::ConnectFailedConsoleUnreachable
        : PsnConnectState::ConnectFailed);
    connection_attempt["errorText"] = QString::fromUtf8(chiaki_error_string(err));
    finishPsnCleanup();
}

void QmlBackend::psnSessionStart()
{
    try {
        session->Start();
        psn_session_started = true;
    } catch (const Exception &e) {
        failConnection(tr("Stream failed"), tr("Failed to start Stream Session: %1").arg(e.what()));
        finishPsnCleanup();
        return;
    }
    sleep_inhibit->inhibit();
}

QVariantMap QmlBackend::connectionTargetKey(const QVariantMap &host)
{
    if (host.value("discovered").toBool() && !host.value("mac").toString().isEmpty())
        return {{"kind", "discovered"}, {"mac", host.value("mac").toString().toLower()}};
    if (!host.value("duid").toString().isEmpty())
        return {{"kind", "psn"}, {"duid", host.value("duid")}};
    if (host.value("manual").toBool() && !host.value("address").toString().isEmpty())
        return {{"kind", "manual"}, {"address", host.value("address")}, {"mac", host.value("mac").toString().toLower()}};
    return {};
}

int QmlBackend::resolveConnectionTarget() const
{
    if (connection_profile_generation != profile_generation || connection_target.isEmpty())
        return -1;
    int found = -1;
    const auto listed = hosts();
    for (int i = 0; i < listed.size(); ++i) {
        const auto host = listed.at(i).toMap();
        if (!host.value("display").toBool()
                || (!connection_attempt.value("registrationOnly").toBool() && !host.value("registered").toBool())
                || connectionTargetKey(host) != connection_target)
            continue;
        if (found != -1)
            return -1;
        found = i;
    }
    return found;
}

bool QmlBackend::connectionTargetValid() const
{
    if (connection_profile_generation != profile_generation)
        return false;
    if (connection_target.value("kind") == "auto") {
        for (const auto &host : settings->GetRegisteredHosts())
            if (host.GetServerMAC().ToString() == connection_target.value("mac").toString()
                    && !settings->GetHiddenHostHidden(host.GetServerMAC()))
                return true;
        return false;
    }
    if (!connection_target.isEmpty())
        return resolveConnectionTarget() >= 0;
    // Explicit CLI credentials can be used without a saved console record.
    if (connection_direct_credentials)
        return !session_info.host.isEmpty();
    int matches = 0;
    for (const auto &host : settings->GetRegisteredHosts())
        if (host.GetRPRegistKey() == session_info.regist_key && host.GetRPKey() == session_info.morning
                && !settings->GetHiddenHostHidden(host.GetServerMAC()))
            ++matches;
    return matches == 1 && !session_info.host.isEmpty();
}

QVariantMap QmlBackend::connectionAttempt() const
{
    auto result = connection_attempt;
    if (!result.isEmpty()) {
        result["targetValid"] = connectionTargetValid();
        result["presented"] = !connection_dismissed;
    }
    return result;
}

void QmlBackend::setConnectionPhase(const QString &phase)
{
    if (connection_attempt.isEmpty() || connection_attempt.value("phase") == phase)
        return;
    connection_attempt["phase"] = phase;
    emit connectionAttemptChanged();
}

void QmlBackend::beginConnection(const StreamSessionConnectInfo &info, const QVariantMap &target, const QString &context)
{
    session_pin_request.clear();
    session_pin_auto_submitted = false;
    session_pin_accepted = false;
    emit sessionPinRequestChanged();
    connection_target = target;
    connection_direct_credentials = target.isEmpty();
    for (const auto &host : settings->GetRegisteredHosts())
        if (host.GetRPRegistKey() == info.regist_key && host.GetRPKey() == info.morning)
            connection_direct_credentials = false;
    connection_profile_generation = profile_generation;
    connection_cancelled = false;
    connection_dismissed = false;
    const bool waiting = context == "startup" || !wakeup_nickname.isEmpty();
    connection_attempt = {{"id", QString::number(++connection_generation)},
        {"phase", context == "startup" ? "waiting" : waiting ? "waking" : "connecting"},
        {"route", info.duid.isEmpty() ? "local" : "psn"}, {"name", info.nickname}, {"address", info.host},
        {"ps5", chiaki_target_is_ps5(info.target)}, {"cleanupPending", context != "startup"}, {"cancelable", true},
        {"waitingForWake", waiting}, {"context", context.isEmpty() && waiting ? QStringLiteral("wake") : context},
        {"registrationOnly", info.auto_regist}, {"errorTitle", ""}, {"errorText", ""}};
    const int index = resolveConnectionTarget();
    if (index >= 0)
        connection_attempt["name"] = hosts().at(index).toMap().value("name");
    if (connection_attempt.value("name").toString().isEmpty())
        connection_attempt["name"] = info.host;
    emit connectionAttemptChanged();
}

void QmlBackend::startPsnConnection(const StreamSessionConnectInfo &info, const QVariantMap &target, bool continue_attempt)
{
    if (session || connection_token)
        return;
    session_info = info;
    if (!continue_attempt)
        beginConnection(info, target);
    if (connection_profile_generation != profile_generation || connection_cancelled || connection_dismissed
            || connection_attempt.value("phase") == "failure")
        return;
    const auto attempt_id = connection_generation;
    const auto generation = profile_generation;
    if (continue_attempt && connection_attempt.value("waitingForWake").toBool()) {
        connection_target = target;
        connection_attempt["waitingForWake"] = false;
        connection_attempt["fallback"] = true;
        const int index = resolveConnectionTarget();
        if (index >= 0)
            connection_attempt["name"] = hosts().at(index).toMap().value("name");
        auto_connect_mac = {};
        psn_auto_connect_timer->stop();
        emit autoConnectChanged();
    }
    connection_attempt["route"] = "psn";
    connection_attempt["cleanupPending"] = false;
    setConnectState(PsnConnectState::WaitingForInternet);
    emit showPsnView();
    const auto owned = [this, attempt_id, generation] {
        return connection_generation == attempt_id && profile_generation == generation
            && connection_profile_generation == profile_generation && !connection_cancelled && !connection_dismissed
            && connection_attempt.value("phase") != "failure";
    };
    if (!owned()) return;
    const QString expiry_s = settings->GetPsnAuthTokenExpiry();
    const QString refresh = settings->GetPsnRefreshToken();
    if (expiry_s.isEmpty() || refresh.isEmpty()) {
        failConnection(tr("PSN credentials are unavailable"), tr("Set up your PlayStation Network account before connecting again."));
        return;
    }
    const QDateTime expiry = QDateTime::fromString(expiry_s, settings->GetTimeFormat());
    if (QDateTime::currentDateTime().addSecs(60) < expiry) {
        createSession(info, target, true);
        return;
    }
    auto *token = new PSNToken(settings, this);
    watchConnectionToken(token);
    connect(token, &PSNToken::PSNTokenError, this, [this, owned](const QString &error) {
        if (owned()) failConnection(tr("Could not reach PlayStation Network"), error);
    });
    connect(token, &PSNToken::UnauthorizedError, this, [this, attempt_id, generation] {
        if (connection_generation == attempt_id && profile_generation == generation && !connection_cancelled && !connection_dismissed)
            emit psnCredsExpired();
    });
    connect(token, &PSNToken::PSNTokenSuccess, this, [this, info, target, owned] {
        if (!owned()) return;
        if (!connectionTargetValid()) {
            failConnection(tr("Console is unavailable"), tr("Select the console again from the library."));
            return;
        }
        auto refreshed = info;
        refreshed.psn_token = settings->GetPsnAuthToken();
        createSession(refreshed, target, true);
    });
    token->RefreshPsnToken(refresh);
}

void QmlBackend::watchConnectionToken(PSNToken *token)
{
    connection_token = token;
    const auto attempt_id = connection_generation;
    connection_attempt["cleanupPending"] = true;
    connect(token, &PSNToken::Finished, this, [this, token, attempt_id] {
        if (connection_token != token) return;
        connection_token.clear();
        if (connection_generation == attempt_id && !session) {
            connection_attempt["cleanupPending"] = false;
            emit connectionAttemptChanged();
        }
    });
    connect(token, &PSNToken::Finished, token, &QObject::deleteLater);
}

void QmlBackend::failConnection(const QString &title, const QString &text)
{
    if (connection_attempt.isEmpty() || connection_cancelled || connection_dismissed)
        return;
    connection_attempt["errorTitle"] = title;
    connection_attempt["errorText"] = text;
    connection_attempt["phase"] = "failure";
    connection_attempt["cleanupPending"] = session != nullptr || connection_token;
    session_pin_request.clear();
    emit sessionPinRequestChanged();
    emit connectionAttemptChanged();
}

bool QmlBackend::retryConnection(const QString &attempt_id)
{
    if (attempt_id != connection_attempt.value("id").toString() || connection_dismissed || session
            || connection_attempt.value("cleanupPending").toBool() || connection_attempt.value("phase") != "failure"
            || !connectionTargetValid())
        return false;
    if (connection_target.value("kind") == "auto") {
        for (const auto &host : settings->GetRegisteredHosts())
            if (host.GetServerMAC().ToString() == connection_target.value("mac").toString()) {
                beginAutoConnect(host.GetServerMAC());
                updateDiscoveryHosts();
                return true;
            }
        return false;
    }
    const int index = resolveConnectionTarget();
    if (!connection_target.isEmpty()) {
        const auto host = hosts().at(index).toMap();
        if (connection_attempt.value("registrationOnly").toBool()) {
            auto info = session_info;
            info.settings = settings;
            startPsnConnection(info, connection_target);
        } else
            connectToHost(index, host.value("discovered").toBool() ? host.value("name").toString() : QString());
    } else {
        const auto info = session_info;
        createSession(info);
    }
    return true;
}

bool QmlBackend::cancelConnection(const QString &attempt_id)
{
    const QPointer<StreamSession> owned_session(session);
    const auto attempt_generation = connection_generation;
    const auto phase = connection_attempt.value("phase").toString();
    const bool pin_pending = session_pin_request.value("attemptId") == attempt_id
        && (session_pin_request.value("phase") == "form" || session_pin_request.value("phase") == "submitting");
    if (attempt_id.isEmpty() || connection_attempt.isEmpty()
            || attempt_id != connection_attempt.value("id").toString() || connection_cancelled
            || phase == "failure" || (phase == "ready" && !pin_pending) || phase == "cancelled")
        return false;
    if (!pin_pending && !connection_attempt.value("cancelable", true).toBool())
        return false;
    connection_cancelled = true;
    if (connection_token)
        delete connection_token;
    if (pin_pending) {
        session_pin_request["phase"] = "cancelling";
        emit sessionPinRequestChanged();
        if (session != owned_session || connection_generation != attempt_generation || !connection_cancelled)
            return true;
    }
    clearWakeWait();
    psn_reconnect_timer->stop();
    if (session) {
        setConnectionPhase(QStringLiteral("cancelling"));
        if (!owned_session || session != owned_session || connection_generation != attempt_generation)
            return true;
        if (!pin_pending && !session_info.duid.isEmpty() && !session->IsConnected()) {
            session->CancelPsnConnection(false);
            if (psn_session_started && connection_profile_generation != profile_generation)
                session->Stop();
        } else
            session->Stop();
    } else {
        connection_attempt["cleanupPending"] = false;
        setConnectionPhase(QStringLiteral("cancelled"));
    }
    return true;
}

bool QmlBackend::dismissConnection(const QString &attempt_id)
{
    if (attempt_id.isEmpty() || connection_attempt.isEmpty()
            || attempt_id != connection_attempt.value("id").toString() || connection_dismissed)
        return false;
    if (connection_attempt.value("phase") != "ready")
    {
        // Leaving the route must also stop stages that cannot accept user Cancel.
        connection_attempt["cancelable"] = true;
        cancelConnection(attempt_id);
    }
    connection_dismissed = true;
    session_pin_request.clear();
    emit sessionPinRequestChanged();
    emit connectionAttemptChanged();
    return true;
}

void QmlBackend::createSession(const StreamSessionConnectInfo &connect_info, const QVariantMap &target, bool continue_attempt)
{
    if (continue_attempt && (connection_profile_generation != profile_generation || connection_cancelled
            || connection_dismissed || connection_attempt.value("phase") == "failure"))
        return;
    pending_recovered_frame.storeRelaxed(0);

    if (autoConnect()) {
        auto_connect_mac = {};
        emit autoConnectChanged();
    }

    if (session) {
        qCWarning(chiakiGui) << "Another session is already active";
        return;
    }

    session_info = connect_info;
    if (!continue_attempt)
        beginConnection(connect_info, target);
    else if (connection_attempt.value("waitingForWake").toBool()) {
        connection_target = target;
        connection_attempt["route"] = connect_info.duid.isEmpty() ? "local" : "psn";
        connection_attempt["address"] = connect_info.host;
        connection_attempt["waitingForWake"] = !wakeup_nickname.isEmpty();
        setConnectionPhase(wakeup_nickname.isEmpty() ? QStringLiteral("connecting") : QStringLiteral("waking"));
    }
    if (connection_cancelled || connection_dismissed || connection_profile_generation != profile_generation)
        return;
    if (!target.isEmpty() && !connectionTargetValid()) {
        failConnection(tr("Console is unavailable"), tr("Select the console again from the current profile."));
        return;
    }
    connection_attempt["cleanupPending"] = true;
    psn_session_started = false;
    disable_zero_copy = !settings->GetUseZeroCopy();
    {
        QMutexLocker locker(&hw_transfer_state_mutex);
        logged_hw_transfer_formats.clear();
        logged_hw_transfer_failures.clear();
    }
    QStringList availableDecoders = settings_qml->availableDecoders();
    if (!session_info.hw_decoder.isEmpty()
        && session_info.hw_decoder != "auto"
        && session_info.hw_decoder != "none"
        && !availableDecoders.contains(session_info.hw_decoder)) {
        qCInfo(chiakiGui) << "Configured hw decoder" << session_info.hw_decoder
                          << "is not currently available, falling back to auto";
        session_info.hw_decoder = "auto";
    }
    bool use_opengl_renderer = window && window->runtimeRendererBackend() == static_cast<int>(RenderBackend::OpenGL);
    bool prefer_cuda = window && window->nvidiaCard() && availableDecoders.contains("cuda");
    auto fallbackVulkanDecoderForOpenGL = [&]() {
        if (!use_opengl_renderer || session_info.hw_decoder != "vulkan")
            return;

        bool fallbackApplied = false;
#if defined(Q_OS_LINUX)
        if (prefer_cuda)
        {
            qCInfo(chiakiGui) << "Renderer backend is OpenGL, falling back from vulkan decoder to cuda";
            session_info.hw_decoder = "cuda";
            fallbackApplied = true;
        }
        else
        if (availableDecoders.contains("vaapi"))
        {
            qCInfo(chiakiGui) << "Renderer backend is OpenGL, falling back from vulkan decoder to vaapi";
            session_info.hw_decoder = "vaapi";
            fallbackApplied = true;
        }
#elif defined(Q_OS_WIN)
        if (prefer_cuda)
        {
            qCInfo(chiakiGui) << "Renderer backend is OpenGL, falling back from vulkan decoder to cuda";
            session_info.hw_decoder = "cuda";
            fallbackApplied = true;
        }
        else
        if (availableDecoders.contains("d3d11va"))
        {
            qCInfo(chiakiGui) << "Renderer backend is OpenGL, falling back from vulkan decoder to d3d11va";
            session_info.hw_decoder = "d3d11va";
            fallbackApplied = true;
        }
#elif defined(Q_OS_MACOS)
        if (availableDecoders.contains("videotoolbox"))
        {
            qCInfo(chiakiGui) << "Renderer backend is OpenGL, falling back from vulkan decoder to videotoolbox";
            session_info.hw_decoder = "videotoolbox";
            fallbackApplied = true;
        }
#endif
        if (!fallbackApplied)
        {
            qCInfo(chiakiGui) << "Renderer backend is OpenGL, falling back from vulkan decoder to software decoder";
            session_info.hw_decoder.clear();
        }
    };
    if(session_info.hw_decoder == "auto")
    {
        session_info.hw_decoder = QString();
#if defined(Q_OS_LINUX)
        if(!use_opengl_renderer && availableDecoders.contains("vulkan"))
        {
            qCInfo(chiakiGui) << "Auto hw decoder selecting vulkan";
            session_info.hw_decoder = "vulkan";
        }
        else if(prefer_cuda)
        {
            qCInfo(chiakiGui) << "Auto hw decoder selecting cuda";
            session_info.hw_decoder = "cuda";
        }
        else if(availableDecoders.contains("vaapi"))
        {
            qCInfo(chiakiGui) << "Auto hw decoder selecting vaapi";
            session_info.hw_decoder = "vaapi";
        }
#elif defined(Q_OS_WIN)
        if(!use_opengl_renderer && availableDecoders.contains("vulkan"))
        {
            qCInfo(chiakiGui) << "Auto hw decoder selecting vulkan";
            session_info.hw_decoder = "vulkan";
        }
        else if(prefer_cuda)
        {
            qCInfo(chiakiGui) << "Auto hw decoder selecting cuda";
            session_info.hw_decoder = "cuda";
        }
        else if(availableDecoders.contains("d3d11va"))
        {
            qCInfo(chiakiGui) << "Auto hw decoder selecting d3d11va";
            session_info.hw_decoder = "d3d11va";
        }
#elif defined(Q_OS_MACOS)
        if(availableDecoders.contains("videotoolbox"))
        {
            qCInfo(chiakiGui) << "Auto hw decoder selecting videotoolbox";
            session_info.hw_decoder = "videotoolbox";
        }
#endif
    }
    fallbackVulkanDecoderForOpenGL();
    if (session_info.hw_decoder == "vulkan") {
#if defined(Q_OS_LINUX)
        if(qEnvironmentVariableIsSet("APPIMAGE") && (qEnvironmentVariableIsSet("SteamDeck") || qEnvironmentVariable("DESKTOP_SESSION").contains("steamos")))
        {
            qCInfo(chiakiGui) << "Auto hw decoder falling back to vaapi because radv has a bug with vulkan hw decode in SteamOS 3.6";
            session_info.hw_decoder = "vaapi";
        }
        else
        {
#endif
            session_info.hw_device_ctx = window->vulkanHwDeviceCtx();
            if (!session_info.hw_device_ctx)
            {
                session_info.hw_decoder.clear();
                qCInfo(chiakiGui) << "vulkan video decoding not supported by your gpu driver, retrying other hw video decoders";
#if defined(Q_OS_LINUX)
                if(prefer_cuda)
                {
                    qCInfo(chiakiGui) << "Falling back to cuda";
                    session_info.hw_decoder = "cuda";
                }
                else if(availableDecoders.contains("vaapi"))
                {
                    qCInfo(chiakiGui) << "Falling back to vaapi";
                    session_info.hw_decoder = "vaapi";
                }
#elif defined(Q_OS_WIN)
                if(prefer_cuda)
                {
                    qCInfo(chiakiGui) << "Falling back to cuda";
                    session_info.hw_decoder = "cuda";
                }
                else if(availableDecoders.contains("d3d11va"))
                {
                    qCInfo(chiakiGui) << "Falling back to d3d11va";
                    session_info.hw_decoder = "d3d11va";
                }
#endif
            }
            if(session_info.hw_decoder.isEmpty())
                qCInfo(chiakiGui) << "Falling back to software decoder";
#if defined(Q_OS_LINUX)
        }
#endif
    }

    try {
        session = new StreamSession(session_info, this);
    } catch (const Exception &e) {
        failConnection(tr("Stream failed"), tr("Failed to initialize Stream Session: %1").arg(e.what()));
        emit error(tr("Stream failed"), tr("Failed to initialize Stream Session: %1").arg(e.what()));
        return;
    }

    const QPointer<StreamSession> owned_session(session);
    const auto attempt_id = connection_generation;

    connect(session, &StreamSession::FfmpegFrameAvailable, frame_thread->parent(), [this, use_opengl_renderer, owned_session, attempt_id]() {
        if (!owned_session)
            return;
        ChiakiFfmpegDecoder *decoder = owned_session->GetFfmpegDecoder();
        if (!decoder) {
            qCCritical(chiakiGui) << "Session has no FFmpeg decoder";
            return;
        }
        int32_t frames_lost;
        ChiakiFfmpegFrame frame = chiaki_ffmpeg_decoder_pull_frame(decoder, &frames_lost);
        if (!frame.frame)
            return;
        if (frame.recovered)
            pending_recovered_frame.storeRelaxed(1);

        if (!prepareFrameForPresentation(frame, use_opengl_renderer))
        {
            av_frame_free(&frame.frame);
            return;
        }

        if (pending_recovered_frame.fetchAndStoreRelaxed(0) != 0) {
            frame.recovered = true;
        }

        QMetaObject::invokeMethod(window, [this, owned_session, attempt_id, frame, frames_lost]() mutable {
            if (owned_session && session == owned_session && connection_generation == attempt_id && !connection_cancelled
                    && !connection_dismissed && connection_profile_generation == profile_generation
                    && connection_attempt.value("phase") != "failure")
                window->presentFrame(frame, frames_lost);
            else
                av_frame_free(&frame.frame);
        });
    });

    connect(session, &StreamSession::SessionQuit, this, [this, owned_session, attempt_id](ChiakiQuitReason reason, const QString &reason_str) {
        if (!owned_session || session != owned_session || connection_generation != attempt_id)
            return;
        if (chiaki_quit_reason_is_error(reason) && !connection_cancelled && !connection_dismissed
                && connection_profile_generation == profile_generation) {
            QString m = tr("Chiaki Session has quit") + ":\n" + chiaki_quit_reason_string(reason);
            if (!reason_str.isEmpty())
                m += "\n" + tr("Reason") + ": \"" + reason_str + "\"";
            failConnection(tr("Session has quit"), m);
            emit sessionError(tr("Session has quit"), m);
        }

        chiaki_log_mutex.lock();
        chiaki_log_ctx = nullptr;
        chiaki_log_mutex.unlock();
        pending_recovered_frame.storeRelaxed(0);
        {
            QMutexLocker locker(&hw_transfer_state_mutex);
            logged_hw_transfer_formats.clear();
            logged_hw_transfer_failures.clear();
        }

        session->deleteLater();
        session = nullptr;
        connection_attempt["cleanupPending"] = false;
        if (connection_cancelled)
            connection_attempt["phase"] = "cancelled";
        else if (connection_attempt.value("phase") == "ready")
            connection_dismissed = true;
        else if (connection_attempt.value("phase") != "failure" && connection_attempt.value("phase") != "registered")
            connection_attempt["phase"] = "cancelled";
        if (session_pin_request.value("phase") == "cancelling" && !connection_dismissed
                && connection_profile_generation == profile_generation)
            session_pin_request["phase"] = "cancelled";
        else
            session_pin_request.clear();
        emit sessionPinRequestChanged();
        psn_session_started = false;
        emit connectionAttemptChanged();
        emit sessionChanged(session);

        sleep_inhibit->release();
        setDiscoveryEnabled(true);
#ifdef Q_OS_WINDOWS
        qCInfo(chiakiGui) << "Checking sleep state: ";
        if(windows_wake_sleep->getWakeState() == WindowsWakeState::Awake)
            QTimer::singleShot(2000, this, &QmlBackend::resumeFromSleep);
        else
            windows_wake_sleep->setWakeState(WindowsWakeState::Sleeping);
#endif
    });

    connect(session, &StreamSession::LoginPINRequested, this, [this, connect_info, owned_session, attempt_id](bool incorrect) {
        if (!owned_session || session != owned_session || connection_generation != attempt_id
                || connection_cancelled || connection_dismissed || connection_profile_generation != profile_generation
                || connection_attempt.value("phase") == "failure" || session_pin_accepted)
            return;
        if (!connect_info.initial_login_pin.isEmpty() && !incorrect && !session_pin_auto_submitted && session_pin_request.isEmpty()) {
            session_pin_auto_submitted = true;
            session->SetLoginPIN(connect_info.initial_login_pin);
            return;
        }
        if (!incorrect && session_pin_auto_submitted && session_pin_request.isEmpty())
            return;
        if (!session_pin_request.isEmpty() && ((session_pin_request.value("phase") == "form"
                && (session_pin_request.value("incorrect").toBool() || !incorrect))
                || (!incorrect && session_pin_request.value("phase") == "submitting")))
            return;
        session_pin_request = {{"id", QString::number(++session_pin_generation)},
            {"attemptId", QString::number(attempt_id)}, {"phase", "form"}, {"incorrect", incorrect},
            {"name", connection_attempt.value("name")}};
        emit sessionPinRequestChanged();
        emit sessionPinDialogRequested(incorrect);
    });

    connect(session, &StreamSession::DataHolepunchProgress, this, [this, owned_session, attempt_id](bool finished) {
        if (!owned_session || session != owned_session || connection_generation != attempt_id
                || connection_cancelled || connection_dismissed || connection_profile_generation != profile_generation
                || connection_attempt.value("phase") == "failure" || session_info.auto_regist
                || connection_attempt.value("phase") == "ready")
            return;
        if(finished)
        {
            setConnectState(PsnConnectState::DataConnectionFinished);
            emit sessionChanged(session);
        }
        else
            setConnectState(PsnConnectState::DataConnectionStart);
    });

    connect(session, &StreamSession::NicknameReceived, this, [this, owned_session, attempt_id](QString nickname) {
        if (session == owned_session && connection_generation == attempt_id && !connection_cancelled
                && !connection_dismissed && connection_profile_generation == profile_generation)
            checkNickname(nickname);
    });

    connect(session, &StreamSession::AutoRegistSucceeded, this, [this, owned_session, attempt_id](const ChiakiRegisteredHost &host) {
        if (session == owned_session && connection_generation == attempt_id && !connection_cancelled
                && !connection_dismissed && connection_profile_generation == profile_generation)
            finishAutoRegister(host);
    });

    connect(session, &StreamSession::ConnectedChanged, this, [this, owned_session, attempt_id]() {
        if (!owned_session || session != owned_session || connection_generation != attempt_id
                || connection_cancelled || connection_dismissed || connection_profile_generation != profile_generation
                || connection_attempt.value("phase") == "failure" || session_info.auto_regist)
            return;
        if (session->IsConnected()) {
            session_pin_accepted = true;
            session_pin_request.clear();
            emit sessionPinRequestChanged();
            if (!owned_session || session != owned_session || connection_generation != attempt_id
                    || connection_cancelled || connection_dismissed || connection_profile_generation != profile_generation)
                return;
            setDiscoveryEnabled(false);
            if (!wakeup_start) {
                wakeup_nickname.clear();
                connection_attempt["waitingForWake"] = false;
            }
            connection_attempt["cancelable"] = true;
            setConnectionPhase((session_info.audio_video_disabled & CHIAKI_VIDEO_DISABLED) || window->hasVideo()
                ? QStringLiteral("ready") : QStringLiteral("video"));
            if (!session_info.duid.isEmpty())
                emit sessionChanged(session);
        }
    });
    QmlMainWindow *window_ptr = window;
    chiaki_log_mutex.lock();
    chiaki_log_ctx = session->GetChiakiLog();
    chiaki_log_mutex.unlock();

    if(connect_info.duid.isEmpty())
    {
        if(!wakeup_nickname.isEmpty())
        {
            wakeup_start = true;
            if (!sendWakeup(connect_info.host, connect_info.regist_key, chiaki_target_is_ps5(connect_info.target)))
                return;
            wakeup_start_timer->start(WAKEUP_WAIT_SECONDS * 1000);
            emit wakeupStartInitiated();
            updateDiscoveryHosts();
        }
        else
        {
            try {
                session->Start();
            } catch (const Exception &e) {
                chiaki_log_mutex.lock();
                chiaki_log_ctx = nullptr;
                chiaki_log_mutex.unlock();
                session->deleteLater();
                session = nullptr;
                failConnection(tr("Stream failed"), tr("Failed to start Stream Session: %1").arg(e.what()));
                emit error(tr("Stream failed"), tr("Failed to start Stream Session: %1").arg(e.what()));
                return;
            }
            emit sessionChanged(session);

            sleep_inhibit->inhibit();
        }
    }
    else
    {
        setDiscoveryEnabled(false);
        emit showPsnView();
        setConnectState(PsnConnectState::InitiatingConnection);
        psn_worker_pending = true;
        emit psnConnect(session, session_info.duid, chiaki_target_is_ps5(session_info.target), connection_generation);
    }
    if (window->windowState() != Qt::WindowFullScreen)
    {
        if(settings->GetWindowType() == WindowType::CustomResolution)
        {
            window->resize(settings->GetCustomResolutionWidth(), settings->GetCustomResolutionHeight());
            window->setMaximumSize(QSize(settings->GetCustomResolutionWidth(), settings->GetCustomResolutionHeight()));
        }
        else if(settings->GetWindowType() == WindowType::AdjustableResolution)
        {
            window->normalTime();
            if(!settings->GetStreamGeometry().isEmpty())
                window->setGeometry(settings->GetStreamGeometry());
        }
        else
            window->resize(connect_info.video_profile.width, connect_info.video_profile.height);
    }
}

bool QmlBackend::closeRequested()
{
    if (!session_pin_request.isEmpty()) {
        cancelSessionPin(session_pin_request.value("id").toString());
        return false;
    }
    if (!connection_dismissed && !connection_attempt.isEmpty()
            && connection_attempt.value("phase") != "ready" && connection_attempt.value("cleanupPending").toBool()) {
        cancelConnection(connection_attempt.value("id").toString());
        return false;
    }
    if (!session)
        return true;

    bool stop = true;
    if (session->IsConnected()) {
        switch (settings->GetDisconnectAction()) {
        case DisconnectAction::Ask:
            stop = false;
            emit sessionStopDialogRequested();
            break;
        case DisconnectAction::AlwaysSleep:
            session->GoToBed();
            break;
        default:
            break;
        }
    }

    if (stop)
        session->Stop();

    return false;
}

void QmlBackend::deleteHost(int index)
{
    auto server = displayServerAt(index);
    auto id = server.manual_host.GetID();
    if (!server.valid || (id < 0))
        return;
    settings->RemoveManualHost(id);
}

void QmlBackend::wakeUpHost(int index, QString nickname)
{
    auto server = displayServerAt(index);
    if (!server.valid)
        return;
    if (!nickname.isEmpty())
    {
        waking_sleeping_nicknames.append(nickname);
        const auto generation = profile_generation;
        QTimer::singleShot(WAKEUP_PSN_IGNORE_SECONDS * 1000, this, [this, nickname, generation] {
            if (generation != profile_generation) return;
            waking_sleeping_nicknames.removeOne(nickname);
            emit hostsChanged();
        });
    }
    sendWakeup(server);
}

bool QmlBackend::setConsolePin(int index, QString console_pin)
{
    if (console_pin.size() != 4 || !QRegularExpression(QStringLiteral("^[0-9]{4}$")).match(console_pin).hasMatch())
        return false;
    auto server = displayServerAt(index);
    if (!server.valid || !server.registered)
        return false;
    const auto generation = profile_generation;
    server.registered_host.SetConsolePin(server.registered_host, std::move(console_pin));
    settings->AddRegisteredHost(server.registered_host);
    return generation == profile_generation;
}

void QmlBackend::addManualHost(int index, const QString &address)
{
    HostMAC hmac;
    QList<RegisteredHost> registered_hosts = settings->GetRegisteredHosts();
    bool registered = (index >= 0 && (index < registered_hosts.length()));
    if (registered)
        hmac = registered_hosts.at(index).GetServerMAC();
    ManualHost host(-1, address, registered, hmac);
    settings->SetManualHost(host);
}

void QmlBackend::hideHost(const QString &mac_string, const QString &host_nickname)
{
    QByteArray mac_array = QByteArray::fromHex(mac_string.toUtf8());
    if (mac_array.size() != 6)
    {
        qCCritical(chiakiGui) << "Invalid host mac:" << mac_string.toUtf8();
        qCCritical(chiakiGui) << "Aborting hidden host creation because mac string couldn't be converted to a valid host mac!";
        qCCritical(chiakiGui) << "Received an array of unexpected size. Expected: 6 bytes, Received:" << mac_array.size() << "bytes";
        return;
    }
    HostMAC mac((const uint8_t *)mac_array.constData());
    HiddenHost hidden_host(mac, host_nickname);
    settings->AddHiddenHost(hidden_host);
    emit hostsChanged();
}

void QmlBackend::unhideHost(const QString &mac_string)
{
    QByteArray mac_array = QByteArray::fromHex(mac_string.toUtf8());
    const char *mac_ptr = mac_array.constData();
    if (mac_array.size() != 6)
    {
        qCCritical(chiakiGui) << " Aborting hidden host creation because mac string couldn't be converted to a valid host mac!";
        return;
    }
    HostMAC mac((const uint8_t *)mac_ptr);
    settings->RemoveHiddenHost(mac);
    emit hostsChanged();
}

QVariantList QmlBackend::hiddenHosts() const
{
    QVariantList out;
    for (const auto &host : settings->GetHiddenHosts()) {
        QVariantMap m;
        m["name"] = host.GetNickname();
        m["mac"] = host.GetMAC().ToString();
        out.append(m);
    }
    return out;
}


QVariantMap QmlBackend::createRegistrationContext() const
{
    return {{"profileGeneration", QVariant::fromValue(profile_generation)},
        {"discovered", false}, {"manualId", -1}, {"manualAddress", QString()}};
}

bool QmlBackend::registerHost(const QString &host, const QString &psn_id, const QString &pin, const QString &cpin, bool broadcast, int target, const QJSValue &callback, const QVariantMap &context)
{
    if(!context.contains("profileGeneration") || context.value("profileGeneration").toULongLong() != profile_generation)
    {
        emit error(tr("Profile changed"), tr("Close this screen and open registration again for the current profile."));
        return false;
    }
    if(registration_active)
    {
        emit error(tr("Registration in progress"), tr("Wait for the current registration to finish before starting another request."));
        return false;
    }
    ChiakiRegistInfo info = {};
    QByteArray hostb = host.toUtf8();
    info.host = hostb.constData();
    info.target = static_cast<ChiakiTarget>(target);
    info.broadcast = broadcast;
    info.pin = (uint32_t)pin.toULong();
    info.console_pin = (uint32_t)cpin.toULong();
    info.holepunch_info = nullptr;
    info.rudp = nullptr;
    QByteArray psn_idb;
    if (target == CHIAKI_TARGET_PS4_8) {
        psn_idb = psn_id.toUtf8();
        info.psn_online_id = psn_idb.constData();
    } else {
        QByteArray account_id = QByteArray::fromBase64(psn_id.toUtf8());
        if (account_id.size() != CHIAKI_PSN_ACCOUNT_ID_SIZE) {
            emit error(tr("Invalid Account-ID"), tr("The PSN Account-ID must be exactly %1 bytes encoded as base64.").arg(CHIAKI_PSN_ACCOUNT_ID_SIZE));
            return false;
        }
        info.psn_online_id = nullptr;
        memcpy(info.psn_account_id, account_id.constData(), CHIAKI_PSN_ACCOUNT_ID_SIZE);
    }
    auto regist = new QmlRegist(settings->GetLogLevelMask(), this);
    auto completed = std::make_shared<bool>(false);
    const auto generation = profile_generation;
    registration_active = true;
    connect(regist, &QmlRegist::log, this, [callback, completed](ChiakiLogLevel level, QString msg) {
        if(*completed)
            return;
        QJSValue cb = callback;
        if (cb.isCallable())
            cb.call({QString("[%1] %2").arg(chiaki_log_level_char(level)).arg(msg), true, false});
    });
    connect(regist, &QmlRegist::failed, this, [this, callback, completed, generation]() {
        if(*completed)
            return;
        *completed = true;
        registration_active = false;
        QJSValue cb = callback;
        if (cb.isCallable())
            cb.call({generation == profile_generation ? QString() : tr("The profile changed. The registration was not saved."), false, true});
    });
    connect(regist, &QmlRegist::success, this, [this, host, callback, context, generation, completed](const RegisteredHost &rhost) {
        if(*completed)
            return;
        *completed = true;
        QString completion_error;
        if(generation != profile_generation)
            completion_error = tr("The profile changed. The registration was not saved.");
        const int manual_id = context.value("manualId").toInt();
        const QString address = context.value("manualAddress").toString();
        const auto manual_target_valid = [this, manual_id, address] {
            return manual_id < 0 || (settings->GetManualHostExists(manual_id)
                && settings->GetManualHost(manual_id).GetHost() == address);
        };
        if(completion_error.isEmpty() && !context.value("discovered").toBool() && !manual_target_valid())
            completion_error = tr("The original console address changed or was removed. Open registration again.");
        if(completion_error.isEmpty())
        {
            settings->AddRegisteredHost(rhost);
            if(generation != profile_generation)
                completion_error = tr("The profile changed during the update. Check the original profile before trying again.");
            else if(!context.value("discovered").toBool())
            {
                if(!manual_target_valid())
                    completion_error = tr("The console address changed during the update. Check registration before trying again.");
                else
                {
                    ManualHost manual_host(manual_id, address.isEmpty() ? host : address, false, {});
                    manual_host.Register(rhost);
                    settings->SetManualHost(manual_host);
                }
            }
        }
        registration_active = false;
        QJSValue cb = callback;
        if(cb.isCallable())
            cb.call({completion_error, completion_error.isEmpty(), true});
    });
    const auto err = regist->start(info);
    if(err != CHIAKI_ERR_SUCCESS)
    {
        *completed = true;
        registration_active = false;
        regist->deleteLater();
        emit error(tr("Registration did not start"), QString::fromUtf8(chiaki_error_string(err)));
        return false;
    }
    return true;
}

void QmlBackend::autoRegister(const QVariantMap &context)
{
    if (regist_dialog_generation != profile_generation || session || connection_token
            || context.value("profileGeneration").toULongLong() != profile_generation
            || !context.contains("automaticRequest") || context.value("automaticRequest").toULongLong() != automatic_registration_request)
        return;
    const auto &server = regist_dialog_server;
    if (!server.valid || server.duid.isEmpty())
        return;
    QVariantMap target;
    for (const auto &value : hosts()) {
        const auto host = value.toMap();
        if (host.value("display").toBool() && host.value("discovered").toBool()
                && host.value("mac").toString() == server.discovery_host.GetHostMAC().ToString()
                && host.value("duid").toString() == server.duid) {
            if (!target.isEmpty()) return;
            target = connectionTargetKey(host);
        }
    }
    if (target.isEmpty())
        return;
    ++automatic_registration_request;
    resume_session = false;
    StreamSessionConnectInfo info(settings, server.discovery_host.target, QString(), server.discovery_host.host_name,
        QByteArray(), QByteArray(), QString(), server.duid, true, false, false, false);
    startPsnConnection(info, target);
}

void QmlBackend::finishAutoRegister(const ChiakiRegisteredHost &host)
{
    if (!session || !connection_attempt.value("registrationOnly").toBool()
            || connection_attempt.value("phase") == "registered" || connection_attempt.value("phase") == "failure"
            || !connectionTargetValid())
        return;
    QString nickname(host.server_nickname);
    if(!regist_dialog_server.discovery_host.ps5 && regist_dialog_server.discovery_host.host_name != nickname)
    {
        failConnection(tr("PS4 Console Not Main"), tr("Can't proceed...%1 is not your main PS4 console in PSN").arg(regist_dialog_server.discovery_host.host_name));
        session->Stop();
        return;
    }
    const auto generation = profile_generation;
    const auto attempt_id = connection_generation;
    settings->AddRegisteredHost(host);
    if (generation != profile_generation || attempt_id != connection_generation || connection_cancelled || connection_dismissed)
        return;
    setConnectState(PsnConnectState::RegistrationFinished);
    updatePsnHosts();
}

#ifdef CHIAKI_HAVE_WEBENGINE
void QmlBackend::clearCookies(QQuickWebEngineProfile *profile)
{
    auto cookieStore = profile->cookieStore();
    cookieStore->deleteAllCookies();
}

void QmlBackend::setWebEngineHints(QQuickWebEngineProfile *profile)
{
    QDate starting_release_date(2025, 02, 18);
    QDate today = QDate::currentDate();
    auto daysSinceStart = starting_release_date.daysTo(today);
    qint64 versionsSinceStart = daysSinceStart / 28;
    qint64 release = 133 + versionsSinceStart;
    QString chrome_version = QString::number(release);
    auto userAgent = profile->httpUserAgent();
    userAgent = userAgent.replace(QRegularExpression(" \\bQtWebEngine[^ ]*\\b"), "");
    userAgent = userAgent.replace(QRegularExpression("\\bChrome[^ ]*\\b"), QString("Chrome/%1.0.0.0").arg(chrome_version));
#ifdef Q_OS_WINDOWS
    userAgent = userAgent.replace("Windows NT 6.2", "Windows NT 10.0");
    userAgent += QString(" Edg/%1.0.0.0").arg(chrome_version);
#endif
    profile->setHttpUserAgent(userAgent);
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    auto hints = profile->clientHints();
    hints->setFullVersion(QString("%1.0.0.0").arg(chrome_version));
    QMap<QString, QVariant> fullVersionList;
    fullVersionList.insert("Not A(Brand", "99.0.0.0");
#ifdef Q_OS_WINDOWS
    fullVersionList.insert("Microsoft Edge", QString("%1.0.0.0").arg(chrome_version));
#else
    fullVersionList.insert("Google Chrome", QString("%1.0.0.0").arg(chrome_version));
#endif
    fullVersionList.insert("Chromium", QString("%1.0.0.0").arg(chrome_version));
    hints->setFullVersionList(fullVersionList);
#endif
    request_interceptor = new SecUaRequestInterceptor(chrome_version);
    profile->setUrlRequestInterceptor(request_interceptor);
}
#endif

void QmlBackend::connectToHost(int index, QString nickname)
{
    if (session)
        return;
    window->setWindowAdjustable(false);
    auto server = displayServerAt(index);
    if (!server.valid)
        return;

    if (!server.registered) {
        regist_dialog_server = server;
        regist_dialog_generation = profile_generation;
        auto context = createRegistrationContext();
        context["automaticRequest"] = QVariant::fromValue(++automatic_registration_request);
        context["discovered"] = server.discovered;
        if(!server.discovered)
        {
            context["manualId"] = server.manual_host.GetID();
            context["manualAddress"] = server.manual_host.GetHost();
        }
        emit registDialogRequested(server.GetHostAddr(), server.IsPS5(), server.duid, context);
        return;
    }

    const auto listed = hosts();
    if (index < 0 || index >= listed.size() || !listed.at(index).toMap().value("display").toBool())
        return;
    const auto target = connectionTargetKey(listed.at(index).toMap());
    const bool continue_attempt = connection_attempt.value("phase") == "waiting"
        && connection_attempt.value("waitingForWake").toBool() && !connection_cancelled && !connection_dismissed
        && connection_profile_generation == profile_generation && connectionTargetValid();
    if (continue_attempt) {
        const auto attempt_id = connection_generation;
        setConnectionPhase(QStringLiteral("found"));
        if (connection_generation != attempt_id || connection_cancelled || connection_dismissed
                || connection_profile_generation != profile_generation || !connectionTargetValid())
            return;
    }

    if (server.discovered && server.discovery_host.state == CHIAKI_DISCOVERY_HOST_STATE_STANDBY)
    {
        if(nickname.isEmpty())
        {
            qCWarning(chiakiGui) << "No nickname given for registered connection, not connecting...";
            return;
        }
        waking_sleeping_nicknames.append(nickname);
        const auto generation = profile_generation;
        QTimer::singleShot(WAKEUP_PSN_IGNORE_SECONDS * 1000, this, [this, nickname, generation] {
            if (generation != profile_generation) return;
            waking_sleeping_nicknames.removeOne(nickname);
            emit hostsChanged();
        });
        wakeup_nickname = nickname;
    }

    bool fullscreen = false, zoom = false, stretch = false;
    switch (settings->GetWindowType()) {
    case WindowType::SelectedResolution:
        break;
    case WindowType::CustomResolution:
        break;
    case WindowType::AdjustableResolution:
        break;
    case WindowType::Fullscreen:
        fullscreen = true;
        break;
    case WindowType::Zoom:
        zoom = true;
        break;
    case WindowType::Stretch:
        stretch = true;
        break;
    default:
        break;
    }
    emit windowTypeUpdated(settings->GetWindowType());

    resume_session = false;
    if(server.duid.isEmpty())
    {
        QString host = server.GetHostAddr();
        StreamSessionConnectInfo info(
                settings,
                server.registered_host.GetTarget(),
                std::move(host),
                std::move(nickname),
                server.registered_host.GetRPRegistKey(),
                server.registered_host.GetRPKey(),
                server.registered_host.GetConsolePin(),
                server.duid,
                false,
                fullscreen,
                zoom,
                stretch);
        createSession(info, target, continue_attempt);
    }
    else
    {
        StreamSessionConnectInfo info(
                settings,
                server.psn_host.GetTarget(),
                QString(),
                QString(),
                QByteArray(),
                QByteArray(),
                server.registered_host.GetConsolePin(),
                server.duid,
                false,
                fullscreen,
                zoom,
                stretch);

        startPsnConnection(info, target, continue_attempt);
    }
}

void QmlBackend::stopSession(bool sleep)
{
    if (!session)
        return;

    if (!connection_attempt.isEmpty() && connection_attempt.value("phase") != "ready") {
        cancelConnection(connection_attempt.value("id").toString());
        return;
    }

    if (!session_info.nickname.isEmpty())
    {
        const auto nickname = session_info.nickname;
        const auto generation = profile_generation;
        waking_sleeping_nicknames.append(nickname);
        QTimer::singleShot(WAKEUP_PSN_IGNORE_SECONDS * 1000, this, [this, nickname, generation] {
            if (generation != profile_generation) return;
            waking_sleeping_nicknames.removeOne(nickname);
            emit hostsChanged();
        });
    }

    if (sleep)
        session->GoToBed();

    session->Stop();
}

void QmlBackend::sessionGoHome()
{
    if (!session)
        return;

    session->GoHome();
}

bool QmlBackend::enterPin(const QString &request_id, const QString &pin)
{
    if (request_id.isEmpty() || request_id != session_pin_request.value("id").toString()
            || session_pin_request.value("attemptId") != connection_attempt.value("id")
            || session_pin_request.value("phase") != "form" || !session || connection_cancelled || connection_dismissed
            || connection_profile_generation != profile_generation || pin.size() != 4)
        return false;
    for (const QChar digit : pin)
        if (digit < QLatin1Char('0') || digit > QLatin1Char('9'))
            return false;
    const QPointer<StreamSession> owned_session(session);
    const auto attempt_generation = connection_generation;
    session_pin_request["phase"] = "submitting";
    emit sessionPinRequestChanged();
    if (!owned_session || session != owned_session || connection_generation != attempt_generation || connection_cancelled
            || connection_dismissed || connection_profile_generation != profile_generation
            || session_pin_request.value("id") != request_id || session_pin_request.value("phase") != "submitting")
        return false;
    owned_session->SetLoginPIN(pin);
    return true;
}

bool QmlBackend::cancelSessionPin(const QString &request_id)
{
    if (request_id.isEmpty() || request_id != session_pin_request.value("id").toString()
            || session_pin_request.value("attemptId") != connection_attempt.value("id")
            || connection_profile_generation != profile_generation || !session)
        return false;
    return cancelConnection(session_pin_request.value("attemptId").toString());
}

bool QmlBackend::dismissSessionPin(const QString &request_id)
{
    if (request_id.isEmpty() || request_id != session_pin_request.value("id").toString()
            || session_pin_request.value("phase") != "cancelled")
        return false;
    return dismissConnection(session_pin_request.value("attemptId").toString());
}

QUrl QmlBackend::psnLoginUrl() const
{
    size_t duid_size = CHIAKI_DUID_STR_SIZE;
    char duid[duid_size];
    chiaki_holepunch_generate_client_device_uid(duid, &duid_size);
    return QUrl(PSNAuth::LOGIN_URL + "duid=" + QString(duid) + "&");
}

bool QmlBackend::handlePsnLoginRedirect(const QUrl &url)
{
    const auto id = startPsnAuth(url);
    if (id.isEmpty() || psn_auth_attempt.value("phase") == "failure") {
        emit psnLoginAccountIdError(id.isEmpty() ? tr("PSN sign-in is already in progress.") : psn_auth_attempt.value("message").toString());
        return false;
    }
    const auto generation = profile_generation;
    auto connection = std::make_shared<QMetaObject::Connection>();
    *connection = connect(this, &QmlBackend::psnAuthAttemptChanged, this, [this, id, generation, connection] {
        if (psn_auth_attempt.value("id") != id || generation != profile_generation) { disconnect(*connection); return; }
        const auto phase = psn_auth_attempt.value("phase").toString();
        if (phase != "success" && phase != "failure") return;
        disconnect(*connection);
        if (phase == "success") emit psnLoginAccountIdDone(psn_auth_attempt.value("accountId").toString());
        else emit psnLoginAccountIdError(psn_auth_attempt.value("message").toString());
    });
    return true;
}

void QmlBackend::clearWakeWait()
{
    const bool allocated = wakeup_start;
    wakeup_start = false;
    wakeup_nickname.clear();
    wakeup_start_timer->stop();
    psn_auto_connect_timer->stop();
    auto_connect_mac = {};
    if (allocated && session) {
        QMutexLocker locker(&chiaki_log_mutex);
        chiaki_log_ctx = nullptr;
        session->deleteLater();
        session = nullptr;
        connection_attempt["cleanupPending"] = false;
    }
    emit autoConnectChanged();
}

void QmlBackend::stopAutoConnect()
{
    if (connection_attempt.value("waitingForWake").toBool())
        cancelConnection(connection_attempt.value("id").toString());
    else
        clearWakeWait();
}

void QmlBackend::beginAutoConnect(const HostMAC &mac)
{
    auto_connect_mac = mac;
    const auto host = settings->GetRegisteredHost(mac);
    auto_connect_nickname = host.GetServerNickname();
    StreamSessionConnectInfo info(settings, host.GetTarget(), {}, auto_connect_nickname,
        host.GetRPRegistKey(), host.GetRPKey(), {}, {}, false, false, false, false);
    beginConnection(info, {{"kind", "auto"}, {"mac", mac.ToString()}}, QStringLiteral("startup"));
    if (connection_cancelled || connection_dismissed)
        return;
    if (!connectionTargetValid()) {
        clearWakeWait();
        failConnection(tr("Console is unavailable"), tr("Select the console again from the current profile."));
        return;
    }
    psn_auto_connect_timer->start(PSN_INTERNET_WAIT_SECONDS * 1000);
}

void QmlBackend::autoConnectPsn()
{
    if ((sender() && sender() != psn_auto_connect_timer) || !autoConnect()
            || connection_attempt.value("phase") != "waiting" || connection_cancelled
            || connection_dismissed || connection_profile_generation != profile_generation)
        return;
    if (!connectionTargetValid()) {
        clearWakeWait();
        failConnection(tr("Console is unavailable"), tr("Select the console again from the current profile."));
        return;
    }
    if (settings->GetPsnAuthTokenExpiry().isEmpty() || settings->GetPsnRefreshToken().isEmpty())
        return;
    const auto listed = hosts();
    int found = -1;
    for (int i = 0; i < listed.size(); ++i) {
        const auto host = listed.at(i).toMap();
        if (!host.value("discovered").toBool() && !host.value("duid").toString().isEmpty()
                && host.value("name").toString() == auto_connect_nickname && host.value("display").toBool()) {
            if (found != -1) return;
            found = i;
        }
    }
    if (found >= 0)
        connectToHost(found);
}

QmlBackend::DisplayServer QmlBackend::displayServerAt(int index) const
{
    if (index < 0)
        return {};
    const int source_index = index;
    auto discovered = discovery_manager.GetHosts();
    auto manual = settings->GetManualHosts();
    if (index < discovered.size()) {
        DisplayServer server;
        server.valid = true;
        server.discovered = true;
        server.discovery_host = discovered.at(index);
        auto host_mac = server.discovery_host.GetHostMAC();
        server.registered = settings->GetRegisteredHostRegistered(host_mac);
        QString duid = "";
        if(!server.registered)
        {
            if(psn_nickname_hosts.contains(server.discovery_host.host_name))
                duid = psn_nickname_hosts.value(server.discovery_host.host_name).GetDuid();
            else if(!server.discovery_host.ps5)
                duid =  psn_nickname_hosts.value(QString("Main PS4 Console")).GetDuid();
        }
        server.duid = std::move(duid);
        if (server.registered)
            server.registered_host = settings->GetRegisteredHost(host_mac);
        for (int i = 0; i < manual.size(); i++)
        {
            const auto &manual_host = manual.at(i);
            if(manual_host.GetRegistered() && manual_host.GetMAC() == host_mac && manual_host.GetHost() == server.discovery_host.host_addr)
            {
                server.manual_host = std::move(manual_host);
                break;
            }
        }
        return server;
    }
    index -= discovered.size();
    if (index < manual.size()) {
        DisplayServer server;
        server.valid = true;
        server.discovered = false;
        server.manual_host = manual.at(index);
        server.registered = false;
        server.duid = QString();
        if (server.manual_host.GetRegistered() && settings->GetRegisteredHostRegistered(server.manual_host.GetMAC())) {
            server.registered = true;
            server.registered_host = settings->GetRegisteredHost(server.manual_host.GetMAC());
        }
        return server;
    }
    const auto listed_hosts = hosts();
    if (source_index >= listed_hosts.size())
        return {};
    const auto duid = listed_hosts.at(source_index).toMap().value("duid").toString();
    const auto psn_host = psn_hosts.constFind(duid);
    if (psn_host == psn_hosts.constEnd())
        return {};
    DisplayServer server;
    server.valid = true;
    server.discovered = false;
    server.psn_host = psn_host.value();
    server.duid = duid;
    server.registered = true;
    server.registered_host = settings->GetNicknameRegisteredHost(server.psn_host.GetName());
    return server;
}

bool QmlBackend::sendWakeup(const DisplayServer &server)
{
    if (!server.registered)
        return false;
    return sendWakeup(server.GetHostAddr(), server.registered_host.GetRPRegistKey(), server.IsPS5());
}

bool QmlBackend::sendWakeup(const QString &host, const QByteArray &regist_key, bool ps5)
{
    try {
        discovery_manager.SendWakeup(host, regist_key, ps5);
        return true;
    } catch (const Exception &e) {
        const auto title = tr("Wake-up failed");
        const auto text = tr("Failed to send wake-up packet:\n%1").arg(e.what());
        if (wakeup_start) {
            clearWakeWait();
            failConnection(title, text);
        } else
            emit error(title, text);
        return false;
    }
}

void QmlBackend::setAllowJoystickBackgroundEvents()
{
    ControllerManager::GetInstance()->SetAllowJoystickBackgroundEvents(settings->GetAllowJoystickBackgroundEvents());
}

void QmlBackend::setIsAppActive()
{
    ControllerManager::GetInstance()->SetIsAppActive(window->isActive());
}

uint32_t QmlBackend::getStreamShortcut() const
{
    if(!settings->GetStreamMenuEnabled())
        return 0;
	uint32_t stream_menu_shortcut1 = settings->GetStreamMenuShortcut1();
	if(stream_menu_shortcut1 > 0)
		stream_menu_shortcut1 = 1 << (stream_menu_shortcut1 - 1);
	uint32_t stream_menu_shortcut2 = settings->GetStreamMenuShortcut2();
	if(stream_menu_shortcut2 > 0)
		stream_menu_shortcut2 = 1 << (stream_menu_shortcut2 - 1);
	uint32_t stream_menu_shortcut3 = settings->GetStreamMenuShortcut3();
	if(stream_menu_shortcut3 > 0)
		stream_menu_shortcut3 = 1 << (stream_menu_shortcut3 - 1);
	uint32_t stream_menu_shortcut4 = settings->GetStreamMenuShortcut4();
	if(stream_menu_shortcut4 > 0)
		stream_menu_shortcut4 = 1 << (stream_menu_shortcut4 - 1);
    uint32_t shortcut = stream_menu_shortcut1 | stream_menu_shortcut2 | stream_menu_shortcut3 | stream_menu_shortcut4;
    return shortcut;
}

void QmlBackend::updateStreamShortcut()
{
    uint32_t shortcut = getStreamShortcut();
    for (const auto &controller : std::as_const(controllers)) {
       controller->setEscapeShortcut(shortcut);
    }
}

void QmlBackend::updateControllers()
{
    bool changed = false;
    QMap<QString,QString> controller_mappings = settings->GetControllerMappings();
    for (auto it = controllers.begin(); it != controllers.end();) {
        if (ControllerManager::GetInstance()->GetAvailableControllers().contains(it.key())) {
            it++;
            continue;
        }
        if(it.key() == controller_mapping_id)
        {
            controller_mapping_controller = nullptr;
            controllerMappingQuit();
        }
        QString vidpid = it.value()->GetVIDPID();
        QString guid = it.value()->GetGUID();
        it.value()->deleteLater();
        it = controllers.erase(it);
        changed = true;
    }
    uint32_t stream_shortcut = getStreamShortcut();
    for (auto id : ControllerManager::GetInstance()->GetAvailableControllers()) {
        if (controllers.contains(id))
            continue;
        auto controller = ControllerManager::GetInstance()->OpenController(id);
        if (!controller)
            continue;
        controllers[id] = new QmlController(controller, stream_shortcut ,window, this);
        QString vidpid = controller->GetVIDPIDString();
        QString guid = controller->GetGUIDString();
        QStringList existing_vidpid;
        if(controller_guids_to_update.contains(vidpid))
            existing_vidpid.append(controller_guids_to_update.value(vidpid));
        existing_vidpid.append(guid);
        controller_guids_to_update.insert(vidpid, existing_vidpid);
        // replace old guid with new vid/pid
        if(controller_mappings.contains(guid))
        {
            QString old_mapping = controller_mappings.value(guid);
            qsizetype guid_string_length = old_mapping.indexOf(",") + 1;
            old_mapping.remove(0, guid_string_length);
            settings->RemoveControllerMapping(guid);
            settings->SetControllerMapping(vidpid, old_mapping);
            qCInfo(chiakiGui) << "Migrated controller mapping from platform-specific GUID: " << guid << " to platform-agnostic VID/PID: " << vidpid;
        }
        controller_guids_to_update.insert(vidpid, existing_vidpid);
        connect(controller, &Controller::UpdatingControllerMapping, this, &QmlBackend::controllerMappingUpdate);
        connect(controller, &Controller::NewButtonMapping, this, &QmlBackend::controllerMappingChangeButton);
        changed = true;
    }
    if (changed)
        emit controllersChanged();
}

void QmlBackend::setControllerMappingDefaultMapping(bool is_default_mapping)
{
    if(controller_mapping_default_mapping != is_default_mapping)
    {
        controller_mapping_default_mapping = is_default_mapping;
        emit controllerMappingDefaultMappingChanged();
    }
}

void QmlBackend::setControllerMappingAltered(bool altered)
{
    if(controller_mapping_altered != altered)
    {
        controller_mapping_altered = altered;
        emit controllerMappingAlteredChanged();
    }
}

void QmlBackend::setControllerMappingInProgress(bool is_in_progress)
{
    if(controller_mapping_in_progress != is_in_progress)
    {
        controller_mapping_in_progress = is_in_progress;
        emit controllerMappingInProgressChanged();
    }
}

void QmlBackend::setEnableAnalogStickMapping(bool enabled)
{
    if(enable_analog_stick_mapping != enabled)
    {
        if(controller_mapping_in_progress && controller_mapping_controller)
            controller_mapping_controller->EnableAnalogStickMapping(enabled);
        enable_analog_stick_mapping = enabled;
        emit enableAnalogStickMappingChanged();
    }
}

QVariantList QmlBackend::currentControllerMapping() const
{
    QVariantList out;
    if(!controller_mapping_in_progress)
        return out;
	QMap<int, QStringList> result =
	{
		{CHIAKI_CONTROLLER_BUTTON_CROSS     , controller_mapping_controller_mappings.contains("a") ? controller_mapping_controller_mappings.value("a") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_MOON      , controller_mapping_controller_mappings.contains("b") ? controller_mapping_controller_mappings.value("b") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_BOX       , controller_mapping_controller_mappings.contains("x") ? controller_mapping_controller_mappings.value("x") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_PYRAMID   , controller_mapping_controller_mappings.contains("y") ? controller_mapping_controller_mappings.value("y") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_DPAD_LEFT , controller_mapping_controller_mappings.contains("dpleft") ? controller_mapping_controller_mappings.value("dpleft") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_DPAD_RIGHT, controller_mapping_controller_mappings.contains("dpright") ? controller_mapping_controller_mappings.value("dpright") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_DPAD_UP   , controller_mapping_controller_mappings.contains("dpup") ? controller_mapping_controller_mappings.value("dpup") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_DPAD_DOWN , controller_mapping_controller_mappings.contains("dpdown") ? controller_mapping_controller_mappings.value("dpdown") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_L1        , controller_mapping_controller_mappings.contains("leftshoulder") ? controller_mapping_controller_mappings.value("leftshoulder") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_R1        , controller_mapping_controller_mappings.contains("rightshoulder") ? controller_mapping_controller_mappings.value("rightshoulder") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_L3        , controller_mapping_controller_mappings.contains("leftstick") ? controller_mapping_controller_mappings.value("leftstick") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_R3        , controller_mapping_controller_mappings.contains("rightstick") ? controller_mapping_controller_mappings.value("rightstick") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_OPTIONS   , controller_mapping_controller_mappings.contains("start") ? controller_mapping_controller_mappings.value("start") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_SHARE     , controller_mapping_controller_mappings.contains("back") ? controller_mapping_controller_mappings.value("back") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_TOUCHPAD  , controller_mapping_controller_mappings.contains("touchpad") ? controller_mapping_controller_mappings.value("touchpad") : QStringList()},
		{CHIAKI_CONTROLLER_BUTTON_PS        , controller_mapping_controller_mappings.contains("guide") ? controller_mapping_controller_mappings.value("guide") : QStringList()},
		{CHIAKI_CONTROLLER_ANALOG_BUTTON_L2 , controller_mapping_controller_mappings.contains("lefttrigger") ? controller_mapping_controller_mappings.value("lefttrigger") : QStringList()},
		{CHIAKI_CONTROLLER_ANALOG_BUTTON_R2 , controller_mapping_controller_mappings.contains("righttrigger") ? controller_mapping_controller_mappings.value("righttrigger") : QStringList()},
		{static_cast<int>(ControllerButtonExt::ANALOG_STICK_LEFT_X)   , controller_mapping_controller_mappings.contains("leftx") ? controller_mapping_controller_mappings.value("leftx") : QStringList()},
		{static_cast<int>(ControllerButtonExt::ANALOG_STICK_LEFT_Y)   , controller_mapping_controller_mappings.contains("lefty") ? controller_mapping_controller_mappings.value("lefty") : QStringList()},
		{static_cast<int>(ControllerButtonExt::ANALOG_STICK_RIGHT_X)  , controller_mapping_controller_mappings.contains("rightx") ? controller_mapping_controller_mappings.value("rightx") : QStringList()},
		{static_cast<int>(ControllerButtonExt::ANALOG_STICK_RIGHT_Y)  , controller_mapping_controller_mappings.contains("righty") ? controller_mapping_controller_mappings.value("righty") : QStringList()},
        {static_cast<int>(ControllerButtonExt::MISC1)   ,      controller_mapping_controller_mappings.contains("misc1") ? controller_mapping_controller_mappings.value("misc1") : QStringList()},
	};

    for (auto it = result.cbegin(); it != result.cend(); ++it) {
        QVariantMap m;
        m["buttonName"] = Settings::GetChiakiControllerButtonName(it.key());
        m["buttonValue"] = it.key();
        m["physicalButton"] = it.value();
        out.append(m);
    }
    return out;
}

void QmlBackend::updateControllerMappings()
{
    if(SDL_WasInit(SDL_INIT_GAMECONTROLLER)==0)
        return;
    QMap<QString,QString> controller_mappings = settings->GetControllerMappings();
    QStringList mapping_vidpids = controller_mappings.keys();
    for(int i=0; i<mapping_vidpids.length(); i++)
    {
        QString vidpid = mapping_vidpids.at(i);
        if(!controller_guids_to_update.contains(vidpid))
            continue;
        QStringList guids_to_update = controller_guids_to_update.value(vidpid);
        for(int j=0; j<guids_to_update.length(); j++)
        {
            QString guid = guids_to_update.at(j);
            if(!controller_mapping_original_controller_mappings.contains(guid))
            {
                const SDL_JoystickGUID real_guid = SDL_JoystickGetGUIDFromString(guid.toUtf8().constData());
                const char *mapping = SDL_GameControllerMappingForGUID(real_guid);
                QString original_controller_mapping(mapping);
                SDL_free((char *)mapping);
                if(original_controller_mapping.isEmpty())
                {
                    qCWarning(chiakiGui) << "Error retrieving controller mapping of GUID " << guid << "with error: " << SDL_GetError();
                    return;
                }
                controller_mapping_original_controller_mappings.insert(guid, original_controller_mapping);
            }
            QString controller_mapping_to_add = controller_mappings.value(vidpid);
            controller_mapping_to_add.prepend(QString("%1,").arg(guid));
            int result = SDL_GameControllerAddMapping(controller_mapping_to_add.toUtf8().constData());
            switch(result)
            {
                case -1:
                    qCWarning(chiakiGui) << "Error setting controller mapping for guid: " << guid << " with error: " << SDL_GetError();
                    break;
                case 0:
                    qCInfo(chiakiGui) << "Updated controller mapping for guid: " << guid;
                    break;
                case 1:
                    qCInfo(chiakiGui) << "Added controller mapping for guid: " << guid;
                    break;
                default:
                    qCInfo(chiakiGui) << "Unidentified problem mapping for guid: " << guid;
                    break;
            }
        }
        controller_guids_to_update.remove(vidpid);
    }
}

void QmlBackend::creatingControllerMapping(bool creating_controller_mapping)
{
    ControllerManager::GetInstance()->creatingControllerMapping(creating_controller_mapping);
}

void QmlBackend::beginControllerMapping(bool reset_mapping)
{
    controller_mapping_reset_requested = reset_mapping;
    creatingControllerMapping(true);
}

void QmlBackend::controllerMappingChangeButton(QString button)
{
    if(!controller_mapping_in_progress || !controller_mapping_controller)
        return;
    emit controllerMappingButtonSelected(std::move(button));
}

void QmlBackend::updateButton(int chiaki_button, QString physical_button, int new_index)
{
	QMap<int, QString> button_map =
	{
		{CHIAKI_CONTROLLER_BUTTON_CROSS     , "a"},
		{CHIAKI_CONTROLLER_BUTTON_MOON      , "b"},
		{CHIAKI_CONTROLLER_BUTTON_BOX       , "x"},
		{CHIAKI_CONTROLLER_BUTTON_PYRAMID   , "y"},
		{CHIAKI_CONTROLLER_BUTTON_DPAD_LEFT , "dpleft"},
		{CHIAKI_CONTROLLER_BUTTON_DPAD_RIGHT, "dpright"},
		{CHIAKI_CONTROLLER_BUTTON_DPAD_UP   , "dpup"},
		{CHIAKI_CONTROLLER_BUTTON_DPAD_DOWN , "dpdown"},
		{CHIAKI_CONTROLLER_BUTTON_L1        , "leftshoulder"},
		{CHIAKI_CONTROLLER_BUTTON_R1        , "rightshoulder"},
		{CHIAKI_CONTROLLER_BUTTON_L3        , "leftstick"},
		{CHIAKI_CONTROLLER_BUTTON_R3        , "rightstick"},
		{CHIAKI_CONTROLLER_BUTTON_OPTIONS   , "start"},
		{CHIAKI_CONTROLLER_BUTTON_SHARE     , "back"},
		{CHIAKI_CONTROLLER_BUTTON_TOUCHPAD  , "touchpad"},
		{CHIAKI_CONTROLLER_BUTTON_PS        , "guide"},
		{CHIAKI_CONTROLLER_ANALOG_BUTTON_L2 , "lefttrigger"},
		{CHIAKI_CONTROLLER_ANALOG_BUTTON_R2 , "righttrigger"},
		{static_cast<int>(ControllerButtonExt::ANALOG_STICK_LEFT_X)   , "leftx"},
		{static_cast<int>(ControllerButtonExt::ANALOG_STICK_LEFT_Y)   , "lefty"},
		{static_cast<int>(ControllerButtonExt::ANALOG_STICK_RIGHT_X)  , "rightx"},
		{static_cast<int>(ControllerButtonExt::ANALOG_STICK_RIGHT_Y)  , "righty"},
        {static_cast<int>(ControllerButtonExt::MISC1)   , "misc1"},
	};
    QString button = button_map.value(chiaki_button);
    QString old_mapping = controller_mapping_physical_button_mappings.contains(physical_button) ? controller_mapping_physical_button_mappings.value(physical_button) : QString();
    if(button == old_mapping)
        return;
    if(!old_mapping.isEmpty())
    {
        QStringList old_mapping_buttons = controller_mapping_controller_mappings.value(old_mapping);
        if(old_mapping_buttons.length() > 1)
        {
            old_mapping_buttons.removeOne(physical_button);
            controller_mapping_controller_mappings.insert(old_mapping, old_mapping_buttons);
        }
        else
            controller_mapping_controller_mappings.remove(old_mapping);
    }
    QStringList new_mapping_buttons = controller_mapping_controller_mappings.value(button);
    if(new_mapping_buttons.length() > new_index)
    {
        controller_mapping_physical_button_mappings.insert(new_mapping_buttons.at(new_index), QString());
        new_mapping_buttons.remove(new_index);
    }
    if(new_index == 0)
        new_mapping_buttons.prepend(physical_button);
    else
        new_mapping_buttons.append(physical_button);
    controller_mapping_controller_mappings.insert(button, new_mapping_buttons);
    controller_mapping_physical_button_mappings.insert(physical_button, button);
    if(controller_mapping_controller_mappings == controller_mapping_applied_controller_mappings)
        setControllerMappingAltered(false);
    else
        setControllerMappingAltered(true);
    emit currentControllerMappingChanged();
}

void QmlBackend::controllerMappingUpdate(Controller *controller)
{
    if(SDL_WasInit(SDL_INIT_GAMECONTROLLER)==0)
        return;
    if(controller_mapping_in_progress)
        return;
    controller_mapping_controller = controller;
    if(controller->IsSteamVirtual() && !controller_mapping_reset_requested)
    {
        controllerMappingQuit();
        emit controllerMappingSteamControllerSelected();
        return;
    }
    controller_mapping_id = controller->GetDeviceID();
    const char *mapping = SDL_GameControllerMapping(controller->GetController());
    QString original_controller_mapping(mapping);
    qCInfo(chiakiGui) << "Original controller mapping: " << original_controller_mapping;
    SDL_free((char *)mapping);
    if(original_controller_mapping.isEmpty())
    {
        qCWarning(chiakiGui) << "Error retrieving controller mapping " << SDL_GetError();
        controller_mapping_id = -1;
        controller_mapping_controller = nullptr;
        return;
    }
	QStringList mapping_results = original_controller_mapping.split(u',');
    if(mapping_results.length() < 2)
    {
        qCWarning(chiakiGui) << "Received invalid Mapping List";
        controller_mapping_id = -1;
        controller_mapping_controller = nullptr;
        return;
    }
	controller_mapping_controller_guid = mapping_results.takeFirst();
    controller_mapping_controller_vid_pid = controller_mapping_controller->GetVIDPIDString();
    if(!controller_mapping_original_controller_mappings.contains(controller_mapping_controller_guid))
    {
        setControllerMappingDefaultMapping(true);
        controller_mapping_original_controller_mappings.insert(controller_mapping_controller_guid, original_controller_mapping);
    }
    controller_mapping_controller->EnableAnalogStickMapping(enable_analog_stick_mapping);
	controller_mapping_controller_type = mapping_results.takeFirst();
    if(controller_mapping_controller_type == "*")
    {
        controller_mapping_controller_type = controller_mapping_controller->GetType();
        if(controller_mapping_controller_type.isEmpty())
            controller_mapping_controller_type = QString("Unidentified Controller");
    }
    for(int i=0; i<mapping_results.length(); i++)
    {
        QString individual_mapping = mapping_results.at(i);
        QStringList individual_mapping_list = individual_mapping.split(u':');
        if(individual_mapping_list.length() < 2)
            continue;
        QString key = individual_mapping_list.takeFirst();
        if(controller_mapping_controller_mappings.contains(key))
        {
            auto update_list = controller_mapping_controller_mappings.value(key);
            individual_mapping_list = update_list + individual_mapping_list;
        }
        controller_mapping_controller_mappings.insert(key, individual_mapping_list);
        // only add buttons to physical mapping list
        if(key != "crc" && key != "platform" && key != "type" && key != "hint" && !key.startsWith("sdk"))
        {
            for(int j = 0; j < individual_mapping_list.length(); j++)
                controller_mapping_physical_button_mappings.insert(individual_mapping_list[j], key);
        }
    }

    controller_mapping_applied_controller_mappings = controller_mapping_controller_mappings;
    emit currentControllerTypeChanged();
    emit currentControllerMappingChanged();
    setControllerMappingInProgress(true);
}

void QmlBackend::controllerMappingSelectButton()
{
    if(controller_mapping_in_progress && controller_mapping_controller)
        controller_mapping_controller->IsUpdatingMappingButton(true);
}

void QmlBackend::controllerMappingReset()
{
    if(!controller_mapping_original_controller_mappings.contains(controller_mapping_controller_guid) || SDL_WasInit(SDL_INIT_GAMECONTROLLER)==0 || !settings->GetControllerMappings().keys().contains(controller_mapping_controller_vid_pid))
    {
        controllerMappingQuit();
        return;
    }
    settings->RemoveControllerMapping(controller_mapping_controller_vid_pid);
    int result = SDL_GameControllerAddMapping(controller_mapping_original_controller_mappings.value(controller_mapping_controller_guid).toUtf8().constData());
    switch(result)
    {
        case -1:
            qCWarning(chiakiGui) << "Error setting controller mapping for guid: " << controller_mapping_controller_guid << " with error: " << SDL_GetError();
            break;
        case 0:
            qCInfo(chiakiGui) << "Updated controller mapping for guid: " << controller_mapping_controller_guid;
            break;
        case 1:
            qCInfo(chiakiGui) << "Added controller mapping for guid: " << controller_mapping_controller_guid;
            break;
        default:
            qCInfo(chiakiGui) << "Unidentified problem mapping for guid: " << controller_mapping_controller_guid;
            break;
    }
    controllerMappingQuit();
}

void QmlBackend::controllerMappingQuit()
{
    if(controller_mapping_in_progress && controller_mapping_controller)
        controller_mapping_controller->IsUpdatingMappingButton(false);
    else
        creatingControllerMapping(false);
    controller_mapping_reset_requested = false;
    setEnableAnalogStickMapping(false);
    controller_mapping_controller = nullptr;
    setControllerMappingDefaultMapping(false);
    setControllerMappingAltered(false);
    controller_mapping_id = -1;
    controller_mapping_controller_guid.clear();
    controller_mapping_controller_vid_pid.clear();
    controller_mapping_controller_type.clear();
    controller_mapping_controller_mappings.clear();
    controller_mapping_applied_controller_mappings.clear();
    controller_mapping_physical_button_mappings.clear();
    setControllerMappingInProgress(false);
    emit currentControllerTypeChanged();
    emit currentControllerMappingChanged();
}

void QmlBackend::controllerMappingButtonQuit()
{
    if(controller_mapping_in_progress && controller_mapping_controller)
        controller_mapping_controller->IsUpdatingMappingButton(false);
}

void QmlBackend::controllerMappingApply()
{
    QString new_controller_mapping = controller_mapping_controller_type;
    QMapIterator<QString, QStringList> i(controller_mapping_controller_mappings);
    while (i.hasNext()) {
        i.next();
        const auto &physical_buttons = i.value();
        for(int j = 0; j < physical_buttons.length(); j++)
            new_controller_mapping += "," + (i.key() + ":" + physical_buttons.at(j));
    }
    // if user actually reset to default manually, reset mapping, else add
    if(new_controller_mapping == controller_mapping_original_controller_mappings.value(controller_mapping_controller_guid))
    {
        settings->RemoveControllerMapping(controller_mapping_controller_vid_pid);
        int result = SDL_GameControllerAddMapping(controller_mapping_original_controller_mappings.value(controller_mapping_controller_guid).toUtf8().constData());
        switch(result)
        {
            case -1:
                qCWarning(chiakiGui) << "Error setting controller mapping for guid: " << controller_mapping_controller_guid << " with error: " << SDL_GetError();
                break;
            case 0:
                qCInfo(chiakiGui) << "Updated controller mapping for guid: " << controller_mapping_controller_guid;
                break;
            case 1:
                qCInfo(chiakiGui) << "Added controller mapping for guid: " << controller_mapping_controller_guid;
                break;
            default:
                qCInfo(chiakiGui) << "Unidentified problem mapping for guid: " << controller_mapping_controller_guid;
                break;
        }
    }
    else
    {
        QStringList existing_vidpid;
        if(controller_guids_to_update.contains(controller_mapping_controller_vid_pid))
            existing_vidpid.append(controller_guids_to_update.value(controller_mapping_controller_vid_pid));
        existing_vidpid.append(controller_mapping_controller_guid);
        controller_guids_to_update.insert(controller_mapping_controller_vid_pid, existing_vidpid);
        settings->SetControllerMapping(controller_mapping_controller_vid_pid, new_controller_mapping);
    }
}

void QmlBackend::updateDiscoveryHosts()
{
    if (session && !connection_cancelled && !connection_dismissed && connection_profile_generation == profile_generation
            && connection_attempt.value("phase") != "failure") {
        // Only the current profile and selected console can finish this wake wait.
        for (const auto &host : discovery_manager.GetHosts()) {
            if (host.host_addr != session_info.host || host.ps5 != chiaki_target_is_ps5(session_info.target)
                    || !settings->GetRegisteredHostRegistered(host.GetHostMAC())
                    || settings->GetHiddenHostHidden(host.GetHostMAC()))
                continue;
            if (connection_target.contains("mac")
                    && host.GetHostMAC().ToString() != connection_target.value("mac").toString())
                continue;
            const auto registered = settings->GetRegisteredHost(host.GetHostMAC());
            if (registered.GetRPRegistKey() != session_info.regist_key)
                continue;
            if (!wakeup_nickname.isEmpty() && host.host_name == wakeup_nickname && host.state == CHIAKI_DISCOVERY_HOST_STATE_READY) {
                const QPointer<StreamSession> owned_session(session);
                const auto attempt_id = connection_generation;
                const bool start_session = wakeup_start;
                setConnectionPhase(QStringLiteral("found"));
                if (!owned_session || session != owned_session || connection_generation != attempt_id
                        || connection_cancelled || connection_dismissed || connection_profile_generation != profile_generation
                        || (start_session && !connectionTargetValid()))
                    break;
                wakeup_nickname.clear();
                wakeup_start = false;
                wakeup_start_timer->stop();
                connection_attempt["waitingForWake"] = false;
                try {
                    if (start_session)
                        session->Start();
                } catch (const Exception &e) {
                    {
                        QMutexLocker locker(&chiaki_log_mutex);
                        chiaki_log_ctx = nullptr;
                    }
                    session->deleteLater();
                    session = nullptr;
                    failConnection(tr("Stream failed"), tr("Failed to start Stream Session: %1").arg(e.what()));
                    break;
                }
                setConnectionPhase(QStringLiteral("connecting"));
                emit sessionChanged(session);
                sleep_inhibit->inhibit();
            } else if (host.state == CHIAKI_DISCOVERY_HOST_STATE_STANDBY && session && session->IsConnecting()) {
                if (!sendWakeup(host.host_addr, registered.GetRPRegistKey(), host.ps5))
                    break;
                const QString nickname = host.host_name;
                wakeup_nickname = nickname;
                connection_attempt["waitingForWake"] = true;
                if (connection_attempt.value("context").toString().isEmpty())
                    connection_attempt["context"] = "wake";
                setConnectionPhase(QStringLiteral("waking"));
                waking_sleeping_nicknames.append(nickname);
                const auto generation = profile_generation;
                QTimer::singleShot(WAKEUP_PSN_IGNORE_SECONDS * 1000, this, [this, nickname, generation] {
                    if (generation != profile_generation) return;
                    waking_sleeping_nicknames.removeOne(nickname);
                    emit hostsChanged();
                });
                break;
            }
        }
    }
    if (autoConnect() && !connection_cancelled && !connection_dismissed
            && connection_profile_generation == profile_generation && connection_attempt.value("phase") == "waiting") {
        if (!connectionTargetValid()) {
            clearWakeWait();
            failConnection(tr("Console is unavailable"), tr("Select the console again from the current profile."));
        } else {
            const auto discovered = discovery_manager.GetHosts();
            for (int i = 0; i < discovered.size(); ++i) {
                if (discovered.at(i).GetHostMAC() != auto_connect_mac)
                    continue;
                psn_auto_connect_timer->stop();
                connectToHost(i, discovered.at(i).host_name);
                break;
            }
        }
    }
    emit hostsChanged();
}

#if CHIAKI_GUI_ENABLE_STEAM_SHORTCUT
QString QmlBackend::getExecutable() {
#if defined(Q_OS_LINUX)
    //Check for flatpak
    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString flatpakId = env.value("FLATPAK_ID");
    QString appImagePath = env.value("APPIMAGE");
    if (!flatpakId.isEmpty()) {
        return QString("flatpak");
    }
    if (!appImagePath.isEmpty())
        return appImagePath;
#endif
    return QCoreApplication::applicationFilePath();
}

void QmlBackend::createSteamShortcut(QString shortcutName, QString launchOptions, const QJSValue &callback, QString steamDir)
{
    QJSValue cb = callback;
    QString controller_layout_workshop_id = "3049833406";
    QMap<QString, const QPixmap*> artwork;
    auto landscape = QPixmap(":/icons/steam_landscape.png");
    auto portrait = QPixmap(":/icons/steam_portrait.png");
    QImageReader reader;
    reader.setAllocationLimit(512);
    reader.setFileName(":/icons/steam_hero.png");
    auto hero = QPixmap::fromImageReader(&reader);
    auto icon = QPixmap(":/icons/steam_icon.png");
    auto logo = QPixmap(":/icons/steam_logo.png");
    artwork.insert("landscape", &landscape);
    artwork.insert("portrait", &portrait);
    artwork.insert("hero", &hero);
    artwork.insert("icon", &icon);
    artwork.insert("logo", &logo);
    
    auto infoLambda = [callback](const QString &infoMessage) {
        QJSValue icb = callback;
        if (icb.isCallable())
            icb.call({infoMessage, true, false});
    };

    auto errorLambda = [callback](const QString &errorMessage) {
        QJSValue icb = callback;
        if (icb.isCallable())
            icb.call({errorMessage, false, true});
    };
    SteamTools* steam_tools = new SteamTools(infoLambda, errorLambda, steamDir);
    bool steamExists = steam_tools->steamExists();
    if(!steamExists)
    {
        if (cb.isCallable())
            cb.call({QString("[E] Steam does not exist, cannot create Steam Shortcut"), false, true});
        delete steam_tools;
        return;
    }

    QString executable = getExecutable();
    if(executable == "flatpak")
    {
        const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString flatpakId = env.value("FLATPAK_ID");
        launchOptions.prepend(QString("run %1 ").arg(flatpakId));
    }
    SteamShortcutEntry newShortcut = steam_tools->buildShortcutEntry(std::move(shortcutName), std::move(executable), std::move(launchOptions), std::move(artwork));

    QVector<SteamShortcutEntry> shortcuts = steam_tools->parseShortcuts();
    bool found = false;
    for (auto& map : shortcuts) {
        if (map.getExe() == newShortcut.getExe() && map.getLaunchOptions() == newShortcut.getLaunchOptions()) {
            // Replace the entire map with the new one
            
            if (cb.isCallable())
                cb.call({QString("[I] Updating Steam entry"), true, false});
            map = newShortcut;
            found = true;
            break;  // Stop iterating once a match is found
        }
    }

   //If we didn't find it to update, let's add it to the end
    if (!found) {
        if (cb.isCallable())
            cb.call({QString("[I] Adding Steam entry ") + QString(newShortcut.getAppName().toStdString().c_str()), false, true});
        shortcuts.append(newShortcut);
    }
    steam_tools->updateShortcuts(std::move(shortcuts));
    steam_tools->updateControllerConfig(newShortcut.getAppName(), std::move(controller_layout_workshop_id));
    if (!found)
    {
        if (cb.isCallable())
            cb.call({QString("[I] Added Steam entry: ") + QString(newShortcut.getAppName().toStdString().c_str()), true, true});
    }
    else
    {
        if (cb.isCallable())
            cb.call({QString("[I] Updated Steam entry: ") + QString(newShortcut.getAppName().toStdString().c_str()), true, true});
    }
    delete steam_tools;
}
#endif

QString QmlBackend::openPsnLink()
{
    QUrl url = psnLoginUrl();
    QDesktopServices::openUrl(url);
    return QString(url.toEncoded());
}

QString QmlBackend::openPlaceboOptionsLink()
{
    QUrl url = QUrl("https://libplacebo.org/options/");
    QDesktopServices::openUrl(url);
    url = psnLoginUrl();
    return QString(url.toEncoded());
}

bool QmlBackend::checkPsnRedirectURL(const QUrl &url) const
{
    const QUrl destination(QString::fromStdString(PSNAuth::REDIRECT_PAGE));
    return url.isValid() && url.scheme() == destination.scheme() && url.host() == destination.host()
        && url.port(-1) == -1 && url.userInfo().isEmpty() && !url.hasFragment() && url.path() == destination.path();
}

QString QmlBackend::startPsnAuth(const QUrl &url)
{
    if (psn_auth_pending) return {};
    const QString id = QString::number(++psn_auth_generation);
    const auto codes = QUrlQuery(url).allQueryItemValues("code", QUrl::FullyDecoded);
    psn_auth_attempt = {{"id", id}, {"phase", "exchanging"}, {"profileValid", true}};
    if (!checkPsnRedirectURL(url) || codes.size() != 1 || codes.first().trimmed().isEmpty()) {
        psn_auth_attempt["phase"] = "failure";
        psn_auth_attempt["inputRejected"] = true;
        psn_auth_attempt["message"] = tr("Paste the complete Sign-in redirect URL from your browser. It must contain an authorization code.");
        emit psnAuthAttemptChanged();
        return id;
    }
    psn_auth_pending = true;
    const auto generation = profile_generation;
    auto owned = [this, id, generation] { return psn_auth_pending && profile_generation == generation && psn_auth_attempt.value("id") == id; };
    auto *request = new PSNAccountID(settings, this);
    connect(request, &PSNAccountID::Progress, this, [this, owned] {
        if (!owned()) return;
        psn_auth_attempt["phase"] = "account";
        emit psnAuthAttemptChanged();
    });
    connect(request, &PSNAccountID::AccountIDError, this, [this, owned](const QString &, const QString &message) {
        if (!owned()) return;
        psn_auth_pending = false;
        psn_auth_attempt["phase"] = "failure";
        psn_auth_attempt["message"] = message;
        psn_auth_attempt["errorText"] = message;
        emit psnAuthAttemptChanged();
    });
    connect(request, &PSNAccountID::AccountIDResponse, this, [this, owned, generation](const QString &account_id) {
        if (!owned()) return;
        psn_auth_pending = false;
        psn_auth_attempt["phase"] = "success";
        psn_auth_attempt["accountId"] = account_id;
        settings_qml->setRemotePlayAsk(false);
        if (generation != profile_generation) return;
        emit settings_qml->psnAuthTokenChanged();
        if (generation != profile_generation) return;
        emit settings_qml->psnRefreshTokenChanged();
        if (generation != profile_generation) return;
        emit settings_qml->psnAuthTokenExpiryChanged();
        if (generation != profile_generation) return;
        emit settings_qml->psnAccountIdChanged();
        if (generation != profile_generation) return;
        emit psnTokenChanged();
        if (generation != profile_generation) return;
        emit psnAuthAttemptChanged();
        if (generation == profile_generation) updatePsnHosts();
    });
    connect(request, &PSNAccountID::Finished, request, &QObject::deleteLater);
    emit psnAuthAttemptChanged();
    request->GetPsnAccountId(codes.first());
    return id;
}

void QmlBackend::initPsnAuth(const QUrl &url, const QJSValue &callback)
{
    const auto id = startPsnAuth(url);
    if (!callback.isCallable()) return;
    if (id.isEmpty() || psn_auth_attempt.value("phase") == "failure") {
        callback.call({id.isEmpty() ? tr("PSN sign-in is already in progress.") : psn_auth_attempt.value("message").toString(), false, true});
        return;
    }
    const auto generation = profile_generation;
    auto connection = std::make_shared<QMetaObject::Connection>();
    *connection = connect(this, &QmlBackend::psnAuthAttemptChanged, this, [this, id, generation, callback, connection] {
        if (psn_auth_attempt.value("id") != id || generation != profile_generation) { disconnect(*connection); return; }
        const auto phase = psn_auth_attempt.value("phase").toString();
        if (phase != "success" && phase != "failure") return;
        disconnect(*connection);
        callback.call({phase == "success" ? tr("PSN remote access is ready.") : psn_auth_attempt.value("message").toString(), phase == "success", true});
    });
}

void QmlBackend::refreshAuth()
{
    const auto generation = profile_generation;
    PSNToken *psnToken = new PSNToken(settings, this);
    connect(psnToken, &PSNToken::PSNTokenError, this, [this](const QString &error) {
        qCWarning(chiakiGui) << "Could not refresh token. Automatic PSN Connection Unavailable!" << error;
    });
    connect(psnToken, &PSNToken::UnauthorizedError, this, [this, generation] { if (generation == profile_generation) emit psnCredsExpired(); });
    connect(psnToken, &PSNToken::PSNTokenSuccess, this, []() {
        qCWarning(chiakiGui) << "PSN Remote Connection Tokens Refreshed.";
    });
    connect(psnToken, &PSNToken::PSNTokenSuccess, this, [this, generation] { if (generation == profile_generation) updatePsnHosts(); });
    connect(psnToken, &PSNToken::Finished, psnToken, &QObject::deleteLater);
    QString refresh_token = settings->GetPsnRefreshToken();
    psnToken->RefreshPsnToken(std::move(refresh_token));
}

void QmlBackend::updatePsnHosts()
{
    if(updating_psn_hosts)
    {
        psn_hosts_refresh_pending = true;
        qCInfo(chiakiGui) << "Already updating psn hosts, skipping...";
        return;
    }
    QStringList names;
    for (const auto &host : settings->GetRegisteredHosts()) names.append(host.GetServerNickname());
    psn_hosts_future = QtConcurrent::run(&QmlBackend::updatePsnHostsThread, this,
        settings->GetPsnAuthToken(), settings->GetLogLevelMask(), names, settings->GetPS4RegisteredHostsRegistered(), profile_generation);
    psn_hosts_watcher.setFuture(psn_hosts_future);
    updating_psn_hosts = true;
}

void QmlBackend::updatePsnHostsThread(QString psn_token, uint32_t log_mask, QStringList registered_names, int ps4_count, quint64 generation)
{
    QMap<QString, PsnHost> found_hosts;
    QMap<QString, PsnHost> found_nicknames;
    if(psn_token.isEmpty())
        return;

    ChiakiHolepunchDeviceInfo *device_info_ps5 = nullptr;
    size_t num_devices_ps5 = 0;
    ChiakiLog backend_log;
    chiaki_log_init(&backend_log, log_mask, chiaki_log_cb_print, nullptr);
    for(int i = 0; i < PSN_DEVICES_TRIES; i++)
    {
        ChiakiErrorCode err = chiaki_holepunch_list_devices(psn_token.toUtf8().constData(), CHIAKI_HOLEPUNCH_CONSOLE_TYPE_PS5, &device_info_ps5, &num_devices_ps5, &backend_log);
        if (err != CHIAKI_ERR_SUCCESS)
        {
            if(PSN_DEVICES_TRIES - i > 1)
            {
                qCWarning(chiakiGui) << "Failed to get PS5 devices trying again";
                continue;
            }
            else
            {
                qCWarning(chiakiGui) << "Failed to get PS5 devices after max tries: " << PSN_DEVICES_TRIES;
                num_devices_ps5 = 0;
            }
        }
        break;
    }
    for (size_t i = 0; i < num_devices_ps5; i++)
    {
        ChiakiHolepunchDeviceInfo dev = device_info_ps5[i];
        // skip devices that don't have remote play enabled
        if(!dev.remoteplay_enabled)
            continue;
        QByteArray duid_bytes(reinterpret_cast<char*>(dev.device_uid), sizeof(dev.device_uid));
        QString duid = QString(duid_bytes.toHex());
        QString name = QString(dev.device_name);
        bool ps5 = true;
        PsnHost psn_host(duid, name, ps5);
        if(!found_nicknames.contains(name))
            found_nicknames.insert(name, psn_host);
	    if(!found_hosts.contains(duid) && registered_names.contains(name))
		    found_hosts.insert(duid, psn_host);
    }
    QByteArray duid_bytes(32, 'A');
    QString duid = QString(duid_bytes.toHex());
    QString name = QString("Main PS4 Console");
    bool ps5 = false;
    PsnHost psn_host(duid, name, ps5);
    if(!found_nicknames.contains(name))
        found_nicknames.insert(name, psn_host);
    if(!found_hosts.contains(duid) && (ps4_count > 0))
        found_hosts.insert(duid, psn_host);

    QMetaObject::invokeMethod(this, [this, generation, found_hosts, found_nicknames] {
        if (generation != profile_generation) return;
        psn_hosts = found_hosts;
        psn_nickname_hosts = found_nicknames;
        emit hostsChanged();
    }, Qt::QueuedConnection);
    qCInfo(chiakiGui) << "Updated PSN hosts";
    if(device_info_ps5)
        chiaki_holepunch_free_device_list(&device_info_ps5);
}

void QmlBackend::refreshPsnToken()
{
    QString expiry_s = settings->GetPsnAuthTokenExpiry();
    QString refresh = settings->GetPsnRefreshToken();
    if(expiry_s.isEmpty() || refresh.isEmpty())
        return;
    QDateTime expiry = QDateTime::fromString(expiry_s, settings->GetTimeFormat());
    // give 1 minute buffer
    QDateTime now = QDateTime::currentDateTime().addSecs(60);
    if (now >= expiry)
        refreshAuth();
    else
        updatePsnHosts();
}

void PsnConnectionWorker::ConnectPsnConnection(StreamSession *session, const QString &duid, const bool &ps5, quint64 attempt_id)
{
    ChiakiErrorCode result = session->ConnectPsnConnection(duid, ps5);
    emit resultReady(session, attempt_id, result);
}
