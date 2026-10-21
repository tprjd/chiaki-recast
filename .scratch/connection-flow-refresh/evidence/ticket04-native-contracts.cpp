#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
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
QJsonArray passedChecks;
QmlBackend *backend;
QmlMainWindow *window;
void save() { evidence["checks"] = passedChecks; evidence["checkCount"] = passedChecks.size(); QFile out(runRoot + "/contracts.json"); if (!out.open(QIODevice::WriteOnly)) qFatal("Cannot write connection evidence"); out.write(QJsonDocument(evidence).toJson()); }
void check(bool value, const char *message) { if (value) { passedChecks.append(QString::fromLatin1(message)); return; } evidence["failedCheck"] = message; save(); qFatal("Connection contract failed: %s", message); }
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
    settings.SetAutoConnectHost(QByteArray(reinterpret_cast<const char *>(host(1).GetServerMAC().GetMAC()), 6));
    settings.SetManualHost(ManualHost(-1, "192.0.2.42", true, host(1).GetServerMAC()));
    settings.SetManualHost(ManualHost(-1, "192.0.2.43", true, host(2).GetServerMAC()));
    settings.SetHardwareDecoder(""); settings.SetDecoder(Decoder::Ffmpeg); settings.SetWindowType(WindowType::SelectedResolution);
}
void contracts() {
    for (auto *candidate : QGuiApplication::topLevelWindows()) if (auto *main = qobject_cast<QmlMainWindow *>(candidate)) { window = main; backend = main->getBackend(); }
    check(backend && window, "Production backend must exist");
    check(!window->isVisible() && !window->isExposed() && !backend->discoveryEnabled() && !backend->qmlSession(), "Startup must stay hidden and isolated");
    backend->nativeQaPauseFallback();
    check(phase() == "waiting" && backend->connectionAttempt().value("context") == "startup", "Startup auto-connect must own the selected-console wait");
    check(backend->connectionAttempt().value("targetValid").toBool() && !backend->connectionAttempt().value("cleanupPending").toBool(), "Waiting target must be valid without a session");
    auto *settings = backend->nativeQaSettings();
    const auto savedAuto = settings->GetAutoConnectHost().GetServerMAC();
    backend->nativeQaFallback();
    check(phase() == "waiting" && !backend->connectionAttempt().value("fallback").toBool(), "No PSN credentials must keep the local wait without claiming fallback");
    const QString startupId = id();
    check(backend->cancelConnection(startupId) && !backend->cancelConnection(startupId), "Startup wait cancel must dispatch once");
    backend->nativeQaFallback();
    DiscoveryHost target{}; target.ps5 = true; target.target = CHIAKI_TARGET_PS5_1; target.host_addr = "192.0.2.42";
    target.host_name = "Synthetic target"; target.host_id = "020000000001"; target.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
    backend->nativeQaDiscover({target});
    check(phase() == "cancelled" && !backend->qmlSession() && count("qaStarts") == 0 && count("qaStops") == 0, "Cancelled startup must suppress late discovery and fallback");
    check(settings->GetAutoConnectHost().GetServerMAC() == savedAuto, "Wait cancellation must preserve saved auto-connect preference");
    evidence["startupWaitCancelSuppressesDiscoveryAndFallback"] = true;
    backend->dismissConnection(id());
    backend->nativeQaDiscover({});
    backend->qmlSettings()->setCurrentProfile("other"); backend->qmlSettings()->setCurrentProfile(""); backend->nativeQaPauseFallback(); settings = backend->nativeQaSettings();
    const QString wakingStartup = id(); target.state = CHIAKI_DISCOVERY_HOST_STATE_STANDBY;
    backend->nativeQaDiscover({target});
    check(phase() == "waking" && id() == wakingStartup && backend->connectionAttempt().value("context") == "startup", "Startup discovery of a sleeping target must retain attempt and context");
    auto *allocated = backend->qmlSession();
    check(allocated && count("qaStarts") == 0 && count("qaWakes") == 1, "Wake wait must allocate without starting transport");
    check(backend->cancelConnection(id()), "Allocated wake wait must accept cancel");
    check(phase() == "cancelled" && !backend->qmlSession() && count("qaStops") == 0 && !backend->connectionAttempt().value("cleanupPending").toBool(), "Unstarted wake session must finish cleanup without waiting for nonexistent quit");
    allocated->SessionQuit(CHIAKI_QUIT_REASON_STOPPED, {}); allocated->ConnectedChanged();
    target.state = CHIAKI_DISCOVERY_HOST_STATE_READY; backend->nativeQaDiscover({target}); backend->nativeQaWakeTimeout(); backend->nativeQaFallback();
    check(phase() == "cancelled" && !backend->qmlSession() && count("qaStarts") == 0, "Late allocated-session, timeout, and discovery events must stay cancelled");
    evidence["unstartedWakeCancelCleansUpImmediately"] = true;
    backend->dismissConnection(id()); target.state = CHIAKI_DISCOVERY_HOST_STATE_STANDBY; backend->nativeQaDiscover({target});
    backend->connectToHost(index("192.0.2.42"), target.host_name);
    check(phase() == "waking" && backend->connectionAttempt().value("context") == "wake", "Manual sleeping-console connection must use manual wake context");
    backend->nativeQaWakeTimeout();
    check(phase() == "failure" && !backend->qmlSession() && !backend->connectionAttempt().value("cleanupPending").toBool(), "Wake timeout must persist after allocated-session cleanup");
    const QString timeoutId = id();
    DiscoveryHost peer = target; peer.host_addr = "192.0.2.43"; peer.host_name = "Synthetic peer"; peer.host_id = "020000000002"; peer.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
    backend->nativeQaDiscover({peer, target});
    check(phase() == "failure" && backend->retryConnection(timeoutId), "Timeout retry must resolve the intended console after discovery reorder");
    check(id() != timeoutId && backend->qmlSession() && backend->qmlSession()->GetHost() == "192.0.2.42" && phase() == "waking", "Retry must create one new wake attempt for the current target");
    const auto retryId = id(); const int wakes = count("qaWakes");
    check(!backend->retryConnection(timeoutId) && count("qaWakes") == wakes, "Repeated timeout retry must not wake or connect twice");
    check(backend->cancelConnection(retryId), "Retry wake wait must cancel");
    evidence["manualWakeTimeoutStableRetry"] = true;
    backend->dismissConnection(id()); backend->connectToHost(index("192.0.2.42"), target.host_name);
    auto *handoff = backend->qmlSession(); const auto handoffId = id(); target.state = CHIAKI_DISCOVERY_HOST_STATE_READY;
    const int beforeStart = count("qaStarts"); backend->nativeQaDiscover({peer, target});
    check(backend->qmlSession() == handoff && id() == handoffId && phase() == "connecting" && count("qaStarts") == beforeStart + 1 && !backend->connectionAttempt().value("waitingForWake").toBool(), "Actual ready discovery must hand off once with the same session and attempt");
    event(handoff, CHIAKI_EVENT_CONNECTED); check(phase() == "video", "Console found must still wait for actual video readiness");
    check(backend->cancelConnection(handoffId) && phase() == "cancelling" && count("qaStops") == 1, "Started wake handoff must wait for native stop completion");
    backend->nativeQaWakeTimeout(); backend->nativeQaFallback(); backend->nativeQaDiscover({target});
    check(phase() == "cancelling" && count("qaStarts") == beforeStart + 1, "Competing late wake events must not restart during cancellation");
    event(handoff, CHIAKI_EVENT_QUIT); check(phase() == "cancelled", "Native quit must complete started-session cancellation");
    evidence["wakeFoundHandoffAndStartedCancel"] = true;
    backend->dismissConnection(id()); target.state = CHIAKI_DISCOVERY_HOST_STATE_STANDBY; backend->nativeQaDiscover({target}); backend->connectToHost(index("192.0.2.42"), target.host_name);
    const int beforeRace = count("qaStarts");
    auto cancelAtFound = QObject::connect(backend, &QmlBackend::connectionAttemptChanged, backend, [] { if (phase() == "found") backend->cancelConnection(id()); });
    target.state = CHIAKI_DISCOVERY_HOST_STATE_READY; backend->nativeQaDiscover({target}); QObject::disconnect(cancelAtFound);
    check(phase() == "cancelled" && !backend->qmlSession() && count("qaStarts") == beforeRace, "Cancel while discovery completes must prevent session start");
    evidence["cancelWinsDiscoveryRace"] = true;
    backend->dismissConnection(id()); target.state = CHIAKI_DISCOVERY_HOST_STATE_STANDBY; backend->nativeQaDiscover({target}); backend->connectToHost(index("192.0.2.42"), target.host_name); backend->nativeQaWakeTimeout();
    const auto invalidId = id(); backend->qmlSettings()->setCurrentProfile("other");
    check(!backend->connectionAttempt().value("targetValid").toBool() && !backend->retryConnection(invalidId), "Other profile must reject old wake retry");
    backend->qmlSettings()->setCurrentProfile(""); backend->nativeQaPauseFallback();
    backend->cancelConnection(id()); backend->dismissConnection(id());
    check(!backend->retryConnection(invalidId), "Returning to the old profile cannot restore ownership of old wake retry");
    evidence["profileGenerationInvalidatesWake"] = true;
    backend->nativeQaDiscover({}); backend->qmlSettings()->setCurrentProfile("other"); backend->qmlSettings()->setCurrentProfile(""); backend->nativeQaPauseFallback();
    const auto readyId = id(); const int beforeReady = count("qaStarts"); target.state = CHIAKI_DISCOVERY_HOST_STATE_READY; backend->nativeQaDiscover({target});
    check(id() == readyId && phase() == "connecting" && count("qaStarts") == beforeReady + 1, "Startup ready discovery must preserve its attempt through local handoff");
    auto *ready = backend->qmlSession(); backend->cancelConnection(id()); event(ready, CHIAKI_EVENT_QUIT); backend->dismissConnection(id());
    evidence["startupReadyHandoff"] = true;
    target.state = CHIAKI_DISCOVERY_HOST_STATE_STANDBY; backend->nativeQaDiscover({target});
    const int beforeEntry = count("qaWakes");
    auto cancelAtEntry = QObject::connect(backend, &QmlBackend::connectionAttemptChanged, backend, [] { if (phase() == "waking") backend->cancelConnection(id()); });
    backend->connectToHost(index("192.0.2.42"), target.host_name); QObject::disconnect(cancelAtEntry);
    check(phase() == "cancelled" && !backend->qmlSession() && count("qaWakes") == beforeEntry, "Cancel at wake entry must prevent allocation and wake dispatch");
    backend->dismissConnection(id()); evidence["cancelAtWakeEntry"] = true;
    backend->nativeQaDiscover({}); backend->connectToHost(index("192.0.2.42"));
    auto *lateWake = backend->qmlSession(); const auto lateWakeId = id(); const int lateStart = count("qaStarts");
    target.state = CHIAKI_DISCOVERY_HOST_STATE_STANDBY; backend->nativeQaDiscover({target});
    check(backend->qmlSession() == lateWake && id() == lateWakeId && phase() == "waking" && backend->connectionAttempt().value("waitingForWake").toBool(), "Standby discovery during an existing manual session must show the actual wake request");
    target.state = CHIAKI_DISCOVERY_HOST_STATE_READY; backend->nativeQaDiscover({target});
    check(backend->qmlSession() == lateWake && phase() == "connecting" && count("qaStarts") == lateStart && !backend->connectionAttempt().value("waitingForWake").toBool(), "Late wake handoff must not start the existing session a second time");
    backend->cancelConnection(id()); event(lateWake, CHIAKI_EVENT_QUIT); backend->dismissConnection(id()); evidence["lateStandbyDiscoveryPreservesStartedSession"] = true;
    backend->nativeQaDiscover({}); backend->qmlSettings()->setCurrentProfile("other"); backend->qmlSettings()->setCurrentProfile(""); backend->nativeQaPauseFallback(); settings = backend->nativeQaSettings();
    const auto removedId = id(); settings->RemoveRegisteredHost(host(1).GetServerMAC());
    check(phase() == "failure" && !backend->connectionAttempt().value("targetValid").toBool() && !backend->retryConnection(removedId), "Removed startup target must offer library return instead of an endless wait");
    settings->AddRegisteredHost(host(1)); check(backend->retryConnection(removedId), "Startup retry must validate the current registered target"); backend->nativeQaPauseFallback();
    backend->cancelConnection(id()); backend->dismissConnection(id()); evidence["removedStartupTargetAndValidatedRetry"] = true;
    backend->nativeQaDiscover({}); backend->qmlSettings()->setCurrentProfile("other"); backend->qmlSettings()->setCurrentProfile(""); backend->nativeQaPauseFallback();
    QObject *oldTimer = backend->nativeQaFallbackTimer(); backend->qmlSettings()->setCurrentProfile("other"); backend->qmlSettings()->setCurrentProfile(""); backend->nativeQaPauseFallback();
    settings = backend->nativeQaSettings(); settings->SetPsnAuthToken("synthetic-token"); settings->SetPsnRefreshToken("synthetic-refresh"); settings->SetPsnAuthTokenExpiry(QDateTime::currentDateTime().addSecs(3600).toString(settings->GetTimeFormat())); settings->SetPsnAccountId(QString::fromLatin1(QByteArray(8, 'a').toBase64()));
    backend->nativeQaPsnHost(QString(64, '1'), "Synthetic target");
    const auto fallbackId = id(); const int beforeFallback = count("qaPsnWorkers");
    QMetaObject::invokeMethod(oldTimer, "timeout", Qt::DirectConnection);
    check(phase() == "waiting" && id() == fallbackId && count("qaPsnWorkers") == beforeFallback, "Old profile fallback timer must not advance the current wait");
    qApp->setProperty("qaPsnResult", CHIAKI_ERR_HOST_DOWN); backend->nativeQaFallback(); settle();
    check(id() == fallbackId && backend->connectionAttempt().value("route") == "psn" && backend->connectionAttempt().value("context") == "startup" && backend->connectionAttempt().value("fallback").toBool() && !backend->connectionAttempt().value("waitingForWake").toBool(), "Actual PSN fallback must preserve attempt and context while handing off the wait screen");
    check(count("qaPsnWorkers") == beforeFallback + 1 && phase() == "failure" && !backend->qmlSession(), "Controlled fallback worker result must reach the real PSN failure handler and finish cleanup");
    backend->dismissConnection(id()); evidence["actualPsnFallbackAndOldTimerSuppression"] = true;
    settings = backend->nativeQaSettings();
    check(settings->GetAutoConnectHost().GetServerMAC() == savedAuto && count("qaSleeps") == 0, "Every cancel, timeout, and retry must keep preference and never sleep the console");
    check(!window->isVisible() && !window->isExposed() && !backend->discoveryEnabled() && !backend->qmlSession(), "Native checks must remain hidden and isolated");
    evidence["allPassed"] = true; evidence["starts"] = count("qaStarts"); evidence["stops"] = count("qaStops"); evidence["wakeRequests"] = count("qaWakes"); evidence["sleepRequests"] = count("qaSleeps");
    evidence["settingsRoot"] = runRoot; evidence["transport"] = "Controlled native Start/Stop/Wake, no console or network"; save(); QCoreApplication::quit();

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
