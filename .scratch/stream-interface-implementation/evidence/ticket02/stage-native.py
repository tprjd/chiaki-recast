"""Stage the integrated build inputs and restore every overwritten cache file."""
from pathlib import Path
import hashlib
import json
import os
import shutil
import subprocess
import sys

ROOT = Path(r"C:/Users/david/.cache/chiaki-ui-native")
SOURCE = ROOT / "source"
WORK = Path(__file__).resolve().parents[4]
name = next((arg for arg in sys.argv[1:] if not arg.startswith("--")), "stream-interface-prep01")
PREP = (ROOT / name).resolve()
assert PREP.parent == ROOT.resolve() and name.startswith("stream-interface-prep")


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest() if path.is_file() else None


if len(sys.argv) > 1 and sys.argv[1] == "--restore":
    manifest = json.loads((PREP / "original-source.json").read_text())
    for name, expected in manifest.items():
        target = (SOURCE / name).resolve()
        assert target.is_relative_to(SOURCE.resolve())
        if expected is None:
            if target.exists():
                target.unlink()
        else:
            shutil.copyfile(PREP / "backup" / name, target)
            os.utime(target, None)
        assert digest(target) == expected, name
    shutil.copyfile(PREP / "CMakeCache.txt", ROOT / "build/CMakeCache.txt")
    print(json.dumps({"restoredPaths": len(manifest), "allOriginalHashesMatch": True}))
    sys.exit(0)

if "--refresh" in sys.argv:
    assert PREP.is_dir()
    original = json.loads((PREP / "original-source.json").read_text())
else:
    PREP.mkdir(exist_ok=False)
    original = {}
paths = subprocess.check_output(
    ["git", "ls-files", "CMakeLists.txt", "cmake", "third-party", "gui", "lib", "include", "cli"],
    cwd=WORK, text=True,
).splitlines()
paths = [name for name in paths if (WORK / name).is_file()]
for name in paths:
    target = SOURCE / name
    if name not in original:
        original[name] = digest(target)
        if target.exists():
            backup = PREP / "backup" / name
            backup.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(target, backup)
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(WORK / name, target)
    os.utime(target, None)
    assert digest(target) == digest(WORK / name), name
(PREP / "original-source.json").write_text(json.dumps(original, indent=2))
(PREP / "production-source.json").write_text(json.dumps({name: digest(SOURCE / name) for name in paths}, indent=2))
if not (PREP / "CMakeCache.txt").exists():
    shutil.copyfile(ROOT / "build/CMakeCache.txt", PREP / "CMakeCache.txt")
print(f"Staged and checked {len(paths)} integrated build inputs at {PREP}")
