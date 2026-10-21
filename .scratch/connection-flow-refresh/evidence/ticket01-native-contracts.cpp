#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSettings>
#include <QTimer>
#include <QEventLoop>
#include "qmlbackend.h"
#include "qmlmainwindow.h"

namespace {
QString runRoot;
QJsonObject evidence;
QmlBackend *backend;
QmlMainWindow *window;
void save() { QFile out(runRoot + "/contracts.json"); if (!out.open(QIODevice::WriteOnly)) qFatal("Cannot write connection evidence"); out.write(QJsonDocument(evidence).toJson()); }
void check(bool value, const char *message) { if (value) return; evidence["failedCheck"] = message; save(); qFatal("Connection contract failed: %s", message); }
void settle() { QEventLoop loop; QTimer::singleShot(260, &loop, &QEventLoop::quit); loop.exec(); }
QQuickItem *named(QQuickItem *root, const QString &name) { if (root->objectName() == name) return root; for (auto *child : root->childItems()) if (auto *found = named(child, name)) return found; return nullptr; }
QObject *navigation() { for (auto *candidate : QGuiApplication::allWindows()) if (auto *quick = qobject_cast<QQuickWindow *>(candidate)) if (auto *result = named(quick->contentItem(), "mainNavigation")) return result; return nullptr; }
bool libraryShown() { auto *stack = navigation(); auto *item = stack ? qvariant_cast<QObject *>(stack->property("currentItem")) : nullptr; return item && QByteArray(item->metaObject()->className()).startsWith("MainView_"); }
QString phase() { return backend->connectionAttempt().value("phase").toString(); }
QString id() { return backend->connectionAttempt().value("id").toString(); }
int index(const QString &address) { const auto all = backend->hosts(); for (int i = 0; i < all.size(); ++i) if (all.at(i).toMap().value("address") == address && all.at(i).toMap().value("display").toBool()) return i; return -1; }
void event(StreamSession *session, ChiakiEventType type, ChiakiQuitReason reason = CHIAKI_QUIT_REASON_STOPPED) { ChiakiEvent value{}; value.type = type; if (type == CHIAKI_EVENT_QUIT) value.quit.reason = reason; session->nativeQaEvent(&value); }
int count(const char *name) { return qApp->property(name).toInt(); }
RegisteredHost host(int number) {
    ChiakiRegisteredHost native{}; native.target = CHIAKI_TARGET_PS5_1; native.server_mac[0] = 2; native.server_mac[5] = number;
    qstrncpy(native.server_nickname, number == 1 ? "Synthetic target" : "Synthetic peer", sizeof(native.server_nickname));
    memset(native.rp_regist_key, 'a' + number, sizeof(native.rp_regist_key)); memset(native.rp_key, number, sizeof(native.rp_key));
    return RegisteredHost(native);
}
void seed(Settings &settings) {
    settings.AddRegisteredHost(host(1)); settings.AddRegisteredHost(host(2));
    settings.SetManualHost(ManualHost(-1, "192.0.2.42", true, host(1).GetServerMAC()));
    settings.SetManualHost(ManualHost(-1, "192.0.2.43", true, host(2).GetServerMAC()));
    settings.SetHardwareDecoder(""); settings.SetDecoder(Decoder::Ffmpeg); settings.SetWindowType(WindowType::SelectedResolution);
}
void contracts() {
    for (auto *candidate : QGuiApplication::topLevelWindows()) if (auto *main = qobject_cast<QmlMainWindow *>(candidate)) { window = main; backend = main->getBackend(); }
    check(backend && window, "Production backend must exist"); backend->stopAutoConnect();
    check(!window->isVisible() && !window->isExposed() && !backend->discoveryEnabled() && !backend->qmlSession(), "Startup must stay hidden and isolated");
    check(!backend->cancelConnection(QString()) && !backend->dismissConnection(QString()), "An empty startup ID must not own cancellation or dismissal");
    auto *settings = backend->nativeQaSettings();
    settings->SetAutoConnectHost(QByteArray(reinterpret_cast<const char *>(host(1).GetServerMAC().GetMAC()), 6));
    const auto savedAuto = settings->GetAutoConnectHost().GetServerMAC();
    int pins = 0; QObject::connect(backend, &QmlBackend::sessionPinDialogRequested, backend, [&](bool) { ++pins; });

    QObject::connect(backend, &QmlBackend::error, backend, [&](const QString &title, const QString &text) { evidence["lastError"] = title + ": " + text; });
    backend->connectToHost(index("192.0.2.42"));
    evidence["firstAttempt"] = QJsonObject::fromVariantMap(backend->connectionAttempt()); evidence["startCount"] = count("qaStarts"); evidence["hostCount"] = backend->hosts().size();
    auto *first = backend->qmlSession(); check(first && phase() == "connecting" && count("qaStarts") == 1, "Local connect must dispatch one native start");
    settle(); evidence["grabCount"] = window->nativeQaGrabCount(); if (auto *nav = navigation()) { auto *item=qvariant_cast<QObject *>(nav->property("currentItem")); evidence["currentScreen"] = item ? item->metaObject()->className() : "null"; }

    event(first, CHIAKI_EVENT_CONNECTED); check(phase() == "video" && !window->hasVideo(), "Connection must wait for video");
    window->nativeQaVideo(true); check(phase() == "ready", "Actual video readiness must release progress input");
    event(first, CHIAKI_EVENT_QUIT); settle();
    backend->hostsChanged(); settle();
    check(libraryShown() && !backend->connectionAttempt().value("presented").toBool(), "Normal quit and later discovery must stay in the library");
    evidence["localConnectingVideoReadyNormalQuit"] = true;
    backend->connectToHost(index("192.0.2.42")); auto *earlyStop = backend->qmlSession();
    event(earlyStop, CHIAKI_EVENT_QUIT);
    check(phase() == "cancelled" && !backend->qmlSession(), "A clean native stop before readiness must finish cancellation");
    backend->dismissConnection(id()); evidence["cleanStopBeforeReadiness"] = true;

    qputenv("CHIAKI_QA_REJECT_START", "1"); backend->connectToHost(index("192.0.2.42")); qunsetenv("CHIAKI_QA_REJECT_START");
    check(!backend->qmlSession() && phase() == "failure" && !backend->connectionAttempt().value("cleanupPending").toBool(), "Native start failure must leave a retryable persistent error");
    const QString failed = id();
    const auto original = settings->GetManualHosts().first();
    settings->RemoveManualHost(original.GetID()); settings->SetManualHost(ManualHost(-1, "192.0.2.42", true, host(1).GetServerMAC()));
    check(backend->retryConnection(failed), "Retry must resolve the intended console after row reorder");
    auto *second = backend->qmlSession(); check(second && second->GetHost() == "192.0.2.42" && id() != failed && count("qaStarts") == 4, "Retry must start the same console exactly once");
    check(!backend->retryConnection(failed) && count("qaStarts") == 4, "Old/repeated retry must not start another session");
    const QString secondId = id();
    check(backend->cancelConnection(secondId), "Cancel must accept the active attempt");
    check(!backend->cancelConnection(secondId) && count("qaStops") == 1 && phase() == "cancelling", "Cancel must stop exactly once and wait for cleanup");
    event(second, CHIAKI_EVENT_CONNECTED); second->LoginPINRequested(true); second->DataHolepunchProgress(true);
    check(phase() == "cancelling" && pins == 0 && backend->qmlSession() == second, "Late success and PIN must not complete cancellation");
    check(!backend->retryConnection(secondId), "Retry must wait for the old session to stop");
    event(second, CHIAKI_EVENT_QUIT, CHIAKI_QUIT_REASON_SESSION_REQUEST_UNKNOWN);
    check(phase() == "cancelled" && !backend->qmlSession() && !backend->connectionAttempt().value("cleanupPending").toBool(), "Only native cleanup may show cancelled");
    check(count("qaSleeps") == 0 && settings->GetAutoConnectHost().GetServerMAC() == savedAuto, "Cancellation must not sleep the console or change auto-connect preference");
    evidence["startFailureStableRetryAndCancelOnce"] = true;

    backend->dismissConnection(id()); backend->connectToHost(index("192.0.2.42"));
    auto *third = backend->qmlSession(); const QString thirdId = id();
    second->SessionQuit(CHIAKI_QUIT_REASON_STOPPED, {}); second->LoginPINRequested(true); second->ConnectedChanged(); second->DataHolepunchProgress(true);
    check(backend->qmlSession() == third && id() == thirdId && phase() == "connecting" && pins == 0, "Old session callbacks must not mutate the replacement attempt");
    event(third, CHIAKI_EVENT_CONNECTED);
    bool failureReentry = false;
    auto duringError = QObject::connect(backend, &QmlBackend::sessionError, backend, [&](const QString &, const QString &) {
        window->nativeQaVideo(true); event(third, CHIAKI_EVENT_CONNECTED); failureReentry = phase() == "failure";
    });
    event(third, CHIAKI_EVENT_QUIT, CHIAKI_QUIT_REASON_SESSION_REQUEST_UNKNOWN); QObject::disconnect(duringError);
    check(failureReentry && phase() == "failure", "Late video/connection during an error must not erase persistent failure");
    window->nativeQaVideo(false);
    evidence["oldSessionAndErrorReentryIgnored"] = true;

    settings->SetManualHost(ManualHost(-1, "192.0.2.42", true, host(1).GetServerMAC()));
    check(!backend->connectionAttempt().value("targetValid").toBool() && !backend->retryConnection(id()), "Ambiguous console must not retry");
    const auto duplicate = settings->GetManualHosts().last(); settings->RemoveManualHost(duplicate.GetID());
    DiscoveryHost discovered{}; discovered.ps5 = true; discovered.target = CHIAKI_TARGET_PS5_1; discovered.host_addr = "192.0.2.42";
    discovered.host_name = "Synthetic target"; discovered.host_id = "020000000001"; discovered.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
    backend->nativeQaDiscover({discovered});
    check(!backend->connectionAttempt().value("targetValid").toBool() && !backend->retryConnection(id()), "A hidden old row must not retry");
    backend->nativeQaDiscover({});
    backend->deleteHost(index("192.0.2.42"));
    check(!backend->connectionAttempt().value("targetValid").toBool() && !backend->retryConnection(id()), "Removed console must not retry to a replacement row");
    settings->SetManualHost(ManualHost(-1, "192.0.2.42", true, host(1).GetServerMAC()));
    const QString beforeProfile = id(); backend->qmlSettings()->setCurrentProfile("other"); backend->stopAutoConnect(); backend->qmlSettings()->setCurrentProfile(""); backend->stopAutoConnect();
    check(id() == beforeProfile && !backend->connectionAttempt().value("targetValid").toBool() && !backend->retryConnection(id()), "Away-and-back profile changes must invalidate the old attempt");
    evidence["ambiguousHiddenRemovedAndProfileTargetsRejected"] = true;

    backend->dismissConnection(id()); backend->connectToHost(index("192.0.2.42"));
    auto *closed = backend->qmlSession(); const QString closedId = id(); check(backend->dismissConnection(closedId), "View removal must dismiss the active attempt");
    closed->LoginPINRequested(true); event(closed, CHIAKI_EVENT_CONNECTED); window->nativeQaVideo(true);
    check(!backend->connectionAttempt().value("presented").toBool() && phase() == "cancelling" && pins == 0, "Closed view must ignore late PIN and readiness");
    event(closed, CHIAKI_EVENT_QUIT); window->nativeQaVideo(false);
    evidence["viewRemovalOwnsCancellation"] = true;

    settings = backend->nativeQaSettings(); settings->SetAudioVideoDisabled(CHIAKI_AUDIO_VIDEO_DISABLED);
    backend->connectToHost(index("192.0.2.42")); auto *noVideo = backend->qmlSession();
    check(noVideo, "Video-disabled session must start"); event(noVideo, CHIAKI_EVENT_CONNECTED);
    check(phase() == "ready" && !window->hasVideo(), "Video-disabled sessions must be ready on connection");
    event(noVideo, CHIAKI_EVENT_QUIT); settle();
    evidence["videoDisabledReadiness"] = true;
    backend->connectToHost(index("192.0.2.42")); auto *profileSession = backend->qmlSession();
    backend->qmlSettings()->setCurrentProfile("other"); backend->stopAutoConnect();
    check(phase() == "failure" && backend->connectionAttempt().value("cleanupPending").toBool(), "Profile switch must stop the old attempt and wait for cleanup");
    event(profileSession, CHIAKI_EVENT_CONNECTED); profileSession->LoginPINRequested(true);
    check(phase() == "failure" && pins == 0 && !backend->retryConnection(id()), "Old profile events and retry must be rejected");
    event(profileSession, CHIAKI_EVENT_QUIT); check(!backend->connectionAttempt().value("cleanupPending").toBool(), "Profile-invalid attempt must still complete cleanup");
    backend->qmlSettings()->setCurrentProfile(""); backend->stopAutoConnect(); backend->dismissConnection(id()); settings = backend->nativeQaSettings();
    evidence["activeProfileSwitchStopsAndScopesEvents"] = true;
    const auto registered = host(1);
    StreamSessionConnectInfo direct(settings, registered.GetTarget(), "192.0.2.99", {}, registered.GetRPRegistKey(), registered.GetRPKey(), {}, {}, false, false, false, false);
    backend->createSession(direct); auto *directSession = backend->qmlSession();
    check(directSession && backend->connectionAttempt().value("targetValid").toBool(), "Direct entry must retain valid profile-scoped connection context");
    event(directSession, CHIAKI_EVENT_CONNECTED); event(directSession, CHIAKI_EVENT_QUIT, CHIAKI_QUIT_REASON_SESSION_REQUEST_UNKNOWN);
    const auto directId = id(); check(backend->retryConnection(directId), "Direct entry failure must retry its own connection context");
    auto *directRetry = backend->qmlSession(); check(directRetry && directRetry->GetHost() == "192.0.2.99", "Direct retry must not select a library row with another address");
    settings->RemoveRegisteredHost(registered.GetServerMAC());
    check(!backend->connectionAttempt().value("targetValid").toBool(), "Direct entry must revalidate profile registration");
    backend->cancelConnection(id()); event(directRetry, CHIAKI_EVENT_QUIT); backend->dismissConnection(id());
    evidence["directEntryRetainsAndRevalidatesContext"] = true;

    check(!window->isVisible() && !window->isExposed() && !backend->discoveryEnabled() && !backend->qmlSession(), "Native checks must remain hidden and isolated");
    evidence["noSleepDispatch"] = count("qaSleeps") == 0; evidence["starts"] = count("qaStarts"); evidence["stops"] = count("qaStops");
    evidence["allPassed"] = true; evidence["settingsRoot"] = runRoot; evidence["transport"] = "Controlled Start/Stop only; no console or network"; save(); QCoreApplication::quit();
}
}
void initializeConnectionQa() {
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext &, const QString &msg) { QFile log(qEnvironmentVariable("CHIAKI_NATIVE_QA_ROOT") + ".messages.txt"); if(log.open(QIODevice::WriteOnly | QIODevice::Append)) log.write(msg.toUtf8() + "\n"); });
    runRoot = qEnvironmentVariable("CHIAKI_NATIVE_QA_ROOT"); check(!runRoot.isEmpty() && !QDir(runRoot).exists(), "Use a fresh isolated directory"); QDir().mkpath(runRoot);
    QSettings::setDefaultFormat(QSettings::IniFormat); QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, runRoot); QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, runRoot + "/system");
    QSettings config(QSettings::IniFormat, QSettings::UserScope, "Chiaki", "Chiaki"); check(QFileInfo(config.fileName()).absoluteFilePath().startsWith(runRoot), "Seed must stay isolated");
    config.setValue("settings/remote_play_ask", false); config.setValue("settings/add_steam_shortcut_ask", false); config.setValue("settings/render_backend", "opengl"); config.setValue("settings/auto_discovery", false); config.setValue("settings/auto_connect_mac", ""); config.sync();
    { Settings settings(QString{}); seed(settings); }
    QTimer::singleShot(200, qApp, contracts);
}
