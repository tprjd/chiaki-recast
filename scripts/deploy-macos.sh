#!/bin/bash
set -euo pipefail
: "${QT_ROOT_DIR:?Matching official Qt SDK is required}"
: "${MACOS_ARCH:?Select x86_64 or arm64}"
: "${MACOS_DMG:=chiaki-ng.dmg}"
case "$MACOS_ARCH" in x86_64|arm64) ;; *) echo "Unsupported macOS architecture" >&2; exit 1;; esac
app="$PWD/chiaki-ng.app"
cp -a build/gui/chiaki.app "$app"
mkdir -p "$app/Contents/Resources/licenses"
cp -R macos-install/share/licenses/chiaki-ng/frontend "$app/Contents/Resources/licenses/"
cp COPYING "$app/Contents/Resources/licenses/"
"$QT_ROOT_DIR/bin/macdeployqt" "$app" -qmldir="$PWD/gui/src/qml" -libpath="$(brew --prefix)/lib" -libpath=/usr/local/lib
mkdir -p "$app/Contents/Resources/vulkan/icd.d"
curl --fail --location --retry 3 -o MoltenVK-macos.tar https://github.com/KhronosGroup/MoltenVK/releases/download/v1.2.9/MoltenVK-macos.tar
tar xf MoltenVK-macos.tar
cp MoltenVK/MoltenVK/dylib/macOS/* "$app/Contents/Resources/vulkan/icd.d/"
cp /usr/local/lib/libSDL3.dylib /usr/local/lib/libSDL2-2.0.0.dylib "$app/Contents/MacOS/"
for library in libSDL3.dylib libSDL2-2.0.0.dylib; do
  install_name_tool -id "@executable_path/$library" "$app/Contents/MacOS/$library"
done
for binary in "$app/Contents/MacOS/chiaki" "$app/Contents/MacOS/libSDL2-2.0.0.dylib"; do
  for library in libSDL3.dylib libSDL2-2.0.0.dylib; do
    for prefix in /usr/local/lib /opt/homebrew/lib @rpath; do
      if otool -L "$binary" | grep -Fq "$prefix/$library "; then
        install_name_tool -change "$prefix/$library" "@executable_path/$library" "$binary"
      fi
    done
  done
done
"$QT_ROOT_DIR/bin/macdeployqt" "$app" -qmldir="$PWD/gui/src/qml" -libpath="$(brew --prefix)/lib" -libpath=/usr/local/lib
helper="$app/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app"
# Framework Helpers is itself a symlink. Calculate from its physical location.
python - "$app" "$helper" <<'PY'
from pathlib import Path
import os, sys
app, helper = map(Path, sys.argv[1:])
link = helper / 'Contents/Frameworks'
if link.is_symlink(): link.unlink()
link.symlink_to(os.path.relpath(app / 'Contents/Frameworks', link.parent.resolve()))
PY
ln -s libvulkan.1.dylib "$app/Contents/Frameworks/vulkan"
python scripts/macos-notices.py "$app" "$QT_ROOT_DIR"
python scripts/check-macos-bundle.py "$app" "$MACOS_ARCH" --sdk "$QT_ROOT_DIR"
# Check every real Mach-O, including nested helpers and native libraries.
while IFS= read -r -d '' binary; do
  if file -b "$binary" | grep -q 'Mach-O'; then
    lipo -verify_arch "$MACOS_ARCH" "$binary"
    while IFS= read -r dependency; do
      case "$dependency" in
        /usr/lib/*|/System/Library/*|@rpath/*|@loader_path/*|@executable_path/*) ;;
        *) echo "External build-machine dependency: $binary -> $dependency" >&2; exit 1;;
      esac
    done < <(otool -L "$binary" | tail -n +2 | awk '{print $1}')
    if otool -l "$binary" | awk '/cmd LC_RPATH/ { getline; getline; print $2 }' | grep -Eq '^/'; then
      echo "Absolute runtime search path: $binary" >&2; exit 1
    fi
  fi
done < <(find "$app" -type f -print0)
bash scripts/sign-macos.sh "$app" "$helper" "$MACOS_DMG"
