#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PresetBrowser.h"
#include "Utf8Text.h"
#include "HarmonicDNA.h"
#include "CurveLab.h"
#if JUCE_IOS
#include "MobilePrecision.h"
#endif
#include <vector>

// iOS uses the original drawing surface inside a host-resizable editor.
#if JUCE_IOS
using LoveEQEditorBase = juce::Component;
#else
using LoveEQEditorBase = juce::AudioProcessorEditor;
#endif
class LoveEQAudioProcessorEditor final : public LoveEQEditorBase, private juce::Timer {
public:
    explicit LoveEQAudioProcessorEditor(LoveEQAudioProcessor&);
    ~LoveEQAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
#if JUCE_IOS
    void openMobilePresets() { if (browserButton.onClick) browserButton.onClick(); }
    bool mobilePresetsVisible() const { return presetBrowser && presetBrowser->isVisible(); }
    bool mobileEnglish() const { return english; }
    void syncMobileMode() {
        const bool graphic=processor.parameters.getRawParameterValue("eq_mode")->load()>.5f;
        if(graphic!=(activeTab==2))selectTab(graphic?2:0);
    }
#endif
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
    juce::TextButton tabParam {juce::String::fromUTF8(u8"PARAMÉTRIQUE")}, tabDynamic {"DYNAMIQUE"}, tabGraphic {"GRAPHIQUE"};
    juce::TextButton holdButton {"MAINTIEN PIC"}, freezeButton {"FIGER"};
    juce::TextButton dnaButton {"HARMONIC DNA"}, dnaFocusButton {"CIBLER"}, dnaCutButton {juce::String::fromUTF8(u8"ATTÉNUER")};
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
        juce::TextButton("DOUX"),juce::TextButton(juce::String::fromUTF8(u8"ÉQUILIBRÉ")),
        juce::TextButton(juce::String::fromUTF8(u8"AFFIRMÉ"))
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
    juce::TextButton matchModeButton {"EQ MATCH"},matchReferenceButton {juce::String::fromUTF8(u8"CAPTURER RÉF")},
        matchSourceButton {"CAPTURER SOURCE"},matchProposeButton {"PROPOSER"},
        matchPreviewButton {juce::String::fromUTF8(u8"B ÉCOUTER")},matchCommitButton {"CONSERVER"},
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
    juce::String tr(const char* fr,const char* en) const {
        return loveeq::i18n::translate(english,fr,en);
    }
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
#if JUCE_IOS
    unsigned int mobileLastScope=0, mobileSeenEpoch=0;
    bool mobileWasReady=false;
    void endMobileGraphGesture();
    void beginMobileGraphGesture(const juce::String&,const juce::String& = {});
    std::array<juce::RangedAudioParameter*,2> mobileGestureParams{};
    int mobileRefreshHz=loveeq::mobile::activeHz;
    bool mobileHaveScope=false;
#endif
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

#if JUCE_IOS
#include "MobileTouchPanel.h"
// Keep the full vector-rendered editor and every parameter attachment alive.
// A clipping parent allows deliberate zoom/pan without scrollbars. Touch
// events are mapped through the component transform by JUCE.
class LoveEQMobileEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit LoveEQMobileEditor(LoveEQAudioProcessor& p)
        : juce::AudioProcessorEditor(&p), canvas(p), touch(p) {
        addAndMakeVisible(window);
        window.addAndMakeVisible(canvas);
        addChildComponent(touch);
        addAndMakeVisible(viewMode);
        viewMode.setColour(juce::TextButton::buttonColourId,juce::Colour(0xff84683d));
        viewMode.onClick=[this]{manualView=true;useTouch=!useTouch;resized();};
        for (auto* button : buttons()) {
            addAndMakeVisible(*button);
            button->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff273542));
            button->setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff1dfbd));
        }
        overview.onClick = [this] { zoom = 1.f; centreX = centreY = 0.5f; layoutCanvas(); };
        zoomIn.onClick = [this] { zoom = juce::jmin(5.f, zoom * 1.4f); layoutCanvas(); };
        zoomOut.onClick = [this] { zoom = juce::jmax(1.f, zoom / 1.4f); layoutCanvas(); };
        left.onClick = [this] { pan(-0.65f, 0.f); };
        right.onClick = [this] { pan(0.65f, 0.f); };
        up.onClick = [this] { pan(0.f, -0.65f); };
        down.onClick = [this] { pan(0.f, 0.65f); };
        presets.onClick = [this] {
            zoom = 1.f; centreX = centreY = 0.5f;
            manualView=true;useTouch=false;resized();
            if(!canvas.mobilePresetsVisible())canvas.openMobilePresets(); layoutCanvas();
        };
        setResizable(true, false);
        setResizeLimits(280, 220, 2048, 2048);
        setSize(960, 640);
        updateLabels();
        startTimerHz(5);
    }
    ~LoveEQMobileEditor() override { stopTimer(); }
    void paint(juce::Graphics& g) override { g.fillAll(juce::Colour(0xff151a22)); }
    void resized() override {
        auto area = getLocalBounds().reduced(4);
        const bool supported=loveeq::mobile::supportsTouch(area.getWidth(),area.getHeight()-46);
        if(!supported)useTouch=false;
        else if(!manualView)useTouch=loveeq::mobile::preferTouch(area.getWidth(),area.getHeight());
        viewMode.setEnabled(supported);
        if(!useTouch && touch.isVisible())canvas.syncMobileMode();
        window.setVisible(!useTouch);touch.setVisible(useTouch);
        viewMode.setButtonText(useTouch?"STUDIO":"TOUCH");
        auto allButtons=buttons();
        for(auto* b:allButtons)b->setVisible(!useTouch);
        presets.setVisible(true);
        if(useTouch){
            auto bar=area.removeFromTop(44);
            viewMode.setBounds(bar.removeFromLeft(bar.getWidth()/2).reduced(1));
            presets.setBounds(bar.reduced(1));
            touch.setBounds(area);return;
        }
        const bool compact = area.getWidth() < 480;
        auto toolbar = area.removeFromTop(compact ? 90 : 46);
        viewMode.setBounds(toolbar.removeFromLeft(juce::jmin(84,toolbar.getWidth()/4)).removeFromTop(42).reduced(1));
        auto all = buttons();
        const int columns = compact ? 4 : 8;
        for (int i = 0; i < 8; ++i) {
            const int col = i % columns, row = i / columns;
            const int x = toolbar.getX() + toolbar.getWidth() * col / columns;
            const int nextX = toolbar.getX() + toolbar.getWidth() * (col + 1) / columns;
            all[(size_t)i]->setBounds(x, toolbar.getY() + row * 44, juce::jmax(0, nextX-x-3), 42);
        }
        window.setBounds(area);
        layoutCanvas();
    }
