# LOVE EQ 0.8 — MOBILE PROFESSIONAL QUALITY CONTROL
Status: pre-commercial beta. The v0.5.0 build 2 was tested by the owner on iPhone 15 Pro (sound, presets, analyzer, stability and AUv3 discovery satisfactory; small controls). v0.6 and v0.7 central diagnostics green. No evidence of newer device tests yet.
## Automated release gates
- UTF-8/UI contract, independent DSP, preset, layout and zoom tests.
- Native reference processor: 100 factory presets x 3 sample rates, serialized project state and equivalent stereo output; existing sidechain, mid/side, 1/4/8-instance and sample-rate restart simulations. 30 seconds simulated audio with four instances and parameter updates. Report compute time relative to buffer duration on the runner, not iOS CPU.
- iOS ARM64 app and embedded extension compilation, identifiers, version and registration checks. Signature/upload only through existing Release Center modes.
## Manual device qualification (record version, build, host version, iOS, device, audio route, sample rate/buffer and evidence)
- iPhone 15 Pro and iPad: portrait/landscape, minimum AUv3 host view, Studio/Touch, all pages, no clipped or overlapping labels, FR/EN, keyboard in preset browser, touch controls, pinch and 3rd finger, reset zoom, switching views during drag. No uncommitted audio change after closing Intent/Match.
- Standalone and each available host (Logic Pro iPad, Cubasis 3, AUM, GarageBand, Loopy Pro): discovery, insert, remove, reopen, automation read/write where host supports, project save/reopen, preset recall, sidechain if host exposes, mid/side, bypass, 1/4/8 instances. Mark unsupported host capability N/A with evidence, never pass by assumption. Relationship EQ only within same process.
- Route interruptions (headphones, Bluetooth, background/foreground, incoming system audio), sample-rate change, restore, no stale analyzer or stuck automation gesture.
- 30 min continuous per supported host, repeated editor open/close, playback and silence. Log crashes, audio dropouts and unexpected changes. Profile CPU, memory and energy on actual iOS hardware with Instruments or host meters; report baseline and conditions, not synthetic battery estimates.
- Compare against macOS using same source audio, preset, sample rate and gain staging; capture aligned renders, RMS/peak/difference; subjective listening across vocals, bass, drums and full mix. No claim of sample-identical iOS audio before measurement.
## App Store assets
Capture actual release-candidate screens (no simulated screenshots): iPhone EQ portrait, landscape, DYN, presets, Intent, Match; iPad Studio, EQ, advanced pages. No private data or host branding unless authorized. Check current App Store Connect accepted resolutions and required device classes on upload; do not upscale or misrepresent UI. FR/EN metadata in APP-STORE-CONNECT-DRAFT.md. Confirm export compliance, privacy declarations, merchant status, pricing and review notes before submission.
## Release decision
All critical issues fixed, required device/host tests recorded, signed IPA and TestFlight accepted. Owner approves commercial submission. No automatic publication.
