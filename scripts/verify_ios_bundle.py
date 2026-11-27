#!/usr/bin/env python3
"""Fail closed when the iOS containing app or AUv3 registration is malformed."""
import plistlib, pathlib, sys
app, extension = map(pathlib.Path,sys.argv[1:3])
def read(p):
    with (p/"Info.plist").open("rb") as f:
        return plistlib.load(f)
def check(ok, message):
    if not ok: raise SystemExit(message)
a,e=read(app),read(extension)
check(a.get("CFBundlePackageType")=="APPL","Invalid app package type")
check(e.get("CFBundlePackageType")=="XPC!","Invalid extension package type")
check(extension.parent == app/"PlugIns","AUv3 is not embedded in the app")
check(e.get("CFBundleIdentifier","").startswith(a["CFBundleIdentifier"]+"."),"Extension bundle identifier is not prefixed by app identifier")
n=e.get("NSExtension",{})
check(n.get("NSExtensionPointIdentifier") in ("com.apple.AudioUnit","com.apple.AudioUnit-UI"),"Missing Audio Unit extension point")
components=n.get("NSExtensionAttributes",{}).get("AudioComponents",[])
check(any(c.get("type")=="aufx" and c.get("subtype")=="LoEQ" and c.get("manufacturer")=="LbMs" for c in components),"LOVE EQ audio effect registration missing")
check({1,2}.issubset(set(a.get("UIDeviceFamily",[]))),"iPhone/iPad device family missing")
for key in ("UISupportedInterfaceOrientations","UISupportedInterfaceOrientations~ipad"):
    orientations=a.get(key,[])
    check("UIInterfaceOrientationPortrait" in orientations and "UIInterfaceOrientationLandscapeLeft" in orientations and "UIInterfaceOrientationLandscapeRight" in orientations,"Missing portrait/landscape: "+key)
print("PASS: app, embedded AUv3, identifiers, registration, device families, orientations")
