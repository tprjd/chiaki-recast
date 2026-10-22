#!/bin/bash
# Read-only checks. This does not start QApplication, discovery, or a session.
set -euo pipefail
root=$(realpath "${1:?Usage: check-linux-package.sh APPDIR-or-/app}")
fail() { echo "Package check failed: $*" >&2; exit 1; }
require_file() { test -s "$1" || fail "missing or empty $1"; }
find_file() {
    local file
    file=$(find "${search_roots[@]}" -name "$1" -type f,l -print -quit)
    test -n "$file" || fail "missing $1"
    require_file "$file"
    printf '%s\n' "$file"
}
search_roots=("$root")
if test "$root" = /app; then
    search_roots+=(/usr)
    exe=/app/bin/chiaki
else
    exe="$root/usr/bin/chiaki"
    export LD_LIBRARY_PATH="$root/usr/lib:$root/usr/lib/x86_64-linux-gnu"
fi
test -x "$exe" || fail "missing executable $exe"
helper=$(find_file QtWebEngineProcess)
test -x "$helper" || fail "WebEngine helper is not executable"
for file in libQt6Widgets.so.6 libQt6Core.so.6 libQt6Gui.so.6 libQt6WebEngineCore.so.6 libQt6WebEngineWidgets.so.6 libQt6WebChannel.so.6 libQt6Quick.so.6 libQt6Qml.so.6 qtwebengine_resources.pak qtwebengine_resources_100p.pak qtwebengine_resources_200p.pak icudtl.dat en-US.pak; do
    find_file "$file"
done
find_file libqtquick2plugin.so
find_file libqtquickcontrols2plugin.so
find_file COPYING
find_file THIRD-PARTY-NOTICES.txt
while IFS= read -r -d '' file; do
    case "$file" in */node|*/npm|*/npx) fail "build-only Node tool in runtime: $file";; esac
done < <(find "$root" -type f,l -print0)
if test "$root" = /app; then
    "$exe" --check-package
else
    require_file "$root/AppRun"
    "$root/AppRun" --check-package
fi
for file in "$exe" "$helper"; do
    dependencies=$(ldd "$file" 2>&1) || fail "cannot inspect $file: $dependencies"
    printf '%s\n' "$dependencies"
    if grep -q 'not found' <<< "$dependencies"; then fail "unresolved dependencies in $file"; fi
    if test "$root" != /app; then
        while read -r library arrow resolved rest; do
            case "$library" in libQt6*)
                case "$resolved" in "$root"/*) ;; *) fail "Qt dependency comes from the host: $library $resolved";; esac
            esac
        done <<< "$dependencies"
    fi
done
echo "Linux package files and embedded frontend passed. Desktop sandbox and GPU checks are separate."
