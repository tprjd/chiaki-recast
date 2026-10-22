#!/bin/bash
set -eo pipefail
: "${QT_ROOT_DIR:?}" "${RUNNER_TEMP:?}"
app=$1
helper=$2
dmg=$3
identity=-
sign_options=()
mode=ad-hoc
keychain=""
original_keychains=()
certificate=""
cleanup() {
  status=$?
  if [[ -n "$certificate" ]]; then rm -f "$certificate" || status=1; fi
  if [[ -n "$keychain" ]]; then
    security list-keychains -d user -s "${original_keychains[@]}" || status=1
    security delete-keychain "$keychain" || status=1
  fi
  exit "$status"
}
trap cleanup EXIT
# A partial release configuration must not silently produce an ad-hoc artifact.
configured="${MACOS_SIGNING_CERTIFICATE_P12:-}${MACOS_SIGNING_CERTIFICATE_PASSWORD:-}${MACOS_SIGNING_IDENTITY:-}${MACOS_NOTARY_APPLE_ID:-}${MACOS_NOTARY_TEAM_ID:-}${MACOS_NOTARY_PASSWORD:-}"
if [[ -n "$configured" ]]; then
  : "${MACOS_SIGNING_CERTIFICATE_P12:?}" "${MACOS_SIGNING_CERTIFICATE_PASSWORD:?}" "${MACOS_SIGNING_IDENTITY:?}"
  : "${MACOS_NOTARY_APPLE_ID:?}" "${MACOS_NOTARY_TEAM_ID:?}" "${MACOS_NOTARY_PASSWORD:?}"
  while IFS= read -r item; do original_keychains+=("$item"); done < <(security list-keychains -d user | sed -E 's/^[[:space:]]*"(.*)"$/\1/')
  keychain="$RUNNER_TEMP/chiaki-signing-$(uuidgen).keychain-db"
  password=$(uuidgen)
  certificate=$(mktemp "$RUNNER_TEMP/chiaki-certificate.XXXXXX")
  printf '%s' "$MACOS_SIGNING_CERTIFICATE_P12" | base64 --decode > "$certificate"
  security create-keychain -p "$password" "$keychain"
  security set-keychain-settings -lut 21600 "$keychain"
  security unlock-keychain -p "$password" "$keychain"
  security import "$certificate" -P "$MACOS_SIGNING_CERTIFICATE_PASSWORD" -k "$keychain" -T /usr/bin/codesign
  rm -f "$certificate"
  security list-keychains -d user -s "$keychain" "${original_keychains[@]}"
  security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k "$password" "$keychain"
  identity="$MACOS_SIGNING_IDENTITY"
  case "$identity" in "Developer ID Application:"*) ;; *) echo "A Developer ID Application identity is required" >&2; exit 1;; esac
  sign_options=(--options runtime --timestamp)
  mode=developer-id
fi
sdk_entitlements="$QT_ROOT_DIR/lib/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app/Contents/Resources/QtWebEngineProcess.entitlements"
test -s "$sdk_entitlements"
# Sign contained code first. Sign bundle directories only after their children.
while IFS= read -r -d '' binary; do
  if file -b "$binary" | grep -q 'Mach-O'; then
    entitlements=()
    case "$binary" in
      */QtWebEngineProcess) entitlements=(--entitlements "$sdk_entitlements");;
      "$app/Contents/MacOS/chiaki") entitlements=(--entitlements gui/entitlements.xml);;
    esac
    codesign --force --sign "$identity" "${sign_options[@]}" "${entitlements[@]}" "$binary"
  fi
done < <(find "$app" -depth -type f -print0)
while IFS= read -r -d '' bundle; do
  entitlements=()
  if [[ "$bundle" == *.app ]]; then entitlements=(--entitlements "$sdk_entitlements"); fi
  codesign --force --sign "$identity" "${sign_options[@]}" "${entitlements[@]}" "$bundle"
done < <(find "$app/Contents" -depth -type d \( -name '*.framework' -o -name '*.app' -o -name '*.xpc' \) -print0)
codesign --force --sign "$identity" "${sign_options[@]}" --entitlements gui/entitlements.xml "$app"
codesign --verify --deep --strict --verbose=2 "$app"
codesign --display --entitlements :- "$helper" > "$RUNNER_TEMP/chiaki-helper-entitlements.plist"
codesign --display --entitlements :- "$app" > "$RUNNER_TEMP/chiaki-app-entitlements.plist"
python - "$sdk_entitlements" "$RUNNER_TEMP/chiaki-helper-entitlements.plist" "$RUNNER_TEMP/chiaki-app-entitlements.plist" <<'PY'
import plistlib, sys
expected, helper, app = [plistlib.load(open(p, 'rb')) for p in sys.argv[1:]]
assert all(helper.get(k) == v for k, v in expected.items()), 'Signed helper lacks matching SDK entitlements'
assert app.get('com.apple.security.device.audio-input') is True, 'Signed app lacks microphone entitlement'
PY
# Signing is the last bundle mutation. Notarization staples the sealed DMG only.
hdiutil create -srcfolder "$app" -volname 'Chiaki Recast' -format UDZO "$dmg"
codesign --force --sign "$identity" "${sign_options[@]}" "$dmg"
codesign --verify --strict --verbose=2 "$dmg"
if [[ "$mode" == developer-id ]]; then
  xcrun notarytool submit "$dmg" --apple-id "$MACOS_NOTARY_APPLE_ID" --team-id "$MACOS_NOTARY_TEAM_ID" --password "$MACOS_NOTARY_PASSWORD" --wait --output-format json > macos-notary-result.json
  python -c 'import json; assert json.load(open("macos-notary-result.json"))["status"] == "Accepted", "Notarization was not accepted"'
  xcrun stapler staple "$dmg"
  xcrun stapler validate "$dmg"
  spctl --assess --type open --context context:primary-signature --verbose=2 "$dmg"
  mode=developer-id-notarized
fi
python - "$dmg" "$mode" <<'PY'
import hashlib, json, os, subprocess, sys
from pathlib import Path
p=Path(sys.argv[1])
p.with_suffix('.json').write_text(json.dumps(dict(source=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(), architecture=os.environ['MACOS_ARCH'], frontend_lock_sha256=hashlib.sha256(Path('gui/web/package-lock.json').read_bytes()).hexdigest(), qt='6.9.3', node='24.16.0', npm='11.13.0', signing=sys.argv[2], dmg=str(p), sha256=hashlib.sha256(p.read_bytes()).hexdigest(), runtime_checked=False),indent=2)+'\n')
PY
