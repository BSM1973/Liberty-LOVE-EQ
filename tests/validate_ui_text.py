#!/usr/bin/env python3
"""Reject broken text encodings and common mojibake in LOVE EQ UI source."""
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
sources = list((root / "Source").rglob("*.cpp")) + list((root / "Source").rglob("*.h"))
bad = []
suspicious = ["\ufffd", "Ã", "Â", "â€", "â„", "\u2028", "\u2029"]
for path in sources:
    raw = path.read_bytes()
    try:
        body = raw.decode("utf-8", "strict")
    except UnicodeDecodeError as ex:
        bad.append(f"{path.relative_to(root)}: invalid UTF-8 {ex}")
        continue
    for marker in suspicious:
        if marker in body:
            bad.append(f"{path.relative_to(root)}: suspicious decoded sequence {marker!r}")
    if any(ord(c) < 32 and c not in "\r\n\t" for c in body):
        bad.append(f"{path.relative_to(root)}: unexpected control character")

ui = (root / "Source" / "PluginEditor.cpp").read_text("utf-8")
header = (root / "Source" / "PluginEditor.h").read_text("utf-8")
for token in ('"LOVE EQ"', '"by Liberty Music Software"', 'setResizable(false,false)',
              'spectrumPanel=', 'bottomPanel=', 'footerPanel=', 'tabGraphic.setEnabled(false)'):
    if token not in ui:
        bad.append(f"UI contract missing: {token}")
for token in ("juce::TextButton tabParam", "juce::TextButton holdButton"):
    if token not in header:
        bad.append(f"UI component missing: {token}")

if bad:
    print("LOVE EQ UI text / layout checks FAILED:")
    for error in bad:
        print(" -", error)
    sys.exit(1)
print(f"LOVE EQ UI text / layout checks passed ({len(sources)} UTF-8 source files).")
