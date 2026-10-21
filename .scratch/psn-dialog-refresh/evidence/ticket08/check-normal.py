"""Check the final executable, runtime DLL, resources, and linked QA exclusion."""
from pathlib import Path
import hashlib
import json
import re
import subprocess
import xml.etree.ElementTree as ET

here = Path(__file__).resolve().parent
repo = here.parents[3]
root = Path(r"C:/Users/david/.cache/chiaki-ui-native")
source = root / "source"
bin_dir = root / "psn-dialog-final-bin"
digest = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
normal = json.loads((here / "normal-source.json").read_text())
assert not normal["qaOverrides"]
for name, expected in normal["productionInputs"].items():
    assert digest(source / name) == expected == digest(repo / name), name
resources = []
for name, generated in [
    ("gui/src/qml/qml.qrc", "gui/chiaki_autogen/VLDSMZLXNG/qrc_qml.cpp"),
    ("gui/res/resources.qrc", "gui/chiaki_autogen/PNK5WDWK6L/qrc_resources.cpp"),
]:
    qrc = source / name
    compiled = (root / "build" / generated).read_text()
    group_resources = []
    for group in ET.parse(qrc).getroot():
        prefix = group.attrib.get("prefix", "")
        for entry in group:
            path = (qrc.parent / entry.text).resolve()
            resource = "/" + "/".join(part.strip("/") for part in [prefix, entry.attrib.get("alias", entry.text)] if part.strip("/"))
            assert ":" + resource in compiled, resource
            group_resources.append(":" + resource)
            assert not re.search(r"fixture|prototype|tests/|native-contract|qa-", resource, re.I), resource
            resources.append({"resource": resource, "source": path.relative_to(source.resolve()).as_posix(), "sha256": digest(path)})
    for resource in set(line.strip() for line in re.findall(r"// (:/[^\r\n]+)", compiled)):
        assert resource in group_resources or any(item.startswith(resource.rstrip("/") + "/") for item in group_resources), resource
required = ["PSNLoginDialog.qml", "PSNTokenDialog.qml", "SteamShortcutDialog.qml", "RemindDialog.qml", "ConfirmDialog.qml", "RegistrationMethodDialog.qml", "RendererFallbackDialog.qml", "StreamDisconnectDialog.qml"]
for name in required:
    assert any(item["resource"].endswith("/" + name) for item in resources), name
symbols = subprocess.check_output([str(root / "msys64/ucrt64/bin/nm.exe"), "-C", str(bin_dir / "chiaki.exe")], text=True)
assert not re.search(r"initialize\w*Qa|nativeQa|qaJsonRequests|qaHostRefreshes", symbols)
dll = bin_dir / "libcpp-steam-tools.dll"
steam_commands = subprocess.check_output([str(root / "msys64/ucrt64/bin/ninja.exe"), "-C", str(root / "build"), "-t", "commands", "cpp-steam-tools"], text=True)
assert str(dll) in steam_commands or dll.as_posix() in steam_commands
assert "cpp-steam-tools-patched" in steam_commands
subprocess.run(["git", "apply", "--reverse", "--check", str(source / "cmake/steam-tools.patch")], cwd=root / "build/third-party/cpp-steam-tools-patched", check=True)
pins = {}
for name in subprocess.check_output(["git", "submodule", "status"], cwd=source, text=True).splitlines():
    parts = name.strip().split()
    revision, path = parts[0], parts[1]
    expected = subprocess.check_output(["git", "ls-tree", "HEAD", path], cwd=repo, text=True).split()[2]
    assert revision.lstrip("+-") == expected, path
    pins[path] = expected
result = {
    "integrationBase": subprocess.check_output(["git", "rev-parse", "codex/psn-dialog-integration"], cwd=repo, text=True).strip(),
    "inputCount": len(normal["productionInputs"]), "allProductionInputsMatch": True,
    "executable": str(bin_dir / "chiaki.exe"), "executableSha256": digest(bin_dir / "chiaki.exe"),
    "appLocalSteamDll": str(dll), "appLocalSteamDllSha256": digest(dll),
    "appLocalSteamDllMatchesBuild": True, "qaSymbolsAbsent": True,
    "patchedSteamSourceSha256": digest(root / "build/third-party/cpp-steam-tools-patched/src/steamtools.cpp"),
    "resources": resources, "requiredScreensPresent": True, "fixturesAndQaResourcesAbsent": True,
    "submodulePins": pins,
}
(here / "normal-build.json").write_text(json.dumps(result, indent=2))
print(f"Checked normal executable, app-local Steam DLL, {len(resources)} resources, {len(pins)} pinned submodules, and {len(normal['productionInputs'])} production inputs")
