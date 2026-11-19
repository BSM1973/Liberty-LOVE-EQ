#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "FactoryPresets.h"

namespace {
using APVTS=juce::AudioProcessorValueTreeState;
std::unique_ptr<juce::RangedAudioParameter> fl(const juce::String& id,const juce::String& name,float low,float high,float def,float step=0.01f) {
    return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>{low,high,step},def);
}
std::unique_ptr<juce::RangedAudioParameter> boolean(const juce::String& id,const juce::String& name,bool def) {
    return std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id,1},name,def);
}
std::unique_ptr<juce::RangedAudioParameter> choice(const juce::String& id,const juce::String& name,const juce::StringArray& opts,int def) {
    return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id,1},name,opts,def);
}
int passBiquads(int slope) {
    const int counts[6]={0,1,1,2,3,4};
    return counts[juce::jlimit(0,5,slope)];
}
}

juce::String LoveEQAudioProcessor::bandId(int band,const juce::String& suffix) {
    return "band" + juce::String(band+1) + "_" + suffix;
}

APVTS::ParameterLayout LoveEQAudioProcessor::makeParameterLayout() {
    APVTS::ParameterLayout p;
    p.add(fl("input_gain","Input Gain",-24.f,24.f,0.f));
    p.add(fl("output_gain","Output Gain",-24.f,24.f,0.f));
    p.add(boolean("bypass","Bypass",false));
    p.add(boolean("external_sc","External Sidechain",false));
    p.add(choice("eq_mode","EQ Mode",{"Parametric","Graphic"},0));
    for(int band=0;band<graphicBandCount;++band)
        p.add(fl(graphicId(band),"Graphic Gain "+juce::String(band+1),-18.f,18.f,0.f,0.1f));
    const juce::StringArray types {"Bell","Low Shelf","High Shelf","High Pass","Low Pass","Notch","Band Pass"};
    const juce::StringArray channels {"Stereo","Left","Right","Mid","Side"};
    const juce::StringArray slopes {"6 dB/oct","12 dB/oct","18 dB/oct","24 dB/oct","36 dB/oct","48 dB/oct"};
    for (int i=0;i<bandCount;++i) {
        const auto id=[i](const juce::String& s){return bandId(i,s);};
        juce::NormalisableRange<float> freq {20.0f,20000.0f,0.f};
        freq.setSkewForCentre(1000.f);
        p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id("hz"),1},"Band " + juce::String(i+1) + " Frequency",freq,63.f*std::pow(2.f,(float)i)));
        p.add(fl(id("gain"),"Band Gain",-24.f,24.f,0.f));
        p.add(fl(id("q"),"Band Q",0.15f,18.f,1.f));
        p.add(choice(id("type"),"Filter Type",types,0));
        p.add(choice(id("channel"),"Channel",channels,0));
        p.add(choice(id("slope"),"Slope",slopes,1));
        p.add(boolean(id("enabled"),"Band Enabled",true));
        p.add(boolean(id("solo"),"Band Solo",false));
        p.add(boolean(id("dynamic"),"Dynamic Enabled",false));
        p.add(fl(id("threshold"),"Dynamic Threshold",-72.f,0.f,-24.f));
        p.add(fl(id("range"),"Dynamic Range",-18.f,18.f,-4.f));
        p.add(fl(id("attack"),"Dynamic Attack",1.f,250.f,10.f));
        p.add(fl(id("release"),"Dynamic Release",10.f,2000.f,160.f));
    }
    return p;
}

