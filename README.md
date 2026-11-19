# LOVE EQ

**by Liberty Music Software**

LOVE EQ is a new real-time professional equalizer for mixing and sound design. This is the **v0.1 development foundation**, not a released or commercially validated audio plugin.

## Implemented in v0.1

- Eight automation-ready parametric bands with bell, low/high shelf, high/low pass, notch and band-pass filters.
- High-pass/low-pass selectable 6, 12, 18, 24, 36 and 48 dB/octave slopes (cascaded IIR sections).
- Per-band stereo, left, right, mid and side routing, enable, solo-EQ-influence, frequency, gain and Q.
- Per-band dynamic bell/shelf EQ with attack, release, threshold, range and an optional external sidechain bus. Detector is band-pass filtered and dynamic coefficients update at a fixed control rate.
- Logarithmic EQ graph with eight draggable, colour-coded band nodes, numeric editors and input/output metering.
- Live FFT input/output spectrum (2048 samples) computed in the GUI, not on the audio thread.
- Bypass smoothing, parameter smoothing, state recall, host automation, A/B snapshots while the editor is open, undo/redo, and 100 original named starting-point presets in 13 categories.
- Mono and stereo, zero-reported-latency IIR EQ, macOS AU/VST3/Standalone CMake targets.
- Responsive metallic-dark interface designed to avoid overlaps and use correct French typography.

**Technical honesty:** The 100 factory presets are generated from deliberate category-specific EQ templates with individual variations. They are not yet auditioned or calibrated on real material. Dynamic filters in this foundation work at a 32-sample control interval; deep validation of automation/sidechain behavior is pending.

## Not yet implemented

Linear Phase, Natural Phase as a distinct processing algorithm, genuine 16/24/32-band graphic EQ, EQ Match, analyzer freeze/peak hold, external reference audio monitor, persistent A/B across sessions, automatic makeup gain, signed and notarized installers, and Windows CI. Those controls are intentionally not shown as functioning features.

## Build

Prerequisites: macOS with Xcode/CMake, JUCE downloaded automatically by CMake on first configure (requires network).

```sh
cmake -S . -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --config Release --target LoveEQ_All
ctest --test-dir build -C Release --output-on-failure
```

Generated plugin bundles will be under `build/LoveEQ_artefacts/Release/`. The GitHub Actions build packages unsigned bundles for testing, without pretending to notarize them.

## UI acceptance rules

Every future build must be checked on a 13-inch display: **no overlapping controls, no corrupted glyphs, correct French accents, all labels legible, no mandatory scrollbars or clipped windows**. Always test AU and VST3 processing before claiming a working release.

Copyright (c) 2026 Liberty Music Software. All rights reserved. No license grant is implied by public repository visibility.
