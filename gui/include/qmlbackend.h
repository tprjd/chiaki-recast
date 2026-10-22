#pragma once

#include "streamsession.h"
#include "discoverymanager.h"
#include "qmlmainwindow.h"
#include "qmlcontroller.h"
#include "qmlsettings.h"
#include "psnrequests.h"
#include "registrationrequests.h"

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QSet>
#include <QUrl>
#include <QFutureWatcher>
#include <QFuture>
#include <QAtomicInteger>
#include <QPointer>
#ifdef CHIAKI_HAVE_WEBENGINE
#include <QQuickWebEngineProfile>
#include <QWebEngineProfile>
#include <QWebEngineUrlRequestInterceptor>
#endif

class SystemdInhibit;
class PSNToken;
#ifdef Q_OS_MACOS
    class MacWakeSleep;
#elif defined(Q_OS_WINDOWS)
    class WindowsWakeSleep;
#endif

class QmlRegist : public QObject
{
    Q_OBJECT

public:
    QmlRegist(uint32_t log_mask, QObject *parent = nullptr, RegistrationRequests requests = {});
    ~QmlRegist();
    ChiakiErrorCode start(const ChiakiRegistInfo &regist_info);

signals:
    void log(ChiakiLogLevel level, const QString &msg);
    void failed();
    void success(const RegisteredHost& host);

private:
    static void log_cb(ChiakiLogLevel level, const char *msg, void *user);
    static void regist_cb(ChiakiRegistEvent *event, void *user);

    ChiakiLog chiaki_log;
    ChiakiRegist chiaki_regist;
    RegistrationRequests requests;
    bool started = false;
};

class PsnConnectionWorker : public QObject
{
    Q_OBJECT

public:
    explicit PsnConnectionWorker(PsnRequests::Connect request = {}) : request(std::move(request)) {}
    void ConnectPsnConnection(StreamSession *session, const QString &duid, const bool &ps5, quint64 attempt_id);

signals:
    void resultReady(StreamSession *session, quint64 attempt_id, const ChiakiErrorCode &err);
private:
    PsnRequests::Connect request;
};

#ifdef CHIAKI_HAVE_WEBENGINE
class SecUaRequestInterceptor : public QWebEngineUrlRequestInterceptor {
    Q_OBJECT

public:
    SecUaRequestInterceptor(QString version, QObject *p = Q_NULLPTR) : QWebEngineUrlRequestInterceptor(p) { chrome_version = version; };
    void interceptRequest(QWebEngineUrlRequestInfo &info) override {
#ifdef Q_OS_WINDOWS
        info.setHttpHeader("Sec-Ch-Ua", QString("\"Not(A:Brand\";v=\"99\", \"Microsoft Edge\";v=\"%1\", \"Chromium\";v=\"%1\"").arg(chrome_version).toUtf8());
#else
        info.setHttpHeader("Sec-Ch-Ua", QString("\"Not(A:Brand\";v=\"99\", \"Google Chrome\";v=\"%1\", \"Chromium\";v=\"%1\"").arg(chrome_version).toUtf8());
#endif
    }
private:
    QString chrome_version;
};
#endif
class QmlBackend : public QObject
{
    friend class StreamInputNativeTest;
    Q_OBJECT
    Q_PROPERTY(QmlMainWindow* window READ qmlWindow NOTIFY windowChanged)
    Q_PROPERTY(QmlSettings* settings READ qmlSettings CONSTANT)
    Q_PROPERTY(StreamSession* session READ qmlSession NOTIFY sessionChanged)
    Q_PROPERTY(QVariantMap connectionAttempt READ connectionAttempt NOTIFY connectionAttemptChanged)
    Q_PROPERTY(QVariantMap sessionPinRequest READ sessionPinRequest NOTIFY sessionPinRequestChanged)
    Q_PROPERTY(QVariantMap psnAuthAttempt READ psnAuthAttempt NOTIFY psnAuthAttemptChanged)
    Q_PROPERTY(QVariantMap psnRefresh READ psnRefresh NOTIFY psnRefreshChanged)
    Q_PROPERTY(QList<QmlController*> controllers READ qmlControllers NOTIFY controllersChanged)
    Q_PROPERTY(bool discoveryEnabled READ discoveryEnabled WRITE setDiscoveryEnabled NOTIFY discoveryEnabledChanged)
    Q_PROPERTY(QVariantList hosts READ hosts NOTIFY hostsChanged)
    Q_PROPERTY(QVariantList hiddenHosts READ hiddenHosts NOTIFY hiddenHostsChanged)
    Q_PROPERTY(bool autoConnect READ autoConnect NOTIFY autoConnectChanged)
    Q_PROPERTY(PsnConnectState connectState READ connectState WRITE setConnectState NOTIFY connectStateChanged)
    Q_PROPERTY(QVariantList currentControllerMapping READ currentControllerMapping NOTIFY currentControllerMappingChanged)
    Q_PROPERTY(QString currentControllerType READ currentControllerType NOTIFY currentControllerTypeChanged)
    Q_PROPERTY(bool controllerMappingDefaultMapping READ controllerMappingDefaultMapping NOTIFY controllerMappingDefaultMappingChanged)
    Q_PROPERTY(bool controllerMappingInProgress READ controllerMappingInProgress NOTIFY controllerMappingInProgressChanged)
    Q_PROPERTY(bool controllerMappingAltered READ controllerMappingAltered NOTIFY controllerMappingAlteredChanged)
    Q_PROPERTY(bool enableAnalogStickMapping READ enableAnalogStickMapping WRITE setEnableAnalogStickMapping NOTIFY enableAnalogStickMappingChanged)

public:

