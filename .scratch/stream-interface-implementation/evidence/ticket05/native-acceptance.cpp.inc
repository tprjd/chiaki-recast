QQuickItem *streamView() { auto *dialog=ui("streamDisconnectDialog"); check(dialog,"Production StreamView is present"); return dialog->parentItem(); }
int pointerPresses=0;
class StreamPointerProbe : public QObject {
 bool eventFilter(QObject *,QEvent *event) override { if(event->type()==QEvent::MouseButtonPress) ++pointerPresses; return false; }
};
void foreground(QWindow *target) {
 target->raise(); target->requestActivate();
#ifdef Q_OS_WIN
 const auto thread=GetWindowThreadProcessId(GetForegroundWindow(),nullptr), current=GetCurrentThreadId();
 const bool attached=thread!=current && AttachThreadInput(current,thread,TRUE);
 SetForegroundWindow(reinterpret_cast<HWND>(target->winId()));
 if(attached) AttachThreadInput(current,thread,FALSE);
#endif
 waitUi(150);
 check(target->isVisible() && target->isExposed(),"Capture owner is visible and exposed");
#ifdef Q_OS_WIN
 check(GetForegroundWindow()==reinterpret_cast<HWND>(target->winId()),"Capture owner is the actual foreground HWND");
#endif
}
QRect physicalClient(QWindow *target) {
 RECT rect{}; POINT origin{}; auto handle=reinterpret_cast<HWND>(target->winId());
 check(GetClientRect(handle,&rect) && ClientToScreen(handle,&origin),"Read actual native client bounds");
 return QRect(origin.x,origin.y,rect.right,rect.bottom);
}
void desktop(const QString &name,QQuickItem *view,QWindow *active) {
 foreground(active); waitUi(350);
 const auto file=runRoot+"/desktop-full.png";
 QProcess capture;
 capture.start("C:/Users/david/.cache/chiaki-ui-native/ffmpeg/ffmpeg-n7.1-latest-win64-gpl-shared-7.1/bin/ffmpeg.exe", {"-hide_banner","-loglevel","error","-y","-f","lavfi","-i","ddagrab=output_idx=0:framerate=1","-vf","hwdownload,format=bgra","-frames:v","1","-update","1",file});
 check(capture.waitForFinished(15000) && capture.exitCode()==0,"DXGI Desktop Duplication capture completes");
 QImage full(file); const auto rect=physicalClient(window);
 check(!full.isNull() && full.rect().contains(rect),"Desktop image contains the exact complete native client");
 QImage image=full.copy(rect); QFile::remove(file);
 const auto dpr=window->devicePixelRatio(); const int background=view->property("qaBackground").toInt();
 const QColor marker=background==1?QColor("#00cc77"):QColor("#ff00ff");
 check(image.pixelColor(qRound(24*dpr),qRound(24*dpr))==marker,"Fresh desktop marker has the exact current app pixels");
 check(image.save(runRoot+"/"+name+"-desktop.png"),"Retain only the app client desktop crop");
 auto *quick=qobject_cast<QQuickWindow *>(active);
 const QImage gpu=quick?quick->grabWindow():window->nativeQaImage(); check(!gpu.isNull(),"Production renderer target readback succeeds");
 check(gpu.save(runRoot+"/"+name+"-target.png"),"Save separately identified component readback");
 captures.append(QJsonObject{{"state",name},{"desktop",name+"-desktop.png"},{"component",name+"-target.png"},{"method","FFmpeg ddagrab DXGI Desktop Duplication, exact Win32 client crop"},{"componentMethod",quick?"Separate QQuickWindow.grabWindow":window->runtimeRendererBackend()==1?"OpenGL render-control framebuffer":"Vulkan Qt Quick target GPU readback"},{"dpr",dpr},{"logicalWidth",window->width()},{"logicalHeight",window->height()},{"physicalWidth",image.width()},{"physicalHeight",image.height()},{"clientX",rect.x()},{"clientY",rect.y()},{"foregroundOwner",QString::number(active->winId())}});
}
void fits(QQuickItem *item) {
 check(item && item->width()>0 && item->height()>0,"Visible control has positive bounds");
 auto *target=owner(item); const QRectF rect(item->mapToScene({0,0}),item->size());
 check(QRectF(0,0,target->width(),target->height()).adjusted(-1,-1,1,1).contains(rect),"Control fits its actual native host");
 if(target!=window && target->type()==Qt::Popup) {
  const auto popup=physicalClient(target), main=physicalClient(window);
  check(main.adjusted(-2,-2,2,2).contains(popup),"Native popup fits the stream desktop client");
 }
}
void emptyHitTest() {
 auto *hwnd=reinterpret_cast<HWND>(window->winId()); POINT point{qRound(60*window->devicePixelRatio()),qRound(90*window->devicePixelRatio())};
 check(ClientToScreen(hwnd,&point),"Empty stream point maps to desktop");
 check(WindowFromPoint(point)==hwnd,"Empty stream region passes native hit testing to the game");
 POINT previous{}; GetCursorPos(&previous); const int presses=pointerPresses; SetCursorPos(point.x,point.y);
 INPUT events[2]{}; events[0].type=events[1].type=INPUT_MOUSE; events[0].mi.dwFlags=MOUSEEVENTF_LEFTDOWN; events[1].mi.dwFlags=MOUSEEVENTF_LEFTUP;
 check(SendInput(2,events,sizeof(INPUT))==2,"Windows sends a real pointer click outside the panels"); waitUi(80); SetCursorPos(previous.x,previous.y);
 evidence["pointerDiagnostics"]=QJsonObject{{"before",presses},{"after",pointerPresses},{"capture",QString::number(reinterpret_cast<quintptr>(GetCapture()))},{"foreground",QString::number(reinterpret_cast<quintptr>(GetForegroundWindow()))},{"main",QString::number(window->winId())},{"picture",bool(ui("streamFit"))},{"quality",bool(ui("streamCustom"))}};
 check(pointerPresses==presses+1,"Actual main stream window receives the outside pointer click");
}
class AcceptanceLabels : public QTranslator {
 QString translate(const char *context,const char *source,const char *,int) const override {
  const QByteArray name(context);
  if(name!="StreamMenuContent" && name!="StreamActionNotice" && name!="StreamPassiveNotices") return {};
  return QString::fromUtf8(source)+" extended translated label";
 }
};
void dragSlider(QQuickItem *item) {
 check(item,"Native drag slider exists"); auto *target=owner(item); const auto local=item->mapToScene({item->width()/2,item->height()/2});
 POINT center{qRound(local.x()*target->devicePixelRatio()),qRound(local.y()*target->devicePixelRatio())},previous{};
 check(ClientToScreen(reinterpret_cast<HWND>(target->winId()),&center),"Slider maps to native desktop"); GetCursorPos(&previous); SetCursorPos(center.x,center.y);
 INPUT down{}; down.type=INPUT_MOUSE; down.mi.dwFlags=MOUSEEVENTF_LEFTDOWN; check(SendInput(1,&down,sizeof(INPUT))==1,"Native slider press delivered"); waitUi(60);
 SetCursorPos(center.x+qRound(item->width()*target->devicePixelRatio()),center.y); waitUi(100);
 INPUT up{}; up.type=INPUT_MOUSE; up.mi.dwFlags=MOUSEEVENTF_LEFTUP; check(SendInput(1,&up,sizeof(INPUT))==1,"Native slider release delivered outside its bounds"); waitUi(100); SetCursorPos(previous.x,previous.y);
 check(item->property("value").toDouble()==item->property("to").toDouble(),"Native pointer drag clamps to slider maximum outside its bounds");
}
void acceptanceContracts() {
 for(auto *candidate:QGuiApplication::topLevelWindows()) if(auto *main=qobject_cast<QmlMainWindow *>(candidate)) window=main;
 check(window,"Real application main window exists"); backend=window->getBackend(); backend->stopAutoConnect();
 StreamPointerProbe pointerProbe; window->installEventFilter(&pointerProbe);
 check(!backend->discoveryEnabled() && !backend->qmlSession(),"Native startup has no discovery or session");
 window->QWindow::showNormal(); window->hide(); waitUi(80); window->QWindow::showNormal(); window->setPosition(20,20); foreground(window);
 const bool separate=window->runtimeRendererBackend()==1;
 evidence["runtimeRendererBackend"]=window->runtimeRendererBackend(); evidence["dpr"]=window->devicePixelRatio();
 check(separate==(qEnvironmentVariable("CHIAKI_QA_RENDERER")=="opengl"),"Actual renderer selects OpenGL separate or Vulkan inline route");
 check(targetIndex()>=0,"Synthetic console exists"); backend->connectToHost(targetIndex()); auto *session=backend->qmlSession(); check(session,"Real controlled StreamSession exists");
 event(session,CHIAKI_EVENT_CONNECTED); window->nativeQaVideoReady(); waitUi();
 auto *view=streamView(); view->forceActiveFocus(Qt::TabFocusReason);
 check(session->IsConnected() && backend->connectionAttempt().value("phase")=="ready","Real connected event reaches ready");
 // The native counter, unlike the supplied-state fixture, resets in sessionChanged.
 session->nativeQaBlocked(true); waitUi();
 check(ui("streamNoticeAction") && window->nativeQaGrabCount()==1 && session->nativeQaInputBlocked(),"Blocked notice captures native input once");
 session=backend->nativeQaReplace(true); waitUi(); window->nativeQaVideoReady(); waitUi(); view=streamView();
 check(ui("streamNoticeAction") && window->nativeQaGrabCount()==1 && session->nativeQaInputBlocked(),"Blocked-to-blocked session replacement preserves one native input owner");
 session->nativeQaBlocked(false); waitUi(); check(window->nativeQaGrabCount()==0 && !session->nativeQaInputBlocked(),"Cleared replacement notice releases native input once");
 QMetaObject::invokeMethod(window,"menuRequested"); waitUi(); check(ui("streamResume"),"Real stream shortcut opens menu");
 session=backend->nativeQaReplace(false); waitUi(); window->nativeQaVideoReady(); waitUi(); view=streamView();
 check(ui("streamResume") && window->nativeQaGrabCount()==1 && session->nativeQaInputBlocked(),"Open-menu replacement preserves native capture");
 key(window,Qt::Key_Escape); waitUi(); check(!ui("streamResume") && window->nativeQaGrabCount()==0 && !session->nativeQaInputBlocked(),"Replacement menu closes with balanced native input");
 QTimer samples; samples.setInterval(10); QObject::connect(&samples,&QTimer::timeout,window,[&] { session->nativeQaMetrics(); }); samples.start();
 for(const QSize size:{QSize(900,600),QSize(1280,720),QSize(1920,1080)}) {
  window->showNormal(); window->resize(size); window->setPosition(8,8);
  if(size==QSize(1920,1080) && window->devicePixelRatio()>=2) window->showFullScreen();
  waitUi(); check(window->size()==size,"Actual native logical size matches acceptance layout");
  view=streamView(); view->setProperty("qaBackground",size.width()==900?1:size.width()==1280?2:0);
  session->nativeQaMetrics(); window->nativeQaSeedStatistics(); waitUi(500);
  evidence["passiveDiagnostics"]=QJsonObject{{"stats",bool(ui("streamStatsReadout"))},{"audio",bool(ui("streamAudioNotice"))},{"network",bool(ui("streamNetworkNotice"))},{"loss",session->GetAveragePacketLoss()},{"ready",view->property("noticesReady").toBool()},{"phase",backend->connectionAttempt().value("phase").toString()}};
  check(ui("streamStatsReadout") && ui("streamAudioNotice") && ui("streamNetworkNotice"),"Statistics and both routine notices coexist");
  view->forceActiveFocus(Qt::TabFocusReason); foreground(window);
  check(window->nativeQaGrabCount()==0 && !session->nativeQaInputBlocked(),"Passive overlays never capture session input");
  check(view->hasActiveFocus(),"Passive overlays leave keyboard focus with the stream"); emptyHitTest();
  for(auto *item:{ui("streamStatsReadout"),ui("streamAudioNotice"),ui("streamNetworkNotice")}) {
   auto *target=owner(item); const auto p=item->mapToScene({item->width()/2,item->height()/2}); POINT point{qRound(p.x()*target->devicePixelRatio()),qRound(p.y()*target->devicePixelRatio())};
   ClientToScreen(reinterpret_cast<HWND>(target->winId()),&point);
   check(WindowFromPoint(point)==reinterpret_cast<HWND>(window->winId()),"Actual passive overlay center passes native pointer hit testing to the game");
  }
  fits(ui("streamStatsReadout")); fits(ui("streamAudioNotice")); fits(ui("streamNetworkNotice"));
  desktop(QString::number(size.width())+"-passive",view,window);
  QMetaObject::invokeMethod(window,"menuRequested"); waitUi();
  auto *resume=ui("streamResume"); check(resume && resume->hasActiveFocus(),"Menu initially focuses Resume");
  check(!ui("streamStatsReadout") && !ui("streamAudioNotice") && !ui("streamNetworkNotice"),"Menu suppresses statistics and routine notices");
  check(window->nativeQaGrabCount()==1 && session->nativeQaInputBlocked(),"Menu captures native input exactly once");
  fits(resume); emptyHitTest(); desktop(QString::number(size.width())+"-menu",view,owner(resume));
  key(window,Qt::Key_Right); check(ui("streamVolume")->hasActiveFocus(),"Main-window controller key reaches volume"); dragSlider(ui("streamVolume"));
  key(window,Qt::Key_Right); key(window,Qt::Key_Return); check(count("qaMute")>0,"Microphone handler reaches controlled audio boundary");
  key(window,Qt::Key_Right); key(window,Qt::Key_Return); waitUi();
  check(ui("streamFit") && ui("streamFit")->hasActiveFocus(),"Controller opens picture flyout");
  key(window,Qt::Key_Right); key(window,Qt::Key_Return); waitUi();
  check(window->videoMode()==QmlMainWindow::VideoMode::Zoom && ui("streamZoomFactor"),"Zoom exposes the actual adjustment");
  window->setZoomFactor(-1); waitUi(); fits(ui("streamDisplaySettings"));
  emptyHitTest();
  key(window,Qt::Key_Escape); waitUi(); check(!ui("streamFit") && ui("streamPicture")->hasActiveFocus(),"Keyboard Escape still reaches the popup after an outside native click");
  nativePointerClick(ui("streamPicture")); waitUi(); dragSlider(ui("streamZoomFactor"));
  desktop(QString::number(size.width())+"-zoom",view,owner(ui("streamFit")));
  const auto origin=window->position(); window->setPosition(origin+QPoint(14,9)); waitUi(); fits(ui("streamFit")); window->setPosition(origin); waitUi();
  key(window,Qt::Key_Escape); waitUi(); check(!ui("streamFit") && ui("streamPicture")->hasActiveFocus(),"Escape closes popup first and restores its opener");
  key(window,Qt::Key_Right); key(window,Qt::Key_Return); waitUi();
  check(ui("streamCustom"),"Quality flyout opens through main-window controller route");
  nativePointerClick(ui("streamPicture")); waitUi(); check(ui("streamFit") && !ui("streamCustom"),"Native pointer switches Quality to Picture");
  nativePointerClick(ui("streamPicture")); waitUi(); check(!ui("streamFit"),"Repeated native opener click closes Picture");
  nativePointerClick(ui("streamQuality")); waitUi();
  nativePointerClick(ui("streamCustom")); waitUi(); check(ui("streamRendererSettings"),"Custom exposes renderer settings");
  fits(ui("streamRendererSettings")); desktop(QString::number(size.width())+"-custom",view,owner(ui("streamCustom")));
  nativePointerClick(ui("streamAdvancedSpatial")); waitUi(); key(window,Qt::Key_Escape); waitUi();
  check(ui("streamQuality")->property("text").toString().contains("HQ + Advanced spatial"),"Longest quality preset stays identified");
  key(window,Qt::Key_Menu); waitUi();
  check(!ui("streamResume") && window->nativeQaGrabCount()==0 && !session->nativeQaInputBlocked(),"Menu key returns input with one balanced release");
  check(ui("streamStatsReadout") && ui("streamAudioNotice"),"Passive overlays return after menu closure");
  session->nativeQaBlocked(true); waitUi(); check(ui("streamNoticeAction") && window->nativeQaGrabCount()==1,"Blocked content owns input beside passive notices");
  const int homes=count("qaHome"); key(window,Qt::Key_Return); check(count("qaHome")==homes+1,"Blocked-content command reaches only intercepted home transport");
  fits(ui("streamNoticeAction")); desktop(QString::number(size.width())+"-blocked",view,owner(ui("streamNoticeAction")));
  session->nativeQaBlocked(false); waitUi(); check(window->nativeQaGrabCount()==0,"Cleared blocked notice releases input");
 }
 window->showFullScreen(); waitUi(); check(ui("streamStatsReadout") && window->nativeQaGrabCount()==0,"Fullscreen preserves passive overlays");
 QMetaObject::invokeMethod(window,"menuRequested"); waitUi(); fits(ui("streamResume")); nativePointerClick(ui("streamPicture")); waitUi(); fits(ui("streamFit"));
 key(window,Qt::Key_Escape); key(window,Qt::Key_Escape); waitUi(); window->showNormal(); window->resize(900,600); waitUi();
 AcceptanceLabels labels; QGuiApplication::installTranslator(&labels); qmlEngine(view)->retranslate(); waitUi();
 view=streamView(); view->setProperty("qaBackground",2); QMetaObject::invokeMethod(window,"menuRequested"); waitUi();
 fits(ui("streamResume")); fits(ui("streamDisconnect")); nativePointerClick(ui("streamPicture")); waitUi(); fits(ui("streamDisplaySettings"));
 desktop("900-long-zoom",view,owner(ui("streamFit"))); key(window,Qt::Key_Escape); nativePointerClick(ui("streamQuality")); nativePointerClick(ui("streamCustom")); waitUi(); fits(ui("streamRendererSettings"));
 desktop("900-long-custom",view,owner(ui("streamCustom"))); key(window,Qt::Key_Escape); key(window,Qt::Key_Escape); waitUi();
 QGuiApplication::removeTranslator(&labels); qmlEngine(view)->retranslate();
 backend->qmlSettings()->setAudioVideoDisabled(2); waitUi(); check(ui("streamNoticeTitle")->property("text")=="Video disabled","Disabled video uses accurate notice");
 backend->qmlSettings()->setAudioVideoDisabled(3); waitUi(); check(ui("streamNoticeTitle")->property("text")=="Audio and video disabled","Disabled audio and video use accurate notice");
 backend->qmlSettings()->setAudioVideoDisabled(1); waitUi();
 samples.stop(); backend->nativeQaEndReplacement(); waitUi();
 check(!backend->qmlSession() && !ui("streamResume") && !ui("streamStatsReadout") && !ui("streamNoticeAction"),"Session destruction removes every overlay");
 check(window->nativeQaGrabCount()==0,"Session destruction leaves no native input capture");
 evidence["allPassed"]=true; evidence["captures"]=captures; evidence["transport"]="Real Main/QmlBackend/QmlMainWindow/StreamSession; Start,Stop,GoToBed,GoHome,ToggleMute,SetLoginPIN intercepted before I/O";
 save(); QCoreApplication::quit();
}
