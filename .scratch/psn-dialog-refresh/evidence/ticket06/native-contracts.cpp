#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>
#include <QSettings>
#include <QTimer>
#include "qmlbackend.h"
#include "qmlmainwindow.h"
#ifdef Q_OS_WIN
#include <windows.h>
#endif
namespace {
QString runRoot;
QmlBackend *backend;
QmlMainWindow *window;
QJsonObject evidence;
QJsonArray captures;
QImage lastImage;
QString lastCaptureState;
int checks=0;
void save() { evidence["checks"]=checks; QFile file(runRoot+"/contracts.json"); if(!file.open(QIODevice::WriteOnly)) qFatal("Cannot write native disconnect evidence"); file.write(QJsonDocument(evidence).toJson()); }
void check(bool ok,const char *message) { ++checks; if(ok) return; evidence["failedCheck"]=message; save(); qFatal("Native disconnect check failed: %s",message); }
void waitUi(int ms=320) { QEventLoop loop; QTimer::singleShot(ms,&loop,&QEventLoop::quit); loop.exec(); }
int count(const char *name) { return qApp->property(name).toInt(); }
QQuickItem *named(QQuickItem *item,const QString &name) { if(!item || !item->isVisible()) return nullptr; if(item->objectName()==name) return item; for(auto *child:item->childItems()) if(auto *found=named(child,name)) return found; return nullptr; }
QQuickItem *ui(const QString &name) { for(auto *candidate:QGuiApplication::allWindows()) if(auto *quick=qobject_cast<QQuickWindow *>(candidate)) if(auto *found=named(quick->contentItem(),name)) return found; return nullptr; }
QQuickWindow *dialogWindow() { for(auto *candidate:QGuiApplication::allWindows()) if(candidate->objectName()=="streamDisconnectWindow") return qobject_cast<QQuickWindow *>(candidate); return nullptr; }
QWindow *owner(QQuickItem *item) { auto *dialog=dialogWindow(); return dialog && item->window()==dialog?static_cast<QWindow *>(dialog):static_cast<QWindow *>(window); }
void key(QWindow *target,int code,Qt::KeyboardModifiers modifiers=Qt::NoModifier) { QKeyEvent press(QEvent::KeyPress,code,modifiers),release(QEvent::KeyRelease,code,modifiers); QCoreApplication::sendEvent(target,&press); QCoreApplication::sendEvent(target,&release); waitUi(70); }
void click(QQuickItem *item) { auto *target=owner(item); const auto local=item->mapToScene({item->width()/2,item->height()/2}); const auto global=target->mapToGlobal(local.toPoint()); QMouseEvent press(QEvent::MouseButtonPress,local,global,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier),release(QEvent::MouseButtonRelease,local,global,Qt::LeftButton,Qt::NoButton,Qt::NoModifier); QCoreApplication::sendEvent(target,&press); QCoreApplication::sendEvent(target,&release); waitUi(); }
void bounds(QQuickItem *item) { if(item) evidence["lastBounds"]=QJsonObject{{"name",item->objectName()},{"width",item->width()},{"height",item->height()},{"visible",item->isVisible()},{"parentWidth",item->parentItem()?item->parentItem()->width():-1},{"parentHeight",item->parentItem()?item->parentItem()->height():-1}}; check(item && item->width()>0 && item->height()>0,"Native action is visible with positive bounds"); const QRectF rect(item->mapToScene({0,0}),item->size()); auto *target=owner(item); check(QRectF(0,0,target->width(),target->height()).adjusted(-1,-1,1,1).contains(rect),"Native action fits within its owner client"); check(item->hasActiveFocus() && item->property("visualFocus").toBool(),"Native action has visible keyboard focus"); }
void event(StreamSession *session,ChiakiEventType type) { ChiakiEvent value{}; value.type=type; if(type==CHIAKI_EVENT_QUIT) value.quit.reason=CHIAKI_QUIT_REASON_STOPPED; session->nativeQaEvent(&value); waitUi(); }
RegisteredHost synthetic() { ChiakiRegisteredHost raw{}; raw.target=CHIAKI_TARGET_PS5_1; raw.server_mac[0]=2; raw.server_mac[5]=1; qstrncpy(raw.server_nickname,"Synthetic disconnect target",sizeof(raw.server_nickname)); memset(raw.rp_regist_key,'a',sizeof(raw.rp_regist_key)); memset(raw.rp_key,1,sizeof(raw.rp_key)); return RegisteredHost(raw); }
int targetIndex() { const auto hosts=backend->hosts(); for(int i=0;i<hosts.size();++i) if(hosts.at(i).toMap().value("address")=="192.0.2.42") return i; return -1; }
void capture(const QString &name) {
 auto *native=dialogWindow(); const bool separate=native && native->isVisible(); auto *target=separate?static_cast<QWindow *>(native):static_cast<QWindow *>(window);
 target->raise(); target->requestActivate();
#ifdef Q_OS_WIN
 const auto nativeHandle=reinterpret_cast<HWND>(target->winId());
 const auto foregroundThread=GetWindowThreadProcessId(GetForegroundWindow(),nullptr);
 const auto currentThread=GetCurrentThreadId();
 const bool attached=foregroundThread!=currentThread && AttachThreadInput(currentThread,foregroundThread,TRUE);
 SetForegroundWindow(nativeHandle);
 if(attached) AttachThreadInput(currentThread,foregroundThread,FALSE);
#endif
 waitUi(350);
#ifdef Q_OS_WIN
 RECT nativeRect{}; GetWindowRect(nativeHandle,&nativeRect); POINT nativeOrigin{}; ClientToScreen(nativeHandle,&nativeOrigin);
 evidence["nativeRect"]=QJsonObject{{"left",int(nativeRect.left)},{"top",int(nativeRect.top)},{"right",int(nativeRect.right)},{"bottom",int(nativeRect.bottom)},{"clientLeft",int(nativeOrigin.x)},{"clientTop",int(nativeOrigin.y)},{"qtX",target->x()},{"qtY",target->y()},{"qtWidth",target->width()},{"qtHeight",target->height()}};
#endif
 check(target->isVisible() && target->isExposed(),"Native capture owner is visible and exposed");
#ifdef Q_OS_WIN
 check(GetForegroundWindow()==reinterpret_cast<HWND>(target->winId()),"Native capture owner is the actual foreground OS window");
#endif
 if(qEnvironmentVariableIsSet("CHIAKI_DISCONNECT_QA_HOLD") && name=="900-cancel-focus") {
  evidence["hold"]="900-cancel-focus"; save(); QEventLoop loop; QTimer poll;
  QObject::connect(&poll,&QTimer::timeout,&loop,[&] { if(QFile::exists(runRoot+"/resume.flag")) loop.quit(); }); poll.start(100); QTimer::singleShot(300000,&loop,&QEventLoop::quit); loop.exec();
 }
 const QImage image=separate?native->grabWindow():window->nativeQaImage();
 evidence["renderDiagnostics"]=QJsonObject{{"renderCalls",count("qaRenderCalls")},{"quickPaints",count("qaQuickPaints")},{"texture",qApp->property("qaQuickTexture").toBool()},{"swapchain",qApp->property("qaSwapchain").toBool()}}; check(!image.isNull(),"Native capture reads rendered pixels"); const auto captureState=name.mid(name.indexOf('-')+1); if(!lastImage.isNull() && lastImage.size()==image.size() && lastCaptureState!=captureState) check(lastImage!=image,"Native pixels change for distinct driven dialog states"); lastImage=image; lastCaptureState=captureState; check(image.save(runRoot+"/"+name+".png"),"Native capture saves");
 captures.append(QJsonObject{{"file",name+".png"},{"separate",separate},{"width",target->width()},{"height",target->height()},{"dpr",target->devicePixelRatio()},{"physicalWidth",image.width()},{"physicalHeight",image.height()},{"method",separate?"QQuickWindow.grabWindow":window->runtimeRendererBackend()==1?"OpenGL render control framebuffer":"Vulkan Qt Quick target GPU readback"}});
}
void sharedPopups() {
 auto *navigation=ui("mainNavigation"); check(navigation,"Production Library navigation exists"); auto *root=navigation->parentItem();
 for(const QSize size:{QSize(900,600),QSize(1280,720)}) {
  window->resize(size); waitUi();
  check(QMetaObject::invokeMethod(root,"showConfirmDialog",Q_ARG(QVariant,QString("Confirm console action")),Q_ARG(QVariant,QString("This controlled confirmation checks visible native focus and viewport sizing.")),Q_ARG(QVariant,QString("Continue")),Q_ARG(QVariant,QVariant()),Q_ARG(QVariant,QVariant())),"Production confirmation entry opens"); waitUi();
  auto *cancel=ui("confirmationCancel"); bounds(cancel); check(cancel->property("text")=="Cancel","Production shared confirmation keeps initial Cancel"); capture(QString::number(size.width())+"-shared-confirmation"); key(owner(cancel),Qt::Key_Escape); waitUi(); check(!ui("confirmationCancel"),"Escape dismisses shared confirmation");
  backend->qmlSettings()->setRemotePlayAsk(true);
  check(QMetaObject::invokeMethod(root,"showRemindDialog",Q_ARG(QVariant,QString("PSN sign-in")),Q_ARG(QVariant,QString("Sign-in saves remote access credentials. You can complete this later in Settings.")),Q_ARG(QVariant,true),Q_ARG(QVariant,QVariant())),"Production reminder entry opens"); waitUi();
  auto *later=ui("confirmationCancel"); bounds(later); check(later->property("text")=="Later","Production shared reminder keeps initial Later"); capture(QString::number(size.width())+"-shared-reminder"); key(owner(later),Qt::Key_Escape); waitUi(); check(!ui("confirmationCancel") && backend->qmlSettings()->remotePlayAsk(),"Escape dismisses reminder without disabling its saved preference"); backend->qmlSettings()->setRemotePlayAsk(false);
 }
}
void contracts() {
 for(auto *candidate:QGuiApplication::topLevelWindows()) if(auto *main=qobject_cast<QmlMainWindow *>(candidate)) window=main;
 check(window,"Production main window exists"); backend=window->getBackend(); backend->stopAutoConnect();
 check(!backend->discoveryEnabled() && !backend->qmlSession(),"Native startup has no discovery or session");
 window->QWindow::showNormal(); window->setPosition(40,40); waitUi(200); window->hide(); waitUi(80); window->QWindow::showNormal(); window->raise(); window->requestActivate(); waitUi(500);
 const bool separate=window->runtimeRendererBackend()==1; evidence["runtimeRendererBackend"]=window->runtimeRendererBackend();
 check(separate==(qEnvironmentVariable("CHIAKI_QA_RENDERER")=="opengl"),"Actual renderer selects the expected presentation route");
 if(!qEnvironmentVariableIsSet("CHIAKI_DISCONNECT_QA_HOLD")) sharedPopups();
 auto *settings=backend->nativeQaSettings(); const auto preference=settings->GetDisconnectAction();
 for(const QSize size:{QSize(900,600),QSize(1280,720)}) {
  for(int action=0;action<5;++action) {
   check(targetIndex()>=0,"Synthetic console is in the production library"); backend->connectToHost(targetIndex()); auto *session=backend->qmlSession(); check(session,"A real StreamSession is constructed"); event(session,CHIAKI_EVENT_CONNECTED);
   check(backend->connectionAttempt().value("phase")=="ready" && session->IsConnected(),"Controlled connected event reaches ready state");
   window->resize(size); window->setPosition(40,40); waitUi();
   check(window->size()==size,"Native client uses the requested logical size");
   auto *prompt=ui("streamDisconnectDialog"); check(prompt,"Production StreamView owns the disconnect presentation"); auto *stream=prompt->parentItem(); stream->forceActiveFocus(Qt::TabFocusReason); waitUi(80);
   const int grabs=window->nativeQaGrabCount(),stops=count("qaStops"),sleeps=count("qaSleeps");
   check(!backend->closeRequested(),"Native close opens confirmation without closing host"); waitUi();
   check(prompt->property("opened").toBool(),"Native close opens production disconnect controls");
   auto *sleep=ui("confirmationAction"),*disconnect=ui("confirmationSecondaryAction"),*cancel=ui("confirmationCancel");
   check(sleep && disconnect && cancel,"All native disconnect choices exist");
   check(sleep->property("text")=="Disconnect and sleep" && disconnect->property("text")=="Disconnect" && cancel->property("text")=="Cancel","Native choices name their exact effects");
   if(auto *popup=prompt->findChild<QObject *>("confirmationDialog")) { auto *p=qvariant_cast<QQuickItem *>(popup->property("parent")); evidence["dialogSizing"]=QJsonObject{{"width",popup->property("width").toDouble()},{"height",popup->property("height").toDouble()},{"parentClass",p?p->metaObject()->className():"missing"},{"parentWidth",p?p->width():-1},{"parentHeight",p?p->height():-1},{"sceneWidth",stream->window()->width()},{"sceneHeight",stream->window()->height()},{"hostWidth",stream->width()},{"hostHeight",stream->height()}}; } bounds(cancel); check(window->nativeQaGrabCount()==grabs+1 && session->nativeQaInputBlocked(),"Dialog captures actual native session input once");
   backend->closeRequested(); waitUi(50); check(window->nativeQaGrabCount()==grabs+1,"Repeated host close does not duplicate capture");
   auto *native=dialogWindow(); check(native && native->isVisible()==separate,"Actual renderer uses the correct owner window");
   if(separate) { check(native->transientParent()==window && native->modality()==Qt::ApplicationModal,"Separate dialog retains native parent and modal input ownership"); check(qAbs(native->x()-(window->x()+(window->width()-native->width())/2))<=1 && qAbs(native->y()-(window->y()+(window->height()-native->height())/2))<=1,"Separate dialog is centered on the host"); }
   if(action==0) {
    capture(QString::number(size.width())+"-cancel-focus"); key(owner(cancel),Qt::Key_Tab); bounds(sleep); key(owner(sleep),Qt::Key_Backtab); bounds(cancel); key(owner(cancel),Qt::Key_Return);
   } else if(action==1) key(owner(cancel),Qt::Key_Escape);
   else if(action==2) { if(separate) { QCloseEvent close; QCoreApplication::sendEvent(native,&close); } else { auto *dialog=cancel; while(dialog && dialog->objectName()!="confirmationDialog") dialog=dialog->parentItem(); if(dialog) QMetaObject::invokeMethod(dialog,"close"); else key(owner(cancel),Qt::Key_Escape); } waitUi(); }
   else if(action==3) { key(owner(cancel),Qt::Key_Down); bounds(sleep); click(sleep); }
   else { key(owner(cancel),Qt::Key_Up); bounds(disconnect); key(owner(disconnect),Qt::Key_Return); }
   waitUi();
   check(!prompt->property("opened").toBool() && !native->isVisible(),"Every chosen outcome closes only the confirmation");
   check(window->nativeQaGrabCount()==grabs && !session->nativeQaInputBlocked(),"Closing releases actual session input to its original level");
   check(count("qaStops")==stops+(action>=3?1:0),"Only explicit disconnect choices reach native Stop");
   check(count("qaSleeps")==sleeps+(action==3?1:0),"Only Disconnect and sleep reaches native GoToBed");
   check(settings->GetDisconnectAction()==preference,"Dialog leaves the saved disconnect preference unchanged");
   check(window->isVisible() && backend->qmlSession()==session && session->IsConnected(),"Dialog close leaves host and current session alive until native cleanup");
   if(action<3) check(stream->hasActiveFocus(),"Cancellation restores focus to the current stream");
   event(session,CHIAKI_EVENT_QUIT); waitUi(); check(!backend->qmlSession(),"Controlled native quit completes session cleanup");
  }
 }
 check(count("qaSleeps")==2 && count("qaStops")==4,"Both sizes dispatch exactly two sleep and four stop requests");
 evidence["allPassed"]=true; evidence["stopRequests"]=count("qaStops"); evidence["sleepRequests"]=count("qaSleeps"); evidence["captures"]=captures; evidence["transport"]="Real QmlBackend and StreamSession handlers, controlled native Start, Stop and GoToBed boundaries"; save(); QCoreApplication::quit();
}
}
void initializeDisconnectQa() {
 runRoot=qEnvironmentVariable("CHIAKI_DISCONNECT_QA_ROOT"); check(!runRoot.isEmpty() && !QDir(runRoot).exists(),"Each native check uses a fresh isolated settings root"); QDir().mkpath(runRoot);
 QSettings::setDefaultFormat(QSettings::IniFormat); QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,runRoot); QSettings::setPath(QSettings::IniFormat,QSettings::SystemScope,runRoot+"/system");
 QSettings config(QSettings::IniFormat,QSettings::UserScope,"Chiaki","Chiaki"); config.setValue("settings/remote_play_ask",false); config.setValue("settings/add_steam_shortcut_ask",false); config.setValue("settings/render_backend",qEnvironmentVariable("CHIAKI_QA_RENDERER","opengl")); config.setValue("settings/auto_discovery",false); config.setValue("settings/auto_connect_mac",""); config.sync();
 { Settings settings(QString{}); const auto registered=synthetic(); settings.AddRegisteredHost(registered); settings.SetManualHost(ManualHost(-1,"192.0.2.42",true,registered.GetServerMAC())); settings.SetHardwareDecoder(""); settings.SetDecoder(Decoder::Ffmpeg); settings.SetWindowType(WindowType::SelectedResolution); settings.SetAudioVideoDisabled(CHIAKI_AUDIO_VIDEO_DISABLED); settings.SetDisconnectAction(DisconnectAction::Ask); }
 config.sync(); check(QFile::copy(config.fileName(),runRoot+"/baseline.ini"),"Native baseline settings are retained"); QTimer::singleShot(500,qApp,contracts);
}