LoveEQAudioProcessor::LoveEQAudioProcessor()
 : AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
                                   .withOutput("Output",juce::AudioChannelSet::stereo(),true)
                                   .withInput("Sidechain",juce::AudioChannelSet::stereo(),false)),
   parameters(*this,&undoManager,"LOVE_EQ_STATE",makeParameterLayout()) {
    inputParam=parameters.getRawParameterValue("input_gain");
    outputParam=parameters.getRawParameterValue("output_gain");
    bypassParam=parameters.getRawParameterValue("bypass");
    sidechainParam=parameters.getRawParameterValue("external_sc");
    eqModeParam=parameters.getRawParameterValue("eq_mode");
    for(int i=0;i<graphicBandCount;++i) graphicParams[(size_t)i]=parameters.getRawParameterValue(graphicId(i));
    for (int i=0;i<bandCount;++i) {
        auto& p=bandParameters[(size_t)i];
        const auto id=[i](const juce::String& s){return bandId(i,s);};
        p.hz=parameters.getRawParameterValue(id("hz"));
        p.gain=parameters.getRawParameterValue(id("gain"));
        p.q=parameters.getRawParameterValue(id("q"));
        p.type=parameters.getRawParameterValue(id("type"));
        p.channel=parameters.getRawParameterValue(id("channel"));
        p.slope=parameters.getRawParameterValue(id("slope"));
        p.enabled=parameters.getRawParameterValue(id("enabled"));
        p.solo=parameters.getRawParameterValue(id("solo"));
        p.dynamic=parameters.getRawParameterValue(id("dynamic"));
        p.threshold=parameters.getRawParameterValue(id("threshold"));
        p.range=parameters.getRawParameterValue(id("range"));
        p.attack=parameters.getRawParameterValue(id("attack"));
        p.release=parameters.getRawParameterValue(id("release"));
    }
}

bool LoveEQAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
    const auto mainIn=l.getMainInputChannelSet(), mainOut=l.getMainOutputChannelSet();
    if (mainIn!=mainOut || (mainOut!=juce::AudioChannelSet::mono() && mainOut!=juce::AudioChannelSet::stereo())) return false;
    if (getBusCount(true)>1) {
        const auto side=l.getChannelSet(true,1);
        if (!side.isDisabled() && side!=juce::AudioChannelSet::mono() && side!=juce::AudioChannelSet::stereo()) return false;
    }
    return true;
}

void LoveEQAudioProcessor::refreshSettings() {
    for (int i=0;i<bandCount;++i) {
        auto& b=settings[(size_t)i];
        const auto& p=bandParameters[(size_t)i];
        b.hz=value(p.hz); b.gain=value(p.gain); b.q=value(p.q);
        b.type=(int)value(p.type); b.channel=(int)value(p.channel); b.slope=(int)value(p.slope);
        b.enabled=value(p.enabled)>0.5f; b.solo=value(p.solo)>0.5f;
        b.dynamic=value(p.dynamic)>0.5f;
        b.threshold=value(p.threshold); b.range=value(p.range);
        b.attack=value(p.attack); b.release=value(p.release);
        b.attackCoefficient=std::exp(-1.0/(rate*std::max(0.001,0.001*(double)b.attack)));
        b.releaseCoefficient=std::exp(-1.0/(rate*std::max(0.001,0.001*(double)b.release)));
    }
}

void LoveEQAudioProcessor::BandRuntime::reset(double sr,const BandSettings& s) {
    for (auto& lane: filters) for (auto& f:lane) f.reset();
    for (auto& f:detectors) f.reset();
    envelope={0,0}; controlCountdown=0;
    smoothedHz.reset(sr,0.018); smoothedHz.setCurrentAndTargetValue(s.hz);
    smoothedGain.reset(sr,0.018); smoothedGain.setCurrentAndTargetValue(s.gain);
    smoothedQ.reset(sr,0.018); smoothedQ.setCurrentAndTargetValue(s.q);
    wet.reset(sr,0.018); wet.setCurrentAndTargetValue(s.enabled?1.f:0.f);
}

void LoveEQAudioProcessor::BandRuntime::updateCoefficients(double sr,const BandSettings& s) {
    const auto t=static_cast<loveeq::Type>(juce::jlimit(0,6,s.type));
    const bool pass=(t==loveeq::Type::LowPass || t==loveeq::Type::HighPass);
    const bool onePole=pass && loveeq::needsFirstOrder(s.slope);
    const int count=pass?passBiquads(s.slope):1;
    const auto detector=loveeq::design(loveeq::Type::BandPass,sr,smoothedHz.getCurrentValue(),0.0,smoothedQ.getCurrentValue());
    for (int ch=0;ch<2;++ch) {
        detectors[(size_t)ch].set(detector);
        const double level=20.0*std::log10(std::max(1e-7,envelope[(size_t)ch]));
        const double dynamicDelta=s.dynamic ? s.range*juce::jlimit(0.0,1.0,(level-s.threshold)/12.0) : 0.0;
        const auto fixed=loveeq::design(t,sr,smoothedHz.getCurrentValue(),smoothedGain.getCurrentValue()+dynamicDelta,smoothedQ.getCurrentValue());
        const auto first=loveeq::design(t,sr,smoothedHz.getCurrentValue(),0.0,smoothedQ.getCurrentValue(),true);
        for (int stage=0;stage<5;++stage) {
            loveeq::Coefficients c{};
            if (pass) {
                if (onePole && stage==0) c=first;
                else if (stage>=1 && stage<=count) c=fixed;
            } else if (stage==0) c=fixed;
            filters[(size_t)ch][(size_t)stage].set(c);
        }
    }
}

