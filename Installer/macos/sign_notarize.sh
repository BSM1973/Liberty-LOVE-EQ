#!/usr/bin/env bash
# LOVE EQ: Pedal Legends / Organix Apple Developer ID + notarization method.
set -euo pipefail
version="${1:-0.1.0}"
[[ "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]] || { echo "::error::Invalid version"; exit 1; }
for key in APPLE_DEVELOPER_ID_APPLICATION_P12_BASE64 APPLE_DEVELOPER_ID_INSTALLER_P12_BASE64 APPLE_P12_PASSWORD APPLE_NOTARY_APPLE_ID APPLE_NOTARY_APP_PASSWORD APPLE_NOTARY_TEAM_ID; do
 if [[ -z "${!key:-}" ]]; then echo "::error::Missing Apple secret: $key"; missing=1; fi
done
if [[ "${missing:-0}" == 1 ]]; then
 echo "::error::Notarization credentials unavailable: refusing to publish an unsigned package as signed."
 exit 1
fi
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$root"
base="$root/build/LoveEQ_artefacts/Release"
au="$base/AU/LOVE EQ.component"
vst3="$base/VST3/LOVE EQ.vst3"
app="$base/Standalone/LOVE EQ.app"
for bundle in "$au" "$vst3" "$app"; do
 test -d "$bundle" || { echo "::error::Missing $bundle";exit 1; }
 exe="$(find "$bundle/Contents/MacOS" -maxdepth 1 -type f -print -quit)"
 test -n "$exe" || { echo "::error::No Mach-O inside $bundle";exit 1; }
 archs="$(/usr/bin/lipo -archs "$exe")"
 for arch in arm64 x86_64; do
  [[ " $archs " == *" $arch "* ]] || { echo "::error::Missing $arch in $bundle";exit 1; }
 done
done
keychain="$RUNNER_TEMP/love-eq-release.keychain-db"
password="$(/usr/bin/uuidgen)"
app_p12="$RUNNER_TEMP/love-eq-app.p12"
pkg_p12="$RUNNER_TEMP/love-eq-pkg.p12"
cleanup() {
 /bin/rm -f "$app_p12" "$pkg_p12"
 /usr/bin/security delete-keychain "$keychain" >/dev/null 2>&1 || true
}
trap cleanup EXIT
/usr/bin/security create-keychain -p "$password" "$keychain"
/usr/bin/security set-keychain-settings -lut 21600 "$keychain"
/usr/bin/security unlock-keychain -p "$password" "$keychain"
/usr/bin/security list-keychains -d user -s "$keychain"
/usr/bin/security default-keychain -s "$keychain"
printf '%s' "$APPLE_DEVELOPER_ID_APPLICATION_P12_BASE64" | /usr/bin/base64 -D > "$app_p12"
printf '%s' "$APPLE_DEVELOPER_ID_INSTALLER_P12_BASE64" | /usr/bin/base64 -D > "$pkg_p12"
/usr/bin/security import "$app_p12" -k "$keychain" -P "$APPLE_P12_PASSWORD" -T /usr/bin/codesign
/usr/bin/security import "$pkg_p12" -k "$keychain" -P "$APPLE_P12_PASSWORD" -T /usr/bin/pkgbuild
/usr/bin/security set-key-partition-list -S apple-tool:,apple: -s -k "$password" "$keychain"
app_id="$(/usr/bin/security find-identity -v -p codesigning "$keychain" | /usr/bin/awk -F '"' '/Developer ID Application:/{print $2;exit}')"
pkg_id="$(/usr/bin/security find-identity -v "$keychain" | /usr/bin/awk -F '"' '/Developer ID Installer:/{print $2;exit}')"
[[ -n "$app_id" && -n "$pkg_id" ]] || { echo "::error::Developer ID Application/Installer certificates missing";exit 1; }
[[ "$app_id" == *"($APPLE_NOTARY_TEAM_ID)"* && "$pkg_id" == *"($APPLE_NOTARY_TEAM_ID)"* ]] || { echo "::error::Developer ID certificate Team ID mismatch";exit 1; }
for bundle in "$au" "$vst3" "$app"; do
 /usr/bin/codesign --force --deep --options runtime --timestamp --sign "$app_id" "$bundle"
 /usr/bin/codesign --verify --deep --strict --verbose=2 "$bundle"
 signature="$(/usr/bin/codesign --display --verbose=4 "$bundle" 2>&1)"
 grep -Fq 'Authority=Developer ID Application:' <<< "$signature"
done
stage="$RUNNER_TEMP/love-eq-installer-root"
rm -rf "$stage"
mkdir -p "$stage/Library/Audio/Plug-Ins/Components" "$stage/Library/Audio/Plug-Ins/VST3" "$stage/Applications" "$root/release-output"
/usr/bin/ditto "$au" "$stage/Library/Audio/Plug-Ins/Components/LOVE EQ.component"
/usr/bin/ditto "$vst3" "$stage/Library/Audio/Plug-Ins/VST3/LOVE EQ.vst3"
/usr/bin/ditto "$app" "$stage/Applications/LOVE EQ.app"
for bundle in "$stage/Library/Audio/Plug-Ins/Components/LOVE EQ.component" "$stage/Library/Audio/Plug-Ins/VST3/LOVE EQ.vst3" "$stage/Applications/LOVE EQ.app";do
 /usr/bin/codesign --verify --deep --strict "$bundle"
done
pkg="$root/release-output/LOVE-EQ-v$version-macOS-Universal-SIGNED-NOTARIZED.pkg"
/usr/bin/pkgbuild --root "$stage" --identifier com.libertymusicsoftware.loveeq.installer --version "$version" --install-location / --ownership recommended --sign "$pkg_id" "$pkg"
/usr/sbin/pkgutil --check-signature "$pkg"
payload="$(/usr/sbin/pkgutil --payload-files "$pkg")"
for required in "Library/Audio/Plug-Ins/Components/LOVE EQ.component" "Library/Audio/Plug-Ins/VST3/LOVE EQ.vst3" "Applications/LOVE EQ.app";do
 grep -Fq "$required" <<< "$payload" || { echo "::error::Missing package content $required";exit 1; }
done
result="$RUNNER_TEMP/love-eq-notary-result.json"
/usr/bin/xcrun notarytool submit "$pkg" --apple-id "$APPLE_NOTARY_APPLE_ID" --password "$APPLE_NOTARY_APP_PASSWORD" --team-id "$APPLE_NOTARY_TEAM_ID" --wait --output-format json > "$result"
status="$(/usr/bin/python3 -c 'import json,sys;print(json.load(open(sys.argv[1])).get("status","Unknown"))' "$result")"
echo "Apple Notary Service: $status"
[[ "$status" == "Accepted" ]] || { echo "::error::Notarization rejected";exit 1; }
/usr/bin/xcrun stapler staple "$pkg"
/usr/bin/xcrun stapler validate "$pkg"
/usr/sbin/pkgutil --check-signature "$pkg"
/usr/sbin/spctl --assess --type install --verbose=2 "$pkg"
test -s "$pkg"
/usr/bin/shasum -a 256 "$pkg"
echo "::notice::APPLE ACCEPTED: LOVE EQ signed, stapled and notarized."
