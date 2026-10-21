"""Identify every staged production input and each explicit native QA override."""
from pathlib import Path
import hashlib
import json
import sys

here = Path(__file__).resolve().parent
repo = here.parents[3]
root = Path(r"C:/Users/david/.cache/chiaki-ui-native")
mode = sys.argv[1]
assert mode in ["disconnect", "integration", "normal"]
staging = sys.argv[2] if len(sys.argv) > 2 else "psn-acceptance-prep01"
files = json.loads((root / staging / "original-source.json").read_text())
digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
production = {name: digest(repo / name) for name in files}
actual = {name: digest(root / "source" / name) for name in files}
overrides = [name for name in files if production[name] != actual[name]]
allowed = {
    "normal": [],
    "integration": ["gui/CMakeLists.txt", "gui/src/main.cpp", "gui/src/settings.cpp", "gui/src/qmlbackend.cpp", "gui/src/jsonrequester.cpp", "gui/src/qmlmainwindow.cpp", "gui/include/qmlmainwindow.h"],
    "disconnect": ["gui/CMakeLists.txt", "gui/src/main.cpp", "gui/src/settings.cpp", "gui/src/qmlbackend.cpp", "gui/include/qmlbackend.h", "gui/src/streamsession.cpp", "gui/include/streamsession.h", "gui/src/sessionlog.cpp", "gui/src/qmlmainwindow.cpp", "gui/include/qmlmainwindow.h"],
}[mode]
assert sorted(overrides) == sorted(allowed), overrides
result = {"productionInputs": production, "stagedInputs": actual, "qaOverrides": overrides, "allOtherInputsMatchWorktree": True}
if mode != "normal":
    prep = sys.argv[3] if len(sys.argv) > 3 else "disconnect-prep-acceptance01" if mode == "disconnect" else "psn-setup-prep-acceptance02"
    name = prep + ("/native-disconnect.cpp" if mode == "disconnect" else "/contracts.cpp")
    result["qaHelperSha256"] = digest(root / name)
(here / f"{mode}-source.json").write_text(json.dumps(result, indent=2))
print(f"Checked {len(files)} inputs with {len(overrides)} explicit {mode} overrides")