float LoveEQAudioProcessor::BandRuntime::filtered(int channel,float input) noexcept {
    for (auto& f:filters[(size_t)channel]) input=f.process(input);
    return input;
}

void LoveEQAudioProcessor::prepareToPlay(double sampleRate,int) {
    rate=sampleRate;
    refreshSettings();
    for (int i=0;i<bandCount;++i) runtime[(size_t)i].reset(rate,settings[(size_t)i]);
    globalWet.reset(rate,0.018); globalWet.setCurrentAndTargetValue(value(bypassParam)>0.5f?0.f:1.f);
    inputGain.reset(rate,0.018); inputGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(value(inputParam)));
    outputGain.reset(rate,0.018); outputGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(value(outputParam)));
    for(int i=0;i<graphicBandCount;++i) {
        auto& gr=graphicRuntime[(size_t)i];
        gr.reset(rate);
        gr.gain.setCurrentAndTargetValue(value(graphicParams[(size_t)i]));
    }
    graphicBlend.reset(rate,0.030);
    graphicBlend.setCurrentAndTargetValue(value(eqModeParam)>0.5f?1.f:0.f);
    peakIn.store(0.f); peakOut.store(0.f);
    resetPeakHold();
    scopeIndex.store(0);
    for(auto& a:scopeBefore) a.store(0.f);
    for(auto& a:scopeAfter) a.store(0.f);
    setLatencySamples(0);
}
void LoveEQAudioProcessor::releaseResources() {}

void LoveEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;
    const int channels=juce::jmin(2,getMainBusNumOutputChannels());
    if (channels<=0) return;
    const int n=buffer.getNumSamples();
    refreshSettings();
    bool anySolo=false;
    for (const auto& s:settings) anySolo=anySolo||s.solo;
    const bool sc=value(sidechainParam)>0.5f && getBusCount(true)>1 && getBus(true,1)->isEnabled();
    const int mainInputChannels=getMainBusNumInputChannels();
    const float* sideL=(sc && buffer.getNumChannels()>mainInputChannels)?buffer.getReadPointer(mainInputChannels):nullptr;
    const float* sideR=(sc && buffer.getNumChannels()>mainInputChannels+1)?buffer.getReadPointer(mainInputChannels+1):sideL;
    auto* left=buffer.getWritePointer(0);
    auto* right=(channels==2)?buffer.getWritePointer(1):nullptr;
    inputGain.setTargetValue(juce::Decibels::decibelsToGain(value(inputParam)));
    outputGain.setTargetValue(juce::Decibels::decibelsToGain(value(outputParam)));
    globalWet.setTargetValue(value(bypassParam)>0.5f?0.f:1.f);
    graphicBlend.setTargetValue(value(eqModeParam)>0.5f?1.f:0.f);
    for(int i=0;i<graphicBandCount;++i)
        graphicRuntime[(size_t)i].gain.setTargetValue(value(graphicParams[(size_t)i]));
    for (int i=0;i<bandCount;++i) {
        const auto& s=settings[(size_t)i]; auto& rt=runtime[(size_t)i];
        rt.smoothedHz.setTargetValue(s.hz);
        rt.smoothedGain.setTargetValue(s.gain);
        rt.smoothedQ.setTargetValue(s.q);
        rt.wet.setTargetValue((s.enabled && (!anySolo || s.solo))?1.f:0.f);
    }
    float inPeak=0.f, outPeak=0.f;
    for (int sample=0;sample<n;++sample) {
        const float origL=left[sample], origR=right?right[sample]:origL;
        inPeak=std::max({inPeak,std::abs(origL),std::abs(origR)});
        float l=origL*inputGain.getNextValue();
        float r=right?origR*inputGain.getCurrentValue():l;
        // Both engines run while crossfading so filter histories remain warm.
        float graphicL=l, graphicR=r;
        for(int gi=0;gi<graphicBandCount;++gi) {
            auto& gr=graphicRuntime[(size_t)gi];
            const float db=gr.gain.getNextValue();
            if(--gr.countdown<=0) {
                const auto coeff=loveeq::design(loveeq::Type::Bell,rate,graphicHz[(size_t)gi],db,1.8);
                for(auto& lane:gr.channels) lane.set(coeff);
                gr.countdown=32;
            }
            graphicL=gr.channels[0].process(graphicL);
            if(right) graphicR=gr.channels[1].process(graphicR);
            else graphicR=graphicL;
        }
        for(int band=0;band<bandCount;++band) {
            auto& rt=runtime[(size_t)band]; const auto& s=settings[(size_t)band];
            rt.smoothedHz.getNextValue(); rt.smoothedGain.getNextValue(); rt.smoothedQ.getNextValue();
            if (--rt.controlCountdown<=0) { rt.updateCoefficients(rate,s); rt.controlCountdown=32; }
            const float dl=sideL?sideL[sample]:l, dr=sideR?sideR[sample]:r;
                        for(int ch=0;ch<(right?2:1);++ch) {
                const double detected=std::abs(rt.detectors[(size_t)ch].process(ch==0?dl:dr));
                auto& env=rt.envelope[(size_t)ch];
                env=(detected>env?s.attackCoefficient:s.releaseCoefficient)*env+(1.0-(detected>env?s.attackCoefficient:s.releaseCoefficient))*detected;
            }
            const float wet=rt.wet.getNextValue();
            if (!right) {
                const float processed=rt.filtered(0,l);
                l+=wet*(processed-l);
            } else if(s.channel==3 || s.channel==4) {
                constexpr float k=0.7071067811865475f;
                const float mid=(l+r)*k, side=(l-r)*k;
                const float pm=s.channel==3?rt.filtered(0,mid):mid;
                const float ps=s.channel==4?rt.filtered(1,side):side;
                const float pl=(pm+ps)*k, pr=(pm-ps)*k;
                l+=wet*(pl-l); r+=wet*(pr-r);
            } else {
                const float pl=rt.filtered(0,l), pr=rt.filtered(1,r);
                if(s.channel!=2) l+=wet*(pl-l);
                if(s.channel!=1) r+=wet*(pr-r);
            }
        }
        const float graphicAmount=graphicBlend.getNextValue();
        l+=graphicAmount*(graphicL-l);
        if(right) r+=graphicAmount*(graphicR-r);
        const float outGainNow=outputGain.getNextValue();
        const float mix=globalWet.getNextValue();
        const float finalL=origL+mix*(l*outGainNow-origL);
        const float finalR=right?origR+mix*(r*outGainNow-origR):finalL;
        left[sample]=std::isfinite(finalL)?finalL:0.f;
        if(right) right[sample]=std::isfinite(finalR)?finalR:0.f;
        outPeak=std::max({outPeak,std::abs(finalL),std::abs(finalR)});
        const auto wi=scopeIndex.load(std::memory_order_relaxed);
        scopeBefore[(size_t)(wi%scopeSize)].store((origL+origR)*0.5f,std::memory_order_relaxed);
        scopeAfter[(size_t)(wi%scopeSize)].store((finalL+finalR)*0.5f,std::memory_order_relaxed);
        scopeIndex.store(wi+1,std::memory_order_release);
    }
    peakIn.store(inPeak,std::memory_order_relaxed);
    peakOut.store(outPeak,std::memory_order_relaxed);
    auto updatePeak=[](std::atomic<float>& hold,float peak) noexcept {
        float previous=hold.load(std::memory_order_relaxed);
        while(previous<peak && !hold.compare_exchange_weak(previous,peak,std::memory_order_release,std::memory_order_relaxed)) {}
    };
    updatePeak(peakHoldIn,inPeak);
    updatePeak(peakHoldOut,outPeak);
}

