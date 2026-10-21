void contracts() {
 for(auto *candidate:QGuiApplication::topLevelWindows()) if(auto *main=qobject_cast<QmlMainWindow *>(candidate)) window=main;
 check(window,"Production main window exists"); backend=window->getBackend(); backend->stopAutoConnect();
 check(!backend->discoveryEnabled() && !backend->qmlSession(),"Native startup has no discovery or session");
 window->QWindow::showNormal(); window->setPosition(40,40); window->raise(); window->requestActivate(); waitUi(500);
 const bool separate=window->runtimeRendererBackend()==1;
 evidence["runtimeRendererBackend"]=window->runtimeRendererBackend();
 check(separate==(qEnvironmentVariable("CHIAKI_QA_RENDERER")=="opengl"),"Actual renderer selects the expected statistics host");
 check(targetIndex()>=0,"Synthetic console exists"); backend->connectToHost(targetIndex());
 auto *session=backend->qmlSession(); check(session,"Real StreamSession is constructed");
 event(session,CHIAKI_EVENT_CONNECTED); window->nativeQaVideoReady(); waitUi();
 check(backend->connectionAttempt().value("phase")=="ready" && session->IsConnected(),"Controlled session reaches ready state");
 check(ui("streamStatsBitrate") && ui("streamStatsAge"),"All statistics are supplied through production StreamView");
 window->nativeQaSeedStatistics(); waitUi(20);
 check(ui("streamStatsDropped")->property("text")=="27" && ui("streamStatsQueue")->property("text")=="3.2","Real window metrics reach the production row");
 event(session,CHIAKI_EVENT_QUIT);
 check(!backend->qmlSession() && !ui("streamStatsReadout"),"Session teardown removes visible statistics");
 check(window->droppedFrames()==0 && window->queueDepthAverage()==0 && window->pendingFrameAge()==0,"Session teardown clears all window statistics");
 waitUi(1100);
 check(window->droppedFrames()==0,"Prior-session pending drop count cannot return on the next timer tick");
 backend->connectToHost(targetIndex()); session=backend->qmlSession(); check(session,"A replacement real StreamSession is constructed");
 event(session,CHIAKI_EVENT_CONNECTED); window->nativeQaVideoReady(); waitUi();
 check(ui("streamStatsDropped")->property("text")=="0" && ui("streamStatsQueue")->property("text")=="0.0" && ui("streamStatsAge")->property("text")=="0 ms","Replacement session starts with zero window metrics");
 for(const QSize size:{QSize(900,600),QSize(1280,720),QSize(1920,1080)}) {
  window->resize(size); window->setPosition(40,40); waitUi();
  auto *row=ui("streamStatsReadout"); check(row,"Statistics row is visible");
  const auto origin=row->mapToScene({0,0});
  check(qAbs(origin.x()-18)<1 && qAbs(window->height()-origin.y()-row->height()-18)<1,"Statistics row keeps the approved bottom-left margins");
  check(row->height()<=36 && row->width()<=window->width()-36,"Statistics remain compact and within the native stream");
  check(window->nativeQaGrabCount()==0 && !session->nativeQaInputBlocked(),"Statistics do not capture actual session input");
  auto *host=separate?row->window():static_cast<QQuickWindow *>(nullptr);
  if(separate) {
   check(host && host->objectName()=="streamStatsWindow" && host->isVisible() && host->transientParent()==window,"Separate statistics retain their actual native owner");
   check(host->flags().testFlag(Qt::WindowTransparentForInput) && host->flags().testFlag(Qt::WindowDoesNotAcceptFocus),"Separate statistics reject native pointer and keyboard focus");
#ifdef Q_OS_WIN
   const auto styles=GetWindowLongPtr(reinterpret_cast<HWND>(host->winId()),GWL_EXSTYLE);
   check((styles & WS_EX_TRANSPARENT) && (styles & WS_EX_NOACTIVATE),"Windows marks statistics as transparent for input and no-activate");
#endif
   check(qAbs(host->x()-window->x())<=1 && qAbs(host->y()-window->y())<=1,"Separate statistics track native movement and resizing");
  }
  const QImage image=separate?host->grabWindow():window->nativeQaImage();
  check(!image.isNull(),"Statistics capture reads the actual renderer target");
  const auto filename=QString::number(size.width())+"-statistics.png";
  check(image.save(runRoot+"/"+filename),"Statistics capture is saved");
  captures.append(QJsonObject{{"file",filename},{"dpr",window->devicePixelRatio()},{"logicalWidth",window->width()},{"logicalHeight",window->height()},{"physicalWidth",image.width()},{"physicalHeight",image.height()},{"method",separate?"Separate QQuickWindow.grabWindow":"Vulkan Qt Quick target GPU readback"}});
 }
 window->setPosition(91,73); waitUi();
 if(separate) {
  auto *host=ui("streamStatsReadout")->window();
  check(qAbs(host->x()-window->x())<=1 && qAbs(host->y()-window->y())<=1,"Statistics follow a native move without a resize");
 }
 window->showFullScreen(); waitUi();
 check(ui("streamStatsReadout") && window->nativeQaGrabCount()==0,"Fullscreen keeps statistics visible and passive");
 window->showNormal(); waitUi();
 check(ui("streamStatsReadout"),"Leaving fullscreen keeps statistics visible");
 backend->qmlSettings()->setShowStreamStats(false); waitUi(); check(!ui("streamStatsReadout"),"Native saved preference hides statistics");
 backend->qmlSettings()->setShowStreamStats(true); waitUi(); check(ui("streamStatsReadout"),"Native saved preference restores statistics");
 event(session,CHIAKI_EVENT_QUIT); check(!backend->qmlSession() && !ui("streamStatsReadout"),"Final teardown leaves no statistics readout");
 evidence["allPassed"]=true; evidence["captures"]=captures;
 evidence["transport"]="Real QmlBackend and StreamSession handlers, intercepted Start/Stop/GoToBed; test-only video-ready and metric inputs";
 save(); QCoreApplication::quit();
