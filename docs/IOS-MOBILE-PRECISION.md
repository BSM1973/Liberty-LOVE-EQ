# LOVE EQ — iOS Mobile Precision (v0.3)
The v0.3 label identifies the mobile development phase, not an App Store release.
The shared DSP, parameter IDs, coefficient update cadence and factory presets remain unchanged.

- iOS only: cached Hann coefficients; adaptive analyzer 24/8/4 Hz; no hidden/browsing FFT outside active Match capture. Match retains 48 frames at 24 Hz. No measured battery improvement claimed yet.
- Native macOS reference QC: four sample rates, three block sizes, three instance counts; automated gain, Mid routing, enabled sidechain, in-process Relationship registry and state recall. This is a smoke/benchmark test, not an AUv3 host or sample-accurate automation test.
- Relationship EQ shares a process-local registry only. Separate AUv3 processes do not communicate. Do not promise universal multi-instance exchange.
- iOS build artifact is unsigned. Real device testing requires signing; configure Apple Distribution, distribution provisioning for the app and embedded extension, App Store Connect records, archive validation, then TestFlight. Never reuse Developer ID/notarization credentials as iOS distribution signing.
- On device: record CPU, memory and battery with Instruments, test mono/stereo, sidechain availability per host, host automation, state recall, portrait/landscape, accents and touch. Capture matched input/preset/output files and perform a level-aligned null comparison with the macOS reference. No iOS performance or sonic equivalence claims before measurement.
