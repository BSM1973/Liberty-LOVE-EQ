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
    return juce::Font(juce::FontOptions(11.2f));
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
LoveEQAudioProcessorEditor::LoveEQAudioProcessorEditor(LoveEQAudioProcessor& p)
 : AudioProcessorEditor(&p),processor(p) {
    setLookAndFeel(&theme);
    english=processor.getUiLanguage()=="en";
    languageBox.addItem("FRAN\u00c7AIS",1);
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
    browserButton.onClick=[this]{if(presetBrowser)presetBrowser->open();};
    addAndMakeVisible(peakResetButton);
    peakResetButton.onClick=[this]{processor.resetPeakHold();repaint();};
    for(int i=0;i<8;++i) {
        auto& b=bandButtons[(size_t)i];
        b.setButtonText("");
        b.setTooltip("S\u00E9lectionner la bande " + juce::String(i+1));
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
    tabParam.onClick=[this]{selectTab(0);};
    tabDynamic.onClick=[this]{selectTab(1);};
    tabGraphic.setEnabled(true);
    tabGraphic.onClick=[this]{selectTab(2);};
    tabGraphic.setTooltip("16 bandes graphiques r\u00e9ellement trait\u00e9es dans le moteur audio");
    holdButton.setClickingTogglesState(true);
    holdButton.onClick=[this]{holdSpectrum=holdButton.getToggleState();
        if(!holdSpectrum){heldPre.fill(-96.f);heldPost.fill(-96.f);}
    };
    freezeButton.setClickingTogglesState(true);
    freezeButton.onClick=[this]{freezeSpectrum=freezeButton.getToggleState();};
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
    buttonA.onClick=[this]{switchSnapshot(false);};
    buttonB.onClick=[this]{switchSnapshot(true);};
    undoButton.onClick=[this]{processor.undoManager.undo();};
    redoButton.onClick=[this]{processor.undoManager.redo();};
    resetButton.onClick=[this]{processor.resetToFlat();selectTab(0);};
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
        presetCombo.setSelectedId(index+1,juce::dontSendNotification);
        selectTab(0);
    };
    presetBrowser->translate(english);
    updateLanguageSafeTexts();
    selectTab(processor.parameters.getRawParameterValue("eq_mode")->load()>0.5f?2:0);
    setResizable(false,false);setSize(1160,760);
    startTimerHz(24);
}
LoveEQAudioProcessorEditor::~LoveEQAudioProcessorEditor(){stopTimer();setLookAndFeel(nullptr);}
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
    for(auto& button:bandButtons)button.setVisible(!graphic);
    for(auto& slider:graphicSliders)slider.setVisible(graphic);
    for(juce::Component* component : std::array<juce::Component*,13>{
            &frequency,&gain,&q,&threshold,&range,&attack,&release,
            &typeBox,&channelBox,&slopeBox,&enabledButton,&soloButton,&dynamicButton})
        component->setVisible(!graphic);
}
void LoveEQAudioProcessorEditor::updateLanguageSafeTexts() {
    tabParam.setButtonText(tr("PARAM\u00c9TRIQUE","PARAMETRIC"));
    tabDynamic.setButtonText(tr("DYNAMIQUE","DYNAMIC"));
    tabGraphic.setButtonText(tr("GRAPHIQUE","GRAPHIC"));
    browserButton.setButtonText("PRESETS");
    browserButton.setTooltip(tr("Ouvrir la biblioth\u00e8que des 100 presets","Open the 100-preset browser"));
    resetButton.setButtonText("RESET / FLAT");
    holdButton.setButtonText(tr("MAINTIEN PIC","PEAK HOLD"));
    freezeButton.setButtonText(tr("FIGER","FREEZE"));
    enabledButton.setButtonText(tr("ACTIF","ON"));
    dynamicButton.setButtonText(tr("DYN","DYN"));
    soloButton.setButtonText("SOLO");
    peakResetButton.setButtonText("PEAK RESET");
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
    redoButton.setTooltip(tr("R\u00e9tablir","Redo"));
    resetButton.setTooltip(tr("Remettre les param\u00e8tres \u00e0 plat","Reset all EQ parameters"));
    peakResetButton.setTooltip(tr("Effacer les cr\u00eates IN/OUT","Reset input and output peak hold"));
    repaint();
}
void LoveEQAudioProcessorEditor::setParam(const juce::String& id,float v) {
    if(auto* p=processor.parameters.getParameter(id)) p->setValueNotifyingHost(p->convertTo0to1(v));
}
void LoveEQAudioProcessorEditor::attachBand(int index) {
    selectedBand=juce::jlimit(0,7,index);
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
    g.setFont(juce::Font(juce::FontOptions(11.5f)));
    g.drawText("by Liberty Music Software",77,51,250,19,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff7e8b98));
    g.setFont(juce::Font(juce::FontOptions(9.8f,juce::Font::bold)));
    g.drawText(tr("BIBLIOTH\u00c8QUE DE PRESETS","PRESET LIBRARY"),350,18,265,15,juce::Justification::centredLeft);
    g.setColour(trim);
    g.drawHorizontalLine(83,23.f,(float)w-23.f);

    g.setColour(juce::Colour(0xff1d252c));
    g.fillRoundedRectangle(24.f,94.f,330.f,35.f,5.f);
    g.setColour(trim);
    g.drawRoundedRectangle(24.f,94.f,330.f,35.f,5.f,1.f);
    g.setColour(gold);
    g.fillEllipse((float)w-275.f,108.f,5.f,5.f);
    g.setColour(juce::Colour(0xffc5cbd0));
    g.setFont(juce::Font(juce::FontOptions(10.f,juce::Font::bold)));
    g.drawText("ZERO LATENCY / 0",w-273,100,150,19,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff84939d));

    g.drawText(tr("COURBE : ST\u00c9R\u00c9O","CURVE: STEREO"),378,99,175,26,juce::Justification::centredLeft);
    g.drawText(tr("\u00c9CHELLE : +/- 18 dB","SCALE: +/- 18 dB"),563,99,195,26,juce::Justification::centredLeft);

    // Single analyzer panel with a dedicated toolbar and a narrow meter strip.
    g.setColour(juce::Colour(0xff10171d));
    g.fillRoundedRectangle(spectrumPanel.toFloat(),7.f);
    g.setColour(juce::Colour(0xff39434b));
    g.drawRoundedRectangle(spectrumPanel.toFloat(),7.f,1.f);
    g.drawHorizontalLine(188,(float)spectrumPanel.getX(),(float)spectrumPanel.getRight());
    g.setColour(juce::Colour(0xff778994));
    g.setFont(juce::Font(juce::FontOptions(10.f,juce::Font::bold)));
    g.drawText(tr("SPECTRE","SPECTRUM"),42,156,73,18,juce::Justification::centredLeft);
    g.drawText(tr("R\u00c9PONSE - dB","RESPONSE - dB"),graph.getX(),190,154,16,juce::Justification::centredLeft);
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
    if(activeTab==2) {
        for(int band=0;band<LoveEQAudioProcessor::graphicBandCount;++band) {
            const auto* gainV=processor.parameters.getRawParameterValue(LoveEQAudioProcessor::graphicId(band));
            if(!gainV) continue;
            const float x=xForHz((float)LoveEQAudioProcessor::graphicHz[(size_t)band]),y=yForDb(gainV->load());
            const auto col=juce::Colour(0xffdfc58c);
            g.setColour(juce::Colour(0xff131b22));
            g.fillEllipse(x-7.f,y-7.f,14.f,14.f);
            g.setColour(col);g.drawEllipse(x-7.f,y-7.f,14.f,14.f,1.6f);
        }
    } else for(int band=0;band<8;++band) {
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
    g.drawText(tr("GLISSER UN POINT : FR\u00c9QUENCE / GAIN","DRAG NODE: FREQUENCY / GAIN"),graph.getRight()-280,spectrumPanel.getBottom()-26,278,18,juce::Justification::centredRight);

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
    if(activeTab==2) {
        g.setColour(juce::Colour(0xffb3c1c9));
        g.setFont(juce::Font(juce::FontOptions(9.f,juce::Font::bold)));
        for(int i=0;i<LoveEQAudioProcessor::graphicBandCount;++i) {
            const auto bounds=graphicSliders[(size_t)i].getBounds();
            const double hz=LoveEQAudioProcessor::graphicHz[(size_t)i];
            const juce::String label=hz>=1000.0?juce::String(hz/1000.0,1)+"k":juce::String((int)hz);
            g.drawText(label,bounds.getX()-3,bounds.getBottom()+1,bounds.getWidth()+6,14,juce::Justification::centred);
        }
    } else {
    // Eight colour-coded full-width tiles like the reference.
    for(int i=0;i<8;++i) {
        const auto b=bandButtons[(size_t)i].getBounds();
        const auto colour=juce::Colour(bandColours[(size_t)i]);
        const bool selected=i==selectedBand;
        g.setColour(juce::Colour(0xff252e35));
        g.fillRoundedRectangle(b.toFloat(),5.f);
        g.setColour(selected?colour:juce::Colour(0xff3b4852));
        g.drawRoundedRectangle(b.toFloat(),5.f,selected?1.8f:1.f);
        g.setColour(colour);
        g.fillRoundedRectangle((float)b.getX()+1.f,(float)b.getY()+1.f,(float)b.getWidth()-2.f,2.8f,1.f);
        g.drawEllipse((float)b.getX()+10.f,(float)b.getY()+12.f,17.f,17.f,1.2f);
        g.setFont(juce::Font(juce::FontOptions(10.f,juce::Font::bold)));
        g.drawText(juce::String(i+1),b.getX()+10,b.getY()+12,17,17,juce::Justification::centred);
        const auto id=[i](const juce::String& name){return LoveEQAudioProcessor::bandId(i,name);};
        const auto* filterType=processor.parameters.getRawParameterValue(id("type"));
        const auto* active=processor.parameters.getRawParameterValue(id("enabled"));
        static constexpr const char* filterNamesEn[]={"BELL","LOW SHELF","HIGH SHELF","HIGH PASS","LOW PASS","NOTCH","BAND PASS"};
        static constexpr const char* filterNamesFr[]={"CLOCHE","SHELF BAS","SHELF HAUT","PASSE-H","PASSE-B","COUPE","BANDE"};
        const int filterIndex=filterType?juce::jlimit(0,6,(int)filterType->load()):0;
        g.setColour(active && active->load()<0.5f?juce::Colour(0xff6c7780):juce::Colour(0xffc3cbd1));
        g.drawText((english?filterNamesEn:filterNamesFr)[filterIndex],b.getX()+33,b.getY()+12,b.getWidth()-42,17,juce::Justification::centredLeft);
        const auto* hz=processor.parameters.getRawParameterValue(id("hz"));
        const auto* gainV=processor.parameters.getRawParameterValue(id("gain"));
        const float f=hz?hz->load():1000.f, val=gainV?gainV->load():0.f;
        const juce::String hzLabel=f>=1000.f?juce::String(f/1000.f,2)+" kHz":juce::String((int)std::round(f))+" Hz";
        g.setFont(juce::Font(juce::FontOptions(12.6f)));
        g.drawText(hzLabel,b.getX()+10,b.getY()+35,b.getWidth()-20,20,juce::Justification::centredLeft);
        g.setColour(colour);
        g.setFont(juce::Font(juce::FontOptions(10.f,juce::Font::bold)));
        g.drawText(juce::String(val,1),b.getX()+7,b.getBottom()-22,b.getWidth()-16,16,juce::Justification::centredRight);
        g.fillEllipse((float)b.getX()+b.getWidth()*0.48f,b.getBottom()-16.f,5.f,5.f);
    }
    }

    // Bottom edit strip: filter, dynamic band, and deliberately honest EQ Match status.
    g.setColour(juce::Colour(0xff252e36));
    g.fillRoundedRectangle(bottomPanel.toFloat(),5.f);
    if(activeTab!=2) {
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
    if(activeTab==2) {
        g.setColour(juce::Colour(0xffe3c892));
        g.setFont(juce::Font(juce::FontOptions(15.f,juce::Font::bold)));
        g.drawText(tr("GRAPHIQUE - 16 BANDES","GRAPHIC - 16 BANDS"),43,bottomPanel.getY()+12,470,28,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xffa7b7c0));
        g.setFont(juce::Font(juce::FontOptions(12.f)));
        g.drawText(tr("16 filtres bell r\u00e9els avec gain ind\u00e9pendant de -18 \u00e0 +18 dB",
                      "16 real bell filters with independent gain from -18 to +18 dB"),
                      43,bottomPanel.getY()+47,bottomPanel.getWidth()-60,25,juce::Justification::centredLeft);
        g.drawText(tr("D\u00e9placez les curseurs au-dessus ou les points du graphe.",
                      "Move the sliders above or drag the graph nodes."),
                      43,bottomPanel.getY()+78,bottomPanel.getWidth()-60,22,juce::Justification::centredLeft);
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
    g.drawText("EQ MATCH",split2+12,bottomPanel.getY()+9,126,20,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff8496a0));
    g.setFont(juce::Font(juce::FontOptions(9.f)));
    const std::array<juce::String,6> labels {
        tr("FILTRE","FILTER"),tr("FR\u00c9QUENCE","FREQUENCY"),
        "GAIN",tr("Q / LARGEUR","Q / WIDTH"),tr("CANAL","CHANNEL"),tr("PENTE","SLOPE")
    };
    const int x0=controls.getX()+10;
    const std::array<int,6> xx {0,94,187,272,362,453};
    for(int i=0;i<6;++i)
        g.drawText(labels[(size_t)i],x0+xx[(size_t)i],bottomPanel.getY()+39,
                   (i==0?84:82),17,juce::Justification::centredLeft);
    const std::array<juce::String,4> dynamics {
        tr("SEUIL","THRESHOLD"),tr("PLAGE","RANGE"),tr("ATTAQUE","ATTACK"),tr("REL\u00c2CHE","RELEASE")
    };
    for(int i=0;i<4;++i)
        g.drawText(dynamics[(size_t)i],split1+12+i*67,bottomPanel.getY()+39,64,17,juce::Justification::centredLeft);
    g.drawText(tr("Correspondance spectrale","Spectrum matching"),split2+12,bottomPanel.getY()+48,202,18,juce::Justification::centredLeft);
    g.drawText(tr("EN COURS DE D\u00c9VELOPPEMENT","IN DEVELOPMENT"),split2+12,bottomPanel.getY()+72,220,17,juce::Justification::centredLeft);
    g.drawText(activeTab==1?tr("R\u00c9GLAGES DYNAMIQUES","DYNAMIC CONTROLS"):tr("R\u00c9GLAGES PARAM\u00c9TRIQUES","PARAMETRIC CONTROLS"),bottomPanel.getX()+10,bottomPanel.getBottom()-22,315,15,juce::Justification::centredLeft);

    }
    g.setColour(juce::Colour(0xffaebbc3));
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.drawText(tr("ENTR\u00c9E","INPUT"),footerPanel.getX()+3,footerPanel.getY()+10,53,15,juce::Justification::centredLeft);
    g.drawText(tr("SORTIE","OUTPUT"),footerPanel.getX()+214,footerPanel.getY()+10,53,15,juce::Justification::centredLeft);
    g.drawText(tr("LOVE EQ - PR\u00caT POUR LE MIXAGE","LOVE EQ - READY FOR MIXING"),footerPanel.getX()+470,footerPanel.getY()+10,335,16,juce::Justification::centredLeft);
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
    if(!graph.contains(e.getPosition())) return;
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
    const float db=juce::jlimit(-18.f,18.f,(graph.getCentreY()-e.position.y)*36.f/graph.getHeight());
    if(draggingGraphic>=0) {
        setParam(LoveEQAudioProcessor::graphicId(draggingGraphic),db);
        repaint();
        return;
    }
    if(draggingBand<0)return;
    const float x=juce::jlimit(0.f,1.f,(e.position.x-graph.getX())/graph.getWidth());
    setParam(LoveEQAudioProcessor::bandId(draggingBand,"hz"),20.f*std::pow(1000.f,x));
    setParam(LoveEQAudioProcessor::bandId(draggingBand,"gain"),db);
    repaint();
}
void LoveEQAudioProcessorEditor::updateFFT(bool before,std::array<float,1025>& out){
    const auto end=processor.scopeIndex.load(std::memory_order_acquire);
    for(int j=0;j<2048;++j) {
        const auto index=(end+(unsigned int)j)%LoveEQAudioProcessor::scopeSize;
        const float x=before?processor.scopeBefore[index].load():processor.scopeAfter[index].load();
        fftScratch[(size_t)j]=x*(0.5f-0.5f*std::cos(2.f*juce::MathConstants<float>::pi*j/2047.f));
    }
    std::fill(fftScratch.begin()+2048,fftScratch.end(),0.f);
    fft.performFrequencyOnlyForwardTransform(fftScratch.data());
    for(int j=0;j<=1024;++j)out[(size_t)j]=juce::Decibels::gainToDecibels(fftScratch[(size_t)j]/512.f,-96.f);
}
void LoveEQAudioProcessorEditor::timerCallback(){
    if(!freezeSpectrum) {
        if(showPre)updateFFT(true,preSpectrum);
        if(showPost)updateFFT(false,postSpectrum);
        if(holdSpectrum) {
            for(size_t i=0;i<heldPre.size();++i) {
                heldPre[i]=std::max(heldPre[i],preSpectrum[i]);
                heldPost[i]=std::max(heldPost[i],postSpectrum[i]);
            }
        }
    }
    repaint();
}