    enum class PsnConnectState
    {
        NotStarted,
        WaitingForInternet,
        InitiatingConnection,
        LinkingConsole,
        RegisteringConsole,
        RegistrationFinished,
        DataConnectionStart,
        DataConnectionFinished,
        ConnectFailed,
        ConnectFailedStart,
        ConnectFailedConsoleUnreachable,
    };
    Q_ENUM(PsnConnectState);
    explicit QmlBackend(Settings *settings, QObject *parent = nullptr, PsnRequests requests = {}, QmlSettings::ProcessLauncher launcher = {}, RegistrationRequests registration = {}, DiscoveryRequests discovery = {});
    ~QmlBackend();

    QmlMainWindow *qmlWindow() const;
    Settings *nativeSettings() const { return settings; }
    DiscoveryManager *discoveryManager() { return &discovery_manager; }
    quint64 profileGeneration() const { return profile_generation; }
    void attachWindow(QmlMainWindow *window);
    void detachWindow(QmlMainWindow *window);
    void setSetupNavigationTarget(QObject *target);
    void setControllerNavigationEnabled(bool enabled);
    QmlSettings *qmlSettings() const;
    StreamSession *qmlSession() const;
    QList<QmlController*> qmlControllers() const;

    bool discoveryEnabled() const;
    void setDiscoveryEnabled(bool enabled);
    void refreshAuth();

    PsnConnectState connectState() const;
    void setConnectState(PsnConnectState connect_state);
    QVariantList hosts() const;

    QVariantList hiddenHosts() const;

    QVariantList currentControllerMapping() const;

    QString currentControllerType() const { return controller_mapping_controller_type; }

    void controllerMappingChangeButton(QString button);

    void controllerMappingUpdate(Controller *controller);

    bool controllerMappingDefaultMapping() const { return controller_mapping_default_mapping; }
    void setControllerMappingDefaultMapping(bool is_default_mapping);

    bool controllerMappingAltered() const { return controller_mapping_altered; }
    void setControllerMappingAltered(bool altered);

    bool controllerMappingInProgress() const  {return controller_mapping_in_progress; }
    void setControllerMappingInProgress(bool is_in_progress);

    bool enableAnalogStickMapping() const { return enable_analog_stick_mapping; }
    void setEnableAnalogStickMapping(bool enabled);

