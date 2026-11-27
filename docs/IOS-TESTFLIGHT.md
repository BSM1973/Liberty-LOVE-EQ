# LOVE EQ iOS — v0.5 TESTFLIGHT EDITION

## App Store Connect (account holder action)
Register the explicit app identifier com.libertymusicsoftware.loveeq and the AUv3 identifier emitted by JUCE. Confirm exact identifiers from the unsigned build or generated Info.plist; do not invent a suffix. Create an iOS app record in App Store Connect: name LOVE EQ (subject to availability), primary language French, SKU LIBERTY-LOVE-EQ-IOS, bundle ID matching the app. Confirm the paid-app agreement, tax and banking information before commercialization.

## Signing and build
GitHub environment: ios-distribution. Required existing secret names: IOS_TEAM_ID, IOS_ASC_KEY_ID, IOS_ASC_ISSUER_ID, IOS_ASC_PRIVATE_KEY. Automatic signing requires the appropriate Apple team permissions and a provisionable signing identity. Alternatively provide IOS_DISTRIBUTION_P12_BASE64, IOS_DISTRIBUTION_P12_PASSWORD, IOS_APP_PROFILE_BASE64 and IOS_AUV3_PROFILE_BASE64 together; the certificate must contain the private key and the two profiles must correspond to the exact app and extension IDs. Never send these in chat or commit them. On a new run choose a unique increasing build number (1–9999); iOS marketing version 0.5.0. Desktop version is untouched. Xcode 26 and iOS SDK 26 or newer are required for upload; minimum deployment target remains 16.0.
Run the manual workflow with upload_testflight=false first. Verify the signed IPA and build-info report, then choose a new build number and upload_testflight=true for a deliberate upload. App Store Connect processing and internal/external beta access must be verified separately. An App Store distribution IPA is not a general sideload installer. No store publication is automated.

## App icon
scripts/generate_ios_icon.py creates a 1024x1024 opaque RGB PNG in the iOS build directory, passed to JUCE ICON_BIG. Dark metal, gold EQ curve, red heart. The creative choice is a proposed identity until owner approval. Check the actual compiled asset catalog, small-size legibility and Apple validation.

## TestFlight acceptance
On actual iPad and iPhone test audio in Logic Pro for iPad, Cubasis, AUM, GarageBand and Loopy Pro as applicable; complete the v0.4 checklist. Test loading, save/reopen, automation, UI orientation, CPU and battery, interruptions and no audio regression. Review privacy disclosures and required-reason API usage including dependencies, and provide actual screenshots, support URL, privacy policy URL and export-compliance answers before submission. Do not claim no data collection without a full audit.
