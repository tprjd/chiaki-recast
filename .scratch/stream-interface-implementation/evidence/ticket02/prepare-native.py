"""Run the approved toolbar through the real native input and transport boundaries."""
from pathlib import Path
import subprocess
import sys
import re
here = Path(__file__).resolve().parent
repo = here.parents[3]
cache = Path('C:/Users/david/.cache/chiaki-ui-native')
name = sys.argv[1]
subprocess.run([sys.executable, str(here.parent / 'ticket01/prepare-native.py'), name], cwd=repo, check=True)
p = cache / name / 'native-disconnect.cpp'
s = p.read_text()
s = s.replace('if (item->window()->objectName()=="streamMenuWindow") return item->window();', 'if (item->window()->objectName()=="streamMenuWindow" || item->window()->flags().testFlag(Qt::Popup)) return item->window();')
old = 'auto *menuAction=ui("streamDisconnect"); check(menuAction && menuAction->hasActiveFocus(),"Shared production menu focuses Disconnect in both renderer routes");'
assert old in s
s = s.replace(old, 'auto *menuAction=ui("streamResume"); check(menuAction && menuAction->hasActiveFocus(),"Shared production menu initially focuses Resume in both renderer routes");')
old = 'key(owner(menuAction),Qt::Key_Return); waitUi(600);'
assert old in s
s = s.replace(old, '''if(action==0 && size.width()==900) {
    key(window,Qt::Key_Right); check(ui("streamVolume")->hasActiveFocus(),"Controller-mapped main-window key reaches the active toolbar");
    key(window,Qt::Key_Right); check(ui("streamMicrophone")->hasActiveFocus(),"Controller navigation reaches the microphone");
    key(window,Qt::Key_Right); check(ui("streamPicture")->hasActiveFocus(),"Controller navigation reaches picture mode");
    key(window,Qt::Key_Return); waitUi();
    auto *fit=ui("streamFit"); check(fit && fit->hasActiveFocus(),"Controller opens the real picture flyout with initial focus");
    if(separate) {
     auto *toolbar=menuAction->window(); auto *popup=fit->window();
     check(toolbar->transientParent()==window && popup->transientParent()==toolbar,"Toolbar and flyout have the real native stream owner");
     check(toolbar->width()==820 && toolbar->height()>=78 && toolbar->height()<120,"Native toolbar has compact desktop bounds");
     check(popup!=toolbar && popup->y()+popup->height()<=toolbar->y(),"Native flyout is above its tight toolbar host");
     const QPoint original=window->position(), before=toolbar->position(); window->setPosition(original+QPoint(45,30)); waitUi();
     check(toolbar->position()-before==QPoint(45,30),"Toolbar follows actual native stream movement");
     check(popup->y()+popup->height()<=toolbar->y(),"Flyout stays above toolbar after native movement");
     window->resize(1000,650); waitUi(); check(qAbs(toolbar->x()-(window->x()+(window->width()-toolbar->width())/2))<=1,"Toolbar follows native resize");
     window->resize(size); window->setPosition(original); waitUi();
    }
    key(window,Qt::Key_Right); key(window,Qt::Key_Return); check(window->videoMode()==QmlMainWindow::VideoMode::Zoom,"Controller selects the real Zoom mode");
    key(window,Qt::Key_Escape); waitUi(); check(ui("streamPicture")->hasActiveFocus(),"Escape restores focus to the picture opener");
    check(!ui("streamFit") && window->nativeQaGrabCount()==grabs+1,"Escape dismisses only the flyout without releasing game input");
    key(window,Qt::Key_Right); key(window,Qt::Key_Return); waitUi(); check(ui("streamHighQuality")->hasActiveFocus(),"Controller opens the quality flyout");
    key(window,Qt::Key_Up); key(window,Qt::Key_Down); key(window,Qt::Key_Return); check(window->videoPreset()==QmlMainWindow::VideoPreset::HighQuality,"Controller selects the real saved quality preset");
    key(window,Qt::Key_Escape); waitUi(); check(ui("streamQuality")->hasActiveFocus(),"Escape restores quality opener focus");
    key(window,Qt::Key_Menu); waitUi(); check(!ui("streamResume") && window->nativeQaGrabCount()==grabs && !session->nativeQaInputBlocked(),"Controller menu key closes all menu windows and returns session input");
    check(stream->hasActiveFocus() && QGuiApplication::focusWindow()==window && window->isActive(),"Controller menu closure restores native stream focus");
    check(QMetaObject::invokeMethod(window,"menuRequested"),"Production menu reopens after controller closure"); waitUi();
   }
   menuAction=ui("streamDisconnect"); check(menuAction,"Named Disconnect entry exists"); menuAction->forceActiveFocus(Qt::TabFocusReason);
   key(window,Qt::Key_Return); waitUi(600);''')
# Dialog outcomes exercise the same target used by QmlController, after session input capture.
a=s.index('void contracts()')
s=s[:a]+s[a:].replace('key(owner(cancel),','key(window,').replace('key(owner(sleep),','key(window,').replace('key(owner(disconnect),','key(window,')
s = re.sub(r'ui\("([^"]+)"\)->hasActiveFocus\(\)', r'(ui("\1") && ui("\1")->hasActiveFocus())', s)
s = s.replace("void sharedPopups() {", 'void nativePointerClick(QQuickItem *item) {\n check(item,"Pointer target exists");\n#ifdef Q_OS_WIN\n auto *target=owner(item); const auto local=item->mapToScene({item->width()/2,item->height()/2});\n POINT point{qRound(local.x()*target->devicePixelRatio()),qRound(local.y()*target->devicePixelRatio())},previous;\n check(ClientToScreen(reinterpret_cast<HWND>(target->winId()),&point),"Pointer target maps to native desktop pixels");\n check(WindowFromPoint(point)==reinterpret_cast<HWND>(target->winId()),"Native hit testing selects the actual toolbar owner");\n GetCursorPos(&previous); SetCursorPos(point.x,point.y); INPUT events[2]{};\n events[0].type=events[1].type=INPUT_MOUSE; events[0].mi.dwFlags=MOUSEEVENTF_LEFTDOWN; events[1].mi.dwFlags=MOUSEEVENTF_LEFTUP;\n check(SendInput(2,events,sizeof(INPUT))==2,"Native Windows pointer events are delivered"); waitUi(); SetCursorPos(previous.x,previous.y);\n#else\n click(item);\n#endif\n}\n' + "void sharedPopups() {")
s = s.replace('    key(window,Qt::Key_Up); key(window,Qt::Key_Down); key(window,Qt::Key_Return); check(window->videoPreset()', '    nativePointerClick(ui("streamPicture")); check(ui("streamFit"),"Native pointer switches Quality to Picture without interception");\n    nativePointerClick(ui("streamPicture")); check(!ui("streamFit") && ui("streamPicture")->hasActiveFocus(),"Second native pointer activation closes the flyout");\n    key(window,Qt::Key_Right); key(window,Qt::Key_Return); waitUi();\n' + '    key(window,Qt::Key_Up); key(window,Qt::Key_Down); key(window,Qt::Key_Return); check(window->videoPreset()')
p.write_text(s)
print('Native toolbar, flyout, move/resize, dialog, and controller-target checks prepared')