    void finishAutoRegister(const ChiakiRegisteredHost &host);

    bool autoConnect() const;

    void psnConnector();

    void createSession(const StreamSessionConnectInfo &connect_info, const QVariantMap &target = {}, bool continue_attempt = false);
    QVariantMap connectionAttempt() const;
    QVariantMap sessionPinRequest() const { return session_pin_request; }

    void psnSessionStart();

    void checkPsnConnection(const ChiakiErrorCode &err);

    void checkNickname(QString nickname);

    bool closeRequested();

    void setAllowJoystickBackgroundEvents();

    void setIsAppActive();

    void profileChanged();
    bool prepareFrameForPresentation(ChiakiFfmpegFrame &frame, bool use_opengl_renderer);

    void goToSleep();

    bool zeroCopy()        { return !disable_zero_copy; };
    void disableZeroCopy() { disable_zero_copy = true; };
    void enableZeroCopy()  { disable_zero_copy = false; };

    Q_INVOKABLE void deleteHost(int index);
    Q_INVOKABLE bool wakeUpHost(int index, QString nickname = QString());
    Q_INVOKABLE void addManualHost(int index, const QString &address);
    Q_INVOKABLE void hideHost(const QString &mac_string, const QString &host_nickname);
    Q_INVOKABLE void unhideHost(const QString &mac_string);
    Q_INVOKABLE QVariantMap createRegistrationContext(int index = -1) const;
    bool registrationContextCurrent(const QVariantMap &context, const QString &address) const;
    QVariantMap registrationAttempt() const { return registration_attempt; }
    bool registrationBusy() const { return registration_active; }
    Q_INVOKABLE bool registerHost(const QString &host, const QString &psn_id, const QString &pin, const QString &cpin, bool broadcast, int target, const QVariantMap &context);
    Q_INVOKABLE void connectToHost(int index, QString nickname = QString());
    Q_INVOKABLE void stopSession(bool sleep);
    Q_INVOKABLE bool retryConnection(const QString &attempt_id);
    Q_INVOKABLE bool cancelConnection(const QString &attempt_id);
    Q_INVOKABLE bool dismissConnection(const QString &attempt_id);
    Q_INVOKABLE void sessionGoHome();
    Q_INVOKABLE bool enterPin(const QString &request_id, const QString &pin);
    Q_INVOKABLE bool cancelSessionPin(const QString &request_id);
    Q_INVOKABLE bool dismissSessionPin(const QString &request_id);
    Q_INVOKABLE QUrl psnLoginUrl() const;
    Q_INVOKABLE bool checkPsnRedirectURL(const QUrl &url) const;
    QVariantMap psnAuthAttempt() const { return psn_auth_attempt; }
    QVariantMap psnRefresh() const { return psn_refresh; }
    Q_INVOKABLE QString startPsnAuth(const QUrl &url);
    QVariantMap accountLookupAttempt() const { return account_lookup_attempt; }
    QString startAccountLookup(const QString &username);
    bool cancelAccountLookup(const QString &id);
    Q_INVOKABLE bool stopAutoConnect(const QString &attempt_id = QString());
    Q_INVOKABLE bool setConsolePin(int index, QString console_pin);
    Q_INVOKABLE QString openPsnLink();
    Q_INVOKABLE QString openPlaceboOptionsLink();
    Q_INVOKABLE void psnCancel(bool stop_thread);
    Q_INVOKABLE void refreshPsnToken();
    Q_INVOKABLE void beginControllerMapping(bool reset_mapping);
    Q_INVOKABLE void creatingControllerMapping(bool creating_controller_mapping);
    Q_INVOKABLE void updateButton(int chiaki_button, QString physical_button, int new_index);
    Q_INVOKABLE void controllerMappingSelectButton();
    Q_INVOKABLE void controllerMappingReset();
    Q_INVOKABLE void controllerMappingQuit();
    Q_INVOKABLE void controllerMappingButtonQuit();
    Q_INVOKABLE void controllerMappingApply();
    bool autoRegisterHost(int index);
    void autoRegister(const QVariantMap &context);
#if CHIAKI_GUI_ENABLE_STEAM_SHORTCUT
    void createSteamShortcut(const QString &operation, QString shortcutName, QString launchOptions, QString steamDir);
#endif
#ifdef CHIAKI_HAVE_WEBENGINE
    Q_INVOKABLE void setWebEngineHints(QQuickWebEngineProfile *profile);
    void setWebEngineHints(QWebEngineProfile *profile);
    Q_INVOKABLE void clearCookies(QQuickWebEngineProfile *profile);
#endif

signals:
    void steamShortcutMessage(const QString &operation, const QString &message, bool ok, bool done);
    void streamWindowRequested();
    void windowChanged();
    void sessionChanged(StreamSession *session);
    void connectionAttemptChanged();
    void sessionPinRequestChanged();
    void psnConnect(StreamSession *session, const QString &duid, const bool &ps5, quint64 attempt_id);
    void showPsnView();
    void connectStateChanged();
    void controllersChanged();
    void currentControllerMappingChanged();
    void currentControllerTypeChanged();
    void controllerMappingInProgressChanged();
    void controllerMappingDefaultMappingChanged();
    void controllerMappingButtonSelected(QString button);
    void controllerMappingAlteredChanged();
    void controllerMappingSteamControllerSelected();
    void enableAnalogStickMappingChanged();
    void discoveryEnabledChanged();
    void hostsChanged();
    void hiddenHostsChanged();
    void psnTokenChanged();
    void psnRefreshChanged();
    void psnAuthAttemptChanged();
    void registrationAttemptChanged();
    void accountLookupChanged();
    void psnCredsExpired();
    void autoConnectChanged();
    void windowTypeUpdated(WindowType type);

