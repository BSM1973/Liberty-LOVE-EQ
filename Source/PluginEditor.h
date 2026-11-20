#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PresetBrowser.h"
#include "HarmonicDNA.h"
#include "CurveLab.h"
#include <vector>

class LoveEQAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit LoveEQAudioProcessorEditor(LoveEQAudioProcessor&);
    ~LoveEQAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
private:
    using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment=juce::AudioProcessorValueTreeState::ButtonAttachment;
    class Theme final : public juce::LookAndFeel_V4 {
    public:
        Theme();
        juce::Font getTextButtonFont(juce::TextButton&,int buttonHeight) override;
        juce::Font getComboBoxFont(juce::ComboBox&) override;
        juce::Font getPopupMenuFont() override;
        void drawComboBox(juce::Graphics&,int,int,bool,int,int,int,int,juce::ComboBox&) override;
        void drawPopupMenuBackground(juce::Graphics&,int,int) override;
        void drawPopupMenuItem(juce::Graphics&,const juce::Rectangle<int>&,bool,bool,bool,bool,
                               bool,const juce::String&,const juce::String&,const juce::Drawable*,
                               const juce::Colour*) override;
        void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
        void drawToggleButton(juce::Graphics&,juce::ToggleButton&,bool,bool) override;
        void getIdealPopupMenuItemSize(const juce::String&,bool,int,int&,int&) override;
    } theme;
    class HistoryButton final : public juce::TextButton {
    public:
        explicit HistoryButton(bool isRedo):redo(isRedo){setButtonText("");}
        void paintButton(juce::Graphics&,bool highlighted,bool down) override;
    private:
        bool redo;
    };
    LoveEQAudioProcessor& processor;
    std::array<juce::TextButton,8> bandButtons;
    juce::TextButton tabParam {u8"PARAMÉTRIQUE"}, tabDynamic {"DYNAMIQUE"}, tabGraphic {"GRAPHIQUE"};
    juce::TextButton holdButton {"MAINTIEN PIC"}, freezeButton {"FIGER"};
    juce::TextButton dnaButton {"HARMONIC DNA"}, dnaFocusButton {"CIBLER"}, dnaCutButton {"ATTÉNUER"};
    juce::TextButton relationModeButton {"RELATION EQ"},relationSuggestButton {"PROPOSER"},
                     relationApplyButton {"ACTIVER"};
    juce::ComboBox relationRoleBox,relationPriorityBox,relationGroupBox;
    bool relationMode=false,relationSuggested=false;
    float relationProposedHz=0.f,relationProposedScore=0.f;
    int relationReferenceCount=0;
    juce::String relationStatus;
    void initializeRelationship();
    void updateRelationshipControls();
    void proposeRelationship();
    void applyRelationship();
    juce::TextButton sketchButton {"DESSIN"}, notesButton {"NOTES"}, grabButton {"CAPTER PIC"};
    // The mode button only opens the editor view; time_lens_enabled controls DSP.
    juce::TextButton timeLensModeButton {"TIME LENS"},timeLensEnableButton {"ACTIVER"};
    bool timeLensMode=false;
    std::array<juce::Slider,LoveEQAudioProcessor::timeStageCount> timeHzSlider,
        timeGainSlider,timeQSlider;
    std::array<std::unique_ptr<SliderAttachment>,LoveEQAudioProcessor::timeStageCount>
        timeHzAttach,timeGainAttach,timeQAttach;
    juce::Slider timeAttackSlider,timeTailSlider,timeThresholdSlider;
    std::unique_ptr<SliderAttachment> timeAttackAttach,timeTailAttach,timeThresholdAttach;
    std::unique_ptr<ButtonAttachment> timeEnableAttach;
    void initializeTimeLens();
    void updateTimeLensVisibility();
    void updateTimeLensButtonLabel();
    juce::TextEditor presetSearch;
    juce::ComboBox languageBox;
    juce::TextButton browserButton {"PRESETS"},peakResetButton {"PEAK RESET"};
    std::unique_ptr<LoveEQPresetBrowser> presetBrowser;
    juce::ComboBox presetCombo, typeBox, channelBox, slopeBox;
    juce::TextButton buttonA {"A"}, buttonB {"B"};
    HistoryButton undoButton {false}, redoButton {true};
    juce::TextButton resetButton {"RESET / FLAT"}, bypassButton {"BYPASS"},
                     preButton {"PRE"}, postButton {"POST"};
    juce::ToggleButton enabledButton {"ACTIF"}, soloButton {"SOLO"}, dynamicButton {"DYNAMIQUE"};
    juce::Slider frequency, gain, q, threshold, range, attack, release, input, output;
    std::array<juce::Slider,LoveEQAudioProcessor::graphicBandCount> graphicSliders;
    std::array<std::unique_ptr<SliderAttachment>,LoveEQAudioProcessor::graphicBandCount> graphicAttachments;
    std::unique_ptr<SliderAttachment> hzAttach, gainAttach, qAttach, thresholdAttach, rangeAttach,
                                       attackAttach, releaseAttach, inputAttach, outputAttach;
    std::unique_ptr<ComboAttachment> typeAttach, channelAttach, slopeAttach;
    std::unique_ptr<ButtonAttachment> enabledAttach, soloAttach, dynamicAttach, bypassAttach;
    juce::ValueTree stateA, stateB;
    bool onB=false, showPre=true, showPost=true, freezeSpectrum=false, holdSpectrum=false;
    bool dnaEnabled=true, dnaTargetArmed=false;
    float dnaArmedHz=0.f;
    int dnaArmedBand=-1;
    unsigned int dnaFrames=0;
    loveeq::dna::Result dnaResult {};
    juce::dsp::FFT dnaFft {13};
    std::array<float,16384> dnaScratch {};
    std::array<float,4097> dnaMagnitudes {};
    void updateHarmonicDNA();
    void focusDnaCandidate();
    void applyDnaSuggestion();
    void updateDnaControls();
    bool sketchMode=false, sketchActive=false, noteGrid=false;
    std::vector<loveeq::curvelab::SketchPoint> sketchPoints;
    void addSketchPoint(const juce::Point<float>&);
    void applySketch();
    void grabSpectrumPeak();
    int activeTab=0;
    bool english=false;
    juce::String tr(const char* fr,const char* en) const {return english?juce::String(en):juce::String(fr);}
    void updateGraphicVisibility();
    std::array<float,1025> heldPre {}, heldPost {};
    void selectTab(int tab);
    void updateLanguageSafeTexts();
    int selectedBand=0, draggingBand=-1, draggingGraphic=-1;
    juce::Rectangle<int> graph, meterBox, bandRow, controls;
    juce::Rectangle<int> spectrumPanel, bottomPanel, footerPanel;
    juce::dsp::FFT fft {11};
    std::array<float,4096> fftScratch{};
    std::array<float,1025> preSpectrum{}, postSpectrum{};
    void timerCallback() override;
    void attachBand(int);
    void refillPresets();
    void switchSnapshot(bool toB);
    void setParam(const juce::String&,float);
    void updateFFT(bool before,std::array<float,1025>&);
    void drawSpectrum(juce::Graphics&,const std::array<float,1025>&,juce::Colour);
    float xForHz(float hz) const;
    float yForDb(float db) const;
    float dbAtHz(float hz) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoveEQAudioProcessorEditor)
};
