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
#include "psntoken.h"
#include <QPointer>
#include <QDateTime>

namespace {
QString runRoot;
QJsonObject evidence;
QmlBackend *backend;
QmlMainWindow *window;
void save() { QFile out(runRoot + "/contracts.json"); if (!out.open(QIODevice::WriteOnly)) qFatal("Cannot write connection evidence"); out.write(QJsonDocument(evidence).toJson()); }
int checks = 0;
void check(bool value, const char *message) { ++checks; if (value) return; evidence["failedCheck"] = message; save(); qFatal("Connection contract failed: %s", message); }
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

ChiakiRegisteredHost rawHost(int number) { ChiakiRegisteredHost value{}; value.target=CHIAKI_TARGET_PS5_1; value.server_mac[0]=2; value.server_mac[5]=number; qstrncpy(value.server_nickname,number==1?"Synthetic target":"Synthetic peer",sizeof(value.server_nickname)); memset(value.rp_regist_key,'a'+number,sizeof(value.rp_regist_key)); memset(value.rp_key,number,sizeof(value.rp_key)); return value; }
QString duid() { return QString(64, '1'); }
int psnIndex() { const auto all=backend->hosts(); for(int i=0;i<all.size();++i) if(all.at(i).toMap().value("duid")==duid() && !all.at(i).toMap().value("discovered").toBool()) return i; return -1; }
void credentials(Settings *settings, bool expired=false) {
    settings->SetPsnAuthToken("synthetic-token"); settings->SetPsnRefreshToken("synthetic-refresh");
    settings->SetPsnAccountId("AAAAAAAAAAA=");
    settings->SetPsnAuthTokenExpiry(QDateTime::currentDateTime().addSecs(expired ? -100 : 3600).toString(settings->GetTimeFormat()));
}
void hole(StreamSession *session, bool finished) { ChiakiEvent value{}; value.type=CHIAKI_EVENT_HOLEPUNCH; value.data_holepunch.finished=finished; session->nativeQaEvent(&value); }
void quit(StreamSession *session, bool failure=false) { event(session,CHIAKI_EVENT_QUIT,failure?CHIAKI_QUIT_REASON_SESSION_REQUEST_UNKNOWN:CHIAKI_QUIT_REASON_STOPPED); }
QString screenClass() { auto *nav=navigation(); auto *item=nav?qvariant_cast<QObject *>(nav->property("currentItem")):nullptr; return item?item->metaObject()->className():""; }
class RetainSession : public QObject { bool eventFilter(QObject *, QEvent *event) override { return event->type()==QEvent::DeferredDelete; } };
void contracts() {
    for(auto *candidate:QGuiApplication::topLevelWindows()) if(auto *main=qobject_cast<QmlMainWindow *>(candidate)) { window=main; backend=main->getBackend(); }
    check(backend && window,"Production backend exists"); backend->stopAutoConnect();
    check(!window->isVisible() && !window->isExposed() && !backend->discoveryEnabled(),"Native run stays hidden and isolated");
    auto *settings=backend->nativeQaSettings(); credentials(settings); backend->nativeQaPsnHost(duid(),"Synthetic target");
    const auto savedAuto=settings->GetAutoConnectHost().GetServerMAC();
    QVariantMap registrationContext;
    QObject::connect(backend,&QmlBackend::registDialogRequested,backend,[&](const QString &,bool,const QString &,const QVariantMap &context) { registrationContext=context; });
    int pins=0; QObject::connect(backend,&QmlBackend::sessionPinDialogRequested,backend,[&](bool){++pins;});
    qApp->setProperty("qaPsnResult",CHIAKI_ERR_SUCCESS);
    backend->connectToHost(psnIndex()); auto *first=backend->qmlSession();
    check(first && phase()=="initiating","Native PSN starts with initiating progress");
    RetainSession retain; first->installEventFilter(&retain);
    const auto firstId=id(); settle();
    check(phase()=="linking" && count("qaStarts")==1,"Worker completion starts native session once and shows linking");
    check(!backend->connectionAttempt().value("cancelable").toBool() && !backend->cancelConnection(id()),"Linking cannot accept cancellation");
    check(backend->connectionAttempt().value("completedStages").toStringList()==QStringList{"psn"},"Only reached PSN is completed while linking");
    check(screenClass().startsWith("PsnView_"),"Production Main routes linking to PSN view");
    hole(first,false); check(phase()=="data" && backend->connectionAttempt().value("cancelable").toBool(),"Real holepunch start allows cancellation");
    check(backend->cancelConnection(id()) && !backend->cancelConnection(id()) && count("qaPsnCancels")==1,"PSN cancel dispatches exactly once");
    hole(first,true); first->LoginPINRequested(true); first->AutoRegistSucceeded(rawHost(2));
    check(phase()=="cancelling" && pins==0 && backend->qmlSession()==first,"Cancelled attempt rejects completion, PIN, and registration");
    check(!backend->retryConnection(id()),"Retry cannot overlap cancellation cleanup");
    quit(first,true); check(phase()=="cancelled" && !backend->qmlSession() && !backend->connectionAttempt().value("cleanupPending").toBool(),"Native quit completes cancelled result");
    backend->dismissConnection(id());
    evidence["stageRestrictionsCancelAndLateEvents"]=true;

    qApp->setProperty("qaPsnResult",CHIAKI_ERR_HOST_DOWN); backend->connectToHost(psnIndex()); settle();
    check(phase()=="failure" && !backend->qmlSession(),"Worker start failure is persistent and cleaned up");
    check(backend->connectionAttempt().value("errorTitle")=="PSN connection did not start","Start failure has useful title");
    const auto failedId=id(); settle(); check(phase()=="failure","Failure has no automatic dismissal");
    settings->RemoveRegisteredHost(host(1).GetServerMAC());
    check(!backend->connectionAttempt().value("targetValid").toBool() && !backend->retryConnection(failedId),"Removed PSN registration cannot retry to a stale target");
    settings->AddRegisteredHost(host(1));
    qApp->setProperty("qaPsnResult",CHIAKI_ERR_SUCCESS);
    check(backend->retryConnection(failedId),"Remote retry accepts valid target after cleanup"); auto *second=backend->qmlSession();
    check(second && id()!=failedId && !backend->retryConnection(failedId),"Remote retry creates one fresh attempt");
    first->DataHolepunchProgress(true); first->SessionQuit(CHIAKI_QUIT_REASON_STOPPED,{}); first->LoginPINRequested(true); first->AutoRegistSucceeded(rawHost(2));
    check(backend->qmlSession()==second && phase()=="initiating" && pins==0,"Old session events cannot replace a fresh native attempt");
    first->removeEventFilter(&retain); first->deleteLater();
    settle(); hole(second,false); second->LoginPINRequested(true); check(pins==1,"PSN PIN request reaches current presentation");
    hole(second,true); settle(); check(phase()=="video" && !window->hasVideo() && screenClass().startsWith("StreamView_"),"PSN data completion hands off to actual video waiting");
    event(second,CHIAKI_EVENT_CONNECTED); check(phase()=="video","Connected session still waits for first frame");
    window->nativeQaVideo(true); check(phase()=="ready","First frame completes PSN progress automatically");
    quit(second); window->nativeQaVideo(false); settle(); check(libraryShown(),"Ready-session quit returns to library");
    backend->connectToHost(psnIndex()); auto *streamError=backend->qmlSession(); settle(); hole(streamError,false); hole(streamError,true); settle();
    event(streamError,CHIAKI_EVENT_CONNECTED); quit(streamError,true); settle();
    check(phase()=="failure" && backend->connectionAttempt().value("presented").toBool() && screenClass().startsWith("StreamView_"),"Failure after PSN handoff stays visible without dismissing its attempt");
    check(backend->retryConnection(id()),"Failure after handoff permits fresh owned retry"); auto *streamRetry=backend->qmlSession(); settle();
    check(phase()=="linking" && backend->connectionAttempt().value("presented").toBool() && screenClass().startsWith("StreamView_"),"Retry after handoff retains its progress host");
    hole(streamRetry,false); backend->cancelConnection(id()); quit(streamRetry); backend->dismissConnection(id()); settle();
    evidence["persistentStartErrorRetryPinAndVideoHandoff"]=true;

    qputenv("CHIAKI_QA_REJECT_START","1"); backend->connectToHost(psnIndex()); settle(); qunsetenv("CHIAKI_QA_REJECT_START");
    check(phase()=="failure" && !backend->qmlSession() && backend->connectionAttempt().value("errorTitle")=="Stream failed","Synchronous PSN session start failure remains readable and retryable"); backend->dismissConnection(id());
    for(auto errorCode:{CHIAKI_ERR_HOST_UNREACH,CHIAKI_ERR_UNKNOWN}) {
        qApp->setProperty("qaPsnResult",errorCode); backend->connectToHost(psnIndex()); settle();
        check(phase()=="failure" && !backend->qmlSession(),"Unreachable/general worker error is persistent and cleaned");
        check(!backend->connectionAttempt().value("errorText").toString().isEmpty(),"Worker error keeps diagnostic detail"); backend->dismissConnection(id());
    }
    qApp->setProperty("qaPsnResult",CHIAKI_ERR_SUCCESS);
    backend->connectToHost(psnIndex()); auto *pending=backend->qmlSession(); check(pending && phase()=="initiating","Pending worker owns allocated session");
    check(backend->cancelConnection(id()) && phase()=="cancelling","Initiating cancel waits for worker return"); settle();
    check(!backend->qmlSession() && phase()=="cancelled","Worker return performs cancelled cleanup without starting"); backend->dismissConnection(id());
    evidence["allWorkerErrorsAndPendingWorkerCleanup"]=true;

    credentials(settings,true); backend->connectToHost(psnIndex());
    auto *token=qobject_cast<PSNToken *>(qApp->property("qaToken").value<QObject *>()); QPointer<PSNToken> oldToken(token);
    check(token && !backend->qmlSession() && phase()=="internet","Expired token shows internet wait before session allocation");
    const int beforeRefresh=count("qaPsnWorkers");
    check(backend->cancelConnection(id()) && phase()=="cancelled" && !oldToken,"Cancel disposes owned token request");
    check(count("qaPsnWorkers")==beforeRefresh,"Cancelled refresh cannot launch a worker"); backend->dismissConnection(id());
    backend->connectToHost(psnIndex()); token=qobject_cast<PSNToken *>(qApp->property("qaToken").value<QObject *>());
    check(token && phase()=="internet","Next attempt owns a new token request");
    token->PSNTokenError("Synthetic network failure"); check(phase()=="failure" && !backend->retryConnection(id()),"Refresh error waits until request has finished");
    token->Finished(); check(!backend->connectionAttempt().value("cleanupPending").toBool(),"Finished token permits retry"); settle();
    credentials(settings); backend->dismissConnection(id());
    settings->SetAudioVideoDisabled(CHIAKI_AUDIO_VIDEO_DISABLED); backend->connectToHost(psnIndex()); auto *noVideo=backend->qmlSession(); settle();
    event(noVideo,CHIAKI_EVENT_CONNECTED); check(phase()=="ready" && !window->hasVideo(),"Video-disabled PSN session is ready on connection"); quit(noVideo); settle(); settings->SetAudioVideoDisabled(static_cast<ChiakiDisableAudioVideo>(0));
    backend->nativeQaResumePsn(); token=qobject_cast<PSNToken *>(qApp->property("qaToken").value<QObject *>());
    check(token && phase()=="internet","Resume uses owned internet-wait attempt");
    for (int attempt=0;attempt<12 && phase()!="failure";++attempt) token->PSNTokenError("Synthetic reconnect failure");
    check(phase()=="failure" && backend->connectionAttempt().value("cleanupPending").toBool(),"Resume failure waits for final token cleanup");
    token->Finished(); check(!backend->connectionAttempt().value("cleanupPending").toBool(),"Resume token cleanup enables recovery"); settle(); backend->dismissConnection(id());
    evidence["ownedRefreshCancellationFailureAndVideoDisabled"]=true;

    credentials(settings,true); backend->connectToHost(psnIndex()); token=qobject_cast<PSNToken *>(qApp->property("qaToken").value<QObject *>()); oldToken=token;
    backend->qmlSettings()->setCurrentProfile("other"); backend->stopAutoConnect();
    check(!oldToken && phase()=="failure" && !backend->retryConnection(id()),"Profile change invalidates internet wait and destroys refresh");
    backend->qmlSettings()->setCurrentProfile(""); backend->stopAutoConnect(); settings=backend->nativeQaSettings(); credentials(settings); backend->nativeQaPsnHost(duid(),"Synthetic target");
    check(!backend->connectionAttempt().value("targetValid").toBool(),"Away-and-back profile cannot restore old retry"); backend->dismissConnection(id());
    backend->connectToHost(psnIndex()); auto *profileWorker=backend->qmlSession(); backend->qmlSettings()->setCurrentProfile("other"); backend->stopAutoConnect(); settle();
    check(!backend->qmlSession() && phase()=="failure" && !backend->connectionAttempt().value("cleanupPending").toBool(),"Old profile worker completes cleanup without session start");
    backend->qmlSettings()->setCurrentProfile(""); backend->stopAutoConnect(); settings=backend->nativeQaSettings(); credentials(settings); backend->nativeQaPsnHost(duid(),"Synthetic target"); backend->dismissConnection(id());
    backend->connectToHost(psnIndex()); auto *linkingProfile=backend->qmlSession(); settle();
    const int previousStops=count("qaStops");
    backend->qmlSettings()->setCurrentProfile("other"); backend->stopAutoConnect();
    check(phase()=="failure" && count("qaStops")==previousStops+1,"Profile change stops an otherwise non-cancellable linking session");
    linkingProfile->AutoRegistSucceeded(rawHost(2)); hole(linkingProfile,true); quit(linkingProfile);
    check(phase()=="failure" && !backend->qmlSession(),"Old linking profile cannot register, hand off, or erase profile failure");
    backend->qmlSettings()->setCurrentProfile(""); backend->stopAutoConnect(); settings=backend->nativeQaSettings(); credentials(settings); backend->nativeQaPsnHost(duid(),"Synthetic target"); backend->dismissConnection(id());
    evidence["profileChangesScopeRefreshWorkerAndRetry"]=true;

    settings->RemoveRegisteredHost(host(1).GetServerMAC());
    DiscoveryHost discovered{}; discovered.ps5=true; discovered.target=CHIAKI_TARGET_PS5_1; discovered.host_addr="192.0.2.42"; discovered.host_name="Synthetic target"; discovered.host_id="020000000001"; discovered.state=CHIAKI_DISCOVERY_HOST_STATE_READY;
    backend->nativeQaDiscover({discovered}); backend->connectToHost(index("192.0.2.42"));
    qApp->setProperty("qaPsnResult",CHIAKI_ERR_HOST_DOWN); backend->autoRegister(registrationContext); settle();
    check(phase()=="failure" && backend->connectionAttempt().value("registrationOnly").toBool(),"Automatic registration preserves separate registration intent on failure");
    qApp->setProperty("qaPsnResult",CHIAKI_ERR_SUCCESS);
    const auto registrationFailedId=id(); backend->autoRegister(registrationContext);
    check(!backend->qmlSession() && id()==registrationFailedId,"Consumed registration confirmation cannot start another attempt");
    check(backend->retryConnection(id()),"Registration failure can retry without a connection intent");
    auto *registration=backend->qmlSession(); settle(); check(phase()=="registering" && !backend->cancelConnection(id()),"Registration retry remains registration-only and non-cancellable");
    const auto savedCount=settings->GetRegisteredHosts().size();
    registration->AutoRegistSucceeded(rawHost(1)); check(phase()=="registered" && settings->GetRegisteredHosts().size()==savedCount+1,"Native registration result saves target and keeps completed result");
    registration->AutoRegistSucceeded(rawHost(2)); check(settings->GetRegisteredHosts().size()==savedCount+1,"Duplicate registration completion cannot save a peer");
    hole(registration,true); check(phase()=="registered","Registration completion cannot open stream route");
    quit(registration); settle(); check(phase()=="registered" && !backend->connectionAttempt().value("cleanupPending").toBool() && screenClass().startsWith("PsnView_"),"Registration result persists after native cleanup");
    backend->dismissConnection(id()); backend->nativeQaDiscover({});
    evidence["registrationOnlyRetryCompletionAndDuplicateOwnership"]=true;
    check(count("qaSleeps")==0 && settings->GetAutoConnectHost().GetServerMAC()==savedAuto,"Connection recovery never sleeps console or changes saved auto-connect");
    check(!window->isVisible() && !window->isExposed() && !backend->discoveryEnabled() && !backend->qmlSession(),"Native check ends hidden, isolated, and clean");
    { Settings finalSettings(QString{}); check(finalSettings.GetRegisteredHosts().size()==2 && finalSettings.GetRegisteredHost(host(1).GetServerMAC()).GetConsolePin()==host(1).GetConsolePin() && finalSettings.GetRegisteredHost(host(2).GetServerMAC()).GetConsolePin()==host(2).GetConsolePin(),"Fresh native settings preserve saved PINs and intended registrations"); }
    evidence["checks"]=checks; evidence["starts"]=count("qaStarts"); evidence["psnWorkers"]=count("qaPsnWorkers"); evidence["psnCancels"]=count("qaPsnCancels"); evidence["allPassed"]=true; save(); QCoreApplication::quit();
}
}
void initializeConnectionQa() {
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext &, const QString &msg) { QFile log(qEnvironmentVariable("CHIAKI_NATIVE_QA_ROOT") + ".messages.txt"); if(log.open(QIODevice::WriteOnly | QIODevice::Append)) log.write(msg.toUtf8() + "\n"); });
    runRoot = qEnvironmentVariable("CHIAKI_NATIVE_QA_ROOT"); check(!runRoot.isEmpty() && !QDir(runRoot).exists(), "Use a fresh isolated directory"); QDir().mkpath(runRoot);
    QSettings::setDefaultFormat(QSettings::IniFormat); QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, runRoot); QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, runRoot + "/system");
    QSettings config(QSettings::IniFormat, QSettings::UserScope, "Chiaki", "Chiaki"); check(QFileInfo(config.fileName()).absoluteFilePath().startsWith(runRoot), "Seed must stay isolated");
    config.setValue("settings/remote_play_ask", false); config.setValue("settings/add_steam_shortcut_ask", false); config.setValue("settings/render_backend", "opengl"); config.setValue("settings/auto_discovery", false); config.setValue("settings/auto_connect_mac", ""); config.sync();
    { Settings settings(QString{}); seed(settings); }
    check(QFile::copy(config.fileName(),runRoot+"/baseline.ini"),"Isolated baseline settings are retained for post-exit check");
    QTimer::singleShot(200, qApp, contracts);
}