    void error(const QString &title, const QString &text);
    void sessionError(const QString &title, const QString &text);
    void sessionPinDialogRequested(bool incorrect);
    void sessionStopDialogRequested();

private:
    struct DisplayServer {
        bool valid = false;

        DiscoveryHost discovery_host;
        ManualHost manual_host;
        bool discovered = false;

        PsnHost psn_host;
        QString duid;
        RegisteredHost registered_host;
        bool registered = false;

        QString GetHostAddr() const { return discovered ? discovery_host.host_addr : manual_host.GetHost(); }
        bool IsPS5() const { return discovered ? discovery_host.ps5 :
            (registered ? chiaki_target_is_ps5(registered_host.GetTarget()) : true); }
    };

    DisplayServer displayServerAt(int index) const;
    static QVariantMap connectionTargetKey(const QVariantMap &host);
    int resolveConnectionTarget() const;
    bool connectionTargetValid() const;
    void beginConnection(const StreamSessionConnectInfo &info, const QVariantMap &target, const QString &context = {});
    void startPsnConnection(const StreamSessionConnectInfo &info, const QVariantMap &target, bool continue_attempt = false);
    void watchConnectionToken(PSNToken *token);
    void finishPsnCleanup();
    void releaseSession();
    void failConnection(const QString &title, const QString &text);
    void setConnectionPhase(const QString &phase);
    void beginAutoConnect(const HostMAC &mac);
    void autoConnectPsn();
    void clearWakeWait();
    bool sendWakeup(const DisplayServer &server);
    bool sendWakeup(const QString &host, const QByteArray &regist_key, bool ps5);
    void updateControllers();
    void updateControllerMappings();
    void updateDiscoveryHosts();
    void updatePsnHosts();
    void updatePsnHostsThread(QString token, uint32_t log_mask, QStringList registered_names, int ps4_count, quint64 generation);
    void updateAudioVolume();
    void resumeFromSleep();
    void reconnectPsn();
    uint32_t getStreamShortcut() const;
    void updateStreamShortcut();
    QString getExecutable();

