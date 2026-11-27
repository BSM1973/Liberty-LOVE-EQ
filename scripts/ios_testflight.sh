#!/bin/bash
set -euo pipefail
: "${TEAM_ID:?Missing Apple team}" "${ASC_KEY_ID:?Missing API key ID}" "${ASC_ISSUER_ID:?Missing API issuer}" "${ASC_KEY:?Missing API private key}"
umask 077
keydir=$(mktemp -d)
trap 'rm -rf "$keydir"' EXIT
key="$keydir/AuthKey_${ASC_KEY_ID}.p8"
printf '%s\n' "$ASC_KEY" > "$key"
unset ASC_KEY
auth=(-allowProvisioningUpdates -authenticationKeyPath "$key" -authenticationKeyID "$ASC_KEY_ID" -authenticationKeyIssuerID "$ASC_ISSUER_ID")
cmake -S . -B build-ios-release -G Xcode \
  -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphoneos \
  -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
  -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM="$TEAM_ID" \
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE=Automatic
project=$(find build-ios-release -maxdepth 1 -name '*.xcodeproj' -print -quit)
test -n "$project"
xcodebuild -project "$project" -list
xcodebuild -project "$project" -scheme LoveEQ_Standalone -configuration Release \
  -destination 'generic/platform=iOS' -archivePath "$PWD/build-ios-release/LOVE-EQ.xcarchive" \
  "${auth[@]}" CURRENT_PROJECT_VERSION="${GITHUB_RUN_NUMBER:-1}" archive
archive="$PWD/build-ios-release/LOVE-EQ.xcarchive"
app=$(find "$archive/Products/Applications" -maxdepth 1 -name '*.app' -print -quit)
ext=$(find "$app/PlugIns" -maxdepth 1 -name '*.appex' -print -quit)
python3 scripts/verify_ios_bundle.py "$app" "$ext"
codesign --verify --deep --strict "$app"
mkdir -p ios-distribution
options="$keydir/ExportOptions.plist"
python3 - "$options" "$TEAM_ID" <<'PY'
import plistlib,sys
with open(sys.argv[1],"wb") as f:
    plistlib.dump(dict(method="app-store-connect",destination="export",signingStyle="automatic",teamID=sys.argv[2],manageAppVersionAndBuildNumber=False),f)
PY
xcodebuild -exportArchive -archivePath "$archive" -exportPath "$PWD/ios-distribution" -exportOptionsPlist "$options" "${auth[@]}"
ipa=$(find ios-distribution -maxdepth 1 -name '*.ipa' -print -quit)
test -n "$ipa"
if [[ "${UPLOAD:-false}" == true ]]; then
    python3 - "$options" <<'PY'
import plistlib,sys
p=sys.argv[1]
with open(p,"rb") as f: d=plistlib.load(f)
d["destination"]="upload"
with open(p,"wb") as f: plistlib.dump(d,f)
PY
    xcodebuild -exportArchive -archivePath "$archive" -exportPath "$PWD/build-ios-release/upload" -exportOptionsPlist "$options" "${auth[@]}"
fi
