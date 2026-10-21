"""Retain the final native records and their actual rendered images."""
from pathlib import Path
import hashlib
import json
import shutil
import subprocess
import sys

here = Path(__file__).resolve().parent
root = Path(r"C:/Users/david/.cache/chiaki-ui-native")
records = {}
for label, run in {
    "integration": "psn-review-integration125",
    "vulkan": "psn-review-vulkan125",
    "opengl": "psn-review-opengl125",
}.items():
    data = json.loads((root / run / "contracts.json").read_text())
    assert data["allPassed"]
    target = here / label
    target.mkdir(exist_ok=True)
    for capture in data["captures"]:
        name = capture["file"]
        shutil.copyfile(root / run / name, target / name)
        capture["file"] = f"{label}/{name}"
        capture["sha256"] = hashlib.sha256((target / name).read_bytes()).hexdigest()
        assert capture["dpr"] == 1.25
    if label == "integration":
        by_name = {item["file"]: item["sha256"] for item in data["captures"]}
        for size in ["900x600", "1280x720"]:
            assert by_name[f"integration/form-{size}.png"] != by_name[f"integration/success-{size}.png"]
        data["distinctFormAndSuccessPixels"] = True
    data["nativeRunDirectory"] = str(root / run)
    (here / f"native-{label}.json").write_text(json.dumps(data, indent=2))
    records[label] = data["checks"]
    if label != "integration":
        checked = subprocess.check_output([sys.executable, str(here.parent / "ticket06/check-post-exit.py"), str(root / run)], text=True)
        (here / f"post-exit-{label}.json").write_text(json.dumps(json.loads(checked), indent=2))
print(json.dumps(records))