    Settings *settings = {};
    QmlSettings *settings_qml = {};
    QPointer<QmlMainWindow> window;
    QPointer<QObject> setup_navigation_target;
    bool controller_navigation_enabled = true;
    QList<QMetaObject::Connection> window_connections;
    StreamSession *session = {};
    QThread *frame_thread = {};
    QObject *frame_worker = {};
    QObject *frame_delivery = {};
    PsnConnectionWorker *psn_connection_worker = {};
    QTimer *psn_reconnect_timer = {};
    QTimer *psn_auto_connect_timer = {};
    QTimer *wakeup_start_timer = {};
    int psn_reconnect_tries = 0;
    QThread psn_connection_thread;
    PsnConnectState psn_connect_state;
    DiscoveryManager discovery_manager;
    QList<QString> waking_sleeping_nicknames;
    QHash<int, QmlController*> controllers;
    DisplayServer regist_dialog_server;
    quint64 profile_generation = 0;
    quint64 psn_auth_generation = 0;
    QVariantMap psn_auth_attempt;
    bool psn_auth_pending = false;
    quint64 account_lookup_generation = 0;
    QVariantMap account_lookup_attempt;
    QPointer<QNetworkReply> account_lookup_reply;
    quint64 connection_generation = 0;
    quint64 connection_profile_generation = 0;
    QVariantMap connection_attempt;
    QVariantMap connection_target;
    bool connection_cancelled = false;
    bool connection_dismissed = false;
    bool connection_direct_credentials = false;
    QVariantMap session_pin_request;
    quint64 session_pin_generation = 0;
    bool session_pin_auto_submitted = false;
    bool session_pin_accepted = false;
    bool psn_worker_pending = false;
    bool psn_session_started = false;
    QPointer<PSNToken> connection_token;
    quint64 regist_dialog_generation = 0;
    quint64 automatic_registration_request = 0;
    bool registration_active = false;
    quint64 registration_generation = 0;
    QVariantMap registration_attempt;
    RegistrationRequests registration_requests;
    StreamSessionConnectInfo session_info = {};
    SystemdInhibit *sleep_inhibit = {};
#ifdef Q_OS_MACOS
    MacWakeSleep *mac_wake_sleep = {};
#elif defined(Q_OS_WINDOWS)
    WindowsWakeSleep *windows_wake_sleep = {};
#endif
    bool controller_mapping_default_mapping = false;
    bool controller_mapping_altered = false;
    bool updating_psn_hosts = false;
    bool psn_hosts_refresh_pending = false;
    QVariantMap psn_refresh;
    QPointer<PSNToken> refresh_token;
    PsnRequests psn_requests;
    QFutureWatcher<void> psn_hosts_watcher;
    QFuture<void> psn_hosts_future;
    bool disable_zero_copy = false;
    QMutex hw_transfer_state_mutex;
    QSet<int> logged_hw_transfer_formats;
    QSet<int> logged_hw_transfer_failures;
    QAtomicInteger<int> pending_recovered_frame = 0;
    Controller *controller_mapping_controller = {};
    QMap<QString, QStringList> controller_guids_to_update = {};
    int controller_mapping_id = -1;
    QString controller_mapping_controller_guid = "";
    QString controller_mapping_controller_vid_pid = "";
    QString controller_mapping_controller_type = "";
    QMap<QString, QStringList> controller_mapping_controller_mappings = {};
    QMap<QString, QStringList> controller_mapping_applied_controller_mappings = {};
    QMap<QString, QString> controller_mapping_physical_button_mappings = {};
    QMap<QString, QString> controller_mapping_original_controller_mappings = {};
    bool controller_mapping_reset_requested = false;
    bool controller_mapping_in_progress = false;
    bool enable_analog_stick_mapping = false;
    bool resume_session = false;
    bool settings_allocd = false;
    HostMAC auto_connect_mac = {};
    QString auto_connect_nickname = "";
    QString wakeup_nickname = "";
    bool wakeup_start = false;
    QMap<QString, PsnHost> psn_hosts = {};
    QMap<QString, PsnHost> psn_nickname_hosts = {};
};
