QQuickItem *visibleNamed(QQuickItem *item,const QString &name) {
 if(!item || !item->isVisible()) return nullptr;
 if(item->objectName()==name) return item;
 for(auto *child:item->childItems()) if(auto *found=visibleNamed(child,name)) return found;
 return nullptr;
}
QQuickItem *visibleUi(const QString &name) {
 for(auto *candidate:QGuiApplication::allWindows()) if(auto *quick=qobject_cast<QQuickWindow *>(candidate)) if(auto *found=visibleNamed(quick->contentItem(),name)) return found;
 return nullptr;
}
void integratedRoutes(QmlBackend *backend,QmlMainWindow *window) {
 auto *navigation=ui("mainNavigation"); check(navigation,"Integrated production navigation exists"); auto *root=navigation->parentItem();
 auto route=[&](const char *method) { check(QMetaObject::invokeMethod(root,method),"Integrated production route opens"); waitUi(450); };
 auto action=[&](const QString &name) { qInfo("Integrated action: %s",qPrintable(name)); auto *item=visibleUi(name); check(item && item->isEnabled(),"Integrated action is visible and enabled"); item->forceActiveFocus(Qt::TabFocusReason); waitUi(); check(item->hasActiveFocus(),"Integrated action receives native focus"); key(window,Qt::Key_Return); waitUi(400); };
 auto setup=[&] { check(QMetaObject::invokeMethod(root,"showRegistDialog",Q_ARG(QVariant,QString("192.0.2.42")),Q_ARG(QVariant,true),Q_ARG(QVariant,QVariant())),"Native registration route opens"); waitUi(450); check(visibleUi("registrationDialog"),"Native registration is current"); };
 auto submit=[&] { auto *field=visibleUi("psnRedirect"); check(field,"Native registration sign-in has an external fallback"); field->setProperty("text",redirect()); action("psnSubmit"); check(phase(backend)=="exchanging","Integrated form starts native authentication"); };
 for(const QSize size:{QSize(900,600),QSize(1280,720)}) {
  window->resize(size); waitUi(); route("showSettingsDialog"); auto *settings=visibleUi("settingsDialog"); check(settings,"Native Settings route opens"); settings->setProperty("currentCategory",7); waitUi();
  if(visibleUi("resetPsnTokens")) action("resetPsnTokens");
  action("openPsnLogin"); check(visibleUi("psnSetup") && visibleUi("psnHeading")->property("text")=="Set up PSN remote access","Settings enters initial PSN setup");
  action("psnClose"); check(visibleUi("settingsDialog") && visibleUi("openPsnLogin")->hasActiveFocus(),"Unsubmitted PSN Cancel returns focus to Settings");
  settings->setProperty("currentCategory",8); waitUi(); action("aboutSettings"); auto *close=visibleUi("confirmationCancel"); check(close && close->property("text")=="Close" && close->hasActiveFocus() && close->property("visualFocus").toBool(),"Native About starts with visible Close focus");
  capture(window,QString("about-%1").arg(size.width()));
  auto *opener=ui("aboutSettings"); key(window,Qt::Key_Return); waitUi(350);
  check(opener->hasActiveFocus() && opener->property("visualFocus").toBool(),"Native About restores visible keyboard focus to its opener");
  key(window,Qt::Key_Return); waitUi(350); check(visibleUi("confirmationCancel") && visibleUi("confirmationCancel")->hasActiveFocus(),"Native About reopens with Return from its restored opener");
  opener->setEnabled(false); key(window,Qt::Key_Return); waitUi(350);
  auto *focus=settings->window()->activeFocusItem(); auto *ancestor=focus; while(ancestor && ancestor!=settings) ancestor=ancestor->parentItem();
  check(ancestor==settings && focus && focus->isVisible() && focus->isEnabled() && focus->activeFocusOnTab() && focus->property("visualFocus").toBool(),"Main fallback restores visible keyboard focus in current Settings after disabled About opener"); opener->setEnabled(true);
  auto *search=visibleUi("settingsSearch"); search->forceActiveFocus(Qt::TabFocusReason);
  check(QMetaObject::invokeMethod(root,"showRendererFallbackDialog",Q_ARG(QVariant,QString("Controlled native renderer reason"))),"Native renderer information route opens"); waitUi();
  close=visibleUi("confirmationCancel"); check(close && close->hasActiveFocus(),"Native renderer notice has initial Close focus"); capture(window,QString("renderer-%1").arg(size.width())); search->setEnabled(false); key(window,Qt::Key_Escape); waitUi(350);
  focus=settings->window()->activeFocusItem(); ancestor=focus; while(ancestor && ancestor!=settings) ancestor=ancestor->parentItem();
  check(ancestor==settings && focus && focus->isVisible() && focus->isEnabled() && focus->activeFocusOnTab() && focus->property("visualFocus").toBool(),"Main fallback restores visible keyboard focus in current Settings after disabled renderer opener"); search->setEnabled(true);
  action("settingsBack"); check(!visibleUi("settingsDialog"),"Settings returns to Library");
  backend->qmlSettings()->setPsnAccountId(""); setup(); action("registrationLogin"); submit(); respond(tokens()); respond({{"user_id","2"}}); waitUi(450);
  check(visibleUi("registrationAccountId") && visibleUi("registrationAccountId")->property("text")=="AgAAAAAAAAA=" && visibleUi("registrationAccountId")->hasActiveFocus(),"Native sign-in returns Account ID and focus to original registration");
  action("registrationCancel");
  backend->qmlSettings()->setPsnAccountId(""); setup(); action("registrationLogin"); submit(); action("psnClose"); action("registrationCancel"); setup(); action("registrationLogin");
  check(visibleUi("psnSetup") && phase(backend)=="exchanging","Replacement registration observes existing request"); respond(tokens()); respond({{"user_id","3"}}); waitUi(450);
  check(visibleUi("psnSetup"),"Late result cannot close replacement sign-in screen"); action("psnClose");
  check(visibleUi("registrationAccountId")->property("text").toString().isEmpty(),"Late result does not fill replacement registration");
  { Settings saved(QString{}); check(saved.GetPsnAccountId()=="AwAAAAAAAAA=","Dismissed original request can still save in its original profile"); }
  action("registrationCancel");
 }
 route("showSettingsDialog"); auto *controllers=visibleUi("settingsDialog"); controllers->setProperty("currentCategory",6); waitUi(); action("controllerMappingChange");
 auto *controllerCapture=controllers->findChild<QObject *>("controllerCapture"); check(controllerCapture && controllerCapture->property("opened").toBool(),"Native controller capture opens");
 backend->controllerMappingSteamControllerSelected(); waitUi(450); auto *steamNotice=controllers->findChild<QObject *>("steamControllerNotice"); check(steamNotice && steamNotice->property("opened").toBool(),"Controlled Steam controller event opens real native notice");
 key(window,Qt::Key_Escape); waitUi(350); auto *mappingOpener=visibleUi("controllerMappingChange"); check(mappingOpener && mappingOpener->hasActiveFocus() && mappingOpener->property("visualFocus").toBool(),"Native Steam controller notice restores visible keyboard focus");
 key(window,Qt::Key_Return); waitUi(350); check(controllerCapture->property("opened").toBool(),"Native Steam controller notice permits keyboard reopening"); key(window,Qt::Key_Escape); waitUi(350); action("settingsBack");
 // Exercise the real startup chain and the real Steam command with a temporary Steam tree.
 backend->qmlSettings()->setPsnAuthToken(""); backend->qmlSettings()->setPsnRefreshToken(""); backend->qmlSettings()->setPsnAuthTokenExpiry(""); backend->qmlSettings()->setPsnAccountId("");
 backend->qmlSettings()->setAddSteamShortcutAsk(true); backend->qmlSettings()->setRemotePlayAsk(true); root->setProperty("initialAsk",false); route("showMainView");
 auto *reminder=root->findChild<QObject *>("startupReminder"); check(reminder && reminder->property("visible").toBool() && reminder->property("title")=="Steam shortcut","Native startup offers Steam first");
 action("confirmationCancel"); check(reminder->property("visible").toBool() && reminder->property("title")=="PSN sign-in" && backend->qmlSettings()->addSteamShortcutAsk(),"Steam Later preserves preference and offers PSN next"); action("confirmationCancel"); check(backend->qmlSettings()->remotePlayAsk(),"PSN Later preserves preference");
 root->setProperty("initialAsk",false); route("showMainView"); action("confirmationAction"); auto *steam=visibleUi("steamShortcutDialog"); check(steam,"Startup Set up now opens actual Steam form");
 const QString base=runRoot+"/steam";
 auto write=[&](const QString &path,const QByteArray &data) { QDir().mkpath(QFileInfo(path).absolutePath()); QFile file(path); check(file.open(QIODevice::WriteOnly) && file.write(data)==data.size(),"Temporary Steam fixture writes"); };
 write(base+"/config/loginusers.vdf","\"users\"\n{\n\"76561197960265729\"\n{\n\"MostRecent\" \"1\"\n}\n}\n");
 write(base+"/steamapps/common/Steam Controller Configs/1/config/configset_controller_neptune.vdf","\"controller_mappings\"\n{\n}\n");
 steam->setProperty("steamBasePath",base); action("steamCreate"); check(steam->property("phase")=="success" && !backend->qmlSettings()->addSteamShortcutAsk(),"Actual temporary Steam write establishes terminal success and disables Steam reminder");
 check(QFileInfo::exists(base+"/userdata/1/config/shortcuts.vdf") && QDir(base+"/userdata/1/config/grid").entryList({"*.png"},QDir::Files).size()==5,"Integrated Steam writes shortcut and five artwork files");
 action("steamDone"); check(reminder->property("visible").toBool() && reminder->property("title")=="PSN sign-in","Steam terminal completion offers the eligible PSN reminder"); action("confirmationAction"); submit(); respond(tokens()); respond({{"user_id","1"}}); waitUi(); action("psnClose");
 check(!backend->qmlSettings()->remotePlayAsk() && !reminder->property("visible").toBool(),"PSN terminal completion disables reminder without a duplicate prompt");
}
