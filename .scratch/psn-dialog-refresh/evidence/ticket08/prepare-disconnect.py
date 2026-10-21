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
(prep / "native-disconnect.cpp").write_text(text)
print("Prepared native disconnect and shared focus checks")
