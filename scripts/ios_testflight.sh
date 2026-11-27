#!/bin/bash
set -euo pipefail
: "${TEAM_ID:?Missing Apple team}" "${ASC_KEY_ID:?Missing API key ID}" "${ASC_ISSUER_ID:?Missing API issuer}" "${ASC_KEY:?Missing API private key}"
umask 077
keydir=$(mktemp -d)
keychain=""
profile_paths=()
cleanup() {
    if [[ -n "$keychain" ]]; then security delete-keychain "$keychain" >/dev/null 2>&1 || true; fi
    for path in "${profile_paths[@]}"; do rm -f "$path"; done
    rm -rf "$keydir"
}
trap cleanup EXIT
key="$keydir/AuthKey_${ASC_KEY_ID}.p8"
printf '%s\n' "$ASC_KEY" > "$key"
unset ASC_KEY
auth=(-allowProvisioningUpdates -authenticationKeyPath "$key" -authenticationKeyID "$ASC_KEY_ID" -authenticationKeyIssuerID "$ASC_ISSUER_ID")
sign_args=()
sign_style=Automatic
if [[ -n "${IOS_P12:-}" || -n "${IOS_APP_PROFILE:-}" || -n "${IOS_AUV3_PROFILE:-}" ]]; then
    : "${IOS_P12:?Missing certificate}" "${IOS_P12_PASSWORD:?Missing certificate password}" "${IOS_APP_PROFILE:?Missing app profile}" "${IOS_AUV3_PROFILE:?Missing AUv3 profile}"
    printf '%s' "$IOS_P12" | base64 -D > "$keydir/distribution.p12"
    printf '%s' "$IOS_APP_PROFILE" | base64 -D > "$keydir/app.mobileprovision"
    printf '%s' "$IOS_AUV3_PROFILE" | base64 -D > "$keydir/auv3.mobileprovision"
    keychain="$keydir/signing.keychain-db"
    keypass=$(openssl rand -hex 24)
    security create-keychain -p "$keypass" "$keychain"
    security set-keychain-settings -lut 3600 "$keychain"
    security unlock-keychain -p "$keypass" "$keychain"
    security import "$keydir/distribution.p12" -k "$keychain" -P "$IOS_P12_PASSWORD" -T /usr/bin/codesign
    security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k "$keypass" "$keychain" >/dev/null
    security list-keychains -d user -s "$keychain" "$HOME/Library/Keychains/login.keychain-db"
    security find-identity -v -p codesigning "$keychain" | grep -q 'Apple Distribution'
    mkdir -p "$HOME/Library/MobileDevice/Provisioning Profiles" "$HOME/Library/Developer/Xcode/UserData/Provisioning Profiles"
    for kind in app auv3; do
        security cms -D -i "$keydir/$kind.mobileprovision" > "$keydir/$kind.plist"
        uuid=$(/usr/libexec/PlistBuddy -c 'Print UUID' "$keydir/$kind.plist")
        profile_name=$(/usr/libexec/PlistBuddy -c 'Print Name' "$keydir/$kind.plist")
        for dir in "$HOME/Library/MobileDevice/Provisioning Profiles" "$HOME/Library/Developer/Xcode/UserData/Provisioning Profiles"; do
            dest="$dir/$uuid.mobileprovision"
            [[ ! -e "$dest" ]] || { echo "Profile already exists on runner"; exit 1; }
            cp "$keydir/$kind.mobileprovision" "$dest"
            profile_paths+=("$dest")
        done
        if [[ "$kind" == app ]]; then
            sign_args+=("-DLOVE_EQ_APP_PROFILE=$profile_name")
        else
            sign_args+=("-DLOVE_EQ_AUV3_PROFILE=$profile_name")
        fi
    done
    sign_style=Manual
fi
unset IOS_P12 IOS_P12_PASSWORD IOS_APP_PROFILE IOS_AUV3_PROFILE
cmake -S . -B build-ios-release -G Xcode \
  -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphoneos \
  -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
  -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM="$TEAM_ID" \
  -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE="$sign_style" \
  -DLOVE_EQ_IOS_BUILD_VERSION="${BUILD_NUMBER:-1}" "${sign_args[@]}"
project=$(find build-ios-release -maxdepth 1 -name '*.xcodeproj' -print -quit)
test -n "$project"
xcodebuild -project "$project" -list
xcodebuild -project "$project" -scheme LoveEQ_Standalone -configuration Release \
  -destination 'generic/platform=iOS' -archivePath "$PWD/build-ios-release/LOVE-EQ.xcarchive" \
  "${auth[@]}" archive
archive="$PWD/build-ios-release/LOVE-EQ.xcarchive"
app=$(find "$archive/Products/Applications" -maxdepth 1 -name '*.app' -print -quit)
ext=$(find "$app/PlugIns" -maxdepth 1 -name '*.appex' -print -quit)
python3 scripts/verify_ios_bundle.py "$app" "$ext"
codesign --verify --deep --strict "$app"
mkdir -p ios-distribution
options="$keydir/ExportOptions.plist"
python3 - "$options" "$TEAM_ID" "$sign_style" "$keydir" "$app" "$ext" "${BUILD_NUMBER:-1}" <<'PY'
import plistlib,sys,pathlib,datetime
options,team,style,keydir,app,ext,build=sys.argv[1:]
def read(p):
    with open(p,"rb") as f: return plistlib.load(f)
bundles=[read(pathlib.Path(p)/"Info.plist") for p in (app,ext)]
for b in bundles:
    if b.get("CFBundleVersion")!=build or b.get("CFBundleShortVersionString")!="0.8.0":
        raise SystemExit("Unexpected archive version/build number")
d=dict(method="app-store-connect",destination="export",signingStyle=style.lower(),teamID=team,manageAppVersionAndBuildNumber=False)
if style=="Manual":
    profiles={}
    for kind,bundle in zip(("app","auv3"),bundles):
        p=read(pathlib.Path(keydir)/(kind+".plist"))
        if p["ExpirationDate"]<=datetime.datetime.utcnow():
            raise SystemExit("Expired distribution profile")
        ent=p["Entitlements"]
        if ent.get("application-identifier")!=team+"."+bundle["CFBundleIdentifier"] or ent.get("get-task-allow",False):
            raise SystemExit("Profile does not match distribution bundle")
        profiles[bundle["CFBundleIdentifier"]]=p["UUID"]
    d.update(provisioningProfiles=profiles,signingCertificate="Apple Distribution")
with open(options,"wb") as f: plistlib.dump(d,f)
with open("ios-distribution/build-info.txt","w") as f:
    for b in bundles:
        f.write(b["CFBundleIdentifier"]+" "+b["CFBundleShortVersionString"]+" ("+b["CFBundleVersion"]+")\n")
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
