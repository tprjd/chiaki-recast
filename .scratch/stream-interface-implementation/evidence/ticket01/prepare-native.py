"""Reuse isolated settings and transport interception with the production menu entry."""
from pathlib import Path
import subprocess
import sys
here = Path(__file__).resolve().parent
repo = here.parents[3]
cache = Path(r"C:/Users/david/.cache/chiaki-ui-native")
name = sys.argv[1]
subprocess.run([sys.executable, str(repo / ".scratch/psn-dialog-refresh/evidence/ticket06/prepare-native.py"), name], cwd=repo, check=True)
p = cache / name / "native-disconnect.cpp"
s = p.read_text()
s = s.replace('if(auto *quick=qobject_cast<QQuickWindow *>(candidate)) if(auto *found=named', 'if(auto *quick=qobject_cast<QQuickWindow *>(candidate)) if(candidate->objectName()!="streamMenuWindow" || candidate->isVisible()) if(auto *found=named')
s = s.replace('return dialog && item->window()==dialog?static_cast<QWindow *>(dialog):static_cast<QWindow *>(window);', 'if (item->window()->objectName()=="streamMenuWindow") return item->window(); return dialog && item->window()==dialog?static_cast<QWindow *>(dialog):static_cast<QWindow *>(window);')
old = 'check(!backend->closeRequested(),"Native close opens confirmation without closing host"); waitUi();'
assert old in s
s = s.replace(old, """check(QMetaObject::invokeMethod(window,"menuRequested"),"Native menu shortcut enters production StreamView"); waitUi();
   auto *menuAction=ui("streamDisconnect"); check(menuAction && menuAction->hasActiveFocus(),"Shared production menu focuses Disconnect in both renderer routes");
   check(window->nativeQaGrabCount()==grabs+1 && session->nativeQaInputBlocked(),"Menu captures actual session input once");
   key(owner(menuAction),Qt::Key_Return); waitUi(600);""")
p.write_text(s)
print("Native checks use the real menu entry and the existing isolated transport boundaries")
