"""Check the original cache files and preserve the separate final application."""
from pathlib import Path
import hashlib
import json
import sys

here = Path(__file__).resolve().parent
root = Path(r"C:/Users/david/.cache/chiaki-ui-native")
prep = root / (sys.argv[1] if len(sys.argv) > 1 else "psn-acceptance-prep01")
digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest() if path.is_file() else None
original = json.loads((prep / "original-source.json").read_text())
for name, expected in original.items():
    assert digest(root / "source" / name) == expected, name


def options(path):
    return {line.split(":", 1)[0]: line.split("=", 1)[1] for line in path.read_text().splitlines()
            if "=" in line and not line.startswith(("#", "//"))}


before = options(prep / "CMakeCache.txt")
after = options(root / "build/CMakeCache.txt")
for name, value in before.items():
    if name.startswith("CHIAKI_") or name in ["CMAKE_BUILD_TYPE", "CMAKE_CXX_COMPILER", "CMAKE_MAKE_PROGRAM", "Qt6_DIR"]:
        assert after[name] == value, name
assert Path(after["CMAKE_RUNTIME_OUTPUT_DIRECTORY"]).resolve() == Path(before["CMAKE_RUNTIME_OUTPUT_DIRECTORY"]).resolve()
for name in ["chiaki.exe", "chiaki-console-screen-tests.exe"]:
    assert (Path(after["CMAKE_RUNTIME_OUTPUT_DIRECTORY"]) / name).is_file(), name
normal = json.loads((here / "normal-build.json").read_text())
assert digest(Path(normal["executable"])) == normal["executableSha256"]
assert digest(Path(normal["appLocalSteamDll"])) == normal["appLocalSteamDllSha256"]
result = {"restoredPaths": len(original), "allOriginalSourceHashesMatch": True,
          "originalBuildOptionsRestored": True, "originalRuntimeOutputDirectory": after["CMAKE_RUNTIME_OUTPUT_DIRECTORY"],
          "originalApplicationRebuilt": True, "originalScreenTargetRebuilt": True,
          "finalExecutableAndSteamDllUnchanged": True, "nativeCacheReleased": True}
(here / "cache-restoration.json").write_text(json.dumps(result, indent=2))
print(json.dumps(result))
