#!/usr/bin/env python3
"""Copy notices from the exact SDK, source dependencies, and shipped Homebrew kegs."""
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import urllib.request


def copy_notices(source, destination):
    notices = [p for p in source.rglob("*") if p.is_file() and re.match(r"^(license|copying|copyright|notice)([._-]|$)", p.name, re.I)]
    if not notices:
        raise ValueError("No license texts in shipped dependency: " + str(source))
    for path in notices:
        target = destination / path.relative_to(source)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, target)


def collect(app, sdk):
    output = app / "Contents/Resources/licenses"
    shutil.copyfile("LICENSES/AGPL-3.0-only-OpenSSL.txt", output / "AGPL-3.0-only-OpenSSL.txt")
    qt = output / "Qt"
    qt.mkdir()
    sboms = list((sdk / "sbom").glob("*.spdx"))
    if not any("qtwebengine-6.9.3" in p.name for p in sboms):
        raise ValueError("Matching Qt/Chromium SDK SPDX notices are missing")
    for path in sboms:
        shutil.copyfile(path, qt / path.name)
    for name in ("LGPL-3.0-only", "GPL-3.0-only", "GPL-2.0-only", "GFDL-1.3-no-invariants-only"):
        urllib.request.urlretrieve("https://raw.githubusercontent.com/qt/qtbase/v6.9.3/LICENSES/" + name + ".txt", qt / (name + ".txt"))
    native = output / "native"
    for name in ("cpp-steam-tools", "curl", "gf-complete", "jerasure", "nanopb"):
        copy_notices(Path("third-party") / name, native / name)
    for source in (Path("SDL3-3.4.0"), Path("sdl2-compat-2.32.64"), Path("MoltenVK")):
        copy_notices(source, native / source.name)
    cellar = Path(subprocess.check_output(["brew", "--cellar"], text=True).strip())
    installed = json.loads(subprocess.check_output(["brew", "info", "--json=v2", "--installed"], text=True))["formulae"]
    shipped = {p.name for p in app.rglob("*.dylib")}
    selected = []
    for formula in installed:
        for version in formula["installed"]:
            keg = cellar / formula["name"] / version["version"]
            if not any(p.name in shipped for p in keg.rglob("*.dylib")):
                continue
            copy_notices(keg, native / "homebrew" / formula["name"] / version["version"])
            selected.append(dict(name=formula["name"], version=version["version"], license=formula.get("license"), source=formula.get("urls")))
    (native / "homebrew-provenance.json").write_text(json.dumps(selected, indent=2) + "\n")


if __name__ == "__main__":
    try:
        collect(Path(sys.argv[1]), Path(sys.argv[2]))
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        sys.exit(str(error))
