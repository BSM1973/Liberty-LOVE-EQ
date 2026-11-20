#include "PluginEditor.h"
#include "FactoryPresets.h"

namespace {
const juce::Colour background {0xff151a22};
const juce::Colour panel {0xff202a32};
const juce::Colour trim {0xff3d4a51};
const juce::Colour gold {0xffdec28a};
const juce::Colour text {0xffdce2e4};
constexpr std::array<juce::uint32,8> bandColours {
    0xffeda761,0xffe9cc61,0xff86cc79,0xff5dc8b1,0xff6bc3db,0xff839ef0,0xffb39fe8,0xffdd8db4
};
}
LoveEQAudioProcessorEditor::Theme::Theme() {
    setColour(juce::ResizableWindow::backgroundColourId,background);
    setColour(juce::TextButton::buttonColourId,panel);
    setColour(juce::TextButton::textColourOffId,text);
    setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xff4d4b3b));
    setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff101820));
    setColour(juce::ComboBox::textColourId,text);
    setColour(juce::ComboBox::outlineColourId,trim);
    setColour(juce::PopupMenu::backgroundColourId,panel);
    setColour(juce::PopupMenu::textColourId,text);
    setColour(juce::TextEditor::backgroundColourId,juce::Colour(0xff101820));
    setColour(juce::TextEditor::textColourId,text);
    setColour(juce::TextEditor::outlineColourId,trim);
    setColour(juce::Slider::textBoxTextColourId,text);
    setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff101820));
    setColour(juce::Slider::textBoxOutlineColourId,trim);
    setColour(juce::Slider::trackColourId,gold);
    setColour(juce::Slider::thumbColourId,gold);
    setColour(juce::ToggleButton::textColourId,text);
}
juce::Font LoveEQAudioProcessorEditor::Theme::getTextButtonFont(juce::TextButton&,int) {
    return juce::Font(juce::FontOptions("Arial",11.2f,juce::Font::plain));
}
juce::Font LoveEQAudioProcessorEditor::Theme::getComboBoxFont(juce::ComboBox&) {
    return juce::Font(juce::FontOptions("Arial",12.0f,juce::Font::plain));
}
juce::Font LoveEQAudioProcessorEditor::Theme::getPopupMenuFont() {
    return juce::Font(juce::FontOptions("Arial",12.4f,juce::Font::plain));
}
void LoveEQAudioProcessorEditor::Theme::drawComboBox(juce::Graphics& g,int width,int height,
      bool pressed,int,int,int,int,juce::ComboBox& box) {
    const auto b=juce::Rectangle<float>(0.f,0.f,(float)width,(float)height).reduced(0.7f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff28333e),0.f,0.f,
                                            juce::Colour(0xff161f29),0.f,(float)height,false));
    g.fillRoundedRectangle(b,7.f);
    g.setColour(box.isPopupActive()||pressed?juce::Colour(0xffdfc18c):juce::Colour(0xff52616c));
    g.drawRoundedRectangle(b,7.f,box.isPopupActive()?1.7f:1.f);
    g.setColour(juce::Colour(0xff41515d));
    g.drawVerticalLine(width-29,7.f,(float)height-7.f);
    juce::Path chevron;
    const float x=(float)width-15.f,y=(float)height*0.5f;
    chevron.startNewSubPath(x-4.f,y-2.f);
    chevron.lineTo(x,y+2.f);
    chevron.lineTo(x+4.f,y-2.f);
    g.setColour(juce::Colour(0xffd4bd90));
    g.strokePath(chevron,juce::PathStrokeType(1.8f));
}
void LoveEQAudioProcessorEditor::Theme::drawPopupMenuBackground(juce::Graphics& g,int w,int h) {
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff24313d),0.f,0.f,
                                            juce::Colour(0xff131c25),0.f,(float)h,false));
    g.fillAll();
    g.setColour(juce::Colour(0xff586775));
    g.drawRect(juce::Rectangle<int>{0,0,w,h},1);
}
void LoveEQAudioProcessorEditor::Theme::drawPopupMenuItem(juce::Graphics& g,
      const juce::Rectangle<int>& area,bool separator,bool active,bool highlighted,bool ticked,
      bool submenu,const juce::String& label,const juce::String& shortcut,
      const juce::Drawable*,const juce::Colour* customColour) {
    if(separator) {
        g.setColour(juce::Colour(0xff3c4b58));
        g.drawHorizontalLine(area.getCentreY(),(float)area.getX()+10.f,(float)area.getRight()-10.f);
        return;
    }
    const auto bounds=area.reduced(4,2).toFloat();
    if(highlighted && active) {
        g.setColour(juce::Colour(0xff44596b));
        g.fillRoundedRectangle(bounds,5.f);
        g.setColour(juce::Colour(0xffb99b69));
        g.fillRoundedRectangle(bounds.getX(),bounds.getY(),2.8f,bounds.getHeight(),1.f);
    }
    const auto fg=!active?juce::Colour(0xff6f7f89):
                 customColour?*customColour:juce::Colour(0xffe0e5e6);
    g.setColour(fg);
    g.setFont(getPopupMenuFont());
    g.drawFittedText(label,area.reduced(30,2).withTrimmedRight(submenu?10:0),
                     juce::Justification::centredLeft,1,0.92f);
    if(shortcut.isNotEmpty()) {
        g.setColour(juce::Colour(0xff91a3ae));
        g.drawText(shortcut,area.withLeft(area.getRight()-72),juce::Justification::centredRight);
    }
    if(ticked) {
        g.setColour(juce::Colour(0xffe6c48e));
        juce::Path check;
        const float x=(float)area.getX()+15.f,y=(float)area.getCentreY();
        check.startNewSubPath(x-4.f,y);
        check.lineTo(x-1.f,y+3.f);
        check.lineTo(x+5.f,y-4.f);
        g.strokePath(check,juce::PathStrokeType(2.0f));
    }
    if(submenu) {
        const float x=(float)area.getRight()-13.f,y=(float)area.getCentreY();
        juce::Path arrow;
        arrow.startNewSubPath(x-3.f,y-4.f);
        arrow.lineTo(x+1.f,y);
        arrow.lineTo(x-3.f,y+4.f);
        g.setColour(fg);
        g.strokePath(arrow,juce::PathStrokeType(1.6f));
    }
}
void LoveEQAudioProcessorEditor::Theme::getIdealPopupMenuItemSize(
      const juce::String& name,bool separator,int standardHeight,int& width,int& height) {
    if(separator){height=9; width=juce::jmax(width,100);return;}
    height=juce::jmax(32,standardHeight);
    width=juce::jmax(width,juce::jmin(390,juce::roundToInt(getPopupMenuFont().getStringWidthFloat(name))+63));
}
void LoveEQAudioProcessorEditor::Theme::drawButtonBackground(juce::Graphics& g,
      juce::Button& button,const juce::Colour&,bool highlighted,bool down) {
    const bool selected=button.getToggleState();
    const auto b=button.getLocalBounds().toFloat().reduced(0.7f);
    const auto hi=selected?juce::Colour(0xff57503e):
                 (down?juce::Colour(0xff3e4c59):
                  highlighted?juce::Colour(0xff354450):juce::Colour(0xff26313c));
    const auto lo=selected?juce::Colour(0xff363226):juce::Colour(0xff18212b);
    g.setGradientFill(juce::ColourGradient(hi,0.f,0.f,lo,0.f,(float)button.getHeight(),false));
    g.fillRoundedRectangle(b,6.f);
    g.setColour(selected?juce::Colour(0xffd9b97e):juce::Colour(0xff52616c));
    g.drawRoundedRectangle(b,6.f,selected?1.5f:1.f);
}
void LoveEQAudioProcessorEditor::Theme::drawToggleButton(juce::Graphics& g,
      juce::ToggleButton& button,bool highlighted,bool down) {
    drawButtonBackground(g,button,juce::Colour(0xff25303a),highlighted,down);
    const auto b=button.getLocalBounds();
    const auto chosen=button.getToggleState();
    g.setColour(chosen?juce::Colour(0xffe8ce9f):juce::Colour(0xffd4dde0));
    g.setFont(juce::Font(juce::FontOptions("Arial",11.3f,juce::Font::bold)));
    g.drawFittedText(button.getButtonText(),b.reduced(7,3),juce::Justification::centred,1,0.90f);
}
void LoveEQAudioProcessorEditor::HistoryButton::paintButton(juce::Graphics& g,bool highlighted,bool down) {
    auto bounds=getLocalBounds().toFloat().reduced(1.f);
    g.setColour(down?juce::Colour(0xff4b4538):
                  highlighted?juce::Colour(0xff35424e):juce::Colour(0xff25313b));
    g.fillRoundedRectangle(bounds,5.f);
    g.setColour(juce::Colour(0xff52616b));
    g.drawRoundedRectangle(bounds,5.f,1.f);
    g.setColour(juce::Colour(0xffdfc894));
    const float cx=(float)getWidth()*0.5f,cy=(float)getHeight()*0.5f;
    const float flip=redo?-1.f:1.f;
    juce::Path curve;
    curve.startNewSubPath(cx+9.f*flip,cy+5.f);
    curve.cubicTo(cx+12.f*flip,cy-7.f,cx-1.f*flip,cy-10.f,cx-7.f*flip,cy-2.f);
    g.strokePath(curve,juce::PathStrokeType(2.3f));
    juce::Path arrow;
    arrow.startNewSubPath(cx-12.f*flip,cy-7.f);
    arrow.lineTo(cx-7.f*flip,cy-2.f);
    arrow.lineTo(cx-2.f*flip,cy-6.f);
    g.strokePath(arrow,juce::PathStrokeType(2.2f));
}
void LoveEQAudioProcessorEditor::BandTileButton::paintButton(
    juce::Graphics& g,bool over,bool pressed) {
    // Drawing must happen ON the clickable child. Drawing its text in the parent
    // was hidden behind opaque TextButton backgrounds, leaving empty tiles.
    const auto rect=getLocalBounds().toFloat().reduced(0.8f);
    const juce::Colour colour(bandColours[(size_t)juce::jlimit(0,7,band)]);
    g.setGradientFill(juce::ColourGradient(
        selected?juce::Colour(0xff32424a):(over?juce::Colour(0xff2d3a43):
        juce::Colour(0xff202c35)),0.f,0.f,
        pressed?juce::Colour(0xff1b252e):juce::Colour(0xff151f28),
        0.f,(float)getHeight(),false));
    g.fillRoundedRectangle(rect,6.f);
    g.setColour(selected?colour:juce::Colour(0xff4c5d68));
    g.drawRoundedRectangle(rect,6.f,selected?1.9f:1.f);
    g.setColour(colour);
    g.fillRoundedRectangle(rect.getX()+1.f,rect.getY()+1.f,
                           rect.getWidth()-2.f,3.3f,1.1f);
    g.setFont(juce::Font(juce::FontOptions("Arial",11.f,juce::Font::bold)));
    const juce::String title=juce::String(band+1);
    g.drawText(title,9,11,23,18,juce::Justification::centred);
    if(!processor)return;
    const auto param=[this](const juce::String& key)->float {
        if(const auto* p=processor->parameters.getRawParameterValue(
                LoveEQAudioProcessor::bandId(band,key)))return p->load();
        return 0.f;
    };
    const bool enabled=param("enabled")>0.5f;
    const int type=juce::jlimit(0,6,(int)param("type"));
    static constexpr const char* fr[7]={"CLOCHE","SHELF BAS","SHELF HAUT",
        "PASSE-HAUT","PASSE-BAS","COUPE","PASSE-BANDE"};
    static constexpr const char* en[7]={"BELL","LOW SHELF","HIGH SHELF",
        "HIGH PASS","LOW PASS","NOTCH","BAND PASS"};
    g.setColour(enabled?juce::Colour(0xffdae3e7):juce::Colour(0xff87939c));
    g.setFont(juce::Font(juce::FontOptions("Arial",10.f,juce::Font::bold)));
    g.drawFittedText(english?en[type]:fr[type],
        33,11,getWidth()-39,19,juce::Justification::centredLeft,1,0.88f);
    const float freq=param("hz");
    const float db=param("gain");
    const juce::String frequency=freq>=1000.f?
        juce::String(freq/1000.f,2)+" kHz":
        juce::String((int)std::round(freq))+" Hz";
    g.setFont(juce::Font(juce::FontOptions("Arial",11.5f,juce::Font::plain)));
    g.drawFittedText(frequency,9,34,getWidth()-16,20,
                     juce::Justification::centredLeft,1,0.90f);
    g.setColour(colour);
    g.setFont(juce::Font(juce::FontOptions("Arial",11.f,juce::Font::bold)));
    const juce::String gainText=(db>0.05f?"+":"")+juce::String(db,1)+" dB";
    g.drawFittedText(gainText,9,getHeight()-24,getWidth()-16,18,
                     juce::Justification::centredRight,1,0.90f);
}
LoveEQAudioProcessorEditor::LoveEQAudioProcessorEditor(LoveEQAudioProcessor& p)
 : AudioProcessorEditor(&p),processor(p) {
    setLookAndFeel(&theme);
    english=processor.getUiLanguage()=="en";
    languageBox.addItem(u8"FRANÇAIS",1);
    languageBox.addItem("ENGLISH",2);
    languageBox.setSelectedId(english?2:1,juce::dontSendNotification);
    languageBox.onChange=[this]{
        english=languageBox.getSelectedId()==2;
        processor.setUiLanguage(english?"en":"fr");
        updateLanguageSafeTexts();
        if(presetBrowser) presetBrowser->translate(english);
        repaint();
    };
    addAndMakeVisible(languageBox);
    addAndMakeVisible(browserButton);
    browserButton.onClick=[this]{if(matchBaselineValid)cancelMatch();if(intentOriginalValid)cancelIntent();if(presetBrowser)presetBrowser->open();};
    addAndMakeVisible(peakResetButton);
    peakResetButton.onClick=[this]{processor.resetPeakHold();repaint();};
    for(int i=0;i<8;++i) {
        auto& b=bandButtons[(size_t)i];
        b.processor=&processor;
        b.band=i;
        b.setButtonText("");
        b.setTooltip(u8"Sélectionner la bande " + juce::String(i+1));
        b.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        b.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
        b.setColour(juce::TextButton::textColourOffId,juce::Colour(bandColours[(size_t)i]));
        b.onClick=[this,i]{attachBand(i);};
        addAndMakeVisible(b);
    }
    presetSearch.setTextToShowWhenEmpty("Rechercher parmi 100 presets...",juce::Colours::grey);
    presetSearch.onTextChange=[this]{refillPresets();};
    addAndMakeVisible(presetSearch);
    presetCombo.onChange=[this]{
        if(presetCombo.getSelectedId()>0) {
            processor.loadFactoryPreset((size_t)(presetCombo.getSelectedId()-1));
            selectTab(0);
        }
    };
    addAndMakeVisible(presetCombo);
    auto show=[this](juce::Component& c){addAndMakeVisible(c);};
    for(juce::TextButton* b : std::array<juce::TextButton*,13>{
            &buttonA,&buttonB,&undoButton,&redoButton,&resetButton,&bypassButton,&preButton,&postButton,
            &tabParam,&tabDynamic,&tabGraphic,&holdButton,&freezeButton}) show(*b);
    auto closeLens=[this]{
        timeLensMode=false;
        timeLensModeButton.setToggleState(false,juce::dontSendNotification);
    };
    tabParam.onClick=[this,closeLens]{closeMatchView();closeIntentView();closeLens();selectTab(0);};
    tabDynamic.onClick=[this,closeLens]{closeMatchView();closeIntentView();closeLens();selectTab(1);};
    tabGraphic.setEnabled(true);
    tabGraphic.onClick=[this,closeLens]{closeMatchView();closeIntentView();closeLens();selectTab(2);};
    tabGraphic.setTooltip(u8"16 bandes graphiques réellement traitées dans le moteur audio");
    holdButton.setClickingTogglesState(true);
    holdButton.onClick=[this]{holdSpectrum=holdButton.getToggleState();
        if(!holdSpectrum){heldPre.fill(-96.f);heldPost.fill(-96.f);}
    };
    freezeButton.setClickingTogglesState(true);
    freezeButton.onClick=[this]{freezeSpectrum=freezeButton.getToggleState();};
    for(juce::TextButton* b : std::array<juce::TextButton*,3>{&sketchButton,&notesButton,&grabButton})show(*b);
    sketchButton.setClickingTogglesState(true);
    notesButton.setClickingTogglesState(true);
    sketchButton.onClick=[this]{
        sketchMode=sketchButton.getToggleState();
        sketchActive=false;
        sketchPoints.clear();
        if(sketchMode && activeTab==2)selectTab(0);
        repaint();
    };
    notesButton.onClick=[this]{noteGrid=notesButton.getToggleState();repaint();};
    grabButton.onClick=[this]{grabSpectrumPeak();};
    for(juce::TextButton* b : std::array<juce::TextButton*,3>{
        &dnaButton,&dnaFocusButton,&dnaCutButton})show(*b);
    dnaButton.setClickingTogglesState(true);
    dnaButton.setToggleState(true,juce::dontSendNotification);
    dnaButton.onClick=[this]{
        dnaEnabled=dnaButton.getToggleState();
        dnaTargetArmed=false;
        updateDnaControls();
        repaint();
    };
    dnaFocusButton.onClick=[this]{focusDnaCandidate();};
    dnaCutButton.onClick=[this]{applyDnaSuggestion();};
    initializeRelationship();
    initializeTimeLens();
    initializeIntent();
    initializeMatch();
    dnaFocusButton.setTooltip(u8"Préparer la fréquence non harmonique sans modifier l’audio. ATTÉNUER valide la coupe.");
    dnaCutButton.setTooltip(u8"Appliquer une coupe de -2 dB après votre confirmation.");
    updateDnaControls();
    for(auto* b:{&enabledButton,&soloButton,&dynamicButton}) show(*b);
    for(auto* b:{&typeBox,&channelBox,&slopeBox}) show(*b);
    const juce::StringArray types {"Bell","Low Shelf","High Shelf","High Pass","Low Pass","Notch","Band Pass"};
    const juce::StringArray channels {"Stereo","Left","Right","Mid","Side"};
    const juce::StringArray slopes {"6 dB/oct","12 dB/oct","18 dB/oct","24 dB/oct","36 dB/oct","48 dB/oct"};
    typeBox.addItemList(types,1); channelBox.addItemList(channels,1); slopeBox.addItemList(slopes,1);
    for(auto* s:{&frequency,&gain,&q,&threshold,&range,&attack,&release,&input,&output}) {
        s->setSliderStyle(juce::Slider::LinearHorizontal);
        s->setTextBoxStyle(juce::Slider::TextBoxRight,false,67,25);
        s->setColour(juce::Slider::backgroundColourId,juce::Colour(0xff0d141c));
        s->setColour(juce::Slider::trackColourId,gold);
        show(*s);
    }
    frequency.setTextValueSuffix(" Hz");gain.setTextValueSuffix(" dB");
    threshold.setTextValueSuffix(" dB");range.setTextValueSuffix(" dB");
    attack.setTextValueSuffix(" ms");release.setTextValueSuffix(" ms");
    input.setTextValueSuffix(" dB");output.setTextValueSuffix(" dB");
    inputAttach=std::make_unique<SliderAttachment>(processor.parameters,"input_gain",input);
    outputAttach=std::make_unique<SliderAttachment>(processor.parameters,"output_gain",output);
    bypassButton.setClickingTogglesState(true);
    bypassAttach=std::make_unique<ButtonAttachment>(processor.parameters,"bypass",bypassButton);
    for(int i=0;i<LoveEQAudioProcessor::graphicBandCount;++i) {
        auto& slider=graphicSliders[(size_t)i];
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
        slider.setColour(juce::Slider::trackColourId,juce::Colour(0xffc8b07c));
        slider.setColour(juce::Slider::thumbColourId,juce::Colour(0xffe1c990));
        slider.setTooltip("Graphic "+juce::String(LoveEQAudioProcessor::graphicHz[(size_t)i])+" Hz");
        show(slider);
        graphicAttachments[(size_t)i]=std::make_unique<SliderAttachment>(processor.parameters,
                                LoveEQAudioProcessor::graphicId(i),slider);
    }
    undoButton.setButtonText("");
    redoButton.setButtonText("");
    undoButton.setTooltip("Annuler / Undo");
    redoButton.setTooltip("Retablir / Redo");
    buttonA.onClick=[this]{if(matchBaselineValid)cancelMatch();if(intentOriginalValid)cancelIntent();switchSnapshot(false);};
    buttonB.onClick=[this]{if(matchBaselineValid)cancelMatch();if(intentOriginalValid)cancelIntent();switchSnapshot(true);};
    undoButton.onClick=[this]{processor.undoManager.undo();};
    redoButton.onClick=[this]{processor.undoManager.redo();};
    resetButton.onClick=[this]{if(matchBaselineValid)cancelMatch();if(intentOriginalValid)cancelIntent();processor.resetToFlat();closeMatchView();closeIntentView();selectTab(0);};
    preButton.onClick=[this]{showPre=!showPre;preButton.setToggleState(showPre,juce::dontSendNotification);repaint();};
    postButton.onClick=[this]{showPost=!showPost;postButton.setToggleState(showPost,juce::dontSendNotification);repaint();};
    preButton.setClickingTogglesState(true);postButton.setClickingTogglesState(true);
    preButton.setToggleState(true,juce::dontSendNotification);postButton.setToggleState(true,juce::dontSendNotification);
    stateA=processor.parameters.copyState();stateB=stateA.createCopy();
    buttonA.setToggleState(true,juce::dontSendNotification);
    refillPresets();attachBand(0);
    heldPre.fill(-96.f); heldPost.fill(-96.f);
    presetBrowser=std::make_unique<LoveEQPresetBrowser>(processor);
    addAndMakeVisible(*presetBrowser);
    presetBrowser->setVisible(false);
    presetBrowser->onLoad=[this](int index){
        presetSearch.clear();
        refillPresets();
        presetCombo.setSelectedId(index>=0?index+1:0,juce::dontSendNotification);
        selectTab(processor.parameters.getRawParameterValue("eq_mode")->load()>0.5f?2:0);
    };
    presetBrowser->translate(english);
    updateLanguageSafeTexts();
    selectTab(processor.parameters.getRawParameterValue("eq_mode")->load()>0.5f?2:0);
    setResizable(false,false);setSize(1160,760);
    startTimerHz(24);
}
LoveEQAudioProcessorEditor::~LoveEQAudioProcessorEditor(){stopTimer();if(intentOriginalValid)restoreIntentBaseline();if(matchBaselineValid)restoreMatchBaseline();setLookAndFeel(nullptr);}
void LoveEQAudioProcessorEditor::initializeMatch() {
    for(juce::TextButton* b:std::array<juce::TextButton*,7>{
        &matchModeButton,&matchReferenceButton,&matchSourceButton,
        &matchProposeButton,&matchPreviewButton,&matchCommitButton,
        &matchCancelButton})addAndMakeVisible(*b);
    addAndMakeVisible(matchAmountSlider);
    matchModeButton.setClickingTogglesState(true);
    matchModeButton.onClick=[this] {
        const bool opening=matchModeButton.getToggleState();
        if(!opening){closeMatchView();return;}
        if(intentMode)closeIntentView();
        if(timeLensMode){
            timeLensMode=false;
            timeLensModeButton.setToggleState(false,juce::dontSendNotification);
        }
        if(relationMode){
            relationMode=false;
            relationModeButton.setToggleState(false,juce::dontSendNotification);
        }
        if(sketchMode) {
            sketchMode=false;sketchActive=false;sketchPoints.clear();
            sketchButton.setToggleState(false,juce::dontSendNotification);
        }
        matchMode=true;
        updateGraphicVisibility();
        repaint();
    };
    matchReferenceButton.onClick=[this]{beginMatchCapture(1);};
    matchSourceButton.onClick=[this]{beginMatchCapture(2);};
    matchProposeButton.onClick=[this]{proposeMatch();};
    matchPreviewButton.onClick=[this]{previewMatch();};
    matchCommitButton.onClick=[this]{commitMatch();};
    matchCancelButton.onClick=[this]{cancelMatch();};
    matchAmountSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    matchAmountSlider.setTextBoxStyle(juce::Slider::TextBoxRight,false,60,22);
    matchAmountSlider.setTextValueSuffix(" %");
    matchAmountSlider.setColour(juce::Slider::backgroundColourId,juce::Colour(0xff111c26));
    matchAmountAttachment=std::make_unique<SliderAttachment>(
        processor.parameters,"match_amount",matchAmountSlider);
    for(int i=0;i<loveeq::match::bands;++i){
        auto& slider=matchGainSliders[(size_t)i];
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,78,20);
        slider.setTextValueSuffix(" dB");
        slider.setColour(juce::Slider::backgroundColourId,juce::Colour(0xff0e1822));
        slider.setColour(juce::Slider::trackColourId,juce::Colour(0xffe2c183));
        addAndMakeVisible(slider);
        matchGainAttachments[(size_t)i]=std::make_unique<SliderAttachment>(
            processor.parameters,"match_gain_"+juce::String(i+1),slider);
    }
    matchStatus=tr(u8"Lecture requise : capturer d'abord la référence",
                   "Play audio and capture the reference first");
    updateMatchLanguage();
    updateMatchVisibility();
}
void LoveEQAudioProcessorEditor::updateMatchLanguage() {
    matchModeButton.setButtonText("EQ MATCH");
    matchReferenceButton.setButtonText(tr(u8"CAPTURER RÉF","CAPTURE REF"));
    matchSourceButton.setButtonText(tr("CAPTURER SOURCE","CAPTURE SOURCE"));
    matchProposeButton.setButtonText(tr("PROPOSER","SUGGEST"));
    matchPreviewButton.setButtonText(matchPreviewing?
        tr("A ORIGINAL","A ORIGINAL"):tr(u8"B ÉCOUTER","B AUDITION"));
    matchCommitButton.setButtonText(tr("CONSERVER","KEEP"));
    matchCancelButton.setButtonText(tr("ANNULER","CANCEL"));
    matchReferenceButton.setTooltip(tr(
        u8"Lire un son de référence pendant deux secondes",
        "Play reference audio for two seconds"));
    matchSourceButton.setTooltip(tr(
        u8"Lire la piste à corriger pendant deux secondes",
        "Play the source to correct for two seconds"));
    matchProposeButton.setTooltip(tr(
        u8"Comparer les timbres mesurés : corrections limitées à +/- 6 dB",
        "Compare measured tonal profiles, maximum +/- 6 dB per band"));
}
void LoveEQAudioProcessorEditor::updateMatchVisibility() {
    const bool show=matchMode;
    for(juce::Component* c:std::array<juce::Component*,7>{
        &matchReferenceButton,&matchSourceButton,&matchProposeButton,
        &matchPreviewButton,&matchCommitButton,&matchCancelButton,
        &matchAmountSlider})c->setVisible(show);
    const auto* active=processor.parameters.getRawParameterValue("match_active");
    const bool on=active&&active->load()>0.5f;
    for(auto& slider:matchGainSliders){
        slider.setVisible(show);
        slider.setEnabled(show&&(matchPreviewing||(!matchBaselineValid&&on)));
    }
    matchAmountSlider.setEnabled(show&&(matchPreviewing||(!matchBaselineValid&&on)));
    matchSourceButton.setEnabled(show&&matchReference.ready()&&matchCaptureMode==0);
    matchReferenceButton.setEnabled(show&&matchCaptureMode==0);
    matchProposeButton.setEnabled(show&&matchReference.ready()&&matchSource.ready()&&
                                  matchCaptureMode==0);
    matchPreviewButton.setEnabled(show&&loveeq::match::finiteAndBounded(matchCurve)&&
                                  matchCaptureMode==0);
    matchCommitButton.setEnabled(show&&matchPreviewing);
    matchCancelButton.setEnabled(show&&matchBaselineValid);
    updateMatchLanguage();
}
void LoveEQAudioProcessorEditor::rememberMatchBaseline() {
    for(int i=0;i<loveeq::match::bands;++i){
        const auto key="match_gain_"+juce::String(i+1);
        if(const auto* p=processor.parameters.getRawParameterValue(key))
            matchBaseline[(size_t)i]=p->load();
    }
    if(const auto* p=processor.parameters.getRawParameterValue("match_amount"))
        matchBaseline[6]=p->load();
    if(const auto* p=processor.parameters.getRawParameterValue("match_active"))
        matchBaseline[7]=p->load();
    matchBaselineValid=true;
}
void LoveEQAudioProcessorEditor::restoreMatchBaseline() {
    if(!matchBaselineValid)return;
    processor.undoManager.beginNewTransaction(tr(
        u8"EQ MATCH : restaurer l'original","EQ MATCH: restore original"));
    for(int i=0;i<loveeq::match::bands;++i)
        setParam("match_gain_"+juce::String(i+1),matchBaseline[(size_t)i]);
    setParam("match_amount",matchBaseline[6]);
    setParam("match_active",matchBaseline[7]);
    processor.undoManager.beginNewTransaction();
    matchPreviewing=false;
}
void LoveEQAudioProcessorEditor::beginMatchCapture(int mode) {
    if(!matchMode || matchCaptureMode!=0)return;
    if(matchBaselineValid)restoreMatchBaseline();
    matchCurve={};
    if(mode==1) {
        matchReference.reset();
        matchSource.reset();
    } else if(mode==2 && matchReference.ready())matchSource.reset();
    else return;
    matchCaptureMode=mode;
    matchFramesRemaining=48; // two seconds at the 24 Hz UI rate.
    matchStatus=mode==1?
       tr(u8"Lecture de référence en cours...","Recording reference..."):
       tr(u8"Lecture de la source en cours...","Recording source...");
    updateMatchVisibility();repaint();
}
void LoveEQAudioProcessorEditor::runMatchCaptureFrame() {
    if(!matchMode||matchCaptureMode==0)return;
    // A real audio spectrum from the incoming channel, unaffected by EQ processing.
    updateFFT(true,preSpectrum);
    if(matchCaptureMode==1)
        matchReference.addFrame(preSpectrum.data(),(int)preSpectrum.size(),
            juce::jmax(8000.0,processor.getSampleRate()),2048);
    else
        matchSource.addFrame(preSpectrum.data(),(int)preSpectrum.size(),
            juce::jmax(8000.0,processor.getSampleRate()),2048);
    if(--matchFramesRemaining>0)return;
    const bool ref=matchCaptureMode==1;
    matchCaptureMode=0;
    const bool ready=ref?matchReference.ready():matchSource.ready();
    matchStatus=!ready?tr(
          u8"Signal trop faible : capture refusée","Signal too quiet: capture rejected"):
          ref?tr(u8"Référence capturée. Lire la source puis CAPTURER SOURCE.",
                  "Reference captured. Play source and capture it."):
          tr(u8"Source capturée : cliquez sur PROPOSER",
             "Source captured: click SUGGEST");
    updateMatchVisibility();repaint();
}
void LoveEQAudioProcessorEditor::proposeMatch() {
    if(!matchMode||!matchReference.ready()||!matchSource.ready())return;
    if(matchBaselineValid)restoreMatchBaseline();
    matchCurve=loveeq::match::calculate(matchReference,matchSource,6.f);
    matchStatus=loveeq::match::finiteAndBounded(matchCurve)?
        tr(u8"Proposition prête : écoutez B avant de confirmer",
           "Suggestion ready: audition B before confirming"):
        tr(u8"Références insuffisantes : aucune correction",
           "Insufficient source analysis: no correction");
    updateMatchVisibility();repaint();
}
void LoveEQAudioProcessorEditor::previewMatch() {
    if(!loveeq::match::finiteAndBounded(matchCurve))return;
    if(matchPreviewing) {
        restoreMatchBaseline();
        matchStatus=tr(u8"A ORIGINAL : état initial restauré",
                       "A ORIGINAL: initial state restored");
    } else {
        if(!matchBaselineValid)rememberMatchBaseline();
        processor.undoManager.beginNewTransaction(tr(
            u8"EQ MATCH : préécoute B","EQ MATCH: audition B"));
        for(int i=0;i<loveeq::match::bands;++i)
            setParam("match_gain_"+juce::String(i+1),matchCurve.gainDb[(size_t)i]);
        setParam("match_active",1.f);
        processor.undoManager.beginNewTransaction();
        matchPreviewing=true;
        matchStatus=tr(u8"B ÉCOUTER : correction active, non conservée",
                       "B AUDITION: correction active, not kept");
    }
    updateMatchVisibility();repaint();
}
void LoveEQAudioProcessorEditor::commitMatch() {
    if(!matchPreviewing||!matchBaselineValid)return;
    matchBaselineValid=false;
    matchPreviewing=false;
    matchStatus=tr(u8"EQ MATCH conservé : sauvegardable dans USER",
                   "EQ MATCH kept: save it in USER presets");
    updateMatchVisibility();repaint();
}
void LoveEQAudioProcessorEditor::cancelMatch() {
    if(matchBaselineValid)restoreMatchBaseline();
    matchPreviewing=false;matchBaselineValid=false;
    matchCurve={};
    matchStatus=tr(u8"EQ MATCH annulé : son original restauré",
                   "EQ MATCH cancelled: original sound restored");
    updateMatchVisibility();repaint();
}
void LoveEQAudioProcessorEditor::closeMatchView() {
    if(matchBaselineValid)cancelMatch();
    matchMode=false;matchCaptureMode=0;matchFramesRemaining=0;
    matchModeButton.setToggleState(false,juce::dontSendNotification);
    updateGraphicVisibility();repaint();
}
void LoveEQAudioProcessorEditor::initializeIntent() {
    for(juce::Component* c:std::array<juce::Component*,8>{
        &intentModeButton,&intentGoalBox,&intentAnalyzeButton,
        &intentOriginalButton,&intentPreviewButton,
        &intentCommitButton,&intentCancelButton,&intentAmountSlider})
        addAndMakeVisible(*c);
    for(auto& button:intentVariantButtons)addAndMakeVisible(button);
    intentModeButton.setClickingTogglesState(true);
    intentModeButton.onClick=[this] {
        if(matchMode)closeMatchView();
        const bool opening=intentModeButton.getToggleState();
        if(!opening)closeIntentView();
        else {
            if(timeLensMode) {
                timeLensMode=false;
                timeLensModeButton.setToggleState(false,juce::dontSendNotification);
            }
            if(relationMode) {
                relationMode=false;
                relationModeButton.setToggleState(false,juce::dontSendNotification);
            }
            if(sketchMode) {
                sketchMode=false;sketchActive=false;sketchPoints.clear();
                sketchButton.setToggleState(false,juce::dontSendNotification);
            }
            intentMode=true;updateGraphicVisibility();repaint();
        }
    };
    intentAnalyzeButton.onClick=[this]{measureIntent();};
    intentGoalBox.onChange=[this]{
        if(intentOriginalValid)restoreIntentBaseline();
        intentSuggestionsReady=false;
        intentMessage=tr(u8"Nouvelle intention : cliquez sur ANALYSER",
                         "New goal: click ANALYZE");
        updateIntentVisibility();repaint();
    };
    for(int i=0;i<3;++i)
        intentVariantButtons[(size_t)i].onClick=[this,i]{setIntentVariant(i);};
    intentOriginalButton.onClick=[this]{listenIntent(false);};
    intentPreviewButton.onClick=[this]{listenIntent(true);};
    intentCommitButton.onClick=[this]{commitIntent();};
    intentCancelButton.onClick=[this]{cancelIntent();};
    intentAmountSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    intentAmountSlider.setTextBoxStyle(juce::Slider::TextBoxRight,false,73,23);
    intentAmountSlider.setTextValueSuffix(" %");
    intentAmountSlider.setColour(juce::Slider::backgroundColourId,juce::Colour(0xff0f1720));
    intentAmountSlider.setColour(juce::Slider::trackColourId,juce::Colour(0xffc5af81));
    intentAmountAttachment=std::make_unique<SliderAttachment>(
         processor.parameters,"intent_amount",intentAmountSlider);
    for(int i=0;i<3;++i) {
        const juce::String id="intent_"+juce::String(i+1)+"_";
        auto add=[this](juce::Slider& slider,const juce::String& path,
                        std::unique_ptr<SliderAttachment>& attachment,const juce::String& unit) {
            addAndMakeVisible(slider);
            slider.setSliderStyle(juce::Slider::LinearHorizontal);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,79,18);
            slider.setTextValueSuffix(unit);
            slider.setColour(juce::Slider::backgroundColourId,juce::Colour(0xff121b24));
            slider.setColour(juce::Slider::trackColourId,juce::Colour(0xffd7b87f));
            attachment=std::make_unique<SliderAttachment>(processor.parameters,path,slider);
        };
        add(intentFrequencySlider[(size_t)i],id+"hz",intentFreqAttach[(size_t)i]," Hz");
        add(intentGainSlider[(size_t)i],id+"gain",intentGainAttach[(size_t)i]," dB");
        add(intentQSlider[(size_t)i],id+"q",intentQAttach[(size_t)i],"");
    }
    intentVariantButtons[1].setToggleState(true,juce::dontSendNotification);
    updateIntentLabels();
    updateIntentVisibility();
}
void LoveEQAudioProcessorEditor::updateIntentLabels() {
    intentModeButton.setButtonText("LOVE INTENT");
    intentAnalyzeButton.setButtonText(tr("ANALYSER","ANALYZE"));
    intentVariantButtons[0].setButtonText(tr("DOUX","SOFT"));
    intentVariantButtons[1].setButtonText(tr(u8"ÉQUILIBRÉ","BALANCED"));
    intentVariantButtons[2].setButtonText(tr(u8"AFFIRMÉ","PRESENT"));
    intentOriginalButton.setButtonText("A ORIGINAL");
    intentPreviewButton.setButtonText(tr("B PROPOSITION","B SUGGESTED"));
    intentCommitButton.setButtonText(tr("CONSERVER","KEEP"));
    intentCancelButton.setButtonText(tr("ANNULER","CANCEL"));
    const auto previous=intentGoalBox.getSelectedId();
    intentGoalBox.clear(juce::dontSendNotification);
    intentGoalBox.addItemList(juce::StringArray{
        tr(u8"Plus clair sans agressivité","Clarity without harshness"),
        tr(u8"Préserver la chaleur de la voix","Preserve vocal warmth"),
        tr(u8"Adoucir les aigus","Soften harsh highs"),
        tr(u8"Définir la basse","Define bass")},1);
    intentGoalBox.setSelectedId(previous>0?previous:1,juce::dontSendNotification);
    intentModeButton.setTooltip(tr(
        u8"Suggestions musicales fondées sur le spectre : aucune action avant votre écoute",
        "Spectrally informed musical suggestions; no audio change until you preview"));
    intentAnalyzeButton.setTooltip(tr(
        u8"Mesurer le spectre d'entrée et préparer trois variantes",
        "Measure the input spectrum and prepare three alternatives"));
    intentOriginalButton.setTooltip(tr(
        u8"Rétablir exactement les réglages LOVE INTENT précédant la proposition",
        "Restore the original LOVE INTENT parameters"));
    intentPreviewButton.setTooltip(tr(
        u8"Écouter la variante choisie, sans modifier les huit bandes d'origine",
        "Audition the chosen variant without changing the original eight bands"));
    intentCommitButton.setTooltip(tr(
        u8"Garder la proposition entendue dans les paramètres du projet",
        "Keep the auditioned suggestion in project parameters"));
    intentCancelButton.setTooltip(tr(
        u8"Annuler et revenir à l'état avant l'analyse",
        "Discard suggestions and restore the original state"));
}
void LoveEQAudioProcessorEditor::updateIntentVisibility() {
    const bool visible=intentMode && !matchMode;
    for(juce::Component* c:std::array<juce::Component*,7>{
       &intentGoalBox,&intentAnalyzeButton,&intentOriginalButton,
       &intentPreviewButton,&intentCommitButton,&intentCancelButton,&intentAmountSlider})
        c->setVisible(visible);
    for(int i=0;i<3;++i) {
        intentVariantButtons[(size_t)i].setVisible(visible);
        intentFrequencySlider[(size_t)i].setVisible(visible);
        intentGainSlider[(size_t)i].setVisible(visible);
        intentQSlider[(size_t)i].setVisible(visible);
        intentVariantButtons[(size_t)i].setToggleState(
            intentVariant==i,juce::dontSendNotification);
    }
    intentPreviewButton.setEnabled(visible&&intentSuggestionsReady);
    intentOriginalButton.setEnabled(visible&&intentOriginalValid);
    intentCommitButton.setEnabled(visible&&intentPreviewing);
    intentCancelButton.setEnabled(visible&&intentOriginalValid);
    const auto* enabled=processor.parameters.getRawParameterValue("intent_enabled");
    const bool editable=intentPreviewing || (!intentOriginalValid &&
        enabled && enabled->load()>0.5f);
    intentAmountSlider.setEnabled(visible && editable);
    for(int i=0;i<3;++i) {
        intentFrequencySlider[(size_t)i].setEnabled(visible && editable);
        intentGainSlider[(size_t)i].setEnabled(visible && editable);
        intentQSlider[(size_t)i].setEnabled(visible && editable);
    }
}
void LoveEQAudioProcessorEditor::captureIntentBaseline() {
    auto value=[this](const juce::String& name) {
        if(const auto* p=processor.parameters.getRawParameterValue(name))return p->load();
        return 0.f;
    };
    intentBaseline[0]=value("intent_enabled");
    intentBaseline[1]=value("intent_amount");
    for(int i=0;i<3;++i) {
        const auto id="intent_"+juce::String(i+1)+"_";
        intentBaseline[(size_t)(2+i*3)]=value(id+"hz");
        intentBaseline[(size_t)(3+i*3)]=value(id+"gain");
        intentBaseline[(size_t)(4+i*3)]=value(id+"q");
    }
    intentOriginalValid=true;
}
void LoveEQAudioProcessorEditor::restoreIntentBaseline() {
    if(!intentOriginalValid)return;
    processor.undoManager.beginNewTransaction(tr(
        u8"LOVE INTENT : restaurer l'original","LOVE INTENT: restore original"));
    // Restore all original parameters, including an existing enabled state.
    for(int i=0;i<3;++i) {
        const auto id="intent_"+juce::String(i+1)+"_";
        setParam(id+"hz",intentBaseline[(size_t)(2+i*3)]);
        setParam(id+"gain",intentBaseline[(size_t)(3+i*3)]);
        setParam(id+"q",intentBaseline[(size_t)(4+i*3)]);
    }
    setParam("intent_amount",intentBaseline[1]);
    setParam("intent_enabled",intentBaseline[0]);
    processor.undoManager.beginNewTransaction();
    intentPreviewing=false;
}
void LoveEQAudioProcessorEditor::measureIntent() {
    if(!intentMode)return;
    if(intentOriginalValid)restoreIntentBaseline();
    updateFFT(true,preSpectrum);
    intentSpectrum=loveeq::intent::inspect(
        preSpectrum.data(),(int)preSpectrum.size(),
        juce::jmax(8000.0,processor.getSampleRate()),2048);
    if(!intentSpectrum.valid) {
        intentSuggestionsReady=false;intentOriginalValid=false;
        intentMessage=tr(u8"Signal trop faible : lancez la lecture, puis ANALYSER",
                         "Source too quiet: play audio, then ANALYZE");
        updateIntentVisibility();repaint();return;
    }
    captureIntentBaseline();
    const int goal=juce::jlimit(0,3,intentGoalBox.getSelectedId()-1);
    for(int i=0;i<3;++i)
        intentSuggestions[(size_t)i]=loveeq::intent::propose(
            static_cast<loveeq::intent::Goal>(goal),i,intentSpectrum);
    intentVariant=1;
    intentSuggestionsReady=true;
    intentMessage=tr(u8"Trois courbes proposées : choisissez, écoutez A/B, puis confirmez",
                     "Three suggestions ready: choose, compare A/B, then confirm");
    updateIntentVisibility();repaint();
}
void LoveEQAudioProcessorEditor::setIntentVariant(int next) {
    if(intentOriginalValid&&intentPreviewing)restoreIntentBaseline();
    intentVariant=juce::jlimit(0,2,next);
    if(intentSuggestionsReady)
        intentMessage=tr(u8"Variante choisie : B PROPOSITION pour écouter",
                         "Variant chosen: press B SUGGESTED to audition");
    updateIntentVisibility();repaint();
}
void LoveEQAudioProcessorEditor::listenIntent(bool suggestion) {
    if(!intentOriginalValid)return;
    if(!suggestion) {
        restoreIntentBaseline();
        intentMessage=tr(u8"A ORIGINAL : son avant la proposition",
                         "A ORIGINAL: sound before suggestion");
    } else {
        if(!intentSuggestionsReady)return;
        const auto& proposal=intentSuggestions[(size_t)intentVariant];
        if(!loveeq::intent::bounded(proposal))return;
        processor.undoManager.beginNewTransaction(tr(
            u8"LOVE INTENT : préécoute de la proposition","LOVE INTENT: preview suggestion"));
        // Explicit B click: only LOVE INTENT's independent three filters are touched.
        for(int i=0;i<3;++i) {
            const auto id="intent_"+juce::String(i+1)+"_";
            const auto& band=proposal.bands[(size_t)i];
            setParam(id+"hz",band.hz);
            setParam(id+"gain",band.gain);
            setParam(id+"q",band.q);
        }
        setParam("intent_enabled",1.f);
        processor.undoManager.beginNewTransaction();
        intentPreviewing=true;
        intentMessage=tr(u8"B PROPOSITION : écoute active, pas encore conservée",
                         "B SUGGESTED: auditioning, not yet kept");
    }
    updateIntentVisibility();repaint();
}
void LoveEQAudioProcessorEditor::commitIntent() {
    if(!intentMode||!intentPreviewing||!intentOriginalValid)return;
    intentOriginalValid=false;
    intentPreviewing=false;
    intentSuggestionsReady=false;
    intentMessage=tr(u8"Courbe conservée ; enregistrez-la dans les presets USER",
                     "Suggestion kept; save it to USER presets");
    updateIntentVisibility();repaint();
}
void LoveEQAudioProcessorEditor::cancelIntent() {
    if(intentOriginalValid)restoreIntentBaseline();
    intentOriginalValid=false;
    intentSuggestionsReady=false;
    intentPreviewing=false;
    intentMessage=tr(u8"Proposition annulée, réglages initiaux restaurés",
                     "Suggestion cancelled, original state restored");
    updateIntentVisibility();repaint();
}
void LoveEQAudioProcessorEditor::closeIntentView() {
    if(intentOriginalValid)cancelIntent();
    intentMode=false;
    intentModeButton.setToggleState(false,juce::dontSendNotification);
    updateGraphicVisibility();
    repaint();
}
void LoveEQAudioProcessorEditor::initializeTimeLens() {
    addAndMakeVisible(timeLensModeButton);
    addAndMakeVisible(timeLensEnableButton);
    timeLensModeButton.setClickingTogglesState(true);
    timeLensModeButton.onClick=[this]{
        if(matchMode)closeMatchView();
        if(intentMode)closeIntentView();
        const bool requested=timeLensModeButton.getToggleState();
        // Opening the editor must NEVER switch the active EQ audio engine.
        if(requested && relationMode){
            relationMode=false;
            relationModeButton.setToggleState(false,juce::dontSendNotification);
        }
        timeLensMode=requested;
        updateGraphicVisibility();
        repaint();
    };
    timeLensModeButton.setTooltip(tr(
        u8"Afficher les trois égalisations de l'attaque, du corps et de la fin",
        "View the independent Attack, Body and Tail equalizers"));
    timeLensEnableButton.setClickingTogglesState(true);
    timeEnableAttach=std::make_unique<ButtonAttachment>(
        processor.parameters,"time_lens_enabled",timeLensEnableButton);
    timeLensEnableButton.onClick=[this]{updateTimeLensButtonLabel();repaint();};
    timeLensEnableButton.setTooltip(tr(
        u8"Activer ou désactiver le DSP TIME LENS, avec fondu sans clic",
        "Enable or disable the TIME LENS DSP with a smoothed transition"));

    auto configureSlider=[this](juce::Slider& slider,const juce::String& id,
                                std::unique_ptr<SliderAttachment>& attachment,
                                const juce::String& unit) {
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,86,18);
        slider.setColour(juce::Slider::backgroundColourId,juce::Colour(0xff0f1821));
        slider.setColour(juce::Slider::trackColourId,juce::Colour(0xffdec28a));
        slider.setColour(juce::Slider::thumbColourId,juce::Colour(0xffe1ceb0));
        slider.setTextValueSuffix(unit);
        addAndMakeVisible(slider);
        attachment=std::make_unique<SliderAttachment>(processor.parameters,id,slider);
    };
    for(int i=0;i<LoveEQAudioProcessor::timeStageCount;++i) {
        configureSlider(timeHzSlider[(size_t)i],
            LoveEQAudioProcessor::timeStageId(i,"hz"),timeHzAttach[(size_t)i]," Hz");
        configureSlider(timeGainSlider[(size_t)i],
            LoveEQAudioProcessor::timeStageId(i,"gain"),timeGainAttach[(size_t)i]," dB");
        configureSlider(timeQSlider[(size_t)i],
            LoveEQAudioProcessor::timeStageId(i,"q"),timeQAttach[(size_t)i],"");
    }
    configureSlider(timeAttackSlider,"time_lens_attack_ms",timeAttackAttach," ms");
    configureSlider(timeTailSlider,"time_lens_tail_ms",timeTailAttach," ms");
    configureSlider(timeThresholdSlider,"time_lens_threshold_db",timeThresholdAttach," dB");
    timeThresholdSlider.setTooltip(tr(
        u8"Seuil de détection des attaques et fins de notes",
        "Level used to detect note attacks and releases"));
    timeAttackSlider.setTooltip(tr(
        u8"Durée de l'égalisation de l'attaque",
        "Duration of the attack EQ stage"));
    timeTailSlider.setTooltip(tr(
        u8"Durée de l'égalisation de fin de note",
        "Duration of the note-tail EQ stage"));
    updateTimeLensVisibility();
    updateTimeLensButtonLabel();
}
void LoveEQAudioProcessorEditor::updateTimeLensButtonLabel() {
    const auto* enabled=processor.parameters.getRawParameterValue("time_lens_enabled");
    const bool on=enabled && enabled->load()>0.5f;
    timeLensEnableButton.setButtonText(on?tr(u8"DÉSACTIVER","DISABLE"):tr("ACTIVER","ENABLE"));
}
void LoveEQAudioProcessorEditor::updateTimeLensVisibility() {
    const bool visible=timeLensMode && !intentMode && !matchMode;
    timeLensEnableButton.setVisible(visible);
    for(int i=0;i<LoveEQAudioProcessor::timeStageCount;++i) {
        timeHzSlider[(size_t)i].setVisible(visible);
        timeGainSlider[(size_t)i].setVisible(visible);
        timeQSlider[(size_t)i].setVisible(visible);
    }
    timeAttackSlider.setVisible(visible);
    timeTailSlider.setVisible(visible);
    timeThresholdSlider.setVisible(visible);
}
void LoveEQAudioProcessorEditor::initializeRelationship() {
    for(juce::Component* c:std::array<juce::Component*,6>{
        &relationModeButton,&relationRoleBox,&relationPriorityBox,
        &relationGroupBox,&relationSuggestButton,&relationApplyButton})
        addAndMakeVisible(*c);
    relationRoleBox.addItemList(juce::StringArray{
        tr(u8"Non affecté","Unassigned"),tr("Voix","Voice"),
        tr("Basse","Bass"),tr("Guitare","Guitar"),tr("Autre","Other")},1);
    relationPriorityBox.addItemList(juce::StringArray{
        tr("Voix","Voice"),tr("Basse","Bass"),tr("Guitare","Guitar"),tr("Autre","Other")},1);
    relationGroupBox.addItemList(juce::StringArray{
        "OFF","1","2","3","4"},1);
    const auto read=[this](const juce::String& key)->int {
        if(const auto* parameter=processor.parameters.getRawParameterValue(key))
            return (int)parameter->load();
        return 0;
    };
    relationRoleBox.setSelectedId(read("relationship_role")+1,juce::dontSendNotification);
    relationPriorityBox.setSelectedId(read("relationship_priority")+1,juce::dontSendNotification);
    relationGroupBox.setSelectedId(read("relationship_group")+1,juce::dontSendNotification);
    const auto changed=[this](const juce::String& id,int next){
        setParam(id,(float)next);
        setParam("relationship_active",0.f);
        relationSuggested=false;
        relationProposedHz=0.f;
        relationStatus=tr(u8"Routage mis à jour","Routing updated");
        updateRelationshipControls();
        repaint();
    };
    relationRoleBox.onChange=[this,changed]{changed("relationship_role",relationRoleBox.getSelectedId()-1);};
    relationPriorityBox.onChange=[this,changed]{changed("relationship_priority",relationPriorityBox.getSelectedId()-1);};
    relationGroupBox.onChange=[this,changed]{changed("relationship_group",relationGroupBox.getSelectedId()-1);};
    relationModeButton.setClickingTogglesState(true);
    relationModeButton.onClick=[this] {
        if(matchMode)closeMatchView();
        if(intentMode)closeIntentView();
        relationMode=relationModeButton.getToggleState();
        if(relationMode && timeLensMode) {
            timeLensMode=false;
            timeLensModeButton.setToggleState(false,juce::dontSendNotification);
        }
        if(relationMode && activeTab==2)selectTab(0);
        updateDnaControls();
        updateRelationshipControls();
        repaint();
    };
    relationSuggestButton.onClick=[this]{proposeRelationship();};
    relationApplyButton.onClick=[this]{applyRelationship();};
    relationModeButton.setTooltip(tr(
       u8"Relier les spectres de plusieurs instances LOVE EQ dans le même processus",
       "Share spectral activity between LOVE EQ instances in one process"));
    relationStatus=tr(u8"Choisir le rôle, la priorité et le groupe","Select role, priority and group");
    relationSuggestButton.setTooltip(relationStatus);
    relationApplyButton.setTooltip(tr(u8"Atténuation spectrale dynamique uniquement après validation",
                                      "Opt-in dynamic spectral reduction"));
    updateRelationshipControls();
}
void LoveEQAudioProcessorEditor::updateRelationshipControls() {
    const bool shown=relationMode && activeTab!=2 && !timeLensMode && !intentMode && !matchMode;
    for(juce::Component* component:std::array<juce::Component*,5>{
        &relationRoleBox,&relationPriorityBox,&relationGroupBox,
        &relationSuggestButton,&relationApplyButton})
        component->setVisible(shown);
    const auto role=processor.parameters.getRawParameterValue("relationship_role");
    const auto group=processor.parameters.getRawParameterValue("relationship_group");
    const auto priority=processor.parameters.getRawParameterValue("relationship_priority");
    const auto active=processor.parameters.getRawParameterValue("relationship_active");
    const int r=role?(int)role->load():0;
    const int g=group?(int)group->load():0;
    const int p=priority?(int)priority->load()+1:1;
    const bool valid=shown && r>0 && g>0 && r!=p && processor.relationshipInstanceId()>=0;
    const bool enabled=active && active->load()>0.5f;
    relationSuggestButton.setEnabled(valid);
    relationApplyButton.setButtonText(enabled?tr(u8"ARRÊTER","STOP"):tr("ACTIVER","ENABLE"));
    const bool guarded=dnaEnabled && dnaResult.reliable && relationSuggested &&
        !loveeq::relationship::isProtected(relationProposedHz,dnaResult.fundamentalHz);
    relationApplyButton.setEnabled(shown && (enabled || (valid && guarded)));
    relationSuggestButton.setTooltip(relationStatus);
    relationApplyButton.setTooltip(enabled?
        tr(u8"Arrêter la réduction dynamique sans changer les autres bandes",
           "Stop dynamic reduction without changing other EQ bands"):
        !dnaEnabled || !dnaResult.reliable?
        tr(u8"Harmonic DNA doit identifier une fondamentale fiable avant toute correction",
           "Harmonic DNA must identify a reliable fundamental before applying"):
        tr(u8"Appliquer la correction dynamique proposée, maximum 3 dB",
           "Apply the proposed dynamic reduction, maximum 3 dB"));
}
void LoveEQAudioProcessorEditor::proposeRelationship() {
    relationSuggested=false;
    const auto own=processor.relationshipOwnFrame();
    const auto other=processor.relationshipPriorityFrame();
    relationReferenceCount=other.count;
    if(!own.valid||!other.valid) {
        relationStatus=tr(u8"Ajouter LOVE EQ sur une autre piste du même groupe",
                           "Add LOVE EQ to another track in the same group");
        updateRelationshipControls();repaint();return;
    }
    const auto suggestion=loveeq::relationship::propose(
        own,other,dnaResult.reliable?dnaResult.fundamentalHz:0.f);
    if(!suggestion.valid) {
        relationStatus=tr(u8"Aucun masquage significatif non protégé",
                           "No meaningful unprotected masking found");
        updateRelationshipControls();repaint();return;
    }
    relationProposedHz=suggestion.hz;
    relationProposedScore=suggestion.score;
    relationSuggested=true;
    relationStatus=dnaResult.reliable?
        tr(u8"Conflit repéré : écoutez avant d'activer","Overlap found: listen before enabling"):
        tr(u8"Harmoniques incertaines : correction bloquée","Pitch uncertain: correction disabled");
    updateRelationshipControls();
    repaint();
}
void LoveEQAudioProcessorEditor::applyRelationship() {
    const auto active=processor.parameters.getRawParameterValue("relationship_active");
    if(active && active->load()>0.5f) {
        setParam("relationship_active",0.f);
        relationStatus=tr(u8"Atténuation dynamique arrêtée","Dynamic reduction stopped");
        updateRelationshipControls();repaint();return;
    }
    if(!relationSuggested || !dnaEnabled || !dnaResult.reliable ||
       loveeq::relationship::isProtected(relationProposedHz,dnaResult.fundamentalHz) ||
       !processor.relationshipPriorityFrame().valid)return;
    processor.undoManager.beginNewTransaction(tr(
        u8"Relation EQ : atténuation dynamique confirmée","Relationship EQ: confirmed dynamic cut"));
    setParam("relationship_hz",relationProposedHz);
    setParam("relationship_q",1.4f);
    setParam("relationship_max_cut",3.f);
    setParam("relationship_guard_hz",dnaResult.fundamentalHz);
    setParam("relationship_active",1.f);
    processor.undoManager.beginNewTransaction();
    relationSuggested=false;
    relationStatus=tr(u8"Correction dynamique active (0 à -3 dB)",
                      "Dynamic correction active (0 to -3 dB)");
    updateRelationshipControls();
    repaint();
}
void LoveEQAudioProcessorEditor::selectTab(int tab) {
    activeTab=juce::jlimit(0,2,tab);
    tabParam.setToggleState(activeTab==0,juce::dontSendNotification);
    tabDynamic.setToggleState(activeTab==1,juce::dontSendNotification);
    tabGraphic.setToggleState(activeTab==2,juce::dontSendNotification);
    setParam("eq_mode",activeTab==2?1.f:0.f);
    updateGraphicVisibility();
    repaint();
}
void LoveEQAudioProcessorEditor::updateGraphicVisibility() {
    const bool graphic=activeTab==2;
    if(graphic && relationMode){
        relationMode=false;
        relationModeButton.setToggleState(false,juce::dontSendNotification);
    }
    for(auto& button:bandButtons)button.setVisible(!graphic && !timeLensMode && !intentMode && !matchMode);
    for(int i=0;i<8;++i) {
        auto& tile=bandButtons[(size_t)i];
        tile.selected=i==selectedBand;
        tile.english=english;
        tile.repaint();
    }
    for(auto& slider:graphicSliders)slider.setVisible(graphic && !timeLensMode && !intentMode && !matchMode);
    updateTimeLensVisibility();
    updateIntentVisibility();
    updateMatchVisibility();
    sketchButton.setEnabled(!intentMode && !matchMode);
    grabButton.setEnabled(!intentMode && !matchMode);
    updateDnaControls();
    updateRelationshipControls();
    for(juce::Component* component : std::array<juce::Component*,13>{
            &frequency,&gain,&q,&threshold,&range,&attack,&release,
            &typeBox,&channelBox,&slopeBox,&enabledButton,&soloButton,&dynamicButton})
        component->setVisible(!graphic && !timeLensMode && !intentMode && !matchMode);
}
void LoveEQAudioProcessorEditor::updateLanguageSafeTexts() {
    tabParam.setButtonText(tr(u8"PARAMÉTRIQUE","PARAMETRIC"));
    tabDynamic.setButtonText(tr("DYNAMIQUE","DYNAMIC"));
    tabGraphic.setButtonText(tr("GRAPHIQUE","GRAPHIC"));
    browserButton.setButtonText("PRESETS");
    browserButton.setTooltip(tr(u8"Ouvrir la bibliothèque des 100 presets","Open the 100-preset browser"));
    resetButton.setButtonText("RESET / FLAT");
    holdButton.setButtonText(tr("MAINTIEN PIC","PEAK HOLD"));
    freezeButton.setButtonText(tr("FIGER","FREEZE"));
    enabledButton.setButtonText(tr("ACTIF","ON"));
    dynamicButton.setButtonText(tr("DYN","DYN"));
    soloButton.setButtonText("SOLO");
    peakResetButton.setButtonText("PEAK RESET");
    sketchButton.setButtonText(tr("DESSIN","SKETCH"));
    notesButton.setButtonText(tr("NOTES","NOTES"));
    grabButton.setButtonText(tr("CAPTER PIC","GRAB PEAK"));
    dnaButton.setButtonText("HARMONIC DNA");
    dnaFocusButton.setButtonText(tr("CIBLER","TARGET"));
    dnaCutButton.setButtonText(tr(u8"ATTÉNUER","CUT -2 dB"));
    relationModeButton.setButtonText("RELATION EQ");
    timeLensModeButton.setButtonText("TIME LENS");
    updateIntentLabels();
    updateMatchLanguage();
    updateTimeLensButtonLabel();
    relationSuggestButton.setButtonText(tr("PROPOSER","SUGGEST"));
    const int keepRole=relationRoleBox.getSelectedId();
    const int keepPriority=relationPriorityBox.getSelectedId();
    if(keepRole>0 && keepPriority>0) {
        relationRoleBox.clear(juce::dontSendNotification);
        relationRoleBox.addItemList(juce::StringArray{
            tr(u8"Non affecté","Unassigned"),tr("Voix","Voice"),
            tr("Basse","Bass"),tr("Guitare","Guitar"),tr("Autre","Other")},1);
        relationRoleBox.setSelectedId(keepRole,juce::dontSendNotification);
        relationPriorityBox.clear(juce::dontSendNotification);
        relationPriorityBox.addItemList(juce::StringArray{
            tr("Voix","Voice"),tr("Basse","Bass"),tr("Guitare","Guitar"),tr("Autre","Other")},1);
        relationPriorityBox.setSelectedId(keepPriority,juce::dontSendNotification);
    }
    updateRelationshipControls();
    sketchButton.setTooltip(tr(u8"Dessiner une courbe : le moteur adapte huit filtres réels",
                                 "Draw a curve, then fit eight real EQ filters"));
    notesButton.setTooltip(tr(u8"Afficher les notes. Alt-glisser pour caler une fréquence",
                                "Display note guides. Alt-drag to snap a frequency"));
    grabButton.setTooltip(tr(u8"Repérer une résonance dans le spectre et placer la bande choisie, sans modifier son gain",
                               "Find a spectral resonance and position the selected band with no gain change"));
    // Localise filter names without changing their underlying parameter indices.
    const int currentType=juce::jmax(1,typeBox.getSelectedId());
    typeBox.clear(juce::dontSendNotification);
    const juce::StringArray types=english?
        juce::StringArray{"Bell","Low Shelf","High Shelf","High Pass","Low Pass","Notch","Band Pass"}:
        juce::StringArray{"Cloche","Shelf bas","Shelf haut","Passe-H","Passe-B","Coupe","Passe-bande"};
    typeBox.addItemList(types,1);
    typeBox.setSelectedId(currentType,juce::dontSendNotification);
    presetSearch.setTextToShowWhenEmpty(tr("Rechercher 100 presets...","Search 100 presets..."),juce::Colours::grey);
    undoButton.setTooltip(tr("Annuler","Undo"));
    redoButton.setTooltip(tr(u8"Rétablir","Redo"));
    resetButton.setTooltip(tr(u8"Remettre les paramètres à plat","Reset all EQ parameters"));
    peakResetButton.setTooltip(tr(u8"Effacer les crêtes IN/OUT","Reset input and output peak hold"));
    repaint();
}
void LoveEQAudioProcessorEditor::setParam(const juce::String& id,float v) {
    if(auto* p=processor.parameters.getParameter(id)) p->setValueNotifyingHost(p->convertTo0to1(v));
}
void LoveEQAudioProcessorEditor::attachBand(int index) {
    if(dnaTargetArmed && index!=selectedBand) dnaTargetArmed=false;
    selectedBand=juce::jlimit(0,7,index);
    for(int i=0;i<8;++i) {
        bandButtons[(size_t)i].selected=i==selectedBand;
        bandButtons[(size_t)i].repaint();
    }
    updateDnaControls();
    hzAttach.reset();gainAttach.reset();qAttach.reset();typeAttach.reset();channelAttach.reset();slopeAttach.reset();
    enabledAttach.reset();soloAttach.reset();dynamicAttach.reset();
    thresholdAttach.reset();rangeAttach.reset();attackAttach.reset();releaseAttach.reset();
    const auto id=[this](const juce::String& s){return LoveEQAudioProcessor::bandId(selectedBand,s);};
    hzAttach=std::make_unique<SliderAttachment>(processor.parameters,id("hz"),frequency);
    gainAttach=std::make_unique<SliderAttachment>(processor.parameters,id("gain"),gain);
    qAttach=std::make_unique<SliderAttachment>(processor.parameters,id("q"),q);
    typeAttach=std::make_unique<ComboAttachment>(processor.parameters,id("type"),typeBox);
    channelAttach=std::make_unique<ComboAttachment>(processor.parameters,id("channel"),channelBox);
    slopeAttach=std::make_unique<ComboAttachment>(processor.parameters,id("slope"),slopeBox);
    enabledAttach=std::make_unique<ButtonAttachment>(processor.parameters,id("enabled"),enabledButton);
    soloAttach=std::make_unique<ButtonAttachment>(processor.parameters,id("solo"),soloButton);
    dynamicAttach=std::make_unique<ButtonAttachment>(processor.parameters,id("dynamic"),dynamicButton);
    thresholdAttach=std::make_unique<SliderAttachment>(processor.parameters,id("threshold"),threshold);
    rangeAttach=std::make_unique<SliderAttachment>(processor.parameters,id("range"),range);
    attackAttach=std::make_unique<SliderAttachment>(processor.parameters,id("attack"),attack);
    releaseAttach=std::make_unique<SliderAttachment>(processor.parameters,id("release"),release);
    for(int i=0;i<8;++i)bandButtons[(size_t)i].setToggleState(i==selectedBand,juce::dontSendNotification);
    repaint();
}
void LoveEQAudioProcessorEditor::refillPresets(){
    const auto previous=presetCombo.getSelectedId();
    presetCombo.clear(juce::dontSendNotification);
    const auto query=presetSearch.getText().trim();
    for(int i=0;i<100;++i) {
        const auto& p=loveeq::factoryPresets()[(size_t)i];
        const juce::String label=juce::String(p.category)+"  |  "+juce::String(p.name);
        if(query.isEmpty() || label.containsIgnoreCase(query)) presetCombo.addItem(label,i+1);
    }
    if(previous>0) presetCombo.setSelectedId(previous,juce::dontSendNotification);
}
void LoveEQAudioProcessorEditor::switchSnapshot(bool toB) {
    if(toB==onB)return;
    if(onB) stateB=processor.parameters.copyState(); else stateA=processor.parameters.copyState();
    const auto favorites=processor.parameters.state.getProperty("factoryFavorites").toString();
    const auto language=processor.getUiLanguage();
    processor.parameters.replaceState(toB?stateB.createCopy():stateA.createCopy());
    // UI language and favorites are preferences, not audio A/B differences.
    processor.parameters.state.setProperty("factoryFavorites",favorites,nullptr);
    processor.setUiLanguage(language);
    selectTab(processor.parameters.getRawParameterValue("eq_mode")->load()>0.5f?2:0);
    onB=toB;
    buttonA.setToggleState(!onB,juce::dontSendNotification);
    buttonB.setToggleState(onB,juce::dontSendNotification);
    repaint();
}
float LoveEQAudioProcessorEditor::xForHz(float hz) const {
    return graph.getX()+graph.getWidth()*std::log(juce::jlimit(20.f,20000.f,hz)/20.f)/std::log(1000.f);
}
float LoveEQAudioProcessorEditor::yForDb(float db) const {
    return graph.getCentreY()-db*(graph.getHeight()/36.f);
}
float LoveEQAudioProcessorEditor::dbAtHz(float hz) const {
    float out=0;
    const double sr=juce::jmax(8000.0,processor.getSampleRate());
    if(activeTab==2) {
        for(int i=0;i<LoveEQAudioProcessor::graphicBandCount;++i) {
            const auto* gainV=processor.parameters.getRawParameterValue(LoveEQAudioProcessor::graphicId(i));
            if(!gainV) continue;
            const auto c=loveeq::design(loveeq::Type::Bell,sr,LoveEQAudioProcessor::graphicHz[(size_t)i],gainV->load(),1.8);
            out+=(float)loveeq::magnitudeDb(c,sr,hz);
        }
        return out;
    }
    for(int i=0;i<8;++i) {
        const auto id=[i](const juce::String& s){return LoveEQAudioProcessor::bandId(i,s);};
        auto value=[this](const juce::String& s){return processor.parameters.getRawParameterValue(s)->load();};
        if(value(id("enabled"))<0.5f)continue;
        const int type=juce::jlimit(0,6,(int)value(id("type")));
        const int slope=juce::jlimit(0,5,(int)value(id("slope")));
        const auto t=static_cast<loveeq::Type>(type);
        const auto c=loveeq::design(t,sr,value(id("hz")),value(id("gain")),value(id("q")));
        if(type==3||type==4) {
            const int count[6]={0,1,1,2,3,4};
            out+=(float)loveeq::magnitudeDb(c,sr,hz)*(float)count[slope];
            if(loveeq::needsFirstOrder(slope))out+=(float)loveeq::magnitudeDb(loveeq::design(t,sr,value(id("hz")),0,1,true),sr,hz);
        } else out+=(float)loveeq::magnitudeDb(c,sr,hz);
    }
    return out;
}
void LoveEQAudioProcessorEditor::drawSpectrum(juce::Graphics& g,const std::array<float,1025>& data,juce::Colour col) {
    if(!graph.isEmpty()&&processor.getSampleRate()>0) {
        juce::Path path;
        for(int x=0;x<graph.getWidth();x+=3) {
            const float hz=20.f*std::pow(1000.f,(float)x/graph.getWidth());
            const int bin=juce::jlimit(1,1024,(int)std::round(hz*LoveEQAudioProcessor::scopeSize/processor.getSampleRate()));
            const float db=juce::jlimit(-96.f,0.f,data[(size_t)bin]);
            const float y=graph.getBottom()-(db+96.f)*(graph.getHeight()/96.f);
            if(x==0)path.startNewSubPath((float)graph.getX(),y);else path.lineTo((float)graph.getX()+x,y);
        }
        g.setColour(col);g.strokePath(path,juce::PathStrokeType(1.4f));
    }
}
void LoveEQAudioProcessorEditor::paint(juce::Graphics& g) {
    const int w=getWidth(), h=getHeight();
    g.fillAll(background);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff222832),0.f,0.f,
                                           juce::Colour(0xff171d25),0.f,(float)h,false));
    g.fillRect(getLocalBounds());

    // Reference-inspired compact header: brand, library, comparisons and bypass.
    g.setColour(gold);
    g.drawLine(31.f,24.f,31.f,54.f,3.f);
    g.drawLine(31.f,54.f,57.f,54.f,3.f);
    g.drawLine(39.f,29.f,39.f,47.f,2.f);
    g.drawLine(39.f,47.f,53.f,47.f,2.f);
    g.setColour(juce::Colour(0xffe8dec9));
    g.setFont(juce::Font(juce::FontOptions(29.f,juce::Font::bold)));
    g.drawText("LOVE EQ",76,16,252,36,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff96a1a9));
    g.setFont(juce::Font(juce::FontOptions("Arial",11.5f,juce::Font::plain)));
    g.drawText("by Liberty Music Software",77,51,250,19,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff7e8b98));
    g.setFont(juce::Font(juce::FontOptions("Arial",9.8f,juce::Font::bold)));
    g.drawText(tr(u8"BIBLIOTHÈQUE DE PRESETS","PRESET LIBRARY"),350,18,265,15,juce::Justification::centredLeft);
    g.setColour(trim);
    g.drawHorizontalLine(83,23.f,(float)w-23.f);

    g.setColour(juce::Colour(0xff1d252c));
    g.fillRoundedRectangle(24.f,94.f,330.f,35.f,5.f);
    g.setColour(trim);
    g.drawRoundedRectangle(24.f,94.f,330.f,35.f,5.f,1.f);
    // The former decorative ZERO LATENCY label makes room for LOVE INTENT.
    // Latency is declared to the host by the actual processor, not guessed here.
    g.setColour(juce::Colour(0xff84939d));

    g.drawText(tr(u8"COURBE : STÉRÉO","CURVE: STEREO"),378,99,175,26,juce::Justification::centredLeft);
    g.drawText(tr(u8"ÉCHELLE : +/- 18 dB","SCALE: +/- 18 dB"),563,99,195,26,juce::Justification::centredLeft);

    // Single analyzer panel with a dedicated toolbar and a narrow meter strip.
    g.setColour(juce::Colour(0xff10171d));
    g.fillRoundedRectangle(spectrumPanel.toFloat(),7.f);
    g.setColour(juce::Colour(0xff39434b));
    g.drawRoundedRectangle(spectrumPanel.toFloat(),7.f,1.f);
    g.drawHorizontalLine(188,(float)spectrumPanel.getX(),(float)spectrumPanel.getRight());
    g.setColour(juce::Colour(0xff778994));
    g.setFont(juce::Font(juce::FontOptions(10.f,juce::Font::bold)));
    g.drawText(tr("SPECTRE","SPECTRUM"),42,156,73,18,juce::Justification::centredLeft);
    g.drawText(tr(u8"RÉPONSE - dB","RESPONSE - dB"),graph.getX(),190,154,16,juce::Justification::centredLeft);
    g.drawText(tr("ANALYSEUR FFT 2048","FFT ANALYZER 2048"),graph.getRight()-196,190,189,16,juce::Justification::centredRight);

    // Logarithmic grid and true biquad response.
    g.setColour(juce::Colour(0xff101820));
    g.fillRect(graph);
    g.saveState();
    g.reduceClipRegion(graph);
    const float freqs[]={20.f,50.f,100.f,200.f,500.f,1000.f,2000.f,5000.f,10000.f,20000.f};
    for(float f:freqs) {
        g.setColour(juce::Colour(0xff29323b));
        g.drawVerticalLine((int)xForHz(f),(float)graph.getY(),(float)graph.getBottom());
    }
    for(int db=-18;db<=18;db+=9) {
        g.setColour(db==0?juce::Colour(0xff655d49):juce::Colour(0xff303943));
        g.drawHorizontalLine((int)yForDb((float)db),(float)graph.getX(),(float)graph.getRight());
    }
    if(showPre)drawSpectrum(g,preSpectrum,juce::Colour(0x885e9db1));
    if(showPost)drawSpectrum(g,postSpectrum,juce::Colour(0x99bcac75));
    if(holdSpectrum) {
        if(showPre)drawSpectrum(g,heldPre,juce::Colour(0x445e9db1));
        if(showPost)drawSpectrum(g,heldPost,juce::Colour(0x55bcac75));
    }
    juce::Path response;
    for(int x=0;x<graph.getWidth();x+=2) {
        const float hz=20.f*std::pow(1000.f,(float)x/graph.getWidth());
        const float y=yForDb(dbAtHz(hz));
        if(x==0) response.startNewSubPath((float)graph.getX(),y);
        else response.lineTo((float)graph.getX()+x,y);
    }
    g.setColour(gold);
    g.strokePath(response,juce::PathStrokeType(2.4f));
    if(matchMode) {
        // Six real filter curves, preview of an unapplied proposal in gold.
        const auto sr=juce::jmax(8000.0,processor.getSampleRate());
        juce::Path matchPath;
        for(int x=0;x<graph.getWidth();x+=3) {
            const float hz=20.f*std::pow(1000.f,(float)x/graph.getWidth());
            double db=0.0;
            for(int i=0;i<loveeq::match::bands;++i) {
                const auto* param=processor.parameters.getRawParameterValue(
                    "match_gain_"+juce::String(i+1));
                const float gainDb=matchCurve.valid&&!matchPreviewing?
                    matchCurve.gainDb[(size_t)i]:param?param->load():0.f;
                const auto coeff=loveeq::design(loveeq::Type::Bell,sr,
                    loveeq::match::centers[(size_t)i],gainDb,0.85f);
                db+=loveeq::magnitudeDb(coeff,sr,hz);
            }
            const float y=yForDb((float)db);
            if(x==0)matchPath.startNewSubPath((float)graph.getX(),y);
            else matchPath.lineTo((float)graph.getX()+x,y);
        }
        g.setColour(juce::Colour(0xffefcc8d));
        g.strokePath(matchPath,juce::PathStrokeType(2.1f));
    }
    if(intentMode) {
        // Display the three real independent Bell corrections. Before B is
        // auditioned these are suggestions only, not processing claims.
        const std::array<juce::Colour,3> colors {
            juce::Colour(0xff87bdc9),juce::Colour(0xffe1bd7f),juce::Colour(0xffbb9ed9)
        };
        const double sampleRate=juce::jmax(8000.0,processor.getSampleRate());
        const bool fromProposal=intentSuggestionsReady && !intentPreviewing;
        const auto& proposal=intentSuggestions[(size_t)intentVariant];
        for(int i=0;i<3;++i) {
            float hz=1000.f,gainDb=0.f,q=1.f;
            if(fromProposal && proposal.valid) {
                hz=proposal.bands[(size_t)i].hz;
                gainDb=proposal.bands[(size_t)i].gain;
                q=proposal.bands[(size_t)i].q;
            } else {
                const auto id="intent_"+juce::String(i+1)+"_";
                auto read=[this](const juce::String& key,float fallback) {
                    if(const auto* v=processor.parameters.getRawParameterValue(key))
                        return v->load();
                    return fallback;
                };
                hz=read(id+"hz",1000.f);
                gainDb=read(id+"gain",0.f);
                q=read(id+"q",1.f);
            }
            const auto coeff=loveeq::design(loveeq::Type::Bell,sampleRate,hz,gainDb,q);
            juce::Path curve;
            for(int x=0;x<graph.getWidth();x+=3) {
                const float frequency=20.f*std::pow(1000.f,(float)x/graph.getWidth());
                const float db=(float)loveeq::magnitudeDb(coeff,sampleRate,frequency);
                const float y=yForDb(db);
                if(x==0)curve.startNewSubPath((float)graph.getX(),y);
                else curve.lineTo((float)graph.getX()+x,y);
            }
            g.setColour(colors[(size_t)i].withAlpha(fromProposal?0.62f:0.92f));
            g.strokePath(curve,juce::PathStrokeType(fromProposal?1.6f:2.2f));
        }
    }
    if(timeLensMode) {
        // These are the actual *individual Bell responses*; the active DSP
        // blends their filtered contributions over time, not the dB curves.
        const std::array<juce::Colour,3> phaseColour {
            juce::Colour(0xffe8bc75),juce::Colour(0xff83cbbd),juce::Colour(0xffbd9be6)
        };
        const int current=processor.timeLensPhase.load(std::memory_order_relaxed);
        const double sampleRate=juce::jmax(8000.0,processor.getSampleRate());
        for(int stage=0;stage<LoveEQAudioProcessor::timeStageCount;++stage) {
            auto read=[this,stage](const juce::String& name) {
                const auto* parameter=processor.parameters.getRawParameterValue(
                    LoveEQAudioProcessor::timeStageId(stage,name));
                return parameter?parameter->load():0.f;
            };
            const auto filter=loveeq::design(loveeq::Type::Bell,sampleRate,
                                               read("hz"),read("gain"),read("q"));
            juce::Path curve;
            for(int x=0;x<graph.getWidth();x+=3) {
                const float hz=20.f*std::pow(1000.f,(float)x/graph.getWidth());
                const float db=(float)loveeq::magnitudeDb(filter,sampleRate,hz);
                const float y=yForDb(db);
                if(x==0)curve.startNewSubPath((float)graph.getX(),y);
                else curve.lineTo((float)graph.getX()+x,y);
            }
            const bool selected=current==stage+1;
            g.setColour(phaseColour[(size_t)stage].withAlpha(selected?0.95f:0.53f));
            g.strokePath(curve,juce::PathStrokeType(selected?2.6f:1.6f));
        }
    }
    if(!timeLensMode && !intentMode && !matchMode && activeTab==2) {
        for(int band=0;band<LoveEQAudioProcessor::graphicBandCount;++band) {
            const auto* gainV=processor.parameters.getRawParameterValue(LoveEQAudioProcessor::graphicId(band));
            if(!gainV) continue;
            const float x=xForHz((float)LoveEQAudioProcessor::graphicHz[(size_t)band]),y=yForDb(gainV->load());
            const auto col=juce::Colour(0xffdfc58c);
            g.setColour(juce::Colour(0xff131b22));
            g.fillEllipse(x-7.f,y-7.f,14.f,14.f);
            g.setColour(col);g.drawEllipse(x-7.f,y-7.f,14.f,14.f,1.6f);
        }
    } else if(!timeLensMode && !intentMode && !matchMode) for(int band=0;band<8;++band) {
        const auto id=[band](const juce::String& name){return LoveEQAudioProcessor::bandId(band,name);};
        const auto* enabled=processor.parameters.getRawParameterValue(id("enabled"));
        const auto* hz=processor.parameters.getRawParameterValue(id("hz"));
        const auto* gainV=processor.parameters.getRawParameterValue(id("gain"));
        if(!enabled || !hz || !gainV || enabled->load()<0.5f) continue;
        const float x=xForHz(hz->load()), y=yForDb(gainV->load());
        const auto col=juce::Colour(bandColours[(size_t)band]);
        const float radius=band==selectedBand?12.f:10.f;
        g.setColour(juce::Colour(0xff10171d));
        g.fillEllipse(x-radius,y-radius,2.f*radius,2.f*radius);
        g.setColour(col);
        g.drawEllipse(x-radius,y-radius,2.f*radius,2.f*radius,band==selectedBand?2.3f:1.5f);
        g.setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
        g.drawText(juce::String(band+1),(int)x-10,(int)y-10,20,20,juce::Justification::centred);
    }
    if(relationMode && !intentMode && !matchMode && relationProposedHz>=65.f && relationProposedHz<=8000.f) {
        const float x=xForHz(relationProposedHz);
        g.setColour(juce::Colour(0x33edc178));
        g.fillRect(juce::Rectangle<float>(x-12.f,(float)graph.getY(),24.f,(float)graph.getHeight()));
        g.setColour(juce::Colour(0xffdfb76b));
        g.drawVerticalLine((int)x,(float)graph.getY(),(float)graph.getBottom());
        g.setFont(juce::Font(juce::FontOptions("Arial",10.f,juce::Font::bold)));
        g.drawText(juce::String((int)relationProposedHz)+" Hz",
           (int)x-42,graph.getY()+9,84,18,juce::Justification::centred);
    }
    if(noteGrid) {
        g.setColour(juce::Colour(0xff809ba9).withAlpha(0.62f));
        g.setFont(juce::Font(juce::FontOptions("Arial",9.f,juce::Font::plain)));
        float lastX=-100.f;
        for(int midi=24;midi<=120;midi+=3) {
            const double hz=440.0*std::pow(2.0,(midi-69)/12.0);
            if(hz<25.0 || hz>18000.0)continue;
            const float x=xForHz((float)hz);
            if(x-lastX<43.f)continue;
            lastX=x;
            g.drawText(juce::String(loveeq::curvelab::noteName(hz)),
                       (int)x-22,graph.getBottom()-22,44,14,juce::Justification::centred);
        }
    }
    if(sketchActive && sketchPoints.size()>1) {
        juce::Path preview;
        for(size_t k=0;k<sketchPoints.size();++k) {
            const auto& point=sketchPoints[k];
            const float px=(float)graph.getX()+(float)point.position*graph.getWidth();
            const float py=yForDb((float)point.gain);
            if(k==0)preview.startNewSubPath(px,py);else preview.lineTo(px,py);
        }
        g.setColour(juce::Colour(0xfff5cd83));
        g.strokePath(preview,juce::PathStrokeType(2.f,juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }
    if(dnaEnabled && !intentMode && !matchMode && activeTab!=2 && dnaResult.reliable) {
        g.setFont(juce::Font(juce::FontOptions("Arial",9.f,juce::Font::bold)));
        float lastX=-100.f;
        for(int i=0;i<dnaResult.harmonicCount;++i) {
            const auto& partial=dnaResult.harmonic[(size_t)i];
            if(partial.hz<20.f||partial.hz>20000.f)continue;
            const float x=xForHz(partial.hz);
            if(x-lastX<9.f)continue;
            lastX=x;
            g.setColour(juce::Colour(0xff79cdbc));
            g.drawVerticalLine((int)x,(float)graph.getY()+38.f,(float)graph.getY()+58.f);
            g.fillEllipse(x-3.f,(float)graph.getY()+35.f,6.f,6.f);
            if(partial.order==1)
                g.drawText("H1",(int)x-15,graph.getY()+18,30,15,juce::Justification::centred);
        }
        if(dnaResult.inspectCount>0) {
            const float x=xForHz(dnaResult.inspect[0].hz);
            g.setColour(juce::Colour(0xffe3a0a8));
            juce::Path diamond;
            diamond.startNewSubPath(x,(float)graph.getY()+72.f);
            diamond.lineTo(x+5.f,(float)graph.getY()+78.f);
            diamond.lineTo(x,(float)graph.getY()+84.f);
            diamond.lineTo(x-5.f,(float)graph.getY()+78.f);
            diamond.closeSubPath();
            g.strokePath(diamond,juce::PathStrokeType(dnaTargetArmed?2.8f:1.6f));
        }
    }
    g.restoreState();
    g.setColour(juce::Colour(0xff91a0a9));
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    for(float f:freqs) {
        const auto label=f>=1000.f?juce::String((int)(f/1000.f))+"k":juce::String((int)f);
        g.drawText(label,(int)xForHz(f)-20,graph.getBottom()+6,40,16,juce::Justification::centred);
    }
    for(int db : {-18,-9,0,9,18}) {
        g.drawText((db>0?"+":"")+juce::String(db),graph.getX()-35,(int)yForDb((float)db)-9,29,18,juce::Justification::centredRight);
    }
    g.setColour(juce::Colour(0xff77b5ad));
    g.fillRoundedRectangle((float)graph.getX(),spectrumPanel.getBottom()-20.f,10.f,3.f,1.f);
    g.setColour(juce::Colour(0xff8c9ba7));
    g.drawText("PRE",graph.getX()+14,spectrumPanel.getBottom()-26,36,18,juce::Justification::centredLeft);
    g.setColour(gold);
    g.fillRoundedRectangle((float)graph.getX()+57.f,spectrumPanel.getBottom()-20.f,10.f,3.f,1.f);
    g.setColour(juce::Colour(0xff8c9ba7));
    g.drawText("POST",graph.getX()+72,spectrumPanel.getBottom()-26,52,18,juce::Justification::centredLeft);
    g.drawText(tr(u8"GLISSER UN POINT : FRÉQUENCE / GAIN","DRAG NODE: FREQUENCY / GAIN"),graph.getRight()-280,spectrumPanel.getBottom()-26,278,18,juce::Justification::centredRight);

    // Two real peak meters (mono peak per bus; no fake L/R subdivisions).
    g.setColour(juce::Colour(0xff182229));
    g.fillRoundedRectangle(meterBox.toFloat(),4.f);
    g.setFont(juce::Font(juce::FontOptions(10.f,juce::Font::bold)));
    const auto meter=[&](float level,float held,int x,const juce::String& caption) {
        g.setColour(juce::Colour(0xffa5b4bc));
        g.drawText(caption,x-12,meterBox.getY()+8,38,18,juce::Justification::centred);
        const float top=(float)meterBox.getY()+34.f;
        const float mh=(float)meterBox.getHeight()-99.f;
        g.setColour(juce::Colour(0xff0b1217));
        g.fillRect(juce::Rectangle<float>((float)x,top,12.f,mh));
        for(int k=0;k<=4;++k) {
            g.setColour(juce::Colour(0xff27343c));
            g.drawHorizontalLine((int)(top+mh*k/4.f),(float)x,(float)x+12.f);
        }
        const float db=juce::Decibels::gainToDecibels(level,-72.f);
        const float dbHeld=juce::Decibels::gainToDecibels(held,-72.f);
        const float filled=juce::jlimit(0.f,mh,mh*(db+72.f)/72.f);
        const float heldHeight=juce::jlimit(0.f,mh,mh*(dbHeld+72.f)/72.f);
        g.setColour(level>=1.f?juce::Colours::orangered:gold);
        g.fillRect(juce::Rectangle<float>((float)x,top+mh-filled,12.f,filled));
        if(held>0.f) {
            g.setColour(held>=1.f?juce::Colours::orangered:juce::Colours::white);
            g.drawHorizontalLine((int)(top+mh-heldHeight),(float)x-1.f,(float)x+13.f);
        }
        g.setColour(juce::Colour(0xffb5c2c9));
        g.setFont(juce::Font(juce::FontOptions(9.f)));
        g.drawText(juce::String(dbHeld,0),x-12,(int)(top+mh+5),38,17,juce::Justification::centred);
    };
    meter(processor.peakIn.load(),processor.peakHoldIn.load(),meterBox.getX()+20,"IN");
    meter(processor.peakOut.load(),processor.peakHoldOut.load(),meterBox.getX()+62,"OUT");
    g.setColour(juce::Colour(0xff95a9b3));
    g.drawText("dBFS",meterBox.getX()+28,meterBox.getBottom()-45,49,14,juce::Justification::centred);

    // Graphic view replaces the eight tiles with sixteen real frequency faders.
    if(activeTab==2 && !timeLensMode && !intentMode && !matchMode) {
        g.setColour(juce::Colour(0xffb3c1c9));
        g.setFont(juce::Font(juce::FontOptions(9.f,juce::Font::bold)));
        for(int i=0;i<LoveEQAudioProcessor::graphicBandCount;++i) {
            const auto bounds=graphicSliders[(size_t)i].getBounds();
            const double hz=LoveEQAudioProcessor::graphicHz[(size_t)i];
            const juce::String label=hz>=1000.0?juce::String(hz/1000.0,1)+"k":juce::String((int)hz);
            g.drawText(label,bounds.getX()-3,bounds.getBottom()+1,bounds.getWidth()+6,14,juce::Justification::centred);
        }
    } else if(matchMode) {
        g.setColour(juce::Colour(0xff202b35));
        g.fillRoundedRectangle(bandRow.toFloat(),6.f);
        g.setColour(juce::Colour(0xff4d5b66));
        g.drawRoundedRectangle(bandRow.toFloat(),6.f,1.f);
        g.setColour(juce::Colour(0xffd0dbe1));
        g.setFont(juce::Font(juce::FontOptions("Arial",10.f,juce::Font::plain)));
        g.drawFittedText(matchStatus,43,547,786,20,
            juce::Justification::centredLeft,1,0.90f);
        g.setColour(juce::Colour(0xffe6c78f));
        g.drawText(tr(u8"INTENSITÉ","AMOUNT"),856,547,118,17,
            juce::Justification::centredLeft);
    } else if(intentMode) {
        g.setColour(juce::Colour(0xff202b35));
        g.fillRoundedRectangle(bandRow.toFloat(),6.f);
        g.setColour(juce::Colour(0xff435360));
        g.drawRoundedRectangle(bandRow.toFloat(),6.f,1.f);
        g.setFont(juce::Font(juce::FontOptions("Arial",10.f,juce::Font::plain)));
        g.setColour(juce::Colour(0xffb6c6d0));
        const auto description=intentSuggestionsReady?
            tr(u8"Trois propositions limitées à +/- 3 dB : écoutez A/B avant de conserver.",
               "Three proposals capped at +/- 3 dB. Audition A/B before keeping."):
            intentMessage;
        g.drawFittedText(description,44,544,740,26,juce::Justification::centredLeft,1,0.87f);
        g.setColour(juce::Colour(0xffe2c89b));
        g.setFont(juce::Font(juce::FontOptions("Arial",10.f,juce::Font::bold)));
        g.drawText(tr(u8"INTENSITÉ","INTENSITY"),810,543,100,17,
                   juce::Justification::centredLeft);
    } else if(timeLensMode) {
        const auto stage=(int)processor.timeLensPhase.load(std::memory_order_relaxed);
        const auto enabled=processor.parameters.getRawParameterValue("time_lens_enabled");
        const bool running=enabled && enabled->load()>0.5f;
        g.setColour(juce::Colour(0xff202a33));
        g.fillRoundedRectangle(bandRow.toFloat(),6.f);
        g.setColour(juce::Colour(0xff43525e));
        g.drawRoundedRectangle(bandRow.toFloat(),6.f,1.f);
        g.setFont(juce::Font(juce::FontOptions("Arial",9.3f,juce::Font::bold)));
        g.setColour(juce::Colour(0xff9db0bd));
        g.drawText(tr("SEUIL","THRESHOLD"),39,507,170,15,juce::Justification::centredLeft);
        g.drawText(tr(u8"DURÉE ATTAQUE","ATTACK TIME"),223,507,173,15,juce::Justification::centredLeft);
        g.drawText(tr(u8"DURÉE FIN","TAIL TIME"),407,507,170,15,juce::Justification::centredLeft);
        const std::array<juce::String,3> names {
            tr("ATTAQUE","ATTACK"),tr("CORPS","BODY"),tr("FIN","TAIL")
        };
        const std::array<juce::String,3> hints {
            tr("TRANSITOIRE","TRANSIENT"),tr("MAINTIEN","SUSTAIN"),tr(u8"DÉCLIN","RELEASE")
        };
        const std::array<juce::Colour,3> colours {
            juce::Colour(0xffe8bc75),juce::Colour(0xff83cbbd),juce::Colour(0xffbd9be6)
        };
        for(int i=0;i<3;++i) {
            const auto rect=juce::Rectangle<float>((float)(714+i*141),513.f,131.f,56.f);
            const bool selected=running && stage==i+1;
            g.setColour(selected?colours[(size_t)i].withAlpha(0.24f):juce::Colour(0xff28343e));
            g.fillRoundedRectangle(rect,6.f);
            g.setColour(selected?colours[(size_t)i]:juce::Colour(0xff4a5963));
            g.drawRoundedRectangle(rect,6.f,selected?1.6f:1.f);
            g.setColour(selected?juce::Colour(0xfff5f0e7):juce::Colour(0xffcad5db));
            g.setFont(juce::Font(juce::FontOptions("Arial",12.f,juce::Font::bold)));
            g.drawText(names[(size_t)i],(int)rect.getX()+5,518,121,19,juce::Justification::centred);
            g.setColour(selected?colours[(size_t)i]:juce::Colour(0xff92a6b2));
            g.setFont(juce::Font(juce::FontOptions("Arial",9.f,juce::Font::plain)));
            g.drawText(hints[(size_t)i],(int)rect.getX()+5,542,121,15,juce::Justification::centred);
        }
    } else {
    // Tiles paint their own content so the clickable buttons never mask
    // labels, frequency, band number or gain drawn behind them.
    }

    // Bottom edit strip: filter, dynamic band, and deliberately honest EQ Match status.
    g.setColour(juce::Colour(0xff252e36));
    g.fillRoundedRectangle(bottomPanel.toFloat(),5.f);
    if(activeTab!=2 && !timeLensMode && !intentMode && !matchMode) {
        const auto selectedColour=juce::Colour(bandColours[(size_t)selectedBand]);
        // Entire band editor follows the selected tile colour, not only its dot.
        g.setColour(selectedColour.withAlpha(0.18f));
        g.fillRoundedRectangle(juce::Rectangle<float>((float)bottomPanel.getX()+1.f,(float)bottomPanel.getY()+1.f,
                              (float)controls.getWidth()-2.f,(float)bottomPanel.getHeight()-2.f),4.f);
        g.setColour(selectedColour);
        g.fillRoundedRectangle((float)bottomPanel.getX()+1.f,(float)bottomPanel.getY()+1.f,
                              (float)controls.getWidth()-2.f,3.f,1.f);
    }
    g.setColour(juce::Colour(0xff40505b));
    g.drawRoundedRectangle(bottomPanel.toFloat(),5.f,1.f);
    if(activeTab==2 && !timeLensMode && !intentMode && !matchMode) {
        g.setColour(juce::Colour(0xffe3c892));
        g.setFont(juce::Font(juce::FontOptions(15.f,juce::Font::bold)));
        g.drawText(tr("GRAPHIQUE - 16 BANDES","GRAPHIC - 16 BANDS"),43,bottomPanel.getY()+12,470,28,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xffa7b7c0));
        g.setFont(juce::Font(juce::FontOptions(12.f)));
        g.drawText(tr(u8"16 filtres bell réels avec gain indépendant de -18 à +18 dB",
                      "16 real bell filters with independent gain from -18 to +18 dB"),
                      43,bottomPanel.getY()+47,bottomPanel.getWidth()-60,25,juce::Justification::centredLeft);
        g.drawText(tr(u8"Déplacez les curseurs au-dessus ou les points du graphe.",
                      "Move the sliders above or drag the graph nodes."),
                      43,bottomPanel.getY()+78,bottomPanel.getWidth()-60,22,juce::Justification::centredLeft);
    } else if(matchMode) {
        const bool on=processor.parameters.getRawParameterValue("match_active")->
                       load()>0.5f;
        for(int i=0;i<loveeq::match::bands;++i) {
            const int x=36+i*183;
            const juce::Rectangle<float> box((float)x,591.f,171.f,112.f);
            g.setColour(juce::Colour(0xff1b2732));
            g.fillRoundedRectangle(box,5.f);
            g.setColour(on?juce::Colour(0xffd9bf89):juce::Colour(0xff526579));
            g.drawRoundedRectangle(box,5.f,1.2f);
            g.setFont(juce::Font(juce::FontOptions("Arial",10.3f,juce::Font::bold)));
            const float hz=loveeq::match::centers[(size_t)i];
            const juce::String label=hz>=1000.f?
                juce::String(hz/1000.f,2)+" kHz":
                juce::String((int)hz)+" Hz";
            g.setColour(juce::Colour(0xffe3d5bc));
            g.drawText(label,x+10,599,150,17,juce::Justification::centredLeft);
            g.setColour(juce::Colour(0xffa1b2bb));
            g.drawText(tr(u8"CORRECTION","CORRECTION"),x+10,619,150,15,
                       juce::Justification::centredLeft);
        }
    } else if(intentMode) {
        const std::array<juce::Colour,3> colours {
            juce::Colour(0xff87bdc9),juce::Colour(0xffe1bd7f),juce::Colour(0xffbb9ed9)
        };
        const int goal=intentGoalBox.getSelectedId()-1;
        const std::array<std::array<const char*,3>,4> explanationsFr {{
            {{u8"Éviter la boue",u8"Présence maîtrisée",u8"Aigus sans dureté"}},
            {{u8"Chaleur naturelle",u8"Corps moins chargé",u8"Douceur dans l'aigu"}},
            {{u8"Conserver le corps",u8"Adoucir la présence",u8"Réduire l'agressivité"}},
            {{u8"Assise dans le grave",u8"Moins de bas-médiums",u8"Attaque plus lisible"}}
        }};
        const std::array<std::array<const char*,3>,4> explanationsEn {{
            {{"Protect low body","Controlled presence","Less harsh treble"}},
            {{"Natural warmth","Reduced low-mid buildup","Softer high presence"}},
            {{"Keep tonal body","Soften presence","Reduce sharp highs"}},
            {{"Low-end foundation","Less low-mid mud","Clearer bass articulation"}}
        }};
        const int safeGoal=juce::jlimit(0,3,goal);
        for(int i=0;i<3;++i) {
            const int x=36+i*369;
            const auto rect=juce::Rectangle<float>((float)x,592.f,352.f,111.f);
            const auto colour=colours[(size_t)i];
            g.setColour(juce::Colour(0xff1b2731));
            g.fillRoundedRectangle(rect,6.f);
            g.setColour(colour.withAlpha(0.72f));
            g.drawRoundedRectangle(rect,6.f,1.1f);
            g.fillRoundedRectangle((float)x+2.f,593.f,348.f,3.f,1.f);
            g.setColour(juce::Colour(0xffe7edf0));
            g.setFont(juce::Font(juce::FontOptions("Arial",11.5f,juce::Font::bold)));
            g.drawText(juce::String(i+1)+" / "+
                juce::String(english?explanationsEn[(size_t)safeGoal][(size_t)i]:
                              explanationsFr[(size_t)safeGoal][(size_t)i]),
                x+10,599,330,20,juce::Justification::centredLeft);
            g.setColour(juce::Colour(0xffa3b4be));
            g.setFont(juce::Font(juce::FontOptions("Arial",9.5f,juce::Font::plain)));
            g.drawText(tr(u8"FRÉQUENCE","FREQUENCY"),x+8,619,105,16,juce::Justification::centred);
            g.drawText("GAIN",x+126,619,106,16,juce::Justification::centred);
            g.drawText("Q",x+244,619,106,16,juce::Justification::centred);
        }
    } else if(timeLensMode) {
        const std::array<juce::String,3> titles {
            tr("ATTAQUE","ATTACK"),tr("CORPS","BODY"),tr("FIN","TAIL")
        };
        const std::array<juce::Colour,3> colors {
            juce::Colour(0xffe8bc75),juce::Colour(0xff83cbbd),juce::Colour(0xffbd9be6)
        };
        const int phase=processor.timeLensPhase.load(std::memory_order_relaxed);
        const auto* enabled=processor.parameters.getRawParameterValue("time_lens_enabled");
        const bool running=enabled && enabled->load()>0.5f;
        for(int i=0;i<3;++i) {
            const int x=36+i*369;
            const auto rect=juce::Rectangle<float>((float)x,592.f,352.f,111.f);
            const auto color=colors[(size_t)i];
            g.setColour(juce::Colour(0xff1c2932));
            g.fillRoundedRectangle(rect,6.f);
            g.setColour(running && phase==i+1?color:color.withAlpha(0.48f));
            g.drawRoundedRectangle(rect,6.f,running && phase==i+1?1.7f:1.0f);
            g.setColour(color);
            g.fillRoundedRectangle((float)x+2.f,593.f,348.f,2.8f,1.f);
            g.setFont(juce::Font(juce::FontOptions("Arial",12.f,juce::Font::bold)));
            g.drawText(titles[(size_t)i],x+12,597,325,20,juce::Justification::centredLeft);
            g.setColour(juce::Colour(0xffaabcc5));
            g.setFont(juce::Font(juce::FontOptions("Arial",9.6f,juce::Font::plain)));
            g.drawText(tr(u8"FRÉQUENCE","FREQUENCY"),x+8,619,105,16,juce::Justification::centred);
            g.drawText("GAIN",x+126,619,106,16,juce::Justification::centred);
            g.drawText("Q",x+244,619,106,16,juce::Justification::centred);
        }
    } else {
    const int split1=controls.getRight()+8;
    const int split2=split1+287;
    g.drawVerticalLine(split1,(float)bottomPanel.getY(),(float)bottomPanel.getBottom());
    g.drawVerticalLine(split2,(float)bottomPanel.getY(),(float)bottomPanel.getBottom());
    g.setFont(juce::Font(juce::FontOptions(10.f,juce::Font::bold)));
    g.setColour(juce::Colour(bandColours[(size_t)selectedBand]));
    g.fillEllipse(37.f,(float)bottomPanel.getY()+17.f,6.f,6.f);
    g.setColour(juce::Colour(0xffc6d0d7));
    g.drawText(tr("BANDE ","BAND ")+juce::String(selectedBand+1),49,bottomPanel.getY()+9,119,20,juce::Justification::centredLeft);
    g.drawText(tr("DYNAMIQUE","DYNAMICS"),split1+12,bottomPanel.getY()+9,131,20,juce::Justification::centredLeft);
    g.drawText(relationMode?"RELATIONSHIP EQ":dnaEnabled?"HARMONIC DNA":"EQ MATCH",
               split2+12,bottomPanel.getY()+(relationMode?7:9),220,relationMode?17:20,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff8496a0));
    g.setFont(juce::Font(juce::FontOptions(9.f)));
    const std::array<juce::String,6> labels {
        tr("FILTRE","FILTER"),tr(u8"FRÉQUENCE","FREQUENCY"),
        "GAIN",tr("Q / LARGEUR","Q / WIDTH"),tr("CANAL","CHANNEL"),tr("PENTE","SLOPE")
    };
    const int x0=controls.getX()+10;
    const std::array<int,6> xx {0,94,187,272,362,453};
    for(int i=0;i<6;++i)
        g.drawText(labels[(size_t)i],x0+xx[(size_t)i],bottomPanel.getY()+39,
                   (i==0?84:82),17,juce::Justification::centredLeft);
    const std::array<juce::String,4> dynamics {
        tr("SEUIL","THRESHOLD"),tr("PLAGE","RANGE"),tr("ATTAQUE","ATTACK"),tr(u8"RELÂCHE","RELEASE")
    };
    for(int i=0;i<4;++i)
        g.drawText(dynamics[(size_t)i],split1+12+i*67,bottomPanel.getY()+39,64,17,juce::Justification::centredLeft);
    if(relationMode) {
        const int x=split2+12, y=bottomPanel.getY();
        g.setColour(juce::Colour(0xffa6bac5));
        g.setFont(juce::Font(juce::FontOptions("Arial",9.f,juce::Font::bold)));
        g.drawText(tr(u8"RÔLE","ROLE"),x,y+25,97,12,juce::Justification::centredLeft);
        g.drawText(tr(u8"PRIORITÉ","PRIORITY"),x+104,y+25,135,12,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xffe0c28e));
        const auto remote=processor.relationshipPriorityFrame();
        if(relationSuggested) {
            g.drawText(tr("Conflit : ","Overlap: ")+
               juce::String((int)relationProposedHz)+" Hz",
               x,y+69,217,14,juce::Justification::centredLeft);
        } else if(remote.valid) {
            g.drawText(juce::String(remote.count)+tr(u8" piste(s) reliée(s)"," linked track(s)"),
                       x,y+69,217,14,juce::Justification::centredLeft);
        } else {
            g.drawText(tr(u8"Aucune piste prioritaire détectée","No priority track detected"),
                       x,y+69,217,14,juce::Justification::centredLeft);
        }
        // Status text is presented as a tooltip, never drawn over the action row.
    } else if(dnaEnabled) {
        const int x=split2+12;
        g.setFont(juce::Font(juce::FontOptions("Arial",11.f,juce::Font::plain)));
        if(dnaResult.reliable) {
            const juce::String pitch=juce::String(loveeq::curvelab::noteName(dnaResult.fundamentalHz));
            const juce::String summary=pitch+" / "+juce::String(dnaResult.fundamentalHz,1)+
                " Hz / "+juce::String((int)(dnaResult.confidence*100.f))+"%";
            g.setColour(juce::Colour(0xff81d9c4));
            g.drawText(summary,x,bottomPanel.getY()+29,231,18,juce::Justification::centredLeft);
            g.setColour(juce::Colour(0xffc3d3d9));
            g.drawText(juce::String(dnaResult.harmonicCount)+
               tr(u8" harmoniques identifiées"," harmonics identified"),
               x,bottomPanel.getY()+47,231,17,juce::Justification::centredLeft);
            if(dnaResult.inspectCount>0) {
                g.setColour(juce::Colour(0xffe6c7a2));
                g.drawText((dnaTargetArmed?tr(u8"Cible prête : ","Target ready: "):tr(u8"Pic à examiner : ","Inspect peak: "))+
                   juce::String((int)std::round(dnaResult.inspect[0].hz))+" Hz",
                   x,bottomPanel.getY()+65,231,16,juce::Justification::centredLeft);
            } else {
                g.drawText(tr(u8"Aucun pic isolé suspect","No isolated peak to inspect"),
                           x,bottomPanel.getY()+65,231,16,juce::Justification::centredLeft);
            }
        } else {
            g.setColour(juce::Colour(0xffbdcbd1));
            g.drawText(tr(u8"Fondamentale non fiable","Pitch not reliable"),
                       x,bottomPanel.getY()+32,231,17,juce::Justification::centredLeft);
            g.setFont(juce::Font(juce::FontOptions("Arial",9.5f,juce::Font::plain)));
            g.drawText(tr(u8"Signal complexe / silence : aucune coupe",
                          "Complex/silent source: no EQ cut"),
                       x,bottomPanel.getY()+55,231,19,juce::Justification::centredLeft);
        }
    } else {
        g.drawText(tr("Correspondance spectrale","Spectrum matching"),
                   split2+12,bottomPanel.getY()+48,202,18,juce::Justification::centredLeft);
        g.drawText(tr(u8"OUVRIR EQ MATCH EN HAUT","OPEN EQ MATCH ABOVE"),
                   split2+12,bottomPanel.getY()+72,220,17,juce::Justification::centredLeft);
    }
    g.drawText(activeTab==1?tr(u8"RÉGLAGES DYNAMIQUES","DYNAMIC CONTROLS"):tr(u8"RÉGLAGES PARAMÉTRIQUES","PARAMETRIC CONTROLS"),bottomPanel.getX()+10,bottomPanel.getBottom()-22,315,15,juce::Justification::centredLeft);

    }
    g.setColour(juce::Colour(0xffaebbc3));
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.drawText(tr(u8"ENTRÉE","INPUT"),footerPanel.getX()+3,footerPanel.getY()+10,53,15,juce::Justification::centredLeft);
    g.drawText(tr("SORTIE","OUTPUT"),footerPanel.getX()+214,footerPanel.getY()+10,53,15,juce::Justification::centredLeft);
    g.drawText(tr(u8"LOVE EQ - PRÊT POUR LE MIXAGE","LOVE EQ - READY FOR MIXING"),footerPanel.getX()+470,footerPanel.getY()+10,335,16,juce::Justification::centredLeft);
    g.drawText("by Liberty Music Software",footerPanel.getRight()-198,footerPanel.getY()+10,194,15,juce::Justification::centredRight);
}
void LoveEQAudioProcessorEditor::resized() {
    const int w=getWidth();
    // Header is deliberately sectioned so the library, language and A/B never overlap.
    presetSearch.setBounds(350,37,120,32);
    presetCombo.setBounds(477,37,176,32);
    browserButton.setBounds(660,37,102,32);
    languageBox.setBounds(769,37,82,32);
    buttonA.setBounds(859,35,30,33);
    buttonB.setBounds(894,35,30,33);
    undoButton.setBounds(932,35,33,33);
    redoButton.setBounds(971,35,33,33);
    bypassButton.setBounds(w-93,35,68,33);
    tabParam.setBounds(27,97,108,29);
    tabDynamic.setBounds(138,97,102,29);
    tabGraphic.setBounds(243,97,107,29);
    resetButton.setBounds(w-116,96,92,30);
    spectrumPanel={24,145,w-48,344};
    preButton.setBounds(130,155,49,25);
    postButton.setBounds(183,155,49,25);
    holdButton.setBounds(245,155,103,25);
    freezeButton.setBounds(352,155,58,25);
    sketchButton.setBounds(418,155,79,25);
    notesButton.setBounds(502,155,72,25);
    grabButton.setBounds(579,155,101,25);
    dnaButton.setBounds(690,155,142,25);
    relationModeButton.setBounds(840,155,146,25);
    timeLensModeButton.setBounds(994,155,141,25);
    intentModeButton.setBounds(852,96,178,30);
    matchModeButton.setBounds(364,96,133,30);
    matchReferenceButton.setBounds(36,510,155,31);
    matchSourceButton.setBounds(198,510,157,31);
    matchProposeButton.setBounds(363,510,107,31);
    matchPreviewButton.setBounds(478,510,120,31);
    matchCommitButton.setBounds(606,510,112,31);
    matchCancelButton.setBounds(726,510,108,31);
    matchAmountSlider.setBounds(851,509,260,34);
    for(int i=0;i<loveeq::match::bands;++i)
        matchGainSliders[(size_t)i].setBounds(43+i*183,639,155,62);
    intentGoalBox.setBounds(36,509,266,30);
    intentAnalyzeButton.setBounds(310,509,95,30);
    for(int j=0;j<3;++j)
        intentVariantButtons[(size_t)j].setBounds(413+j*99,509,94,30);
    intentOriginalButton.setBounds(712,509,73,30);
    intentPreviewButton.setBounds(791,509,105,30);
    intentCommitButton.setBounds(902,509,106,30);
    intentCancelButton.setBounds(1014,509,109,30);
    intentAmountSlider.setBounds(802,542,307,30);
    for(int j=0;j<3;++j) {
        const int x=36+j*369;
        intentFrequencySlider[(size_t)j].setBounds(x+8,637,106,65);
        intentGainSlider[(size_t)j].setBounds(x+126,637,106,65);
        intentQSlider[(size_t)j].setBounds(x+244,637,106,65);
    }
    graph={65,210,w-219,239};
    meterBox={w-132,192,101,272};
    peakResetButton.setBounds(meterBox.getX()+4,meterBox.getBottom()-26,93,23);
    bandRow={24,500,w-48,77};
    const int gap=9,tileW=(bandRow.getWidth()-7*gap)/8;
    for(int i=0;i<8;++i) {
        bandButtons[(size_t)i].setBounds(bandRow.getX()+i*(tileW+gap),bandRow.getY(),tileW,bandRow.getHeight());
    }
    const int gfGap=4,gfWidth=(bandRow.getWidth()-15*gfGap)/16;
    for(int i=0;i<LoveEQAudioProcessor::graphicBandCount;++i) {
        graphicSliders[(size_t)i].setBounds(bandRow.getX()+i*(gfWidth+gfGap)+5,
                          bandRow.getY()+1,gfWidth-10,58);
    }
    bottomPanel={24,587,w-48,122};
    dnaFocusButton.setBounds(889,675,103,26);
    dnaCutButton.setBounds(998,675,121,26);
    relationRoleBox.setBounds(880,625,98,27);
    relationPriorityBox.setBounds(984,625,134,27);
    relationGroupBox.setBounds(880,675,66,26);
    relationSuggestButton.setBounds(952,675,82,26);
    relationApplyButton.setBounds(1040,675,78,26);
    timeThresholdSlider.setBounds(38,529,172,43);
    timeAttackSlider.setBounds(222,529,172,43);
    timeTailSlider.setBounds(406,529,172,43);
    timeLensEnableButton.setBounds(590,519,103,35);
    for(int j=0;j<LoveEQAudioProcessor::timeStageCount;++j) {
        const int x=36+j*369;
        timeHzSlider[(size_t)j].setBounds(x+8,639,106,63);
        timeGainSlider[(size_t)j].setBounds(x+126,639,106,63);
        timeQSlider[(size_t)j].setBounds(x+244,639,106,63);
    }
    controls={24,587,560,122};
    footerPanel={24,718,w-48,35};
    const int baseY=bottomPanel.getY()+57;
    typeBox.setBounds(controls.getX()+10,baseY,87,29);
    frequency.setBounds(controls.getX()+103,baseY-1,89,42);
    gain.setBounds(controls.getX()+196,baseY-1,83,42);
    q.setBounds(controls.getX()+281,baseY-1,86,42);
    channelBox.setBounds(controls.getX()+371,baseY,88,29);
    slopeBox.setBounds(controls.getX()+462,baseY,89,29);
    enabledButton.setBounds(controls.getX()+355,bottomPanel.getY()+8,67,25);
    soloButton.setBounds(controls.getX()+425,bottomPanel.getY()+8,59,25);
    dynamicButton.setBounds(controls.getRight()+156,bottomPanel.getY()+7,68,26);
    const int dynX=controls.getRight()+20;
    threshold.setBounds(dynX,baseY-1,65,42);
    range.setBounds(dynX+67,baseY-1,65,42);
    attack.setBounds(dynX+134,baseY-1,65,42);
    release.setBounds(dynX+201,baseY-1,65,42);
    input.setBounds(footerPanel.getX()+57,footerPanel.getY()+3,132,30);
    output.setBounds(footerPanel.getX()+272,footerPanel.getY()+3,132,30);
    for(auto* slider:{&frequency,&gain,&q,&threshold,&range,&attack,&release})
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow,false,61,18);
    input.setTextBoxStyle(juce::Slider::TextBoxRight,false,57,24);
    output.setTextBoxStyle(juce::Slider::TextBoxRight,false,57,24);
    if(presetBrowser) presetBrowser->setBounds(95,129,w-190,565);
}
void LoveEQAudioProcessorEditor::mouseDown(const juce::MouseEvent& e) {
    draggingBand=-1;
    draggingGraphic=-1;
    if(matchMode || intentMode || !graph.contains(e.getPosition())) return;
    if(sketchMode && activeTab!=2) {
        sketchActive=true;
        sketchPoints.clear();
        addSketchPoint(e.position);
        return;
    }
    float best=21.f;
    if(activeTab==2) {
        for(int i=0;i<LoveEQAudioProcessor::graphicBandCount;++i) {
            const auto* v=processor.parameters.getRawParameterValue(LoveEQAudioProcessor::graphicId(i));
            if(!v)continue;
            const float x=xForHz((float)LoveEQAudioProcessor::graphicHz[(size_t)i]), y=yForDb(v->load());
            const float distance=juce::Point<float>(x,y).getDistanceFrom(e.position);
            if(distance<best){best=distance;draggingGraphic=i;}
        }
        return;
    }
    for(int i=0;i<8;++i) {
        const auto id=[i](const juce::String& name){return LoveEQAudioProcessor::bandId(i,name);};
        auto read=[this](const juce::String& name){return processor.parameters.getRawParameterValue(name)->load();};
        if(read(id("enabled"))<0.5f)continue;
        const float d=juce::Point<float>(xForHz(read(id("hz"))),yForDb(read(id("gain")))).getDistanceFrom(e.position);
        if(d<best){best=d;draggingBand=i;}
    }
    if(draggingBand>=0)attachBand(draggingBand);
}
void LoveEQAudioProcessorEditor::mouseDrag(const juce::MouseEvent& e) {
    if(sketchActive) {addSketchPoint(e.position);repaint();return;}
    const float db=juce::jlimit(-18.f,18.f,(graph.getCentreY()-e.position.y)*36.f/graph.getHeight());
    if(draggingGraphic>=0) {
        setParam(LoveEQAudioProcessor::graphicId(draggingGraphic),db);
        repaint();
        return;
    }
    if(draggingBand<0)return;
    const float x=juce::jlimit(0.f,1.f,(e.position.x-graph.getX())/graph.getWidth());
    float hz=20.f*std::pow(1000.f,x);
    if(noteGrid && e.mods.isAltDown())hz=loveeq::curvelab::noteFrequency(hz);
    setParam(LoveEQAudioProcessor::bandId(draggingBand,"hz"),hz);
    setParam(LoveEQAudioProcessor::bandId(draggingBand,"gain"),db);
    repaint();
}
void LoveEQAudioProcessorEditor::addSketchPoint(const juce::Point<float>& position) {
    if(!graph.isEmpty()) {
        loveeq::curvelab::SketchPoint point;
        point.position=juce::jlimit(0.0,1.0,((double)position.x-graph.getX())/graph.getWidth());
        point.gain=juce::jlimit(-18.0,18.0,((double)graph.getCentreY()-position.y)*36.0/graph.getHeight());
        if(!sketchPoints.empty() && std::abs(sketchPoints.back().position-point.position)<0.005)
            sketchPoints.back()=point;
        else if(sketchPoints.size()<400)sketchPoints.push_back(point);
    }
}
void LoveEQAudioProcessorEditor::applySketch() {
    const auto fitted=loveeq::curvelab::fitSketch(sketchPoints,
        juce::jmax(8000.0,processor.getSampleRate()));
    bool hasBand=false;
    for(const auto& band:fitted)hasBand=hasBand||band.enabled;
    if(!hasBand)return;
    processor.undoManager.beginNewTransaction(tr(u8"Courbe dessinée","Drawn EQ curve"));
    for(int i=0;i<LoveEQAudioProcessor::bandCount;++i) {
        const auto id=[i](const juce::String& suffix){return LoveEQAudioProcessor::bandId(i,suffix);};
        const auto& band=fitted[(size_t)i];
        setParam(id("enabled"),band.enabled?1.f:0.f);
        if(!band.enabled)continue;
        setParam(id("type"),0.f);
        setParam(id("solo"),0.f);
        setParam(id("dynamic"),0.f);
        setParam(id("channel"),0.f);
        setParam(id("hz"),band.hz);
        setParam(id("gain"),band.gain);
        setParam(id("q"),band.q);
    }
    processor.undoManager.beginNewTransaction();
    attachBand(0);
    selectTab(0);
    repaint();
}
void LoveEQAudioProcessorEditor::grabSpectrumPeak() {
    if(activeTab==2)selectTab(0);
    if(!freezeSpectrum)updateFFT(true,preSpectrum);
    const auto peak=loveeq::curvelab::locateSpectrumPeak(preSpectrum.data(),
         (int)preSpectrum.size(),juce::jmax(8000.0,processor.getSampleRate()));
    if(!peak.valid) {
        grabButton.setTooltip(tr(u8"Aucune résonance suffisamment marquée détectée",
                                 "No sufficiently strong spectral peak detected"));
        return;
    }
    processor.undoManager.beginNewTransaction(tr(u8"Capturer une résonance","Capture spectral peak"));
    const auto id=[this](const juce::String& suffix){
        return LoveEQAudioProcessor::bandId(selectedBand,suffix);
    };
    setParam(id("enabled"),1.f);
    setParam(id("type"),0.f);
    setParam(id("hz"),peak.hz);
    setParam(id("q"),5.0f);
    setParam(id("gain"),0.f); // Safe: never cut or boost automatically.
    processor.undoManager.beginNewTransaction();
    attachBand(selectedBand);
    grabButton.setTooltip(tr(u8"Résonance positionnée : ajustez ensuite le gain",
                             "Peak positioned: adjust gain as needed"));
    repaint();
}
void LoveEQAudioProcessorEditor::mouseUp(const juce::MouseEvent& e) {
    if(!sketchActive)return;
    addSketchPoint(e.position);
    sketchActive=false;
    applySketch();
    sketchPoints.clear();
    sketchMode=false;
    sketchButton.setToggleState(false,juce::dontSendNotification);
    repaint();
}
void LoveEQAudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& e) {
    if(matchMode || intentMode || sketchMode || activeTab==2 || !graph.contains(e.getPosition()))return;
    int target=-1;
    for(int i=0;i<LoveEQAudioProcessor::bandCount;++i) {
        const auto* enabled=processor.parameters.getRawParameterValue(
            LoveEQAudioProcessor::bandId(i,"enabled"));
        if(enabled && enabled->load()<0.5f){target=i;break;}
    }
    if(target<0)target=selectedBand;
    const auto id=[target](const juce::String& suffix){
        return LoveEQAudioProcessor::bandId(target,suffix);
    };
    float hz=(float)loveeq::curvelab::hzFromPosition(
        ((double)e.position.x-graph.getX())/graph.getWidth());
    if(noteGrid && e.mods.isAltDown())hz=loveeq::curvelab::noteFrequency(hz);
    const float db=juce::jlimit(-18.f,18.f,
        (graph.getCentreY()-e.position.y)*36.f/graph.getHeight());
    processor.undoManager.beginNewTransaction(tr(u8"Ajouter une bande","Add EQ band"));
    setParam(id("enabled"),1.f);
    setParam(id("type"),0.f);
    setParam(id("hz"),hz);
    setParam(id("gain"),db);
    setParam(id("q"),1.f);
    processor.undoManager.beginNewTransaction();
    attachBand(target);
    repaint();
}
void LoveEQAudioProcessorEditor::updateFFT(bool before,std::array<float,1025>& out){
    const auto end=processor.scopeIndex.load(std::memory_order_acquire);
    for(int j=0;j<2048;++j) {
        const auto index=(end-(unsigned int)2048+(unsigned int)j)%LoveEQAudioProcessor::scopeSize;
        const float x=before?processor.scopeBefore[index].load():processor.scopeAfter[index].load();
        fftScratch[(size_t)j]=x*(0.5f-0.5f*std::cos(2.f*juce::MathConstants<float>::pi*j/2047.f));
    }
    std::fill(fftScratch.begin()+2048,fftScratch.end(),0.f);
    fft.performFrequencyOnlyForwardTransform(fftScratch.data());
    for(int j=0;j<=1024;++j)out[(size_t)j]=juce::Decibels::gainToDecibels(fftScratch[(size_t)j]/512.f,-96.f);
}
void LoveEQAudioProcessorEditor::updateDnaControls() {
    const bool mode=dnaEnabled && activeTab!=2 && !relationMode && !timeLensMode && !intentMode && !matchMode;
    dnaFocusButton.setVisible(mode);
    dnaCutButton.setVisible(mode);
    const bool hasCandidate=dnaResult.reliable && dnaResult.inspectCount>0;
    dnaFocusButton.setEnabled(mode && hasCandidate);
    dnaCutButton.setEnabled(mode && dnaTargetArmed);
}
void LoveEQAudioProcessorEditor::updateHarmonicDNA() {
    const auto sampleRate=processor.getSampleRate();
    if(sampleRate<8000.0) {
        dnaResult={};dnaTargetArmed=false;updateDnaControls();updateRelationshipControls();return;
    }
    const unsigned int end=processor.scopeIndex.load(std::memory_order_acquire);
    constexpr unsigned int fftSize=8192;
    for(unsigned int j=0;j<fftSize;++j) {
        const unsigned int index=(end-fftSize+j)%LoveEQAudioProcessor::scopeSize;
        const float value=processor.scopeBefore[(size_t)index].load(std::memory_order_relaxed);
        const float hann=0.5f-0.5f*std::cos(2.f*juce::MathConstants<float>::pi*(float)j/8191.f);
        dnaScratch[(size_t)j]=value*hann;
    }
    std::fill(dnaScratch.begin()+fftSize,dnaScratch.end(),0.f);
    dnaFft.performFrequencyOnlyForwardTransform(dnaScratch.data());
    for(size_t i=0;i<dnaMagnitudes.size();++i)
        dnaMagnitudes[i]=std::max(0.f,dnaScratch[i]/4096.f);
    const auto updated=loveeq::dna::analyze(
        dnaMagnitudes.data(),(int)dnaMagnitudes.size(),sampleRate,(int)fftSize);
    // Reconfirmation is required after a new or substantially different peak.
    if(!updated.reliable || updated.inspectCount==0 ||
       (dnaTargetArmed && std::abs(updated.inspect[0].hz-dnaArmedHz)>
          std::max(20.f,0.045f*dnaArmedHz)))dnaTargetArmed=false;
    dnaResult=updated;
    updateDnaControls();
    updateRelationshipControls();
}
void LoveEQAudioProcessorEditor::focusDnaCandidate() {
    if(activeTab==2 || !dnaEnabled || !dnaResult.reliable || dnaResult.inspectCount==0)return;
    const float hz=dnaResult.inspect[0].hz;
    if(hz<20.f || hz>20000.f)return;
    // Arming is intentionally visual only: no existing filter is changed.
    dnaArmedHz=hz;
    dnaArmedBand=selectedBand;
    dnaTargetArmed=true;
    updateDnaControls();
    repaint();
}
void LoveEQAudioProcessorEditor::applyDnaSuggestion() {
    if(!dnaEnabled || !dnaTargetArmed || activeTab==2)return;
    if(dnaArmedBand!=selectedBand || dnaArmedBand<0 || dnaArmedBand>=LoveEQAudioProcessor::bandCount) {
        dnaTargetArmed=false;
        updateDnaControls();
        return;
    }
    const auto id=[this](const juce::String& parameter){
        return LoveEQAudioProcessor::bandId(selectedBand,parameter);
    };
    processor.undoManager.beginNewTransaction(tr(u8"Atténuation confirmée par l'utilisateur",
                                                  "User-confirmed spectral reduction"));
    setParam(id("enabled"),1.f);
    setParam(id("solo"),0.f);
    setParam(id("type"),0.f);
    setParam(id("hz"),dnaArmedHz);
    setParam(id("q"),6.f);
    setParam(id("gain"),-2.f);
    processor.undoManager.beginNewTransaction();
    dnaTargetArmed=false;
    updateDnaControls();
    repaint();
}
void LoveEQAudioProcessorEditor::timerCallback(){
    updateTimeLensButtonLabel();
    if(matchCaptureMode!=0)runMatchCaptureFrame();
    for(auto& tile:bandButtons) if(tile.isVisible())tile.repaint();
    if(!freezeSpectrum) {
        if(showPre)updateFFT(true,preSpectrum);
        if(showPost)updateFFT(false,postSpectrum);
        if(dnaEnabled && (++dnaFrames%8u)==0u)updateHarmonicDNA();
        if(holdSpectrum) {
            for(size_t i=0;i<heldPre.size();++i) {
                heldPre[i]=std::max(heldPre[i],preSpectrum[i]);
                heldPost[i]=std::max(heldPost[i],postSpectrum[i]);
            }
        }
    }
    repaint();
}
