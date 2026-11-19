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
        b.setButtonText(juce::String(i+1));
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
    for(auto* b:{&buttonA,&buttonB,&undoButton,&redoButton,&resetButton,&bypassButton,&preButton,&postButton}) show(*b);
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
    setResizable(true,true);setResizeLimits(1000,690,1750,1120);setSize(1220,790);
    startTimerHz(24);
}
LoveEQAudioProcessorEditor::~LoveEQAudioProcessorEditor(){stopTimer();setLookAndFeel(nullptr);}
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
    g.fillAll(background);
    g.setColour(juce::Colour(0xffe8dec9));
    g.setFont(juce::Font(juce::FontOptions(30.f,juce::Font::bold)));
    g.drawText("LOVE EQ",27,15,260,36,juce::Justification::centredLeft);
    g.setFont(juce::Font(juce::FontOptions(12.f)));
    g.setColour(juce::Colour(0xff8997a1));
    g.drawText("by Liberty Music Software",30,49,300,18,juce::Justification::centredLeft);
    g.setColour(trim);g.drawLine(24,79,(float)getWidth()-24,79,1.f);
    g.setColour(panel);g.fillRoundedRectangle(graph.toFloat().expanded(9.f,13.f),8.f);
    g.setColour(juce::Colour(0xff10191e));g.fillRoundedRectangle(graph.toFloat(),4.f);
    g.saveState();g.reduceClipRegion(graph);
    const float freqs[]={20,50,100,200,500,1000,2000,5000,10000,20000};
    g.setColour(juce::Colour(0xff2e3a41));
    for(float f:freqs)g.drawVerticalLine((int)xForHz(f),(float)graph.getY(),(float)graph.getBottom());
    for(int db=-18;db<=18;db+=6) {
        g.setColour(db==0?juce::Colour(0xff65675a):juce::Colour(0xff2e3a41));
        g.drawHorizontalLine((int)yForDb((float)db),(float)graph.getX(),(float)graph.getRight());
    }
    if(showPre)drawSpectrum(g,preSpectrum,juce::Colour(0x774c93a7));
    if(showPost)drawSpectrum(g,postSpectrum,juce::Colour(0x99bfa46d));
    juce::Path response;
    for(int x=0;x<graph.getWidth();x+=2) {
        const float f=20.f*std::pow(1000.f,(float)x/graph.getWidth());
        const float y=yForDb(dbAtHz(f));
        if(x==0)response.startNewSubPath((float)graph.getX(),y);else response.lineTo((float)graph.getX()+x,y);
    }
    g.setColour(gold);g.strokePath(response,juce::PathStrokeType(2.5f));
    for(int i=0;i<8;++i) {
        const auto id=[i](const juce::String& s){return LoveEQAudioProcessor::bandId(i,s);};
        auto val=[this](const juce::String& s){return processor.parameters.getRawParameterValue(s)->load();};
        if(val(id("enabled"))<0.5f)continue;
        const auto colour=juce::Colour(bandColours[(size_t)i]);
        const float x=xForHz(val(id("hz"))),y=yForDb(val(id("gain")));
        g.setColour(juce::Colour(0xff111920));g.fillEllipse(x-10.f,y-10.f,20.f,20.f);
        g.setColour(colour);g.drawEllipse(x-10.f,y-10.f,20.f,20.f,i==selectedBand?2.8f:1.6f);
        g.setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
        g.drawText(juce::String(i+1),(int)x-9,(int)y-8,18,16,juce::Justification::centred);
    }
    g.restoreState();
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.setColour(juce::Colour(0xff84939d));
    for(float f:freqs) {
        auto label=f>=1000?juce::String(f/1000.f, f==1000||f==2000||f==5000||f==10000||f==20000?0:1)+"k":juce::String((int)f);
        g.drawText(label,(int)xForHz(f)-19,graph.getBottom()+5,38,15,juce::Justification::centred);
    }
    g.setColour(trim);g.fillRoundedRectangle(meterBox.toFloat(),6.f);
    auto meter=[&](float level,int x,juce::String label) {
        const auto box=juce::Rectangle<float>((float)x,(float)meterBox.getY()+34,18.f,(float)meterBox.getHeight()-78);
        g.setColour(juce::Colour(0xff0b1218));g.fillRect(box);
        const auto h=juce::jlimit(0.f,box.getHeight(),box.getHeight()*(juce::Decibels::gainToDecibels(level,-72.f)+72.f)/72.f);
        g.setColour(level>1.f?juce::Colours::orangered:gold);
        g.fillRect(juce::Rectangle<float>(box.getX(),box.getBottom()-h,box.getWidth(),h));
        g.setColour(text);g.drawText(label,x-8,meterBox.getY()+9,42,18,juce::Justification::centred);
    };
    meter(processor.peakIn.load(),meterBox.getX()+19,"IN");
    meter(processor.peakOut.load(),meterBox.getX()+60,"OUT");
    g.setColour(panel);g.fillRoundedRectangle(controls.toFloat().expanded(7.f,10.f),6.f);
    g.setColour(juce::Colour(0xffd5d8d0));
    g.setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
    const juce::String labels1[]={"FRÉQUENCE","GAIN","Q / LARGEUR","FILTRE","CANAL","PENTE","ÉTAT"};
    const juce::String labels2[]={"DYNAMIQUE","SEUIL","RANGE","ATTAQUE","RELÂCHEMENT","ENTRÉE","SORTIE"};
    const int cw=controls.getWidth()/7;
    for(int i=0;i<7;++i) {
        g.drawText(labels1[i],controls.getX()+i*cw+3,controls.getY()-3,cw-6,15,juce::Justification::centredLeft);
        g.drawText(labels2[i],controls.getX()+i*cw+3,controls.getY()+71,cw-6,15,juce::Justification::centredLeft);
    }
    g.setFont(juce::Font(juce::FontOptions(10.f)));
    g.setColour(juce::Colour(0xff8fa1a6));
    g.drawText("PARAMETRIC / DYNAMIC  |  MID/SIDE  |  ZERO LATENCY",graph.getX(),graph.getY()-31,620,16,juce::Justification::centredLeft);
    g.drawText("DOUBLE-CLIC : VALEUR NUMÉRIQUE   |   GLISSER LES NŒUDS : FRÉQUENCE / GAIN",30,getHeight()-27,getWidth()-60,17,juce::Justification::centredLeft);
    g.drawText(onB?"COMPARAISON B":"COMPARAISON A",getWidth()-202,getHeight()-28,176,18,juce::Justification::centredRight);
}
void LoveEQAudioProcessorEditor::resized() {
    const int w=getWidth(),h=getHeight();
    const int top=91;
    presetSearch.setBounds(28,top,198,30);presetCombo.setBounds(235,top,juce::jmin(335,w-680),30);
    const int right=w-22;
    bypassButton.setBounds(right-89,20,88,35);
    redoButton.setBounds(right-171,20,76,35);
    undoButton.setBounds(right-257,20,80,35);
    buttonB.setBounds(right-327,20,32,35);buttonA.setBounds(right-363,20,32,35);
    resetButton.setBounds(right-140,top,140,30);
    preButton.setBounds(30,top+39,55,25);postButton.setBounds(91,top+39,60,25);
    graph={45,top+85,w-184,h-(top+85)-276};
    meterBox={w-115,graph.getY(),91,graph.getHeight()};
    bandRow={31,graph.getBottom()+15,w-62,50};
    const int gap=8, bw=(bandRow.getWidth()-(7*gap))/8;
    for(int i=0;i<8;++i)bandButtons[(size_t)i].setBounds(bandRow.getX()+i*(bw+gap),bandRow.getY(),bw,bandRow.getHeight());
    controls={31,bandRow.getBottom()+22,w-62,142};
    const int cw=controls.getWidth()/7;
    auto slot=[&](juce::Component& c,int idx,int row) {
        c.setBounds(controls.getX()+idx*cw+2,controls.getY()+row*74+17,cw-7,36);
    };
    slot(frequency,0,0);slot(gain,1,0);slot(q,2,0);slot(typeBox,3,0);
    slot(channelBox,4,0);slot(slopeBox,5,0);slot(enabledButton,6,0);
    slot(dynamicButton,0,1);slot(threshold,1,1);slot(range,2,1);slot(attack,3,1);
    slot(release,4,1);slot(input,5,1);slot(output,6,1);
    soloButton.setBounds(graph.getRight()-90,top+39,78,25);
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
    if(showPre)updateFFT(true,preSpectrum);
    if(showPost)updateFFT(false,postSpectrum);
    repaint();
}
