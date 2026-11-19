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
LoveEQAudioProcessorEditor::LoveEQAudioProcessorEditor(LoveEQAudioProcessor& p)
 : AudioProcessorEditor(&p),processor(p) {
    setLookAndFeel(&theme);
    for(int i=0;i<8;++i) {
        auto& b=bandButtons[(size_t)i];
        b.setButtonText("");
        b.setTooltip("Selectionner la bande " + juce::String(i+1));
        b.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        b.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
        b.setColour(juce::TextButton::textColourOffId,juce::Colour(bandColours[(size_t)i]));
        b.onClick=[this,i]{attachBand(i);};
        addAndMakeVisible(b);
    }
    presetSearch.setTextToShowWhenEmpty("Rechercher parmi 100 presets...",juce::Colours::grey);
    presetSearch.onTextChange=[this]{refillPresets();};
    addAndMakeVisible(presetSearch);
    presetCombo.onChange=[this]{if(presetCombo.getSelectedId()>0) processor.loadFactoryPreset((size_t)(presetCombo.getSelectedId()-1));};
    addAndMakeVisible(presetCombo);
    auto show=[this](juce::Component& c){addAndMakeVisible(c);};
    for(auto* b:{&buttonA,&buttonB,&undoButton,&redoButton,&resetButton,&bypassButton,&preButton,&postButton,
                 &tabParam,&tabDynamic,&tabGraphic,&holdButton,&freezeButton}) show(*b);
    tabParam.onClick=[this]{selectTab(0);};
    tabDynamic.onClick=[this]{selectTab(1);};
    tabGraphic.setEnabled(false);
    tabGraphic.setTooltip("Egaliseur graphique en developpement - audio non simule");
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
    buttonA.onClick=[this]{switchSnapshot(false);};
    buttonB.onClick=[this]{switchSnapshot(true);};
    undoButton.onClick=[this]{processor.undoManager.undo();};
    redoButton.onClick=[this]{processor.undoManager.redo();};
    resetButton.onClick=[this]{processor.resetToFlat();};
    preButton.onClick=[this]{showPre=!showPre;preButton.setToggleState(showPre,juce::dontSendNotification);repaint();};
    postButton.onClick=[this]{showPost=!showPost;postButton.setToggleState(showPost,juce::dontSendNotification);repaint();};
    preButton.setClickingTogglesState(true);postButton.setClickingTogglesState(true);
    preButton.setToggleState(true,juce::dontSendNotification);postButton.setToggleState(true,juce::dontSendNotification);
    stateA=processor.parameters.copyState();stateB=stateA.createCopy();
    buttonA.setToggleState(true,juce::dontSendNotification);
    refillPresets();attachBand(0);
    heldPre.fill(-96.f); heldPost.fill(-96.f);
    updateLanguageSafeTexts();selectTab(0);
    setResizable(false,false);setSize(1160,760);
    startTimerHz(24);
}
LoveEQAudioProcessorEditor::~LoveEQAudioProcessorEditor(){stopTimer();setLookAndFeel(nullptr);}
void LoveEQAudioProcessorEditor::selectTab(int tab) {
    activeTab=juce::jlimit(0,1,tab);
    tabParam.setToggleState(activeTab==0,juce::dontSendNotification);
    tabDynamic.setToggleState(activeTab==1,juce::dontSendNotification);
    repaint();
}
void LoveEQAudioProcessorEditor::updateLanguageSafeTexts() {
    // Literal words only. No icon glyph fonts or arrow characters replacing letters.
    tabParam.setButtonText("PARAM\u00C9TRIQUE");
    tabDynamic.setButtonText("DYNAMIQUE");
    tabGraphic.setButtonText("GRAPHIQUE");
    resetButton.setButtonText("RESET / FLAT");
    undoButton.setButtonText("ANNULER");
    redoButton.setButtonText("REFAIRE");
    holdButton.setButtonText("MAINTIEN PIC");
    freezeButton.setButtonText("FIGER");
    enabledButton.setButtonText("ACTIF");
    dynamicButton.setButtonText("DYN");
    soloButton.setButtonText("SOLO");
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
    processor.parameters.replaceState(toB?stateB.createCopy():stateA.createCopy());
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
    g.drawText("BIBLIOTH\u00C8QUE DE PRESETS",350,18,265,15,juce::Justification::centredLeft);
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
    g.drawText("ZERO LATENCY",w-264,100,132,19,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff84939d));
    g.drawText("0 sample",w-132,100,87,19,juce::Justification::centredLeft);
    g.drawText("COURBE : STEREO",378,99,175,26,juce::Justification::centredLeft);
    g.drawText("ECHELLE : +/- 18 dB",563,99,195,26,juce::Justification::centredLeft);

    // Single analyzer panel with a dedicated toolbar and a narrow meter strip.
    g.setColour(juce::Colour(0xff10171d));
    g.fillRoundedRectangle(spectrumPanel.toFloat(),7.f);
    g.setColour(juce::Colour(0xff39434b));
    g.drawRoundedRectangle(spectrumPanel.toFloat(),7.f,1.f);
    g.drawHorizontalLine(188,(float)spectrumPanel.getX(),(float)spectrumPanel.getRight());
    g.setColour(juce::Colour(0xff778994));
    g.setFont(juce::Font(juce::FontOptions(10.f,juce::Font::bold)));
    g.drawText("SPECTRE",42,156,87,18,juce::Justification::centredLeft);
    g.drawText("R\u00C9PONSE - dB",graph.getX(),190,154,16,juce::Justification::centredLeft);
    g.drawText("ANALYSEUR FFT 2048",graph.getRight()-196,190,189,16,juce::Justification::centredRight);

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
    for(int band=0;band<8;++band) {
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
    g.drawText("GLISSER UN POINT : FR\u00C9QUENCE / GAIN",graph.getRight()-280,spectrumPanel.getBottom()-26,278,18,juce::Justification::centredRight);

    // Two real peak meters (mono peak per bus; no fake L/R subdivisions).
    g.setColour(juce::Colour(0xff182229));
    g.fillRoundedRectangle(meterBox.toFloat(),4.f);
    g.setFont(juce::Font(juce::FontOptions(10.f,juce::Font::bold)));
    const auto meter=[&](float level,int x,const juce::String& caption) {
        g.setColour(juce::Colour(0xffa5b4bc));
        g.drawText(caption,x-12,meterBox.getY()+8,38,18,juce::Justification::centred);
        const float top=(float)meterBox.getY()+34.f;
        const float mh=(float)meterBox.getHeight()-71.f;
        g.setColour(juce::Colour(0xff0b1217));
        g.fillRect(juce::Rectangle<float>((float)x,top,12.f,mh));
        for(int k=0;k<=4;++k) {
            g.setColour(juce::Colour(0xff27343c));
            g.drawHorizontalLine((int)(top+mh*k/4.f),(float)x,(float)x+12.f);
        }
        const float db=juce::Decibels::gainToDecibels(level,-72.f);
        const float filled=juce::jlimit(0.f,mh,mh*(db+72.f)/72.f);
        g.setColour(level>=1.f?juce::Colours::orangered:gold);
        g.fillRect(juce::Rectangle<float>((float)x,top+mh-filled,12.f,filled));
        g.setColour(juce::Colour(0xffa5b4bc));
        g.setFont(juce::Font(juce::FontOptions(9.f)));
        g.drawText(juce::String(db,0),x-12,(int)(top+mh+5),38,17,juce::Justification::centred);
    };
    meter(processor.peakIn.load(),meterBox.getX()+20,"IN");
    meter(processor.peakOut.load(),meterBox.getX()+62,"OUT");

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
        static constexpr const char* filterNames[]={"BELL","LOW SHELF","HIGH SHELF","HIGH PASS","LOW PASS","NOTCH","BAND PASS"};
        const int filterIndex=filterType?juce::jlimit(0,6,(int)filterType->load()):0;
        g.setColour(active && active->load()<0.5f?juce::Colour(0xff6c7780):juce::Colour(0xffc3cbd1));
        g.drawText(filterNames[filterIndex],b.getX()+33,b.getY()+12,b.getWidth()-42,17,juce::Justification::centredLeft);
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

    // Bottom edit strip: filter, dynamic band, and deliberately honest EQ Match status.
    g.setColour(juce::Colour(0xff252e36));
    g.fillRoundedRectangle(bottomPanel.toFloat(),5.f);
    g.setColour(juce::Colour(0xff40505b));
    g.drawRoundedRectangle(bottomPanel.toFloat(),5.f,1.f);
    const int split1=controls.getRight()+8;
    const int split2=split1+287;
    g.drawVerticalLine(split1,(float)bottomPanel.getY(),(float)bottomPanel.getBottom());
    g.drawVerticalLine(split2,(float)bottomPanel.getY(),(float)bottomPanel.getBottom());
    g.setFont(juce::Font(juce::FontOptions(10.f,juce::Font::bold)));
    g.setColour(juce::Colour(bandColours[(size_t)selectedBand]));
    g.fillEllipse(37.f,(float)bottomPanel.getY()+17.f,6.f,6.f);
    g.setColour(juce::Colour(0xffc6d0d7));
    g.drawText("BANDE "+juce::String(selectedBand+1),49,bottomPanel.getY()+9,119,20,juce::Justification::centredLeft);
    g.drawText("DYNAMIQUE",split1+12,bottomPanel.getY()+9,131,20,juce::Justification::centredLeft);
    g.drawText("EQ MATCH",split2+12,bottomPanel.getY()+9,126,20,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff8496a0));
    g.setFont(juce::Font(juce::FontOptions(9.f)));
    const std::array<const char*,6> labels {"FILTRE","FR\u00C9QUENCE","GAIN","Q / LARGEUR","CANAL","PENTE"};
    const int x0=controls.getX()+10;
    const std::array<int,6> xx {0,94,187,272,362,453};
    for(int i=0;i<6;++i)
        g.drawText(labels[(size_t)i],x0+xx[(size_t)i],bottomPanel.getY()+39,
                   (i==0?84:82),17,juce::Justification::centredLeft);
    for(int i=0;i<4;++i) {
        const char* names[]={"SEUIL","RANGE","ATTAQUE","RELEASE"};
        g.drawText(names[i],split1+12+i*67,bottomPanel.getY()+39,64,17,juce::Justification::centredLeft);
    }
    g.drawText("Correspondance spectrale",split2+12,bottomPanel.getY()+48,202,18,juce::Justification::centredLeft);
    g.drawText("EN COURS DE D\u00C9VELOPPEMENT",split2+12,bottomPanel.getY()+72,220,17,juce::Justification::centredLeft);
    g.drawText(activeTab==1?"EDITION DYNAMIQUE":"EDITION PARAM\u00C9TRIQUE",bottomPanel.getX()+10,bottomPanel.getBottom()-22,315,15,juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xffaebbc3));
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.drawText("ENTR\u00C9E",footerPanel.getX()+3,footerPanel.getY()+10,53,15,juce::Justification::centredLeft);
    g.drawText("SORTIE",footerPanel.getX()+214,footerPanel.getY()+10,53,15,juce::Justification::centredLeft);
    g.drawText("LOVE EQ - PR\u00CAT POUR LE MIXAGE",footerPanel.getX()+470,footerPanel.getY()+10,335,16,juce::Justification::centredLeft);
    g.drawText("by Liberty Music Software",footerPanel.getRight()-198,footerPanel.getY()+10,194,15,juce::Justification::centredRight);
}
void LoveEQAudioProcessorEditor::resized() {
    const int w=getWidth(), h=getHeight();
    (void)h;
    presetSearch.setBounds(350,37,151,32);
    presetCombo.setBounds(508,37,245,32);
    buttonA.setBounds(w-288,34,34,32);
    buttonB.setBounds(w-250,34,34,32);
    undoButton.setBounds(w-210,34,55,32);
    redoButton.setBounds(w-151,34,54,32);
    bypassButton.setBounds(w-91,34,66,32);
    tabParam.setBounds(27,97,108,29);
    tabDynamic.setBounds(138,97,102,29);
    tabGraphic.setBounds(243,97,107,29);
    resetButton.setBounds(w-109,96,86,30);

    spectrumPanel={24,145,w-48,344};
    preButton.setBounds(122,155,49,25);
    postButton.setBounds(175,155,49,25);
    holdButton.setBounds(239,155,103,25);
    freezeButton.setBounds(346,155,58,25);
    graph={65,210,w-219,239};
    meterBox={w-132,192,101,272};
    bandRow={24,500,w-48,77};
    const int gap=9, tileW=(bandRow.getWidth()-7*gap)/8;
    for(int i=0;i<8;++i) {
        bandButtons[(size_t)i].setBounds(bandRow.getX()+i*(tileW+gap),bandRow.getY(),tileW,bandRow.getHeight());
        bandButtons[(size_t)i].toFront(false);
    }
    bottomPanel={24,587,w-48,122};
    controls={24,587,560,122};
    footerPanel={24,718,w-48,35};
    // Compact but non-overlapping controls, no scroll bar, no hidden bottom row.
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
    output.setBounds(footerPanel.getX()+266,footerPanel.getY()+3,132,30);
    // Numeric fields below faders leave enough horizontal space for each label.
    for(auto* slider:{&frequency,&gain,&q,&threshold,&range,&attack,&release})
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow,false,61,18);
    input.setTextBoxStyle(juce::Slider::TextBoxRight,false,57,24);
    output.setTextBoxStyle(juce::Slider::TextBoxRight,false,57,24);
}
void LoveEQAudioProcessorEditor::mouseDown(const juce::MouseEvent& e) {
    draggingBand=-1;
    if(!graph.contains(e.getPosition())) return;
    float best=20.f;
    for(int i=0;i<8;++i) {
        const auto id=[i](const juce::String& s){return LoveEQAudioProcessor::bandId(i,s);};
        auto read=[this](const juce::String& s){return processor.parameters.getRawParameterValue(s)->load();};
        if(read(id("enabled"))<0.5f)continue;
        const float d=juce::Point<float>(xForHz(read(id("hz"))),yForDb(read(id("gain")))).getDistanceFrom(e.position);
        if(d<best){best=d;draggingBand=i;}
    }
    if(draggingBand>=0)attachBand(draggingBand);
}
void LoveEQAudioProcessorEditor::mouseDrag(const juce::MouseEvent& e) {
    if(draggingBand<0)return;
    const float x=juce::jlimit(0.f,1.f,(e.position.x-graph.getX())/graph.getWidth());
    const float db=juce::jlimit(-18.f,18.f,(graph.getCentreY()-e.position.y)*36.f/graph.getHeight());
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
