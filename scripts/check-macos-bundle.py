#!/usr/bin/env python3
"""Check package layout. Mach-O, signatures, and embedded RCC checks run separately."""
import argparse
from pathlib import Path
import plistlib

HELPER = "Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app/Contents"
FRAMEWORKS = ("QtCore", "QtGui", "QtWidgets", "QtWebChannel", "QtWebEngineWidgets", "QtWebEngineCore", "QtQuick", "QtQml")
RESOURCES = ("icudtl.dat", "qtwebengine_resources.pak", "qtwebengine_resources_100p.pak", "qtwebengine_resources_200p.pak", "qtwebengine_devtools_resources.pak")


def check_bundle(app, arch="x86_64", sdk=None):
    if arch not in ("x86_64", "arm64"):
        raise ValueError("Unsupported macOS architecture: " + arch)
    def require(relative):
        path = app / relative
        if not path.is_file() or path.stat().st_size == 0:
            raise ValueError("Missing or empty package file: " + relative)
        return path
    helper = require(HELPER + "/MacOS/QtWebEngineProcess")
    if not helper.stat().st_mode & 0o111:
        raise ValueError("QtWebEngineProcess is not executable")
    for name in FRAMEWORKS:
        require("Contents/Frameworks/" + name + ".framework/" + name)
    for name in ("chiaki", "libSDL3.dylib", "libSDL2-2.0.0.dylib"):
        require("Contents/MacOS/" + name)
    require("Contents/PlugIns/platforms/libqcocoa.dylib")
    require("Contents/Resources/qt.conf")
    require("Contents/Resources/licenses/frontend/THIRD-PARTY-NOTICES.txt")
    require("Contents/Frameworks/QtWebEngineCore.framework/Resources/qtwebengine_locales/en-US.pak")
    resources = "Contents/Frameworks/QtWebEngineCore.framework/Resources/"
    names = set(RESOURCES) | {"v8_context_snapshot." + arch + ".bin"}
    if sdk:
        source = sdk / "lib/QtWebEngineCore.framework/Resources"
        names.update(str(p.relative_to(source)) for p in source.rglob("*.pak"))
    for name in sorted(names):
        require(resources + name)
    link = app / HELPER / "Frameworks"
    if not link.is_symlink() or link.resolve() != (app / "Contents/Frameworks").resolve():
        raise ValueError("QtWebEngineProcess Frameworks symlink does not resolve inside the package")
    require("Contents/Frameworks/vulkan")
    require("Contents/Resources/vulkan/icd.d/MoltenVK_icd.json")
    require("Contents/Resources/vulkan/icd.d/libMoltenVK.dylib")
    with require("Contents/Info.plist").open("rb") as stream:
        info = plistlib.load(stream)
    if not info.get("NSMicrophoneUsageDescription") or info.get("NSSupportsAutomaticGraphicsSwitching") is not True:
        raise ValueError("Missing microphone description or automatic graphics switching")
    for path in app.rglob("*"):
        if path.is_symlink() and (Path(path.readlink()).is_absolute() or not path.exists() or not path.resolve().is_relative_to(app.resolve())):
            raise ValueError("Broken or external bundle symlink: " + str(path))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("app", type=Path)
    parser.add_argument("arch", choices=("x86_64", "arm64"))
    parser.add_argument("--sdk", type=Path)
    args = parser.parse_args()
    try:
        check_bundle(args.app, args.arch, args.sdk)
    except (ValueError, OSError, plistlib.InvalidFileException) as error:
        parser.exit(1, str(error) + "\n")
    print("macOS bundle layout passed; native runtime is not checked")