private:
    juce::Component window;
    LoveEQAudioProcessorEditor canvas;
    LoveEQTouchPanel touch;
    juce::TextButton viewMode{"TOUCH"};
    bool manualView=false,useTouch=false;
    juce::TextButton overview{"VUE"}, zoomIn{"ZOOM +"}, zoomOut{"ZOOM -"},
        presets{"PRESETS"}, left{"<"}, right{">"}, up{"^"}, down{"v"};
    float zoom = 1.f, centreX = 0.5f, centreY = 0.5f;
    float scale = 1.f;
    std::array<juce::TextButton*,8> buttons() {
        return {&overview,&zoomIn,&zoomOut,&presets,&left,&right,&up,&down};
    }
    void updateLabels() {
        const juce::String label=canvas.mobileEnglish() ? "FIT" : "VUE";
        if(overview.getButtonText()!=label)overview.setButtonText(label);
    }
    void timerCallback() override { updateLabels(); }
    void pan(float x, float y) {
        const float w = 1160.f * scale, h = 760.f * scale;
        if (w > 0.f) centreX += x * window.getWidth() / w;
        if (h > 0.f) centreY += y * window.getHeight() / h;
        layoutCanvas();
    }
    void layoutCanvas() {
        const float vw = (float)window.getWidth(), vh = (float)window.getHeight();
        if (vw <= 0.f || vh <= 0.f) return;
        scale = juce::jmin(vw / 1160.f, vh / 760.f) * zoom;
        const float w = 1160.f * scale, h = 760.f * scale;
        const float halfX = juce::jmin(0.5f, vw / (2.f*w));
        const float halfY = juce::jmin(0.5f, vh / (2.f*h));
        centreX = juce::jlimit(halfX, 1.f-halfX, centreX);
        centreY = juce::jlimit(halfY, 1.f-halfY, centreY);
        canvas.setTransform(juce::AffineTransform::scale(scale)
            .translated(vw*0.5f-centreX*w, vh*0.5f-centreY*h));
        zoomIn.setEnabled(zoom < 4.99f);
        zoomOut.setEnabled(zoom > 1.01f);
        left.setEnabled(centreX > halfX + 0.001f);
        right.setEnabled(centreX < 1.f-halfX-0.001f);
        up.setEnabled(centreY > halfY+0.001f);
        down.setEnabled(centreY < 1.f-halfY-0.001f);
        repaint();
    }
};
#endif
