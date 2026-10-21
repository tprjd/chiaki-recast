"""Reuse ticket 06's real-renderer contracts and assert shared return focus."""
from pathlib import Path
import shutil
import subprocess
import sys

here = Path(__file__).resolve().parent
repo = here.parents[3]
root = Path(r"C:/Users/david/.cache/chiaki-ui-native")
name = next((arg for arg in sys.argv[1:] if not arg.startswith("--")), "disconnect-prep-acceptance01")
prep = (root / name).resolve()
assert prep.parent == root.resolve() and name.startswith("disconnect-prep")
if "--refresh" not in sys.argv:
    subprocess.run([sys.executable, str(here.parent / "ticket06/prepare-native.py"), prep.name], cwd=repo, check=True)
for name in ["ConfirmDialog.qml", "SettingsDialog.qml"]:
    shutil.copyfile(repo / "gui/src/qml" / name, root / "source/gui/src/qml" / name)
text = (here.parent / "ticket06/native-contracts.cpp").read_text()
old = 'check(!ui("confirmationCancel"),"Escape dismisses shared confirmation");'
assert old in text
text = text.replace(old, old + '\n  auto *restored=navigation->window()->activeFocusItem(); check(restored && restored->isVisible() && restored->isEnabled() && restored->property("visualFocus").toBool(),"Shared confirmation restores visible native keyboard focus");')
old = 'capture(QString::number(size.width())+"-shared-reminder");'
assert old in text
text = text.replace(old, old + '''
  auto *reminder=root->findChild<QObject *>("startupReminder"); check(reminder,"Native reminder exists for long text");
  reminder->setProperty("text",QString("Read the complete translated instructions before you choose an action. ").repeated(120)); waitUi();
  auto *scroll=qvariant_cast<QQuickItem *>(reminder->property("contentItem")); check(scroll,"Native long reminder has a scroll view");
  auto *flick=qvariant_cast<QQuickItem *>(scroll->property("contentItem")); check(flick,"Native long reminder has actual scrolling content");
  check(scroll->property("contentHeight").toDouble()>scroll->height(),"Native reminder text overflows at the required size");
  key(owner(later),Qt::Key_PageDown); const auto lower=flick->property("contentY").toDouble(); check(lower>0,"Native controller shoulder key scrolls long text from Later"); bounds(later);
  capture(QString::number(size.width())+"-long-reminder-scroll");
  key(owner(later),Qt::Key_PageUp); check(flick->property("contentY").toDouble()<lower,"Native PageUp scrolls back while actions remain focused");
''')
(prep / "native-disconnect.cpp").write_text(text)
print("Prepared native disconnect and shared focus checks")
