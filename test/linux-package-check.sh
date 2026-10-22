#!/bin/bash
set -euo pipefail
checker=$(realpath "$(dirname "$0")/../scripts/check-linux-package.sh")
fixture=$(mktemp -d)
trap 'rm -rf "$fixture"' EXIT
reject() {
    if bash "$checker" "$fixture" > "$fixture/result" 2>&1; then
        echo "Incomplete package was accepted: $1" >&2
        exit 1
    fi
    grep -F "$1" "$fixture/result"
}
reject 'missing executable'
mkdir -p "$fixture/usr/bin"
cp /bin/true "$fixture/usr/bin/chiaki"
reject 'missing QtWebEngineProcess'
cp /bin/true "$fixture/usr/bin/QtWebEngineProcess"
chmod -x "$fixture/usr/bin/QtWebEngineProcess"
reject 'WebEngine helper is not executable'
chmod +x "$fixture/usr/bin/QtWebEngineProcess"
reject 'missing libQt6Widgets.so.6'
echo 'Linux package rejection checks passed'
