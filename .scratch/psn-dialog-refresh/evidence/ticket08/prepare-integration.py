"""Reuse ticket 01's isolated native contracts, then add integrated native routes."""
from pathlib import Path
import subprocess
import sys

here = Path(__file__).resolve().parent
repo = here.parents[3]
name = next((arg for arg in sys.argv[1:] if not arg.startswith("--")), "psn-setup-prep-acceptance02")
if "--refresh" not in sys.argv:
    subprocess.run([sys.executable, str(here.parent / "ticket01-prepare-native.py"), name], cwd=repo, check=True)
contracts = Path(r"C:/Users/david/.cache/chiaki-ui-native") / name / "contracts.cpp"
text = (here.parent / "ticket01-native-contracts.cpp").read_text()
text = text.replace("void contracts() {", (here / "integrated-routes.cpp.inc").read_text() + "\nvoid contracts() {")
text = text.replace("QApplication::clipboard()->clear();", "integratedRoutes(backend,window);\n QApplication::clipboard()->clear();")
contracts.write_text(text)
print("Prepared integrated PSN, Settings, registration, information, and reminder checks")
