#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QTimer>
#include <QPointer>
#include <QQuickItem>
#include <QQuickWindow>
#include <QKeyEvent>
#include <QEventLoop>
#include <QClipboard>
#include <QJSEngine>
#include "qmlbackend.h"
#include "qmlmainwindow.h"
#include "jsonrequester.h"
#include "psnaccountid.h"

namespace {
QString runRoot;
int checks=0;
QJsonArray captures;
void check(bool condition,const char *message) { ++checks; if(!condition) qFatal("PSN setup contract failed: %s",message); }
JsonRequester *reply() { const auto requests=qApp->property("qaJsonRequests").toList(); check(!requests.isEmpty(),"Controlled request exists"); return qobject_cast<JsonRequester *>(requests.last().value<QObject *>()); }
void respond(const QJsonObject &data) { reply()->requestFinished("https://example.invalid/private-authorization-data",QJsonDocument(data)); }
QJsonObject tokens() { return {{"access_token","synthetic-access-private"},{"refresh_token","synthetic-refresh-private"},{"expires_in",3600}}; }
QString redirect() { return "https://remoteplay.dl.playstation.net/remoteplay/redirect?code=synthetic-code-private"; }
QString phase(QmlBackend *backend) { return backend->psnAuthAttempt().value("phase").toString(); }
void waitUi(int ms=100) { QEventLoop loop; QTimer::singleShot(ms,&loop,&QEventLoop::quit); loop.exec(); }
QQuickItem *named(QQuickItem *root,const QString &name) { if(root->objectName()==name) return root; for(auto *child:root->childItems()) if(auto *found=named(child,name)) return found; return nullptr; }
QQuickItem *ui(const QString &name) { for(auto *candidate:QGuiApplication::allWindows()) if(auto *quick=qobject_cast<QQuickWindow *>(candidate)) if(auto *found=named(quick->contentItem(),name)) return found; return nullptr; }
void key(QmlMainWindow *window,int code) { QKeyEvent press(QEvent::KeyPress,code,Qt::NoModifier),release(QEvent::KeyRelease,code,Qt::NoModifier); QCoreApplication::sendEvent(window,&press); QCoreApplication::sendEvent(window,&release); waitUi(); }
void activate(QmlMainWindow *window,const QString &name) { auto *item=ui(name); check(item && item->isVisible() && item->isEnabled(),"Native action is visible and enabled"); item->forceActiveFocus(Qt::TabFocusReason); waitUi(); check(item->hasActiveFocus() && item->property("visualFocus").toBool(),"Native keyboard action has visible focus"); key(window,Qt::Key_Return); }
void openSetup(QmlBackend *backend,bool expired=false) {
 if(expired) backend->psnCredsExpired();
 else { auto *stack=ui("mainNavigation"); check(stack,"Production navigation is loaded"); check(QMetaObject::invokeMethod(stack->parentItem(),"showPSNTokenDialog",Q_ARG(QVariant,QString()),Q_ARG(QVariant,false)),"Production PSN entry route opens"); }
 waitUi(400); check(ui("psnSetup"),"Production PSN screen is loaded");
}
void capture(QmlMainWindow *window,const QString &name) {
 waitUi(180); const auto image=window->nativeQaImage(); check(!image.isNull(),"Visible native screen has a rendered framebuffer");
 check(image.save(runRoot+"/"+name+".png"),"Native screen capture saves");
 captures.append(QJsonObject{{"file",name+".png"},{"logicalWidth",window->width()},{"logicalHeight",window->height()},{"dpr",window->devicePixelRatio()},{"physicalWidth",image.width()},{"physicalHeight",image.height()},{"captureMethod","OpenGL QQuickRenderControl framebuffer"}});
}
void contracts() {
 QmlMainWindow *window=nullptr;
 for(auto *candidate:QGuiApplication::topLevelWindows()) if(auto *main=qobject_cast<QmlMainWindow *>(candidate)) window=main;
 check(window,"Production native window exists");
 auto *backend=window->getBackend(); check(backend,"Production native backend exists");
 backend->stopAutoConnect();
 check(!backend->discoveryEnabled(),"Discovery is disabled for controlled checks");
 const QStringList invalid={"", "https://remoteplay.dl.playstation.net.evil.invalid/remoteplay/redirect?code=synthetic-code-private", "https://remoteplay.dl.playstation.net/remoteplay/redirect/extra?code=synthetic-code-private", "https://remoteplay.dl.playstation.net/remoteplay/redirect", "https://remoteplay.dl.playstation.net/remoteplay/redirect?code=", "https://remoteplay.dl.playstation.net/remoteplay/redirect?code=a&code=b", "https://someone@remoteplay.dl.playstation.net/remoteplay/redirect?code=a", "http://remoteplay.dl.playstation.net/remoteplay/redirect?code=a"};
 for(const auto &url:invalid) { check(!backend->startPsnAuth(url).isEmpty(),"Invalid input has an identified result"); check(phase(backend)=="failure","Invalid redirect is rejected natively"); }
 check(qApp->property("qaJsonRequests").toList().isEmpty(),"Invalid redirects send no network request");
 backend->startPsnAuth(redirect()); const auto id=backend->psnAuthAttempt().value("id");
 check(phase(backend)=="exchanging","Request starts with exchange progress");
 check(backend->startPsnAuth(redirect()).isEmpty() && backend->psnAuthAttempt().value("id")==id,"Duplicate request cannot replace the running request");
 respond({{"access_token",""},{"refresh_token",""},{"expires_in",0}});
 check(phase(backend)=="failure","Incomplete credentials fail");
 { Settings saved(QString{}); check(saved.GetPsnAuthToken().isEmpty() && saved.GetPsnRefreshToken().isEmpty(),"Malformed token response saves nothing"); }
 backend->startPsnAuth(redirect()); respond(tokens());
 check(phase(backend)=="account","Token exchange advances to actual Account ID progress");
 { Settings saved(QString{}); check(saved.GetPsnAuthToken().isEmpty(),"Partial response does not save credentials"); }
 respond({{"user_id","invalid account data"}});
 check(phase(backend)=="failure","Malformed Account ID is recoverable without exception");
 { Settings saved(QString{}); check(saved.GetPsnAccountId().isEmpty() && saved.GetPsnAuthToken().isEmpty(),"Invalid Account ID does not partially save credentials"); }
 backend->startPsnAuth(redirect()); auto *failedReply=reply();
 failedReply->requestError("https://example.invalid/synthetic-access-private?code=synthetic-code-private","synthetic-refresh-private",QNetworkReply::ProtocolInvalidOperationError);
 check(phase(backend)=="failure","Expired or used code produces persistent failure");
 const auto safe=backend->psnAuthAttempt().value("message").toString()+backend->psnAuthAttempt().value("errorText").toString();
 check(!safe.contains("synthetic-") && safe.contains("fresh"),"Error diagnostics exclude private URL and token values and offer fresh sign-in");
 backend->startPsnAuth(redirect()); respond(tokens());
 QPointer<JsonRequester> oldReply=reply();
 backend->qmlSettings()->setCurrentProfile("other"); backend->stopAutoConnect();
 check(phase(backend)=="failure","Profile change invalidates pending network replies");
 oldReply->requestFinished("private",QJsonDocument(QJsonObject{{"user_id","1"}}));
 QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
 check(!oldReply,"Invalidated network replies are deleted after callback return");
 { Settings other("other"); check(other.GetPsnAuthToken().isEmpty() && other.GetPsnAccountId().isEmpty(),"Old profile request cannot save in replacement profile"); }
 backend->qmlSettings()->setCurrentProfile(""); backend->stopAutoConnect();
 { Settings original(QString{}); check(original.GetPsnAuthToken().isEmpty() && original.GetPsnAccountId().isEmpty(),"Invalidated request did not save in original profile"); }
 auto progressObserver=QObject::connect(backend,&QmlBackend::psnAuthAttemptChanged,backend,[backend] {
  if(phase(backend)=="account") backend->qmlSettings()->setCurrentProfile("progress-observer");
 });
 backend->startPsnAuth(redirect()); const auto requestsBeforeProgress=qApp->property("qaJsonRequests").toList().size(); respond(tokens());
 check(phase(backend)=="failure" && qApp->property("qaJsonRequests").toList().size()==requestsBeforeProgress,"Reentrant profile change from progress prevents account request and saves");
 QObject::disconnect(progressObserver); backend->qmlSettings()->setCurrentProfile(""); backend->stopAutoConnect();
 auto completionObserver=QObject::connect(backend,&QmlBackend::psnAuthAttemptChanged,backend,[backend] {
  if(phase(backend)=="success") backend->qmlSettings()->setCurrentProfile("completion-observer");
 });
 backend->startPsnAuth(redirect()); respond(tokens()); const auto refreshesBeforeCompletion=qApp->property("qaHostRefreshes").toInt(); respond({{"user_id","1"}});
 check(phase(backend)=="failure" && qApp->property("qaHostRefreshes").toInt()==refreshesBeforeCompletion,"Completion observer can switch profile without stale host refresh or deleted-object access");
 { Settings replacement("completion-observer"); check(replacement.GetPsnAuthToken().isEmpty() && replacement.GetPsnAccountId().isEmpty(),"Reentrant completion leaves replacement profile credentials unchanged"); }
 QObject::disconnect(completionObserver); backend->qmlSettings()->setCurrentProfile(""); backend->stopAutoConnect();
 backend->startPsnAuth(redirect()); respond(tokens()); auto *accountReply=reply(); respond({{"user_id","1"}});
 check(phase(backend)=="success","Final Account ID response establishes success");
 check(!backend->qmlSettings()->remotePlayAsk(),"Completed sign-in disables setup reminder");
 { Settings saved(QString{}); check(saved.GetPsnAuthToken()=="synthetic-access-private" && saved.GetPsnRefreshToken()=="synthetic-refresh-private" && saved.GetPsnAccountId()=="AQAAAAAAAAA=" && !saved.GetPsnAuthTokenExpiry().isEmpty() && !saved.GetRemotePlayAsk(),"Fresh native Settings reads complete saved result"); }
 accountReply->requestError("private","private",QNetworkReply::UnknownNetworkError);
 accountReply->requestFinished("private",QJsonDocument(QJsonObject{{"user_id","2"}}));
 check(phase(backend)=="success","Repeated final responses cannot replace success");
 { Settings saved(QString{}); check(saved.GetPsnAccountId()=="AQAAAAAAAAA=","Repeated final response cannot overwrite Account ID"); }
 { Settings other("other"); check(other.GetPsnAuthToken().isEmpty() && other.GetPsnAccountId().isEmpty(),"Peer profile remains unchanged"); }
 window->setGeometry(100,100,900,600); window->QWindow::showNormal(); waitUi(200); window->hide(); waitUi(80); window->QWindow::showNormal(); window->raise(); window->requestActivate(); waitUi(400);
 check(window->isVisible() && window->isExposed(),"Native PSN acceptance window is visible");
 for(const QSize size:{QSize(900,600),QSize(1280,720)}) {
  window->resize(size); waitUi(); openSetup(backend,true);
  check(ui("psnHeading")->property("text")=="Renew PSN credentials","Expiry event uses the renewal route and title");
  check(ui("psnRedirect")->property("echoMode").toInt()!=0,"Real fallback redirect field starts masked");
  check(ui("psnPaste")->isVisible() && ui("psnCopy")->isVisible(),"External fallback retains explicit copy and paste actions");
  capture(window,QString("form-%1x%2").arg(size.width()).arg(size.height()));
  QApplication::clipboard()->setText(redirect()); activate(window,"psnPaste");
  check(ui("psnRedirect")->property("text")==redirect(),"Explicit native paste fills the redirect without submission");
  activate(window,"psnReveal"); check(ui("psnRedirect")->property("echoMode").toInt()==0,"Show reveals the app-owned redirect field");
  backend->qmlSettings()->setStreamerMode(true); check(ui("psnRedirect")->property("echoMode").toInt()!=0 && !ui("psnReveal")->isEnabled(),"Streamer mode immediately masks native revealed input"); backend->qmlSettings()->setStreamerMode(false);
  activate(window,"psnSubmit"); check(phase(backend)=="exchanging","Native submit dispatches the production request");
  check(ui("psnClose")->property("text")=="Close","Submitted native screen uses Close");
  const auto activeId=backend->psnAuthAttempt().value("id"); const auto requestCount=qApp->property("qaJsonRequests").toList().size();
  key(window,Qt::Key_Escape); waitUi(400); check(!ui("psnSetup") && phase(backend)=="exchanging","Escape closes the screen while native sign-in continues");
  openSetup(backend); check(backend->psnAuthAttempt().value("id")==activeId && qApp->property("qaJsonRequests").toList().size()==requestCount,"Re-entry attaches to the running request without duplicate dispatch");
  respond(tokens()); respond({{"user_id","1"}}); waitUi();
  check(ui("psnStatus")->property("text")=="PSN remote access is ready","Reopened production screen shows actual terminal success");
  capture(window,QString("success-%1x%2").arg(size.width()).arg(size.height()));
  activate(window,"psnClose"); waitUi(400); check(!ui("psnSetup"),"Completion action returns to the underlying view");
 }
 QApplication::clipboard()->clear();
 int legacyCompleted=0;
 auto legacyObserver=QObject::connect(backend,&QmlBackend::psnLoginAccountIdDone,backend,[&](const QString &){++legacyCompleted;});
 check(backend->handlePsnLoginRedirect(redirect()),"Existing registration sign-in entry remains accepted");
 respond(tokens()); respond({{"user_id","1"}});
 check(legacyCompleted==1,"Existing registration signal delivers only the final result"); QObject::disconnect(legacyObserver);
 QJSEngine engine; const auto callback=engine.evaluate("(function(message,ok,done) { this.finalCalls=(this.finalCalls||0)+1; this.finalOk=ok; this.finalDone=done; })");
 backend->initPsnAuth(redirect(),callback); respond(tokens()); respond({{"user_id","1"}});
 check(engine.globalObject().property("finalCalls").toInt()==1 && engine.globalObject().property("finalOk").toBool() && engine.globalObject().property("finalDone").toBool(),"Existing callback entry remains compatible and receives one final success");
 QJsonObject evidence{{"allPassed",true},{"checks",checks},{"realPsnRequests",0},{"actualNativePersistence",true},{"controlledResponses",true},{"captures",captures},{"visibleNativeChecks",true}};
 QFile output(runRoot+"/contracts.json"); check(output.open(QIODevice::WriteOnly),"Evidence can be written"); output.write(QJsonDocument(evidence).toJson()); output.close();
 QCoreApplication::quit();
}
}
void initializePsnSetupQa() {
 runRoot=qEnvironmentVariable("CHIAKI_PSN_SETUP_QA_ROOT"); check(!runRoot.isEmpty() && !QDir(runRoot).exists(),"Each run uses a fresh isolated directory"); QDir().mkpath(runRoot);
 QSettings::setDefaultFormat(QSettings::IniFormat); QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,runRoot); QSettings::setPath(QSettings::IniFormat,QSettings::SystemScope,runRoot+"/system");
 QSettings config(QSettings::IniFormat,QSettings::UserScope,"Chiaki","Chiaki"); check(QFileInfo(config.fileName()).absoluteFilePath().startsWith(runRoot),"Settings seed is isolated");
 config.setValue("settings/remote_play_ask",false); config.setValue("settings/add_steam_shortcut_ask",false); config.setValue("settings/render_backend","opengl"); config.setValue("settings/auto_discovery",false); config.setValue("settings/auto_connect_mac",""); config.sync();
 QTimer::singleShot(300,qApp,contracts);
}
