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
              'spectrumPanel=', 'bottomPanel=', 'footerPanel=', 'tabGraphic.onClick=[this]{selectTab(2)', 'languageBox.setBounds(', 'browserButton.setBounds(', 'peakResetButton.setBounds('):
    if token not in ui:
        bad.append(f"UI contract missing: {token}")
for token in ("juce::TextButton tabParam", "juce::TextButton holdButton", "juce::ComboBox languageBox", "std::unique_ptr<LoveEQPresetBrowser>"):
    if token not in header:
        bad.append(f"UI component missing: {token}")

if bad:
    print("LOVE EQ UI text / layout checks FAILED:")
    for error in bad:
        print(" -", error)
    sys.exit(1)


# Fail CI if the most frequent 13-inch layout collision regressions reappear.
layout_contract={
    "header_browse":(660,37,102,32),
    "header_lang":(769,37,82,32),
    "compare_a":(859,35,30,33),
    "compare_b":(894,35,30,33),
    "undo":(932,35,33,33),
    "redo":(971,35,33,33),
    "bypass":(1067,35,68,33),
    "reset":(1044,96,92,30),
    "graphic_tab":(243,97,107,29),
}
def collide(a,b):
    x,y,w,h=a; xx,yy,ww,hh=b
    return min(x+w,xx+ww)>max(x,xx) and min(y+h,yy+hh)>max(y,yy)
for key,a in layout_contract.items():
    for key2,b in layout_contract.items():
        if key<key2 and collide(a,b):
            bad.append(f"overlapping header controls: {key}, {key2}")
if bad:
    print("LOVE EQ layout contract FAILED")
    for error in bad:
        print(" -",error)
    sys.exit(1)

print(f"LOVE EQ UI text and layout checks passed ({len(sources)} UTF-8 source files).")
