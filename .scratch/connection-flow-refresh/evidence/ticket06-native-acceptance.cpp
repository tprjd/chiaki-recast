#include "psntoken.h"
#include "qmlbackend.h"
#include "qmlmainwindow.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPointer>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>
#include <QSettings>
#include <QTimer>
#include <QTranslator>
#include <cmath>
#include <functional>

namespace qalive {

namespace {
QString runRoot;
QJsonObject evidence;
QmlBackend *backend;
QmlMainWindow *window;
void save() { evidence["captureTrace"]=QJsonArray::fromStringList(qApp->property("qaCaptureTrace").toStringList()); QFile out(runRoot + "/contracts.json"); if (!out.open(QIODevice::WriteOnly)) qFatal("Cannot write connection evidence"); out.write(QJsonDocument(evidence).toJson()); }
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
    RegisteredHost value(native); value.SetConsolePin(value, QString()); return value;
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
void pin(StreamSession *session, bool incorrect) { ChiakiEvent value{}; value.type=CHIAKI_EVENT_LOGIN_PIN_REQUEST; value.login_pin_request.pin_incorrect=incorrect; session->nativeQaEvent(&value); }
QString requestId() { return backend->sessionPinRequest().value("id").toString(); }
QString pinPhase() { return backend->sessionPinRequest().value("phase").toString(); }
QQuickItem *control(const QString &name) { for(auto *candidate:QGuiApplication::allWindows()) if(auto *quick=qobject_cast<QQuickWindow *>(candidate)) if(auto *found=named(quick->contentItem(),name)) return found; return nullptr; }
QQuickWindow *pinWindow() { for(auto *candidate:QGuiApplication::allWindows()) if(candidate->objectName()=="liveConsolePinWindow") return qobject_cast<QQuickWindow *>(candidate); return nullptr; }
void assertHidden() { check(window->isVisible() && window->isExposed(),"Production native window is exposed"); }
RegisteredHost registered(Settings &settings, int number) { for (const auto &value : settings.GetRegisteredHosts()) if (value.GetServerMAC() == host(number).GetServerMAC()) return value; qFatal("Synthetic registered host missing"); }
void savedPinsUnchanged() {
    Settings fresh(QString{});
    check(registered(fresh,1).GetConsolePin()=="0789", "The target saved PIN is unchanged");
    check(registered(fresh,2).GetConsolePin().isEmpty(), "The peer saved PIN is unchanged");
}
void returnToLibrary() {
    if(!backend->sessionPinRequest().isEmpty()) check(backend->dismissSessionPin(requestId()),"Cancelled prompt can return to consoles");
    else backend->dismissConnection(id());
    if(auto *item=control("liveConsolePinHost")) QMetaObject::invokeMethod(item->parentItem(),"showMainView");
    settle();
}
void contracts() {
    for(auto *candidate:QGuiApplication::topLevelWindows()) if(auto *main=qobject_cast<QmlMainWindow *>(candidate)) { window=main; backend=main->getBackend(); }
    check(backend && window,"Production backend exists"); backend->stopAutoConnect(); window->showNormal(); window->requestActivate(); settle(); assertHidden();
    check(!backend->discoveryEnabled() && !backend->qmlSession(),"Startup is isolated with no discovery or session");
    check(!backend->enterPin({},"0123") && !backend->cancelSessionPin({}) && !backend->dismissSessionPin({}),"Empty request cannot submit, cancel, or dismiss");
    auto *settings=backend->nativeQaSettings();
    auto target=registered(*settings,1); target.SetConsolePin(target,"0789"); settings->AddRegisteredHost(target);
    settings->SetAutoConnectHost(QByteArray(reinterpret_cast<const char *>(host(1).GetServerMAC().GetMAC()),6));
    const auto savedAuto=settings->GetAutoConnectHost().GetServerMAC();
    int notifications=0; bool lastIncorrect=false;
    QObject::connect(backend,&QmlBackend::sessionPinDialogRequested,backend,[&](bool incorrect){++notifications;lastIncorrect=incorrect;});
    backend->connectToHost(index("192.0.2.42")); auto *first=backend->qmlSession(); settle();
    check(first,"Local session exists"); const int localCaptures=window->nativeQaGrabCount(); evidence["hiddenLocalProgressCaptures"]=localCaptures;
    pin(first,false); check(count("qaPins")==1 && backend->sessionPinRequest().isEmpty(),"Saved PIN is submitted automatically without opening prompt");
    pin(first,false); check(count("qaPins")==1 && backend->sessionPinRequest().isEmpty(),"Repeated initial request cannot resubmit saved PIN");
    pin(first,true); const auto firstRequest=requestId(); settle();
    check(!firstRequest.isEmpty() && lastIncorrect && notifications==1 && pinPhase()=="form", "Actual incorrect event reaches one correction prompt");
    check(control("pinHeading") && control("pinHeading")->property("text")=="Incorrect console login PIN", "Production PIN content displays incorrect feedback");
    check(window->nativeQaGrabCount()==localCaptures+1,"Prompt adds one input capture to local progress");
    pin(first,true); check(requestId()==firstRequest && notifications==1 && window->nativeQaGrabCount()==localCaptures+1,"Repeated incorrect request preserves the current prompt and capture");
    for(const QString &invalid:QStringList{QString(),"123","abcd",QString(4,QChar(0x0661)),"1234\n"}) check(!backend->enterPin(firstRequest,invalid),"Native validation rejects incomplete and non-ASCII PIN input");
    check(backend->enterPin(firstRequest,"0123") && count("qaPins")==2,"Valid text submits exactly once");
    check(qApp->property("qaPinValue").toString()=="0123" && qvariant_cast<QObject *>(qApp->property("qaPinSession"))==first,"Leading zero text reaches captured native session");
    check(!backend->enterPin(firstRequest,"0456") && count("qaPins")==2 && pinPhase()=="submitting", "Repeated submission waits for real response");
    pin(first,false); check(requestId()==firstRequest && pinPhase()=="submitting","Repeated initial request does not end submission wait");
    pin(first,true); const auto corrected=requestId(); check(corrected!=firstRequest && notifications==2 && pinPhase()=="form","Native rejection creates fresh request ownership");
    check(!backend->enterPin(firstRequest,"0123") && !backend->cancelSessionPin(firstRequest),"Rejected prompt actions cannot affect new request");
    check(backend->enterPin(corrected,"0456"),"Corrected PIN can submit");
    event(first,CHIAKI_EVENT_CONNECTED); settle();
    check(backend->sessionPinRequest().isEmpty() && phase()=="video" && window->nativeQaGrabCount()==localCaptures,"Native connection accepts PIN and releases its input capture");
    pin(first,true); check(backend->sessionPinRequest().isEmpty() && notifications==2,"Repeated PIN after acceptance does not reopen prompt");
    window->nativeQaVideo(true); settle(); check(phase()=="ready" && window->nativeQaGrabCount()==0,"Ready stream has no remaining input capture");
    savedPinsUnchanged(); quit(first); window->nativeQaVideo(false); settle();
    evidence["savedPinRejectionCorrectionAndAcceptedHandoff"]=true;

    for(int action=0; action<3; ++action) {
        backend->connectToHost(index("192.0.2.43")); auto *session=backend->qmlSession(); pin(session,false); settle(); const auto request=requestId();
        auto *field=control("consoleLoginPin"); check(field,"Production live field exists"); field->setProperty("text","0123"); const int stops=count("qaStops");
        if(action==0) { auto *button=control("pinCancel"); check(button,"Production Cancel exists"); QMetaObject::invokeMethod(button,"clicked"); }
        if(action==1) {
            if(qEnvironmentVariableIsSet("CHIAKI_QA_INLINE_PIN")) {
                auto *quick=qobject_cast<QQuickWindow *>(field->window()); QKeyEvent press(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier); QCoreApplication::sendEvent(quick,&press);
            } else { QKeyEvent press(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier); QCoreApplication::sendEvent(pinWindow(),&press); }
        }
        if(action==2) { if(qEnvironmentVariableIsSet("CHIAKI_QA_INLINE_PIN")) check(!backend->closeRequested(),"Host close preserves pending cancellation"); else { QCloseEvent close; QCoreApplication::sendEvent(pinWindow(),&close); check(!close.isAccepted(),"PIN window close is handled as cancellation"); } }
        check(pinPhase()=="cancelling" && phase()=="cancelling" && count("qaStops")==stops+1,"Every cancellation input stops once and waits for cleanup");
        check(!backend->cancelSessionPin(request) && !backend->enterPin(request,"0123") && count("qaStops")==stops+1,"Repeated cancel and submit cannot dispatch during cleanup");
        event(session,CHIAKI_EVENT_CONNECTED); pin(session,true); check(pinPhase()=="cancelling", "Late connection or PIN cannot finish cancellation");
        quit(session); settle(); check(pinPhase()=="cancelled" && !backend->qmlSession(),"Native quit completes the cancelled prompt");
        check(control("pinHeading")->property("text")=="Connection cancelled", "Cancelled result uses production PIN content");
        returnToLibrary(); check(window->nativeQaGrabCount()==0,"Cancellation releases prompt and progress input");
        savedPinsUnchanged();
    }
    evidence["cancelButtonEscapeAndWindowClose"]=true;

    backend->connectToHost(index("192.0.2.43")); auto *old=backend->qmlSession(); pin(old,false); const auto oldRequest=requestId();
    bool reentered=false; auto reentrant=QObject::connect(backend,&QmlBackend::sessionPinRequestChanged,backend,[&] {
        if(reentered || pinPhase()!="submitting") return; reentered=true;
        quit(old); backend->connectToHost(index("192.0.2.43"));
    });
    const int beforePins=count("qaPins"); check(!backend->enterPin(oldRequest,"0123"),"Submit reentry that replaces session is rejected"); QObject::disconnect(reentrant);
    auto *replacement=backend->qmlSession(); check(reentered && replacement && replacement!=old && count("qaPins")==beforePins,"No PIN reaches replacement session after synchronous notification");
    old->LoginPINRequested(true); check(backend->sessionPinRequest().isEmpty(),"Old session request cannot reopen replacement prompt");
    pin(replacement,false); const auto newRequest=requestId(); check(!backend->enterPin(oldRequest,"0123") && !backend->cancelSessionPin(oldRequest),"Captured old request cannot submit or stop replacement");
    backend->cancelSessionPin(newRequest); quit(replacement); returnToLibrary();
    evidence["staleOwnershipAndSubmitReentry"]=true;
    backend->connectToHost(index("192.0.2.43")); auto *cancelOld=backend->qmlSession(); pin(cancelOld,false); const auto cancelOldRequest=requestId();
    bool cancelReentered=false; auto cancelListener=QObject::connect(backend,&QmlBackend::connectionAttemptChanged,backend,[&] {
        if(cancelReentered || phase()!="cancelling") return; cancelReentered=true; quit(cancelOld); backend->connectToHost(index("192.0.2.43"));
    });
    const int stopsBeforeReentry=count("qaStops"); check(backend->cancelSessionPin(cancelOldRequest),"Original cancellation is accepted before session replacement"); QObject::disconnect(cancelListener);
    check(cancelReentered && backend->qmlSession()!=cancelOld && count("qaStops")==stopsBeforeReentry && phase()=="connecting","Cancellation notification cannot stop or change replacement session");
    auto *cancelReplacement=backend->qmlSession(); backend->cancelConnection(id()); quit(cancelReplacement); returnToLibrary();
    evidence["cancelReentryScopesNativeStop"]=true;


    backend->connectToHost(index("192.0.2.43")); auto *profileSession=backend->qmlSession(); pin(profileSession,false); const auto profileRequest=requestId();
    backend->qmlSettings()->setCurrentProfile("other"); backend->stopAutoConnect();
    check(backend->sessionPinRequest().isEmpty() && !backend->enterPin(profileRequest,"0123") && !backend->cancelSessionPin(profileRequest),"Profile switch invalidates live request actions");
    quit(profileSession); backend->qmlSettings()->setCurrentProfile(""); backend->stopAutoConnect(); returnToLibrary(); settings=backend->nativeQaSettings();
    evidence["profileInvalidation"]=true;

    credentials(settings); backend->nativeQaPsnHost(duid(),"Synthetic target"); qApp->setProperty("qaPsnResult",CHIAKI_ERR_SUCCESS);
    backend->connectToHost(psnIndex()); auto *remote=backend->qmlSession(); settle(); check(remote && screenClass().startsWith("PsnView_"),"Production PSN route exists before handoff");
    pin(remote,true); settle(); const auto remoteRequest=requestId(); auto *remoteField=control("consoleLoginPin"); remoteField->setProperty("text","0123");
    check(window->nativeQaGrabCount()==2,"PSN progress and prompt each own one capture");
    hole(remote,true); settle();
    check(screenClass().startsWith("StreamView_") && requestId()==remoteRequest && pinPhase()=="form", "PSN data completion preserves pending prompt through real route handoff");
    check(control("consoleLoginPin")==remoteField && remoteField->property("text")=="0123", "Handoff keeps the same live form and transient text");
    auto *stream=qvariant_cast<QObject *>(navigation()->property("currentItem")); const int streamCapture=stream->property("connectionInputGrabbed").toBool()?1:0; evidence["hiddenStreamProgressCaptures"]=streamCapture; evidence["hiddenHandoffCaptureCount"]=window->nativeQaGrabCount(); check(window->nativeQaGrabCount()==streamCapture+1,"Handoff preserves exactly the PIN capture and any active stream progress capture");
    check(backend->enterPin(remoteRequest,"0123"),"PSN handoff prompt submits to current session"); event(remote,CHIAKI_EVENT_CONNECTED); window->nativeQaVideo(true); settle();
    check(backend->sessionPinRequest().isEmpty() && phase()=="ready" && window->nativeQaGrabCount()==0,"Accepted PSN handoff releases every input capture");
    quit(remote); window->nativeQaVideo(false); settle();
    backend->connectToHost(psnIndex()); auto *cancelRemote=backend->qmlSession(); settle(); pin(cancelRemote,true); hole(cancelRemote,true); settle();
    const int remoteStops=count("qaStops"); const int remoteCancels=count("qaPsnCancels");
    check(backend->cancelSessionPin(requestId()) && count("qaStops")==remoteStops+1 && count("qaPsnCancels")==remoteCancels,"Pending post-handoff PIN uses native stop despite ordinary PSN cancellation restriction");
    quit(cancelRemote); returnToLibrary(); check(window->nativeQaGrabCount()==0,"Cancelled PSN prompt leaves no capture");
    savedPinsUnchanged(); check(settings->GetAutoConnectHost().GetServerMAC()==savedAuto && count("qaSleeps")==0,"Live PIN never changes auto-connect preference or sleeps console");
    backend->connectToHost(psnIndex()); auto *fast=backend->qmlSession(); settle(); pin(fast,true);
    const auto fastRequest=requestId(); check(backend->enterPin(fastRequest,"0123"),"Fast handoff owns PIN submission");
    hole(fast,true); event(fast,CHIAKI_EVENT_CONNECTED); settle();
    check(phase()=="video" && window->nativeQaGrabCount()==1,"Fast connected handoff retains only video waiting capture after old PSN view exits");
    window->nativeQaVideo(true); settle();
    check(phase()=="ready" && backend->sessionPinRequest().isEmpty() && window->nativeQaGrabCount()==0,"Immediate data and connected events balance exiting PSN view capture");
    quit(fast); window->nativeQaVideo(false); settle();
    assertHidden(); check(!backend->discoveryEnabled(),"Discovery remains disabled after every cleanup");
    evidence["psnPromptContinuityAndCaptureBalance"]=true;
    evidence["checks"]=checks; evidence["allPassed"]=true; evidence["mode"]=qEnvironmentVariableIsSet("CHIAKI_QA_INLINE_PIN")?"inline":"separate-visible"; evidence["transport"]="Controlled native events and transport calls, no hardware"; save(); QCoreApplication::quit();
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

}

namespace qapsn {

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
    check(!backend->discoveryEnabled(),"Native run stays isolated");
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
    check(!backend->discoveryEnabled() && !backend->qmlSession(),"Native check ends isolated and clean");
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

}

namespace qawake {

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
    check(!backend->discoveryEnabled() && !backend->qmlSession(), "Startup stays isolated");
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
    check(!backend->discoveryEnabled() && !backend->qmlSession(), "Native checks remain isolated");
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

}
namespace visible {
using namespace qalive;
QJsonArray captures;
QImage lastInlineImage;
QSize requestedSize(900,600);
class ExpandedTranslator : public QTranslator {
public:
 bool isEmpty() const override { return false; }
 QString translate(const char *context,const char *source,const char *,int) const override {
  const QString scope=QString::fromUtf8(context), text=QString::fromUtf8(source);
  if (!(scope.contains("Connection") || scope.contains("ConsolePin") || scope.contains("PsnView") || scope.contains("AutoConnect") || scope.contains("StreamView")) || text.size()<8) return {};
  QString expanded="[QA] "+text; while(expanded.size()<text.size()*1.5) expanded+=" extra"; return expanded;
 }
} translator;
void waitUi(int ms=420) { QEventLoop loop; QTimer::singleShot(ms,&loop,&QEventLoop::quit); loop.exec(); }
QQuickItem *in(QQuickItem *item,const QString &name) {
 if(!item || !item->isVisible() || item->opacity()<.01) return nullptr;
 if(item->objectName()==name) return item;
 for(auto *child:item->childItems()) if(auto *found=in(child,name)) return found;
 return nullptr;
}
QQuickItem *mainRoot() { for(auto *candidate:QGuiApplication::allWindows()) if(auto *quick=qobject_cast<QQuickWindow *>(candidate)) if(in(quick->contentItem(),"mainNavigation")) return quick->contentItem(); return nullptr; }
QQuickItem *ui(const QString &name) {
 if(auto *pin=pinWindow(); pin && pin->isVisible()) if(auto *found=in(pin->contentItem(),name)) return found;
 return in(mainRoot(),name);
}
QWindow *owner(QQuickItem *item) { return pinWindow() && item->window()==pinWindow()?static_cast<QWindow *>(pinWindow()):static_cast<QWindow *>(window); }
QQuickItem *active(QWindow *target) { if(auto *quick=qobject_cast<QQuickWindow *>(target)) return quick->activeFocusItem(); return qobject_cast<QQuickItem *>(target->focusObject()); }
void bounds(QQuickItem *item) {
 check(item && item->isVisible() && item->width()>0 && item->height()>0,"Focused action exists and has a visible size");
 const QRectF rect(item->mapToScene({0,0}),item->size()); auto *target=owner(item);
 check(QRectF(0,0,target->width(),target->height()).adjusted(-1,-1,1,1).contains(rect),"Focused action remains inside its native owner window");
 for(auto *parent=item->parentItem();parent;parent=parent->parentItem()) if(parent->clip()) check(QRectF(parent->mapToScene({0,0}),parent->size()).adjusted(-1,-1,1,1).contains(rect),"Focused action remains inside each clipped viewport");
}
QQuickItem *focusUi(const QString &name) { auto *item=ui(name); check(item && item->isEnabled(),"Required native control exists and is enabled"); owner(item)->requestActivate(); item->forceActiveFocus(Qt::TabFocusReason); waitUi(90); bounds(item); return item; }
void keyTo(QWindow *target,int code,Qt::KeyboardModifiers modifiers=Qt::NoModifier,const QString &text={}) {
 QKeyEvent press(QEvent::KeyPress,code,modifiers,text),release(QEvent::KeyRelease,code,modifiers,text); QCoreApplication::sendEvent(target,&press); QCoreApplication::sendEvent(target,&release); waitUi(45);
}
void clickUi(const QString &name) {
 auto *item=focusUi(name); auto *target=owner(item); auto local=item->mapToScene({item->width()/2,item->height()/2}); auto global=target->mapToGlobal(local.toPoint());
 QMouseEvent press(QEvent::MouseButtonPress,local,global,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier),release(QEvent::MouseButtonRelease,local,global,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
 QCoreApplication::sendEvent(target,&press); QCoreApplication::sendEvent(target,&release); waitUi();
}
void digits() {
 auto *field=focusUi("consoleLoginPin"); auto *target=owner(field);
 keyTo(target,Qt::Key_A,Qt::ControlModifier); keyTo(target,Qt::Key_Backspace);
 for(QChar c:QString("0123")) keyTo(target,c.unicode(),Qt::NoModifier,QString(c));
 check(field->property("text").toString()=="0123","Native key events enter four digits and preserve a leading zero");
 check(field->property("echoMode").toInt()!=0,"Entered live or saved PIN is masked");
}
void labels(QQuickItem *item) {
 if(!item || !item->isVisible() || item->opacity()<.01)return;
 if(item->property("text").toString().startsWith("[QA]")) {
  check(!item->property("truncated").toBool(),"Expanded translated label is not truncated");
  if(QByteArray(item->metaObject()->className())!="QQuickMnemonicLabel" && item->property("contentHeight").isValid() && item->property("contentHeight").toDouble()>item->height()+2) { evidence["labelFailure"]=QJsonObject{{"name",item->objectName()},{"class",item->metaObject()->className()},{"text",item->property("text").toString()},{"height",item->height()},{"contentHeight",item->property("contentHeight").toDouble()}}; check(false,"Expanded translated label has sufficient height"); }
 }
 for(auto *child:item->childItems()) labels(child);
}
QJsonObject geometry(QWindow *target) {
 return {{"x",target->x()},{"y",target->y()},{"width",target->width()},{"height",target->height()},{"dpr",target->devicePixelRatio()},{"screenDpr",target->screen()->devicePixelRatio()},{"screen",target->screen()->name()},{"visible",target->isVisible()},{"exposed",target->isExposed()},{"active",target->isActive()}};
}
void fit() { window->showNormal(); window->resize(requestedSize); window->setPosition(30,40); waitUi(100); }
void capture(const QString &name,bool separate=false,const QString &focused={}) {
 fit();
 if(!focused.isEmpty())focusUi(focused); waitUi();
 auto *target=separate?static_cast<QWindow *>(pinWindow()):static_cast<QWindow *>(window);
 check(target && target->isVisible() && target->isExposed(),"Capture uses a visible and exposed native window");
 if(auto *field=ui("consoleLoginPin")) check(field->property("text").toString().isEmpty() || field->property("echoMode").toInt()!=0,"No capture can expose a PIN");
 const auto image=separate?pinWindow()->grabWindow():window->runtimeRendererBackend()==1?window->nativeQaImage():window->screen()->grabWindow(0,window->mapToGlobal(QPoint(0,0)).x(),window->mapToGlobal(QPoint(0,0)).y(),window->width(),window->height()).toImage(); const double dpr=target->devicePixelRatio();
 check(qAbs(dpr-qEnvironmentVariable("CHIAKI_NATIVE_QA_DPR","1").toDouble())<.02,"Measured DPR matches requested DPR");
 check(!image.isNull() && qAbs(image.width()-target->width()*dpr)<1.1 && qAbs(image.height()-target->height()*dpr)<1.1,"Native image has the measured physical dimensions");
 bool varied=false;for(int y=0;y<image.height();y+=qMax(1,image.height()/13))for(int x=0;x<image.width();x+=qMax(1,image.width()/13))varied|=image.pixelColor(x,y)!=image.pixelColor(0,0);
 check(varied,"Native image contains rendered content");if(window->runtimeRendererBackend()==0){check(lastInlineImage.isNull() || image!=lastInlineImage,"Vulkan visible pixels update for each driven PIN state");lastInlineImage=image;}
 image.save(runRoot+"/current.png"); labels(separate?pinWindow()->contentItem():mainRoot());
 check(image.save(runRoot+"/"+name+".png"),"Native image saves");
 QJsonObject entry=geometry(target);entry["image"]=name+".png";entry["physicalWidth"]=image.width();entry["physicalHeight"]=image.height();entry["captureMethod"]=separate?"QQuickWindow.grabWindow":window->runtimeRendererBackend()==1?"OpenGL QQuickRenderControl framebuffer":"QScreen composed desktop region of active Vulkan QA client";entry["attemptId"]=id();entry["phase"]=phase();entry["requestId"]=requestId();entry["pinPhase"]=pinPhase();entry["inputCount"]=window->nativeQaGrabCount();entry["settingsRoot"]=runRoot;entry["discoveryEnabled"]=backend->discoveryEnabled();entry["transientParentIsHost"]=target->transientParent()==window;
 if(auto *item=active(target); item && item->isVisible() && item->width()>0 && item->height()>0){bounds(item);QRectF rect(item->mapToScene({0,0}),item->size());entry["focus"]=QJsonObject{{"name",item->objectName()},{"x",rect.x()},{"y",rect.y()},{"width",rect.width()},{"height",rect.height()}};}
 if(auto *heading=ui("pinHeading")){const QRectF rect(heading->mapToScene({0,0}),heading->size());entry["pinHeadingBounds"]=QJsonObject{{"x",rect.x()},{"y",rect.y()},{"width",rect.width()},{"height",rect.height()},{"truncated",heading->property("truncated").toBool()}};}
 captures.append(entry);evidence["captures"]=captures;save();
}
void verifyPinGeometry() {
 auto *pin=pinWindow();check(pin && pin->isVisible() && pin->isExposed(),"Production OpenGL separate PIN is visible");
 check(pin->transientParent()==window && pin->modality()==Qt::ApplicationModal,"Separate PIN is application modal and belongs to its host");
 check(pin->width()==qMin(720,window->width()) && pin->height()==qMin(560,window->height()),"Separate PIN uses production compact dimensions");
 check(qAbs(pin->x()-qRound(window->x()+(window->width()-pin->width())/2.0))<=1 && qAbs(pin->y()-qRound(window->y()+(window->height()-pin->height())/2.0))<=1,"Separate PIN remains centered on host client geometry");
 check(qAbs(pin->devicePixelRatio()-window->devicePixelRatio())<.02,"Host and separate PIN use the same measured DPR");
 evidence["hostGeometry"]=geometry(window);evidence["pinGeometry"]=geometry(pin);
}
void backLibrary() { returnToLibrary();waitUi();check(libraryShown(),"Completed native route returns to production library");window->requestActivate(); }
void finish(StreamSession *session) { backend->cancelConnection(id());quit(session);waitUi();backLibrary();window->nativeQaVideo(false); }
void openSaved() { fit(); clickUi("pinShortcutButton");waitUi();check(ui("consoleLoginPin"),"Production library command opens saved PIN form"); }
void savedFlow(const QString &prefix,bool extra) {
 openSaved();if(extra)capture(prefix+"-saved-form",false,"consoleLoginPin");
 check(!ui("pinSubmit")->isEnabled(),"Saved form rejects empty input");digits();
 const int starts=count("qaStarts");clickUi("pinSubmit");check(ui("pinDone") && !backend->qmlSession() && count("qaStarts")==starts,"Production save confirms result without connecting");
 {Settings saved(QString{});check(registered(saved,1).GetConsolePin()=="0123" && registered(saved,2).GetConsolePin().isEmpty(),"Only intended synthetic console receives saved PIN");}
 capture(prefix+"-saved-result",false,"pinDone");clickUi("pinDone");check(libraryShown(),"Saved result Done restores library");
 check(active(window) && active(window)->isEnabled() && active(window)->isVisible(),"Save result restores a valid library focus");
}
void liveFlow(const QString &prefix,bool extra) {
 backend->connectToHost(index("192.0.2.43"));auto *session=backend->qmlSession();waitUi();fit();pin(session,false);waitUi();
 check(ui("consoleLoginPin") && ui("consoleLoginPin")->hasActiveFocus(),"Visible native live PIN starts in the field");verifyPinGeometry();
 capture(prefix+"-live-form",true,"consoleLoginPin");
 if(extra)capture(prefix+"-live-host");
 const auto request=requestId();const int captured=window->nativeQaGrabCount();auto *field=ui("consoleLoginPin");digits();
 keyTo(pinWindow(),Qt::Key_Tab);check(active(pinWindow()) && active(pinWindow())!=field,"Native Tab moves focus from field");bounds(active(pinWindow()));
 keyTo(pinWindow(),Qt::Key_Tab,Qt::ShiftModifier);check(field->hasActiveFocus(),"Native Shift Tab returns to PIN field");
 clickUi("pinReveal");check(field->property("echoMode").toInt()==0,"Native mouse Show reveals only by explicit action");
 backend->qmlSettings()->setStreamerMode(true);waitUi();check(field->property("echoMode").toInt()!=0 && !ui("pinReveal")->isEnabled(),"Streamer mode remasks revealed input and disables Show");
 check(!ui("pinConsoleName")->property("text").toString().contains("Synthetic"),"Streamer mode hides live console identity");
 if(extra)capture(prefix+"-live-private",true,"consoleLoginPin");
 backend->qmlSettings()->setStreamerMode(false);waitUi();
 const QPoint original=window->position();const QSize originalSize=window->size();window->setPosition(original+QPoint(31,23));window->resize(originalSize+QSize(18,14));waitUi();verifyPinGeometry();
 check(requestId()==request && ui("consoleLoginPin")==field && field->property("text").toString()=="0123" && window->nativeQaGrabCount()==captured,"Host move and resize preserve request field text and input ownership");
 if(QGuiApplication::screens().size()>1) {auto *second=QGuiApplication::screens().at(1);window->setPosition(second->geometry().topLeft()+QPoint(40,45));waitUi(600);verifyPinGeometry();check(window->screen()==second && pinWindow()->screen()==second,"Pending PIN follows host onto the second native screen");check(requestId()==request && field==ui("consoleLoginPin") && field->property("text").toString()=="0123","Native screen move preserves the pending PIN field");evidence["secondScreenHostGeometry"]=geometry(window);evidence["secondScreenPinGeometry"]=geometry(pinWindow());}
 window->resize(originalSize);window->setPosition(original);waitUi();verifyPinGeometry();
 auto *root=ui("liveConsolePinHost")->parentItem();const auto before=screenClass();const int grabs=window->nativeQaGrabCount();
 for(const char *method:{"showSettingsDialog","openDisplaySettings","openPlaceboSettings"}) QMetaObject::invokeMethod(root,method);
 QMetaObject::invokeMethod(root,"showPlaceboColorMappingDialog",Q_ARG(QVariant,QVariant("color")));
 QPointF background(40,150);QPoint global=window->mapToGlobal(background.toPoint());QMouseEvent bgPress(QEvent::MouseButtonPress,background,global,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier),bgRelease(QEvent::MouseButtonRelease,background,global,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);QCoreApplication::sendEvent(window,&bgPress);QCoreApplication::sendEvent(window,&bgRelease);
 keyTo(window,Qt::Key_Menu);keyTo(window,Qt::Key_O,Qt::ControlModifier);waitUi();
 check(screenClass()==before && window->nativeQaGrabCount()==grabs && !navigation()->property("enabled").toBool(),"Pending PIN blocks background settings and menu commands");
 if(qEnvironmentVariableIsSet("CHIAKI_QA_MANUAL")) {
  field->setProperty("text","");focusUi("consoleLoginPin");evidence["manualReady"]=true;save();
  QEventLoop manual; QTimer poll; QObject::connect(&poll,&QTimer::timeout,&manual,[&]{if(QFile::exists(runRoot+"/continue"))manual.quit();});poll.start(200);QTimer::singleShot(180000,&manual,&QEventLoop::quit);manual.exec();
  if(pinPhase()=="cancelling"){quit(session);waitUi();check(pinPhase()=="cancelled","Manual OS Escape completes only after native quit");capture(prefix+"-manual-cancelled",true,"pinBack");evidence["manualStage"]="cancelled";save();QEventLoop finishManual;QTimer finishPoll;QObject::connect(&finishPoll,&QTimer::timeout,&finishManual,[&]{if(QFile::exists(runRoot+"/finish"))finishManual.quit();});finishPoll.start(200);QTimer::singleShot(180000,&finishManual,&QEventLoop::quit);finishManual.exec();check(libraryShown() && window->nativeQaGrabCount()==0,"Manual native Back action restores library with input released");evidence["manualCompleted"]=true;evidence["allPassed"]=true;save();QCoreApplication::quit();return;}check(pinPhase()=="form","Manual input check leaves live request available");field->setProperty("text","");digits();
 }
 focusUi("consoleLoginPin");keyTo(pinWindow(),Qt::Key_Return);check(pinPhase()=="submitting" && field->property("text").toString().isEmpty(),"Native Enter submits once and clears transient input");
 check(!ui("pinSubmit")->isEnabled(),"Submit remains disabled during actual response wait");
 if(extra)capture(prefix+"-live-submitting",true);
 pin(session,true);waitUi();check(pinPhase()=="form" && requestId()!=request && ui("consoleLoginPin")->property("text").toString().isEmpty(),"Actual incorrect response creates empty correction form");
 if(extra)capture(prefix+"-live-incorrect",true,"consoleLoginPin");
 const int stops=count("qaStops");keyTo(pinWindow(),Qt::Key_Escape);waitUi();check(pinPhase()=="cancelling" && count("qaStops")==stops+1,"Native Escape stops pending session once and shows cancelling");
 if(extra)capture(prefix+"-live-cancelling",true);
 keyTo(pinWindow(),Qt::Key_Escape);check(count("qaStops")==stops+1,"Repeated Escape does not dispatch another stop");quit(session);waitUi();check(pinPhase()=="cancelled","Actual quit shows cancelled result");
 if(extra)capture(prefix+"-live-cancelled",true,"pinBack");clickUi("pinBack");waitUi();
 check(libraryShown() && !pinWindow()->isVisible() && window->nativeQaGrabCount()==0,"Cancelled result closes PIN window and releases input");
 window->requestActivate();waitUi();check(active(window) && active(window)->isVisible() && active(window)->isEnabled(),"Cancelled route restores valid library focus");
 {Settings saved(QString{});check(registered(saved,1).GetConsolePin()=="0123" && registered(saved,2).GetConsolePin().isEmpty(),"Live entry never changes saved PIN records");}
 // Close button and host close use the same native stop decision as Escape.
 for(int action=0;action<2;++action){backend->connectToHost(index("192.0.2.43"));auto *closing=backend->qmlSession();waitUi();pin(closing,false);waitUi();const int beforeStops=count("qaStops");if(action==0){QCloseEvent close;QCoreApplication::sendEvent(pinWindow(),&close);check(!close.isAccepted(),"PIN window close waits for session cancellation");}else check(!backend->closeRequested(),"Host close waits for pending PIN cancellation");check(pinPhase()=="cancelling" && count("qaStops")==beforeStops+1,"Window close stops current PIN session exactly once");quit(closing);waitUi();clickUi("pinBack");waitUi();}
}
void connectionFlow(const QString &prefix,bool error) {
 backend->connectToHost(index("192.0.2.42"));auto *session=backend->qmlSession();waitUi();check(phase()=="connecting" && ui("connectionTitle"),"Local native connecting state reaches production screen");
 if(!error)capture(prefix+"-connection-progress",false,"connectionCancel");
 event(session,CHIAKI_EVENT_CONNECTED);waitUi();check(phase()=="video" && ui("connectionTitle"),"Native connected event still shows video waiting");
 if(error){QByteArray detail=QString("Controlled network failure for Synthetic target at 192.0.2.42. Check console power and the local network connection. ").repeated(8).toUtf8();ChiakiEvent failure{};failure.type=CHIAKI_EVENT_QUIT;failure.quit.reason=CHIAKI_QUIT_REASON_SESSION_REQUEST_UNKNOWN;failure.quit.reason_str=detail.constData();session->nativeQaEvent(&failure);waitUi();check(phase()=="failure" && ui("connectionRetry"),"Native failure remains actionable after cleanup");clickUi("connectionDetailsToggle");capture(prefix+"-connection-error",false,"connectionRetry");if(qEnvironmentVariable("CHIAKI_NATIVE_QA_DPR")=="1"){backend->qmlSettings()->setStreamerMode(true);waitUi();check(!ui("connectionAddress")->property("text").toString().contains("192.0.2.") && !ui("connectionDetails")->property("text").toString().contains("Synthetic") && !ui("connectionConsole")->property("text").toString().contains("Synthetic"),"Streamer mode removes private address and diagnostic detail");capture(prefix+"-connection-private-error",false,"connectionRetry");backend->qmlSettings()->setStreamerMode(false);waitUi();}const auto failed=id();clickUi("connectionRetry");check(id()!=failed && backend->qmlSession(),"Native mouse Retry starts current target after cleanup");session=backend->qmlSession();}
 finish(session);
}
void handoff() {
 auto *settings=backend->nativeQaSettings();credentials(settings);backend->nativeQaPsnHost(duid(),registered(*settings,1).GetServerNickname());qApp->setProperty("qaPsnResult",CHIAKI_ERR_SUCCESS);
 backend->connectToHost(psnIndex());auto *remote=backend->qmlSession();waitUi();pin(remote,true);waitUi();const auto request=requestId();auto *field=ui("consoleLoginPin");digits();hole(remote,true);waitUi();
 check(screenClass().startsWith("StreamView_") && requestId()==request && field==ui("consoleLoginPin") && field->property("text").toString()=="0123","Visible PSN to Stream handoff keeps the exact pending prompt and text");
 focusUi("consoleLoginPin");keyTo(owner(field),Qt::Key_Return);event(remote,CHIAKI_EVENT_CONNECTED);window->nativeQaVideo(true);waitUi();
 check(phase()=="ready" && backend->sessionPinRequest().isEmpty() && !pinWindow()->isVisible() && window->nativeQaGrabCount()==0,"Native accepted handoff opens stream and releases all input");
 keyTo(window,Qt::Key_O,Qt::ControlModifier);waitUi();evidence["activeStreamMenuInputCount"]=window->nativeQaGrabCount();keyTo(window,Qt::Key_O,Qt::ControlModifier);quit(remote);window->nativeQaVideo(false);waitUi();backLibrary();
}
void registrationResult() {
 auto *settings=backend->nativeQaSettings();const auto original=registered(*settings,1);settings->RemoveRegisteredHost(original.GetServerMAC());
 DiscoveryHost target{};target.ps5=true;target.target=CHIAKI_TARGET_PS5_1;target.host_addr="192.0.2.42";target.host_name=original.GetServerNickname();target.host_id="020000000001";target.state=CHIAKI_DISCOVERY_HOST_STATE_READY;backend->nativeQaDiscover({target});
 QVariantMap context;auto listener=QObject::connect(backend,&QmlBackend::registDialogRequested,backend,[&](const QString &,bool,const QString &,const QVariantMap &value){context=value;});
 backend->connectToHost(index("192.0.2.42"));QObject::disconnect(listener);check(!context.isEmpty(),"Native registration route supplies current target context");waitUi();keyTo(window,Qt::Key_Return);waitUi();auto *session=backend->qmlSession();check(session && phase()=="registering","Actual automatic registration session reaches production progress");session->AutoRegistSucceeded(rawHost(1));quit(session);waitUi();check(phase()=="registered" && ui("connectionBack"),"Native registration completion remains visible with library action");capture("1280-registration-result",false,"connectionBack");clickUi("connectionBack");settings->AddRegisteredHost(original);backend->nativeQaDiscover({});waitUi();
}
void inlineFlow() {
 evidence["vulkanAvailable"]=window->runtimeRendererBackend()==0;
 if(window->runtimeRendererBackend()!=0){evidence["vulkanFallback"]=true;evidence["allPassed"]=true;save();QCoreApplication::quit();return;}
 backend->connectToHost(index("192.0.2.43"));auto *session=backend->qmlSession();waitUi();fit();pin(session,false);waitUi();
 check(ui("liveConsolePinHost") && !ui("liveConsolePinHost")->property("separate").toBool() && !pinWindow()->isVisible(),"Actual Vulkan renderer selects production inline PIN");check(ui("consoleLoginPin")->hasActiveFocus(),"Actual inline PIN starts with field focus");capture("900-vulkan-inline-form",false,"consoleLoginPin");digits();
 auto *field=ui("consoleLoginPin");keyTo(window,Qt::Key_Tab);bounds(active(window));keyTo(window,Qt::Key_Tab,Qt::ShiftModifier);check(field->hasActiveFocus(),"Inline Tab and Shift Tab preserve keyboard navigation");keyTo(window,Qt::Key_Return);check(pinPhase()=="submitting","Inline native Enter submits current request");pin(session,true);waitUi();check(ui("pinHeading")->property("text").toString().contains("Incorrect"),"Actual inline content displays incorrect PIN feedback");capture("900-vulkan-inline-incorrect",false,"consoleLoginPin");
 keyTo(window,Qt::Key_Escape);check(pinPhase()=="cancelling","Inline Escape uses native cancellation");quit(session);waitUi();check(ui("pinHeading")->property("text").toString().contains("cancelled"),"Actual inline content displays cancelled result");capture("900-vulkan-inline-cancelled",false,"pinBack");clickUi("pinBack");waitUi();check(window->nativeQaGrabCount()==0 && libraryShown(),"Inline cancellation restores library and releases input");
 // Use the same native handoff under the actual renderer.
 handoff();evidence["allPassed"]=true;evidence["checks"]=checks;save();QCoreApplication::quit();
}
void resultScreens() {
 requestedSize=QSize(1280,720);fit();auto *settings=backend->nativeQaSettings();const auto original=registered(*settings,1);openSaved();digits();settings->RemoveRegisteredHost(original.GetServerMAC());waitUi();clickUi("pinSubmit");check(ui("pinFailure") && ui("pinBack") && !ui("pinDone"),"Invalidated saved-PIN target shows failure without false success");check(!backend->qmlSession(),"Failed saved-PIN action cannot start a connection");capture("1280-saved-pin-failure",false,"pinBack");clickUi("pinBack");settings->AddRegisteredHost(original);waitUi();credentials(settings);backend->nativeQaPsnHost(duid(),original.GetServerNickname());qApp->setProperty("qaPsnResult",CHIAKI_ERR_SUCCESS);registrationResult();evidence["allPassed"]=true;evidence["checks"]=checks;save();QCoreApplication::quit();
}
void reviewContracts() {
 const auto mode=qEnvironmentVariable("CHIAKI_QA_REVIEW");auto *settings=backend->nativeQaSettings();
 if(mode=="auth") {
  credentials(settings,true);backend->nativeQaPsnHost(duid(),registered(*settings,1).GetServerNickname());backend->connectToHost(psnIndex());auto *token=qobject_cast<PSNToken *>(qApp->property("qaToken").value<QObject *>());check(token && phase()=="internet","Review regression owns expired-token request");
  token->PSNTokenError("Synthetic unauthorized token response");token->UnauthorizedError();waitUi();check(screenClass().startsWith("PSNTokenDialog_"),"Unauthorized token opens existing authentication screen");auto *auth=qvariant_cast<QObject *>(navigation()->property("currentItem"));token->Finished();waitUi();check(qvariant_cast<QObject *>(navigation()->property("currentItem"))==auth && screenClass().startsWith("PSNTokenDialog_"),"Token Finished must retain the same authentication screen");
  QMetaObject::invokeMethod(auth,"close");waitUi();check(screenClass().startsWith("PsnView_") && phase()=="failure" && !backend->connectionAttempt().value("cleanupPending").toBool(),"Closing authentication returns to persistent failure after cleanup");backLibrary();
 } else {
  settings->RemoveRegisteredHost(host(1).GetServerMAC());settings->RemoveRegisteredHost(host(2).GetServerMAC());check(settings->GetRegisteredHosts().isEmpty(),"Explicit CLI credentials require no saved console records");
  StreamSessionConnectInfo info(settings,CHIAKI_TARGET_PS5_1,"192.0.2.88",QString(),QByteArray(16,'a'),QByteArray(16,'b'),QString(),QString(),false,false,false,false);
  backend->createSession(info);auto *first=backend->qmlSession();check(first && first->GetHost()=="192.0.2.88","Direct CLI path constructs real session for explicit address");quit(first,true);waitUi();check(phase()=="failure" && backend->connectionAttempt().value("targetValid").toBool(),"Explicit CLI connection keeps a valid retry without registration");const int starts=count("qaStarts");const auto failed=id();check(backend->retryConnection(failed) && backend->qmlSession() && backend->qmlSession()->GetHost()=="192.0.2.88" && count("qaStarts")==starts+1,"CLI Retry starts exactly one session at original explicit address");check(!backend->retryConnection(failed),"Old direct CLI retry cannot duplicate replacement session");auto *retry=backend->qmlSession();quit(retry,true);waitUi();const auto profileAttempt=id();backend->qmlSettings()->setCurrentProfile("other");backend->stopAutoConnect();check(!backend->retryConnection(profileAttempt),"Profile change invalidates direct CLI retry");backend->qmlSettings()->setCurrentProfile("");backend->stopAutoConnect();check(!backend->retryConnection(profileAttempt),"Returning to profile cannot restore direct CLI ownership");backLibrary();
  settings=backend->nativeQaSettings();settings->AddRegisteredHost(host(1));StreamSessionConnectInfo stored(settings,CHIAKI_TARGET_PS5_1,"192.0.2.42",QString(),host(1).GetRPRegistKey(),host(1).GetRPKey(),QString(),QString(),false,false,false,false);backend->createSession(stored);auto *storedSession=backend->qmlSession();quit(storedSession,true);waitUi();check(backend->connectionAttempt().value("targetValid").toBool(),"Stored direct credentials remain retryable while registration exists");settings->RemoveRegisteredHost(host(1).GetServerMAC());check(!backend->connectionAttempt().value("targetValid").toBool() && !backend->retryConnection(id()),"Removing originally stored target invalidates direct retry");backLibrary();
 }
 evidence["reviewMode"]=mode;evidence["allPassed"]=true;evidence["checks"]=checks;save();QCoreApplication::quit();
}
void contracts() {
 for(auto *candidate:QGuiApplication::topLevelWindows())if(auto *main=qobject_cast<QmlMainWindow *>(candidate)){window=main;backend=main->getBackend();}
 check(window && backend,"Integrated production native backend exists");backend->stopAutoConnect();window->showNormal();window->resize(900,600);window->setPosition(30,40);window->requestActivate();waitUi();
 evidence["runtimeRendererBackend"]=window->runtimeRendererBackend();evidence["requestedRenderer"]=qEnvironmentVariable("CHIAKI_QA_RENDERER","opengl");QJsonArray screens;for(auto *screen:QGuiApplication::screens())screens.append(QJsonObject{{"name",screen->name()},{"dpr",screen->devicePixelRatio()},{"width",screen->geometry().width()},{"height",screen->geometry().height()}});evidence["screens"]=screens;
 if(qEnvironmentVariableIsSet("CHIAKI_QA_RESULTS")){resultScreens();return;}
 if(qEnvironmentVariableIsSet("CHIAKI_QA_REVIEW")){reviewContracts();return;}
 if(qEnvironmentVariable("CHIAKI_QA_RENDERER")=="vulkan") {inlineFlow();return;}
 check(window->runtimeRendererBackend()==1,"Visible separate PIN acceptance uses actual OpenGL renderer");check(!backend->discoveryEnabled(),"Native visible acceptance keeps discovery disabled");
 if(qEnvironmentVariableIsSet("CHIAKI_QA_MANUAL")){liveFlow("900",false);return;}
 const bool extra=qEnvironmentVariable("CHIAKI_NATIVE_QA_DPR")=="1";
 for(const QSize size:{QSize(900,600),QSize(1280,720)}) {requestedSize=size;window->resize(size);window->setPosition(30,40);waitUi();check(window->size()==size,"Native client size matches requested logical geometry");const QString prefix=QString::number(size.width());connectionFlow(prefix,size.width()==1280);savedFlow(prefix,extra && size.width()==900);liveFlow(prefix,extra && size.width()==900);if(evidence["manualCompleted"].toBool())return;}
 handoff();if(extra)registrationResult();check(!backend->discoveryEnabled() && !backend->qmlSession() && count("qaSleeps")==0,"Visible acceptance finishes isolated without sleep requests");
 evidence["allPassed"]=true;evidence["checks"]=checks;evidence["transport"]="Controlled native transport boundaries; production Main/views/session handlers";evidence["captures"]=captures;save();QCoreApplication::quit();
}
void initialize() {
 qInstallMessageHandler([](QtMsgType,const QMessageLogContext &,const QString &msg){QFile log(qEnvironmentVariable("CHIAKI_NATIVE_QA_ROOT")+".messages.txt");if(log.open(QIODevice::WriteOnly|QIODevice::Append))log.write(msg.toUtf8()+"\n");});
 runRoot=qEnvironmentVariable("CHIAKI_NATIVE_QA_ROOT");check(!runRoot.isEmpty() && !QDir(runRoot).exists(),"Visible acceptance uses a new isolated settings root");QDir().mkpath(runRoot);QSettings::setDefaultFormat(QSettings::IniFormat);QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,runRoot);QSettings::setPath(QSettings::IniFormat,QSettings::SystemScope,runRoot+"/system");
 QSettings config(QSettings::IniFormat,QSettings::UserScope,"Chiaki","Chiaki");config.setValue("settings/remote_play_ask",false);config.setValue("settings/add_steam_shortcut_ask",false);config.setValue("settings/render_backend",qEnvironmentVariable("CHIAKI_QA_RENDERER","opengl"));config.setValue("settings/auto_discovery",false);config.setValue("settings/auto_connect_mac","");config.sync();
 {Settings settings(QString{});seed(settings);for(int number:{1,2}){auto raw=rawHost(number);qstrncpy(raw.server_nickname,number==1?"Synthetic target with a long living room console identity":"Synthetic peer with a long upstairs console identity",sizeof(raw.server_nickname));RegisteredHost named(raw);named.SetConsolePin(named,QString());settings.AddRegisteredHost(named);}}
 config.sync();check(QFile::copy(config.fileName(),runRoot+"/baseline.ini"),"Visible baseline settings are retained for post-exit proof");
 if(qEnvironmentVariableIsSet("CHIAKI_NATIVE_QA_EXPAND"))qApp->installTranslator(&translator);
 QTimer::singleShot(400,qApp,contracts);
}
}
void initializeConnectionQa() {
 const auto mode=qEnvironmentVariable("CHIAKI_QA_SCENARIO","visible");
 if(mode=="psn")qapsn::initializeConnectionQa();else if(mode=="wake")qawake::initializeConnectionQa();else if(mode=="live")qalive::initializeConnectionQa();else visible::initialize();
}
