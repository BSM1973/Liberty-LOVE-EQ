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
    class BandTileButton final : public juce::TextButton {
    public:
        LoveEQAudioProcessor* processor=nullptr;
        int band=0;
        bool selected=false, english=false;
        void paintButton(juce::Graphics&,bool isMouseOver,bool isButtonDown) override;
    };
    LoveEQAudioProcessor& processor;
    std::array<BandTileButton,8> bandButtons;
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
    // LOVE INTENT: 4 musical goals, 3 explanatory suggestions, reversible A/B.
    juce::TextButton intentModeButton {"LOVE INTENT"},intentAnalyzeButton {"ANALYSER"};
    std::array<juce::TextButton,3> intentVariantButtons {{
        juce::TextButton("DOUX"),juce::TextButton(u8"ÉQUILIBRÉ"),
        juce::TextButton(u8"AFFIRMÉ")
    }};
    juce::TextButton intentOriginalButton {"A ORIGINAL"},intentPreviewButton {"B PROPOSITION"},
                     intentCommitButton {"CONSERVER"},intentCancelButton {"ANNULER"};
    juce::ComboBox intentGoalBox;
    juce::Slider intentAmountSlider;
    std::unique_ptr<SliderAttachment> intentAmountAttachment;
    std::array<juce::Slider,3> intentFrequencySlider,intentGainSlider,intentQSlider;
    std::array<std::unique_ptr<SliderAttachment>,3> intentFreqAttach,intentGainAttach,intentQAttach;
    bool intentMode=false,intentSuggestionsReady=false,intentPreviewing=false,
         intentOriginalValid=false;
    int intentVariant=1;
    std::array<float,11> intentBaseline {};
    loveeq::intent::Spectrum intentSpectrum {};
    std::array<loveeq::intent::Suggestion,3> intentSuggestions {};
    juce::String intentMessage;
    void initializeIntent();
    void updateIntentVisibility();
    void updateIntentLabels();
    void measureIntent();
    void setIntentVariant(int);
    void listenIntent(bool);
    void commitIntent();
    void cancelIntent();
    void captureIntentBaseline();
    void restoreIntentBaseline();
    void closeIntentView();
    // EQ MATCH: capture two real signals, compare tonality, optionally apply.
    juce::TextButton matchModeButton {"EQ MATCH"},matchReferenceButton {"CAPTURER RÉF"},
        matchSourceButton {"CAPTURER SOURCE"},matchProposeButton {"PROPOSER"},
        matchPreviewButton {"B ÉCOUTER"},matchCommitButton {"CONSERVER"},
        matchCancelButton {"ANNULER"};
    juce::Slider matchAmountSlider;
    std::unique_ptr<SliderAttachment> matchAmountAttachment;
    std::array<juce::Slider,loveeq::match::bands> matchGainSliders;
    std::array<std::unique_ptr<SliderAttachment>,loveeq::match::bands> matchGainAttachments;
    loveeq::match::Profile matchReference {},matchSource {};
    loveeq::match::Curve matchCurve {};
    bool matchMode=false,matchPreviewing=false,matchBaselineValid=false;
    std::array<float,loveeq::match::bands+2> matchBaseline {};
    int matchCaptureMode=0,matchFramesRemaining=0;
    juce::String matchStatus;
    void initializeMatch();
    void updateMatchVisibility();
    void updateMatchLanguage();
    void beginMatchCapture(int);
    void runMatchCaptureFrame();
    void proposeMatch();
    void previewMatch();
    void commitMatch();
    void cancelMatch();
    void closeMatchView();
    void rememberMatchBaseline();
    void restoreMatchBaseline();
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
