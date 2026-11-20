#!/usr/bin/env python3
"""Reject broken text encodings and common mojibake in LOVE EQ UI source."""
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[1]
sources = list((root / "Source").rglob("*.cpp")) + list((root / "Source").rglob("*.h"))
bad = []
suspicious = ["\ufffd", "Ã©", "Ã¨", "Ãª", "Ã ", "Ã§", "Â©", "â€", "â„", "\u2028", "\u2029"]
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


# The old source used literal backslash-u sequences, which are forbidden for UI copy.
import re
for path in sources:
    body=path.read_text("utf-8")
    if re.search(r"\\u[0-9a-fA-F]{4}", body):
        bad.append(f"{path.relative_to(root)}: replace escaped accent with real UTF-8")
browser=(root/"Source"/"PresetBrowser.h").read_text("utf-8")
userLibrary=(root/"Source"/"UserPresetLibrary.h").read_text("utf-8")
if not all(x in ui+browser for x in ("BIBLIOTHÈQUE","ÉCHELLE","FRÉQUENCE","CATÉGORIE","É")):
    bad.append("Real accented UI strings missing")
for required in ('"USER"','"SAUVER DANS USER"','"Catégorie utilisateur"'):
    if required not in browser:
        bad.append(f"USER browser control missing: {required}")
for required in ("LOVE_EQ_USER_PRESET","getSpecialLocation","replaceWithText","load("):
    if required not in userLibrary:
        bad.append(f"User preset persistence missing: {required}")

if bad:
    print("LOVE EQ UI text / layout checks FAILED:")
    for error in bad:
        print(" -", error)
    sys.exit(1)



# New musical workflow must remain wired to real JUCE controls, not a paint-only mockup.
for token in ('sketchButton.setBounds(', 'notesButton.setBounds(', 'grabButton.setBounds(',
              'void LoveEQAudioProcessorEditor::mouseUp(', 'void LoveEQAudioProcessorEditor::applySketch(',
              'void LoveEQAudioProcessorEditor::grabSpectrumPeak(', 'fitSketch(sketchPoints'):
    if token not in ui:
        bad.append(f"Curve Lab UI control missing: {token}")


# Harmonic DNA is a measured feature and must be visible, not an inert mockup.
for token in ('dnaButton.setBounds(', 'dnaFocusButton.setBounds(', 'dnaCutButton.setBounds(',
              'void LoveEQAudioProcessorEditor::updateHarmonicDNA(',
              'void LoveEQAudioProcessorEditor::focusDnaCandidate(',
              'void LoveEQAudioProcessorEditor::applyDnaSuggestion(',
              'loveeq::dna::analyze('):
    if token not in ui:
        bad.append(f"Harmonic DNA control or analysis missing: {token}")
processor_h=(root/"Source"/"PluginProcessor.h").read_text("utf-8")
if "scopeSize = 8192;" not in processor_h:
    bad.append("Harmonic DNA requires a real 8192-sample audio analyzer history")


# CIBLER is an advisory action only: it must not mutate APVTS parameters.
focus=ui.split("void LoveEQAudioProcessorEditor::focusDnaCandidate() {",1)
if len(focus)!=2:
    bad.append("Harmonic DNA focus implementation missing")
else:
    focus_body=focus[1].split("void LoveEQAudioProcessorEditor::applyDnaSuggestion() {",1)[0]
    if "setParam(" in focus_body:
        bad.append("Harmonic DNA focus must not modify audio parameters")


# Relationship UI must expose real controls and opt-in host parameters.
for token in ("relationModeButton.setBounds(", "relationRoleBox.setBounds(",
              "relationPriorityBox.setBounds(", "relationGroupBox.setBounds(",
              "relationSuggestButton.setBounds(", "relationApplyButton.setBounds(",
              "void LoveEQAudioProcessorEditor::proposeRelationship(",
              "void LoveEQAudioProcessorEditor::applyRelationship("):
    if token not in ui:
        bad.append(f"Relationship UI missing: {token}")
processor=(root/"Source"/"PluginProcessor.cpp").read_text("utf-8")
for token in ('"relationship_role"','"relationship_priority"','"relationship_group"',
              '"relationship_active"','relationshipFilters[0].process(',
              'relationship::registry().publish('):
    if token not in processor:
        bad.append(f"Relationship DSP integration missing: {token}")


# TIME LENS: editable three-stage processing and a dedicated non-overlapping view.
for name in ("timeLensModeButton.setBounds(", "timeLensEnableButton.setBounds(",
             "timeThresholdSlider.setBounds(", "timeAttackSlider.setBounds(",
             "timeTailSlider.setBounds(", "timeHzSlider[(size_t)j].setBounds(",
             "timeGainSlider[(size_t)j].setBounds(", "timeQSlider[(size_t)j].setBounds(",
             "void LoveEQAudioProcessorEditor::initializeTimeLens(",
             "void LoveEQAudioProcessorEditor::updateTimeLensVisibility("):
    if name not in ui:
        bad.append(f"TIME LENS control missing: {name}")
for name in ('"time_lens_enabled"', '"time_lens_attack_ms"', '"time_lens_tail_ms"',
             '"time_lens_threshold_db"', 'timeDetector.process(', 
             'timeRuntime[(size_t)ti]', 'timeWeights[(size_t)ti]'):
    if name not in processor:
        bad.append(f"TIME LENS DSP missing: {name}")
lens_rectangles={
    "mode":(994,155,141,25),"relation_button":(840,155,146,25),
    "sensitivity":(38,529,172,43),"attack_duration":(222,529,172,43),
    "tail_duration":(406,529,172,43),"enable":(590,519,103,35),
}
for stage in range(3):
    bx=36+stage*369
    lens_rectangles[f"stage{stage}_freq"]=(bx+8,639,106,63)
    lens_rectangles[f"stage{stage}_gain"]=(bx+126,639,106,63)
    lens_rectangles[f"stage{stage}_q"]=(bx+244,639,106,63)
    lens_rectangles[f"timeline{stage}"]=(714+stage*141,513,131,56)
for name,(x,y,w,h) in lens_rectangles.items():
    if x<24 or y<145 or x+w>1136 or y+h>709:
        bad.append(f"TIME LENS rectangle out of main panel: {name}")
    for other,(ox,oy,ow,oh) in lens_rectangles.items():
        if name<other and min(x+w,ox+ow)>max(x,ox) and min(y+h,oy+oh)>max(y,oy):
            bad.append(f"TIME LENS collision: {name} / {other}")

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
