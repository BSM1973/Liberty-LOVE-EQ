#pragma once
#if JUCE_IOS
#include "MobileTouchLayout.h"
#include <cmath>
#include <map>

// UI-only. All writes use the existing host-notified RangedAudioParameter.
class LoveEQTouchPanel final : public juce::Component, private juce::Timer {
    using Param=juce::RangedAudioParameter;
    class Tile final : public juce::Component {
    public:
        ~Tile() override { finish(); }
        void bind(Param* p,juce::String name) {
            finish();param=p;label=std::move(name);repaint();
            setAccessible(true);setTitle(label);
        }
        void paint(juce::Graphics& g) override {
            auto r=getLocalBounds().toFloat().reduced(1.f);
            g.setColour(juce::Colour(0xff263541));g.fillRoundedRectangle(r,7.f);
            g.setColour(juce::Colour(0xff637484));g.drawRoundedRectangle(r,7.f,1.f);
            g.setColour(juce::Colour(0xffe7c58e));
            g.setFont(juce::Font(juce::FontOptions(12.f)));
            g.drawFittedText(label,getLocalBounds().removeFromTop(19).reduced(3,0),juce::Justification::centred,1);
            if(!param)return;
            const auto value=param->convertFrom0to1(param->getValue());
            juce::String valueText;
            if(label=="Hz") valueText=juce::String(value,value>=1000.f?0:1)+" Hz";
            else if(label=="Q") valueText=juce::String(value,2);
            else if(label=="ms" || label=="Att." || label=="Rel.") valueText=juce::String(value,0)+" ms";
            else valueText=juce::String(value,1)+" dB";
            g.setColour(juce::Colour(0xfff4e8d3));
            g.setFont(juce::Font(juce::FontOptions(19.f,juce::Font::bold)));
            g.drawFittedText(valueText,getLocalBounds().withTrimmedTop(19).withTrimmedBottom(8).reduced(3,0),juce::Justification::centred,1);
            g.setColour(juce::Colour(0xffb99760));
            g.fillRoundedRectangle(r.getX()+5,r.getBottom()-6,(r.getWidth()-10)*param->getValue(),3.f,1.5f);
        }
        void mouseDown(const juce::MouseEvent&) override {
            finish();if(!param)return;param->beginChangeGesture();dragging=true;initial=param->getValue();
        }
        void mouseDrag(const juce::MouseEvent& e) override {
            if(!dragging||!param)return;
            const float sensitivity=e.mods.isShiftDown()||fine?0.00065f:0.0035f;
            param->setValueNotifyingHost(juce::jlimit(0.f,1.f,initial-e.getDistanceFromDragStartY()*sensitivity));repaint();
        }
        void mouseUp(const juce::MouseEvent&) override {finish();}
        void mouseDoubleClick(const juce::MouseEvent&) override {
            finish();if(param){param->beginChangeGesture();param->setValueNotifyingHost(param->getDefaultValue());param->endChangeGesture();repaint();}
        }
        void finish(){if(dragging&&param)param->endChangeGesture();dragging=false;}
        bool fine=false;
    private:
        Param* param=nullptr;juce::String label;float initial=0.f;bool dragging=false;
    };
    class Plot final : public juce::Component {
    public:
        explicit Plot(LoveEQTouchPanel& p):owner(p){}
        ~Plot() override {finish();}
        void paint(juce::Graphics& g) override {
            auto r=plot();g.setColour(juce::Colour(0xff0d151e));g.fillRoundedRectangle(getLocalBounds().toFloat(),6.f);
            if(r.getWidth()<10||r.getHeight()<10)return;
            g.setColour(juce::Colour(0xff293b49));
            for(float hz:{20.f,100.f,1000.f,10000.f,20000.f})if(x(hz)>=r.getX()&&x(hz)<=r.getRight())g.drawVerticalLine((int)x(hz),r.getY(),r.getBottom());
            for(float db:{-24.f,-12.f,0.f,12.f,24.f})g.drawHorizontalLine((int)y(db),r.getX(),r.getRight());
            g.saveState();g.reduceClipRegion(r.toNearestInt());
            if(owner.audioFresh){
                juce::Path spectrum;
                const double sr=owner.processor.getSampleRate();
                if(sr>0.)for(int i=0;i<=100;++i){
                    float f=frequencyAt((float)i/100.f);
                    int bin=juce::jlimit(1,1024,(int)std::round(f*2048./sr));
                    float py=r.getBottom()-r.getHeight()*(juce::jlimit(-96.f,0.f,owner.spectrum[(size_t)bin])+96.f)/96.f;
                    if(i==0)spectrum.startNewSubPath(x(f),py);else spectrum.lineTo(x(f),py);
                }
                g.setColour(juce::Colour(0xff699eaa));g.strokePath(spectrum,juce::PathStrokeType(1.25f));
            }
            const double sr=juce::jmax(8000.,owner.processor.getSampleRate());
            struct Filter{loveeq::Coefficients b;int count;};
            std::vector<Filter> filters;filters.reserve(16);
            const bool graphic=owner.graphic();
            for(int i=0;i<(graphic?16:8);++i){
                if(graphic){filters.push_back({loveeq::design(loveeq::Type::Bell,sr,LoveEQAudioProcessor::graphicHz[(size_t)i],owner.read(LoveEQAudioProcessor::graphicId(i)),1.8),1});continue;}
                if(owner.readBand(i,"enabled")<0.5f)continue;
                int t=juce::jlimit(0,6,(int)owner.readBand(i,"type")),sl=juce::jlimit(0,5,(int)owner.readBand(i,"slope"));
                auto type=static_cast<loveeq::Type>(t);
                auto hz=owner.readBand(i,"hz");
                auto b=loveeq::design(type,sr,hz,owner.readBand(i,"gain"),owner.readBand(i,"q"));
                if(t==3||t==4){
                    constexpr int counts[]={0,1,1,2,3,4};filters.push_back({b,counts[sl]});
                    if(loveeq::needsFirstOrder(sl))filters.push_back({loveeq::design(type,sr,hz,0,1,true),1});
                }else filters.push_back({b,1});
            }
            juce::Path response;
            for(int i=0;i<=120;++i){
                float hz=frequencyAt((float)i/120.f),db=0.f;
                for(auto& f:filters)db+=(float)loveeq::magnitudeDb(f.b,sr,hz)*f.count;
                if(i==0)response.startNewSubPath(x(hz),y(db));else response.lineTo(x(hz),y(db));
            }
            g.setColour(juce::Colour(0xffe8c18a));g.strokePath(response,juce::PathStrokeType(2.f));
            for(int i=0;i<(graphic?16:8);++i){
                auto pt=node(i,graphic);bool selected=i==(graphic?owner.graphicBand:owner.band);
                g.setColour(juce::Colour(0xff17232d));g.fillEllipse(pt.x-9,pt.y-9,18,18);
                g.setColour(selected?juce::Colour(0xffffdda0):juce::Colour(0xff819baa));g.drawEllipse(pt.x-9,pt.y-9,18,18,selected?2.5f:1.f);
                if(selected){g.setFont(juce::Font(juce::FontOptions(10.f,juce::Font::bold)));g.drawText(juce::String(i+1),(int)pt.x-9,(int)pt.y-9,18,18,juce::Justification::centred);}
            }
            g.restoreState();
            g.setColour(juce::Colour(0xffa5b7c5));g.setFont(juce::Font(juce::FontOptions(10.f)));
            for(float hz:{20.f,100.f,1000.f,10000.f,20000.f})
                if(x(hz)>=r.getX()&&x(hz)<=r.getRight())g.drawFittedText(hz>=1000?juce::String((int)(hz/1000))+"k":juce::String((int)hz),juce::Rectangle<int>((int)x(hz)-14,getHeight()-15,28,14),juce::Justification::centred,1);
        }
        void mouseDown(const juce::MouseEvent& e) override {
            if(!plot().expanded(5.f).contains(e.position))return;
            contacts[e.source.getIndex()]=e.position;
            if(contacts.size()>=2){
                finish();pinching=true;
                auto a=contacts.begin(),b=std::next(a);
                initialDistance=juce::jmax(8.f,a->second.getDistanceFrom(b->second));
                initialSpan=span;
                initialAnchor=domainStart+span*positionFraction((a->second.x+b->second.x)*.5f);
                return;
            }
            if(pinching)return;
            finish();
            const bool graphic=owner.graphic();int best=-1;float distance=23.f;
            // Give the current selection priority when nodes overlap.
            int selected=graphic?owner.graphicBand:owner.band;
            float d=node(selected,graphic).getDistanceFrom(e.position);
            if(d<distance){best=selected;distance=d;}
            for(int i=0;i<(graphic?16:8);++i){
                d=node(i,graphic).getDistanceFrom(e.position);
                if(d<distance-1.f){best=i;distance=d;}
            }
            if(best<0)return;
            if(graphic)owner.graphicBand=best;else owner.band=best;
            owner.bindPage();
            hzParam=graphic?nullptr:owner.parameter(LoveEQAudioProcessor::bandId(best,"hz"));
            gainParam=owner.parameter(graphic?LoveEQAudioProcessor::graphicId(best):LoveEQAudioProcessor::bandId(best,"gain"));
            if(hzParam){hzParam->beginChangeGesture();startHz=hzParam->convertFrom0to1(hzParam->getValue());}
            if(gainParam){gainParam->beginChangeGesture();startGain=gainParam->convertFrom0to1(gainParam->getValue());}
        }
        void mouseDrag(const juce::MouseEvent& e) override {
            auto r=plot();if(r.isEmpty())return;
            auto it=contacts.find(e.source.getIndex());if(it==contacts.end())return;
            it->second=e.position;
            if(pinching){
                if(contacts.size()>=2){
                    auto a=contacts.begin(),b=std::next(a);
                    const float distance=juce::jmax(8.f,a->second.getDistanceFrom(b->second));
                    span=juce::jlimit(.2f,1.f,initialSpan*initialDistance/distance);
                    domainStart=juce::jlimit(0.f,1.f-span,initialAnchor-span*positionFraction((a->second.x+b->second.x)*.5f));
                    repaint();
                }
                return;
            }
            float factor=owner.fine.getToggleState()?0.2f:1.f;
            if(hzParam){auto hz=startHz*std::pow(1000.f,e.getDistanceFromDragStartX()*factor*span/r.getWidth());hzParam->setValueNotifyingHost(hzParam->convertTo0to1(juce::jlimit(20.f,20000.f,hz)));}
            if(gainParam){float db=startGain-e.getDistanceFromDragStartY()*48.f*factor/r.getHeight();auto range=gainParam->getNormalisableRange();gainParam->setValueNotifyingHost(gainParam->convertTo0to1(juce::jlimit(range.start,range.end,db)));}
            repaint();
        }
        void mouseUp(const juce::MouseEvent& e) override {contacts.erase(e.source.getIndex());finish();if(contacts.empty())pinching=false;}
        void resetZoom(){domainStart=0.f;span=1.f;repaint();}
        void cancelContacts(){finish();contacts.clear();pinching=false;}
        void finish(){if(hzParam)hzParam->endChangeGesture();if(gainParam)gainParam->endChangeGesture();hzParam=gainParam=nullptr;}
    private:
        LoveEQTouchPanel& owner;Param* hzParam=nullptr;Param* gainParam=nullptr;float startHz=1000,startGain=0;
        juce::Rectangle<float> plot() const {return getLocalBounds().toFloat().reduced(14.f,5.f).withTrimmedBottom(13.f);}
        std::map<int,juce::Point<float>> contacts;
        bool pinching=false;float domainStart=0.f,span=1.f,initialDistance=8.f,initialSpan=1.f,initialAnchor=.5f;
        float positionFraction(float px) const {auto r=plot();return juce::jlimit(0.f,1.f,(px-r.getX())/juce::jmax(1.f,r.getWidth()));}
        float frequencyAt(float t) const {return 20.f*std::pow(1000.f,domainStart+span*t);}
        float x(float hz) const {auto r=plot();return r.getX()+r.getWidth()*(std::log(juce::jlimit(20.f,20000.f,hz)/20.f)/std::log(1000.f)-domainStart)/span;}
        float y(float db) const {auto r=plot();return r.getCentreY()-juce::jlimit(-24.f,24.f,db)*r.getHeight()/48.f;}
        juce::Point<float> node(int i,bool graphic) const {
            return graphic?juce::Point<float>{x((float)LoveEQAudioProcessor::graphicHz[(size_t)i]),y(owner.read(LoveEQAudioProcessor::graphicId(i)))}:
                juce::Point<float>{x(owner.readBand(i,"hz")),y(owner.readBand(i,"gain"))};
        }
    };
public:
    explicit LoveEQTouchPanel(LoveEQAudioProcessor& p):processor(p),graph(*this){
        for(auto* b:{&eq,&dyn,&mix,&fine,&more,&mode,&enabled,&solo,&prev,&next,&bypass})style(*b);
        more.onClick=[this]{openTools();};
        fine.setClickingTogglesState(true);
        eq.onClick=[this]{page=0;bindPage();};
        dyn.onClick=[this]{page=1;bindPage();};
        mix.onClick=[this]{page=2;bindPage();};
        mode.onClick=[this]{change("eq_mode",graphic()?0.f:1.f);page=0;bindPage();};
        enabled.onClick=[this]{toggleBand(page==1?"dynamic":"enabled");};
        solo.onClick=[this]{toggleBand("solo");};
        bypass.onClick=[this]{change("bypass",read("bypass")>.5f?0.f:1.f);};
        prev.onClick=[this]{if(graphic())graphicBand=(graphicBand+15)%16;else band=(band+7)%8;bindPage();};
        next.onClick=[this]{if(graphic())graphicBand=(graphicBand+1)%16;else band=(band+1)%8;bindPage();};
        for(int i=0;i<8;++i){auto& b=bands[(size_t)i];style(b);b.onClick=[this,i]{if(graphic())graphicBand=(graphicBand/8)*8+i;else band=i;bindPage();};}
        for(auto& t:tiles)addAndMakeVisible(t);
        addAndMakeVisible(graph);
        bindPage();startTimerHz(18);
    }
    ~LoveEQTouchPanel() override {stopTimer();finishGestures();}
    void visibilityChanged() override {if(!isShowing())finishGestures();else bindPage();}
    std::function<void(int,int)> onAdvanced;
    void finishGestures(){graph.cancelContacts();for(auto& t:tiles)t.finish();}
    void paint(juce::Graphics& g) override {g.fillAll(juce::Colour(0xff151a22));}
    void resized() override {
        auto a=getLocalBounds().reduced(3);
        auto row=a.removeFromTop(44);
        place(row,{&eq,&dyn,&mix,&fine,&more});
        a.removeFromTop(3);
        juce::Rectangle<int> controls;
        if(loveeq::mobile::sideBySide(getWidth(),getHeight())){
            controls=a.removeFromRight(a.getWidth()*49/100);a.removeFromRight(5);
            graph.setBounds(a);
        }else {
            const int reserve=getWidth()<402?232:189;
            graph.setBounds(a.removeFromTop(juce::jmax(48,a.getHeight()-reserve)));
            a.removeFromTop(3);controls=a;
        }
        auto toolbar=controls.removeFromTop(44);place(toolbar,{&prev,&mode,&next,&bypass});
        controls.removeFromTop(3);
        const int cols=controls.getWidth()<394?4:8;
        const int rows=8/cols;
        for(int i=0;i<8;++i){
            auto r=juce::Rectangle<int>(controls.getX(),controls.getY()+(i/cols)*46,controls.getWidth(),44);
            auto c=loveeq::mobile::cell({r.getX(),r.getY(),r.getWidth(),r.getHeight()},cols,i%cols);
            bands[(size_t)i].setBounds(c.x,c.y,c.w,c.h);
        }
        controls.removeFromTop(rows*46);
        // Page-specific toggles share the same row with the numeric controls.
        const bool g=graphic();const bool toggle=page!=2&&!g;
        const int count=page==2?2:g?1:page==1?4:3;
        auto valueArea=controls;
        if(toggle){
            const int tw=juce::jmin(84,valueArea.getWidth()/4);
            auto right=valueArea.removeFromRight(tw);
            enabled.setBounds(right.removeFromTop(right.getHeight()/2).reduced(1));
            solo.setBounds(right.reduced(1));
        }else{enabled.setBounds(juce::Rectangle<int>{});solo.setBounds(juce::Rectangle<int>{});}
        for(int i=0;i<4;++i){
            if(i<count){auto c=loveeq::mobile::cell({valueArea.getX(),valueArea.getY(),valueArea.getWidth(),valueArea.getHeight()},count,i);tiles[(size_t)i].setBounds(c.x,c.y,c.w,c.h);}
            else tiles[(size_t)i].setBounds(juce::Rectangle<int>{});
        }
    }
private:
    LoveEQAudioProcessor& processor;
    Plot graph;
    std::array<Tile,4> tiles;
    std::array<juce::TextButton,8> bands;
    juce::TextButton eq{"EQ"},dyn{"DYN"},mix{"MIX"},fine{"FIN"},more{"PLUS"},mode{"PARAM"},enabled{"ACTIF"},solo{"SOLO"},prev{"<"},next{">"},bypass{"BYPASS"};
    int page=0,band=0,graphicBand=0;bool lastGraphic=false;
    unsigned int scope=0,epoch=0;bool audioFresh=false;
    juce::dsp::FFT fft{11};std::array<float,4096> scratch{};std::array<float,1025> spectrum{};
    int refresh=18;
    void openTools(){
        finishGestures();
        juce::PopupMenu menu;
        menu.addItem(1,processor.getUiLanguage()=="en"?"Reset graph zoom":"Zoom : vue globale");
        if(!graphic()){
            juce::PopupMenu types,channels,slopes;
            const juce::StringArray t{"Bell","Low Shelf","High Shelf","High Pass","Low Pass","Notch","Band Pass"};
            const juce::StringArray c{"Stereo","Left","Right","Mid","Side"};
            const juce::StringArray sl{"6 dB/oct","12 dB/oct","18 dB/oct","24 dB/oct","36 dB/oct","48 dB/oct"};
            for(int i=0;i<t.size();++i)types.addItem(10+i,t[i],true,(int)readBand(band,"type")==i);
            for(int i=0;i<c.size();++i)channels.addItem(20+i,c[i],true,(int)readBand(band,"channel")==i);
            for(int i=0;i<sl.size();++i)slopes.addItem(30+i,sl[i],true,(int)readBand(band,"slope")==i);
            menu.addSubMenu("Type",types);menu.addSubMenu("Canal / Channel",channels);menu.addSubMenu("Pente / Slope",slopes);
            menu.addItem(41,"Harmonic DNA");menu.addItem(42,"Relationship EQ");
        }
        menu.addItem(43,"Time Lens");menu.addItem(44,"LOVE INTENT");menu.addItem(45,"EQ MATCH");
        const int selected=band;
        auto safe=juce::Component::SafePointer<LoveEQTouchPanel>(this);
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&more).withMinimumWidth(230).withStandardItemHeight(44),[safe,selected](int id){
            if(!safe||!id)return;
            if(id==1)safe->graph.resetZoom();
            else if(id>=10&&id<17)safe->change(LoveEQAudioProcessor::bandId(selected,"type"),(float)(id-10));
            else if(id>=20&&id<25)safe->change(LoveEQAudioProcessor::bandId(selected,"channel"),(float)(id-20));
            else if(id>=30&&id<36)safe->change(LoveEQAudioProcessor::bandId(selected,"slope"),(float)(id-30));
            else if(id>=41&&id<=45&&safe->onAdvanced)safe->onAdvanced(id-40,selected);
            safe->repaint();
        });
    }
    void style(juce::TextButton& b){addAndMakeVisible(b);b.setColour(juce::TextButton::buttonColourId,juce::Colour(0xff273542));b.setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xff84683d));b.setColour(juce::TextButton::textColourOffId,juce::Colour(0xfff1dfbd));}
    Param* parameter(const juce::String& id){return processor.parameters.getParameter(id);}
    float read(const juce::String& id) const {auto* p=processor.parameters.getRawParameterValue(id);return p?p->load():0.f;}
    float readBand(int i,const char* suffix) const {return read(LoveEQAudioProcessor::bandId(i,suffix));}
    bool graphic() const {return read("eq_mode")>.5f;}
    void change(const juce::String& id,float value){if(auto* p=parameter(id)){p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(value));p->endChangeGesture();}}
    void toggleBand(const char* suffix){auto id=LoveEQAudioProcessor::bandId(band,suffix);change(id,read(id)>.5f?0.f:1.f);updateLabels();}
    static void place(juce::Rectangle<int> r,std::initializer_list<juce::TextButton*> bs){int i=0;for(auto* b:bs){auto c=loveeq::mobile::cell({r.getX(),r.getY(),r.getWidth(),r.getHeight()},(int)bs.size(),i++);b->setBounds(c.x,c.y,c.w,c.h);}}
    void bindPage(){
        graph.finish();
        for(auto& t:tiles)t.bind(nullptr,{});
        const bool g=graphic();lastGraphic=g;
        if(g&&page==1)page=0;
        auto bind=[this](int i,const char* id,const char* label){tiles[(size_t)i].bind(parameter(LoveEQAudioProcessor::bandId(band,id)),label);};
        if(page==2){tiles[0].bind(parameter("input_gain"),"IN");tiles[1].bind(parameter("output_gain"),"OUT");}
        else if(g){tiles[0].bind(parameter(LoveEQAudioProcessor::graphicId(graphicBand)),"GAIN");}
        else if(page==1){bind(0,"threshold","SEUIL");bind(1,"range","PLAGE");bind(2,"attack","Att.");bind(3,"release","Rel.");}
        else {bind(0,"hz","Hz");bind(1,"gain","GAIN");bind(2,"q","Q");}
        updateLabels();resized();repaint();graph.repaint();
    }
    void updateLabels(){
        const bool g=graphic(),en=processor.getUiLanguage()=="en";
        fine.setButtonText(en?"FINE":"FIN");mode.setButtonText(g?"GRAPH":"PARAM");
        dyn.setEnabled(!g);
        eq.setToggleState(page==0,juce::dontSendNotification);dyn.setToggleState(page==1,juce::dontSendNotification);mix.setToggleState(page==2,juce::dontSendNotification);
        bypass.setToggleState(read("bypass")>.5f,juce::dontSendNotification);
        enabled.setButtonText(page==1?"DYN":en?"ON":"ACTIF");
        enabled.setToggleState(readBand(band,page==1?"dynamic":"enabled")>.5f,juce::dontSendNotification);
        solo.setToggleState(readBand(band,"solo")>.5f,juce::dontSendNotification);
        for(int i=0;i<8;++i){int n=g?(graphicBand/8)*8+i:i;bands[(size_t)i].setButtonText(juce::String(n+1));bands[(size_t)i].setToggleState(n==(g?graphicBand:band),juce::dontSendNotification);}
        for(auto& t:tiles)t.fine=fine.getToggleState();
    }
    void timerCallback() override {
        if(!isShowing()){if(refresh!=2){refresh=2;startTimerHz(refresh);}return;}
        const auto wanted=processor.mobileAudioReady.load(std::memory_order_acquire)?18:8;
        if(refresh!=wanted){refresh=wanted;startTimerHz(refresh);}
        if(graphic()!=lastGraphic){finishGestures();bindPage();}
        updateLabels();
        auto e=processor.mobileAudioEpoch.load(std::memory_order_acquire);
        if(e!=epoch){epoch=e;scope=0;audioFresh=false;}
        if(!processor.mobileAudioReady.load(std::memory_order_acquire))audioFresh=false;
        else {
            auto end=processor.scopeIndex.load(std::memory_order_acquire);
            if(end!=0&&end!=scope){
                scope=end;auto& w=loveeq::mobile::hannWindow<2048>();
                for(unsigned int i=0;i<2048;++i)scratch[i]=processor.scopeAfter[(end-2048+i)%LoveEQAudioProcessor::scopeSize].load()*w[i];
                std::fill(scratch.begin()+2048,scratch.end(),0.f);
                fft.performFrequencyOnlyForwardTransform(scratch.data());
                for(int i=0;i<=1024;++i)spectrum[(size_t)i]=juce::Decibels::gainToDecibels(scratch[(size_t)i]/512.f,-96.f);
                audioFresh=true;
            }
        }
        graph.repaint();for(auto& t:tiles)if(!t.getBounds().isEmpty())t.repaint();
    }
};
#endif