void LoveEQAudioProcessor::getStateInformation(juce::MemoryBlock& out) {
    auto state=parameters.copyState();
    if (auto xml=state.createXml()) copyXmlToBinary(*xml,out);
}
void LoveEQAudioProcessor::setStateInformation(const void* data,int size) {
    if (auto xml=getXmlFromBinary(data,size)) {
        auto state=juce::ValueTree::fromXml(*xml);
        if (state.isValid() && state.hasType(parameters.state.getType())) parameters.replaceState(state);
    }
}
void LoveEQAudioProcessor::loadFactoryPreset(size_t index) {
    if(index>=loveeq::factoryPresets().size()) return;
    const auto data=loveeq::makePreset(index);
    auto set=[this](const juce::String& id,float value) {
        if(auto* p=parameters.getParameter(id)) p->setValueNotifyingHost(p->convertTo0to1(value));
    };
    undoManager.beginNewTransaction("Factory preset");
    set("input_gain",0); set("output_gain",0); set("bypass",0); set("external_sc",0);
    set("eq_mode",0.f);
    for(int i=0;i<bandCount;++i) {
        const auto& b=data[(size_t)i];
        const auto id=[i](const juce::String& s){return bandId(i,s);};
        set(id("hz"),b.hz); set(id("gain"),b.gain); set(id("q"),b.q);
        set(id("type"),(float)b.type); set(id("channel"),(float)b.channel); set(id("slope"),(float)b.slope);
        set(id("enabled"),b.enabled?1.f:0.f); set(id("solo"),0.f);
        set(id("dynamic"),b.dynamic?1.f:0.f); set(id("threshold"),b.threshold);
        set(id("range"),b.range); set(id("attack"),10.f); set(id("release"),160.f);
    }
    undoManager.beginNewTransaction();
}
void LoveEQAudioProcessor::resetToFlat() {
    auto set=[this](const juce::String& id,float value) {
        if(auto* p=parameters.getParameter(id)) p->setValueNotifyingHost(p->convertTo0to1(value));
    };
    undoManager.beginNewTransaction("Reset flat");
    set("input_gain",0.f);set("output_gain",0.f);set("bypass",0.f);set("external_sc",0.f);
    set("eq_mode",0.f);
    for(int i=0;i<graphicBandCount;++i) set(graphicId(i),0.f);
    for(int i=0;i<bandCount;++i) {
        const auto id=[i](const juce::String& s){return bandId(i,s);};
        set(id("hz"),63.f*std::pow(2.f,(float)i));
        set(id("gain"),0.f);set(id("q"),1.f);set(id("type"),0.f);
        set(id("channel"),0.f);set(id("slope"),1.f);
        set(id("enabled"),1.f);set(id("solo"),0.f);set(id("dynamic"),0.f);
        set(id("threshold"),-24.f);set(id("range"),-4.f);
        set(id("attack"),10.f);set(id("release"),160.f);
    }
    undoManager.beginNewTransaction();
}
juce::AudioProcessorEditor* LoveEQAudioProcessor::createEditor() { return new LoveEQAudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new LoveEQAudioProcessor(); }

bool LoveEQAudioProcessor::isFavorite(size_t index) const {
    if(index>=100) return false;
    const auto stored=parameters.state.getProperty("factoryFavorites").toString();
    return (int)index<stored.length() && stored[(int)index]=='1';
}
void LoveEQAudioProcessor::setFavorite(size_t index,bool favorite) {
    if(index>=100) return;
    auto stored=parameters.state.getProperty("factoryFavorites").toString().paddedRight('0',100);
    stored=stored.replaceSection((int)index,1,favorite?"1":"0");
    parameters.state.setProperty("factoryFavorites",stored,nullptr);
}
juce::String LoveEQAudioProcessor::getUiLanguage() const {
    return parameters.state.getProperty("uiLanguage","fr").toString()=="en"?"en":"fr";
}
void LoveEQAudioProcessor::setUiLanguage(const juce::String& language) {
    parameters.state.setProperty("uiLanguage",language=="en"?"en":"fr",nullptr);
}
