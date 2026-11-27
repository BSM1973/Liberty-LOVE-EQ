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
    p.add(choice("relationship_role","Relationship Track Role",
                 {"Unassigned","Voice","Bass","Guitar","Other"},0));
    p.add(choice("relationship_group","Relationship Group",
                 {"Off","Group 1","Group 2","Group 3","Group 4"},0));
    p.add(choice("relationship_priority","Relationship Priority",
                 {"Voice","Bass","Guitar","Other"},0));
    p.add(boolean("relationship_active","Relationship Dynamic Active",false));
    // TIME LENS: 3 independent bell filters, disabled until explicitly enabled.
    p.add(boolean("time_lens_enabled","Time Lens Active",false));
    p.add(boolean("intent_enabled","LOVE INTENT Preview Active",false));
    p.add(boolean("match_active","EQ MATCH Active",false));
    p.add(fl("match_amount","EQ MATCH Strength",0.f,100.f,75.f,1.f));
    for(int i=0;i<loveeq::match::bands;++i)
        p.add(fl("match_gain_"+juce::String(i+1),"EQ MATCH Gain "+juce::String(i+1),
            -6.f,6.f,0.f,0.1f));
    p.add(fl("intent_amount","LOVE INTENT Intensity",0.f,100.f,75.f,1.f));
    const std::array<float,3> startHz {245.f,2300.f,5100.f};
    for(int i=0;i<loveeq::intent::bandCount;++i) {
        const juce::String id="intent_"+juce::String(i+1)+"_";
        const juce::String name="LOVE INTENT Band "+juce::String(i+1)+" ";
        p.add(fl(id+"hz",name+"Frequency",30.f,18000.f,startHz[(size_t)i]));
        p.add(fl(id+"gain",name+"Gain",-6.f,6.f,0.f,0.1f));
        p.add(fl(id+"q",name+"Q",0.35f,8.f,1.f));
    }
    p.add(fl("time_lens_attack_ms","Time Lens Attack Duration",5.f,150.f,35.f,1.f));
    p.add(fl("time_lens_tail_ms","Time Lens Tail Duration",60.f,1200.f,400.f,1.f));
    p.add(fl("time_lens_threshold_db","Time Lens Trigger Threshold",-65.f,-12.f,-38.f,1.f));
    constexpr std::array<float,3> timeHz {3900.f,280.f,1100.f};
    constexpr std::array<float,3> timeGain {-3.0f,1.0f,0.6f};
    constexpr std::array<float,3> timeQ {2.1f,0.85f,1.1f};
    constexpr std::array<const char*,3> stageNames {"Attack","Body","Tail"};
    for(int j=0;j<timeStageCount;++j) {
        const auto label=juce::String("Time Lens ") + stageNames[(size_t)j];
        p.add(fl(timeStageId(j,"hz"),label+" Frequency",30.f,18000.f,timeHz[(size_t)j]));
        p.add(fl(timeStageId(j,"gain"),label+" Gain",-12.f,12.f,timeGain[(size_t)j],0.1f));
        p.add(fl(timeStageId(j,"q"),label+" Q",0.35f,8.f,timeQ[(size_t)j]));
    }
    p.add(fl("relationship_hz","Relationship Center Frequency",65.f,8000.f,1000.f));
    p.add(fl("relationship_q","Relationship Q",0.5f,6.f,1.4f));
    p.add(fl("relationship_max_cut","Relationship Maximum Reduction",0.f,6.f,3.f));
    p.add(fl("relationship_guard_hz","Relationship Protected Fundamental",0.f,1400.f,0.f));
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
    intentActiveParam=parameters.getRawParameterValue("intent_enabled");
    matchActiveParam=parameters.getRawParameterValue("match_active");
    matchAmountParam=parameters.getRawParameterValue("match_amount");
    for(int i=0;i<loveeq::match::bands;++i)
        matchGainParams[(size_t)i]=parameters.getRawParameterValue(
            "match_gain_"+juce::String(i+1));
    intentAmountParam=parameters.getRawParameterValue("intent_amount");
    for(int i=0;i<loveeq::intent::bandCount;++i){
        const auto id="intent_"+juce::String(i+1)+"_";
        auto& stage=intentParams[(size_t)i];
        stage.hz=parameters.getRawParameterValue(id+"hz");
        stage.gain=parameters.getRawParameterValue(id+"gain");
        stage.q=parameters.getRawParameterValue(id+"q");
    }
    timeEnabledParam=parameters.getRawParameterValue("time_lens_enabled");
    timeAttackParam=parameters.getRawParameterValue("time_lens_attack_ms");
    timeTailParam=parameters.getRawParameterValue("time_lens_tail_ms");
    timeThresholdParam=parameters.getRawParameterValue("time_lens_threshold_db");
    for(int i=0;i<timeStageCount;++i) {
        auto& stage=timeParams[(size_t)i];
        stage.hz=parameters.getRawParameterValue(timeStageId(i,"hz"));
        stage.gain=parameters.getRawParameterValue(timeStageId(i,"gain"));
        stage.q=parameters.getRawParameterValue(timeStageId(i,"q"));
    }
    relationshipRoleParam=parameters.getRawParameterValue("relationship_role");
    relationshipGroupParam=parameters.getRawParameterValue("relationship_group");
    relationshipPriorityParam=parameters.getRawParameterValue("relationship_priority");
    relationshipActiveParam=parameters.getRawParameterValue("relationship_active");
    relationshipHzParam=parameters.getRawParameterValue("relationship_hz");
    relationshipQParam=parameters.getRawParameterValue("relationship_q");
    relationshipMaxCutParam=parameters.getRawParameterValue("relationship_max_cut");
    relationshipGuardHzParam=parameters.getRawParameterValue("relationship_guard_hz");
    relationSlot=loveeq::relationship::registry().reserve();
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

LoveEQAudioProcessor::~LoveEQAudioProcessor() {
    loveeq::relationship::registry().release(relationSlot);
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
#if JUCE_IOS
    mobileAudioReady.store(false,std::memory_order_release);
    loveeq::relationship::registry().publish(relationSlot,0,0,{});
#endif
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
    relationMeter.prepare(rate);
    relationPreviousLevels.fill(0.f);
    relationshipGainDb=0.0;
    relationshipCoeffCountdown=0;
    relationshipAttackCoeff=std::exp(-1.0/(rate*0.035));
    relationshipReleaseCoeff=std::exp(-1.0/(rate*0.24));
    for(auto& lane:relationshipFilters)lane.reset();
    timeDetector.prepare(rate);
    timeDetector.configure(value(timeAttackParam),value(timeTailParam),value(timeThresholdParam));
    for(int i=0;i<timeStageCount;++i) {
        const auto& params=timeParams[(size_t)i];
        timeRuntime[(size_t)i].reset(rate,value(params.hz),value(params.gain),value(params.q));
    }
    for(int i=0;i<loveeq::intent::bandCount;++i){
        const auto& par=intentParams[(size_t)i];
        intentRuntime[(size_t)i].prepare(rate,value(par.hz),value(par.gain),value(par.q));
    }
    for(auto& b:matchRuntime)b.prepare(rate);
    matchWet.reset(rate,0.03);
    matchWet.setCurrentAndTargetValue(value(matchActiveParam)>0.5f?1.f:0.f);
    matchDose.reset(rate,0.03);
    matchDose.setCurrentAndTargetValue(juce::jlimit(0.f,1.f,value(matchAmountParam)*0.01f));
    intentWet.reset(rate,0.03);
    intentWet.setCurrentAndTargetValue(value(intentActiveParam)>0.5f?1.f:0.f);
    intentDose.reset(rate,0.03);
    intentDose.setCurrentAndTargetValue(juce::jlimit(0.f,1.f,value(intentAmountParam)*0.01f));
    timeWet.reset(rate,0.025);
    timeWet.setCurrentAndTargetValue(value(timeEnabledParam)>0.5f?1.f:0.f);
    timeLensPhase.store(0,std::memory_order_relaxed);
    timeLensEnvelope.store(0.f,std::memory_order_relaxed);
    setLatencySamples(0);
#if JUCE_IOS
    mobileAudioEpoch.fetch_add(1,std::memory_order_release);
    mobileAudioReady.store(true,std::memory_order_release);
#endif
}
void LoveEQAudioProcessor::releaseResources() {
#if JUCE_IOS
    mobileAudioReady.store(false,std::memory_order_release);
    mobileAudioEpoch.fetch_add(1,std::memory_order_release);
    loveeq::relationship::registry().publish(relationSlot,0,0,{});
    peakIn.store(0.f,std::memory_order_relaxed);
    peakOut.store(0.f,std::memory_order_relaxed);
#endif
}

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
    matchWet.setTargetValue(value(matchActiveParam)>0.5f?1.f:0.f);
    matchDose.setTargetValue(juce::jlimit(0.f,1.f,value(matchAmountParam)*0.01f));
    for(int i=0;i<loveeq::match::bands;++i)
        matchRuntime[(size_t)i].gain.setTargetValue(value(matchGainParams[(size_t)i]));
    intentWet.setTargetValue(value(intentActiveParam)>0.5f?1.f:0.f);
    intentDose.setTargetValue(juce::jlimit(0.f,1.f,value(intentAmountParam)*0.01f));
    for(int i=0;i<loveeq::intent::bandCount;++i) {
        const auto& par=intentParams[(size_t)i];
        auto& stage=intentRuntime[(size_t)i];
        stage.frequency.setTargetValue(value(par.hz));
        stage.gain.setTargetValue(value(par.gain));
        stage.q.setTargetValue(value(par.q));
    }
    timeDetector.configure(value(timeAttackParam),value(timeTailParam),
                           value(timeThresholdParam));
    timeWet.setTargetValue(value(timeEnabledParam)>0.5f?1.f:0.f);
    for(int i=0;i<timeStageCount;++i) {
        const auto& controls=timeParams[(size_t)i];
        auto& stage=timeRuntime[(size_t)i];
        stage.hz.setTargetValue(value(controls.hz));
        stage.gain.setTargetValue(value(controls.gain));
        stage.q.setTargetValue(value(controls.q));
    }
    const int relationGroup=(int)value(relationshipGroupParam);
    const int relationRole=(int)value(relationshipRoleParam);
    const int relationPriority=(int)value(relationshipPriorityParam)+1;
    const float relationHz=value(relationshipHzParam);
    const bool relationEnabled=value(relationshipActiveParam)>0.5f &&
        relationRole>0 && relationRole!=relationPriority && relationGroup>0 &&
        !loveeq::relationship::isProtected(relationHz,value(relationshipGuardHzParam));
    const auto priorityFrame=relationEnabled?
        loveeq::relationship::registry().aggregate(relationGroup,relationPriority,relationSlot):
        loveeq::relationship::Frame{};
    const int targetBin=loveeq::relationship::nearestBand(relationHz);
    const float targetDb=priorityFrame.valid?
        loveeq::relationship::dynamicCut(
            relationPreviousLevels[(size_t)targetBin],
            priorityFrame.rms[(size_t)targetBin],
            value(relationshipMaxCutParam)):0.f;
    const float relationQ=juce::jlimit(0.5f,6.f,value(relationshipQParam));
    float inPeak=0.f, outPeak=0.f;
    float lastEnvelope=0.f;
    for (int sample=0;sample<n;++sample) {
        const float origL=left[sample], origR=right?right[sample]:origL;
        inPeak=std::max({inPeak,std::abs(origL),std::abs(origR)});
        const float mono=0.5f*(origL+origR);
        relationMeter.push(mono);
        // Do not cancel stereo content when the two channels are antiphase.
        const auto timePhase=timeDetector.process(std::max(std::abs(origL),std::abs(origR)));
        lastEnvelope=timePhase.envelope;
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
        const double coeff=targetDb<relationshipGainDb?
            relationshipAttackCoeff:relationshipReleaseCoeff;
        relationshipGainDb=coeff*relationshipGainDb+(1.0-coeff)*targetDb;
        if(--relationshipCoeffCountdown<=0) {
            const auto c=loveeq::design(loveeq::Type::Bell,rate,
                juce::jlimit(65.f,8000.f,relationHz),relationshipGainDb,relationQ);
            for(auto& lane:relationshipFilters)lane.set(c);
            relationshipCoeffCountdown=32;
        }
        l=relationshipFilters[0].process(l);
        if(right)r=relationshipFilters[1].process(r);
        // Three parallel Bell filters maintain continuous histories.
        // Weighted differences give seamless Attack/Body/Tail crossfades.
        const float dryTimeL=l,dryTimeR=r;
        float correctionL=0.f,correctionR=0.f;
        const float stageMix=timeWet.getNextValue();
        std::array<float,timeStageCount> timeWeights {};
        float totalWeight=0.f;
        for(int ti=0;ti<timeStageCount;++ti) {
            auto& stage=timeRuntime[(size_t)ti];
            stage.weight.setTargetValue(timePhase.stage[(size_t)ti]);
            timeWeights[(size_t)ti]=stage.weight.getNextValue();
            totalWeight+=timeWeights[(size_t)ti];
        }
        const float weightScale=stageMix/std::max(1.f,totalWeight);
        for(int ti=0;ti<timeStageCount;++ti) {
            auto& stage=timeRuntime[(size_t)ti];
            const float weight=timeWeights[(size_t)ti]*weightScale;
            const double hz=stage.hz.getNextValue();
            const double gainDb=stage.gain.getNextValue();
            const double qValue=stage.q.getNextValue();
            if(--stage.coeffCountdown<=0) {
                const auto coeff=loveeq::design(loveeq::Type::Bell,rate,hz,gainDb,qValue);
                for(auto& c:stage.channels)c.set(coeff);
                stage.coeffCountdown=32;
            }
            const float processedL=stage.channels[0].process(dryTimeL);
            correctionL+=weight*(processedL-dryTimeL);
            if(right) {
                const float processedR=stage.channels[1].process(dryTimeR);
                correctionR+=weight*(processedR-dryTimeR);
            }
        }
        l=dryTimeL+correctionL;
        if(right)r=dryTimeR+correctionR;
        // LOVE INTENT additive independent Bell bank. No existing EQ nodes
        // are overwritten, and its own A/B is smoothly faded.
        const float intentMix=intentWet.getNextValue()*intentDose.getNextValue();
        const float intentDryL=l,intentDryR=r;
        float intentDeltaL=0.f,intentDeltaR=0.f;
        for(int i=0;i<loveeq::intent::bandCount;++i) {
            auto& band=intentRuntime[(size_t)i];
            const double frequency=band.frequency.getNextValue();
            const double db=band.gain.getNextValue();
            const double width=band.q.getNextValue();
            if(--band.countdown<=0) {
                const auto coefficients=loveeq::design(
                    loveeq::Type::Bell,rate,frequency,db,width);
                for(auto& c:band.channels)c.set(coefficients);
                band.countdown=32;
            }
            intentDeltaL+=band.channels[0].process(intentDryL)-intentDryL;
            if(right)intentDeltaR+=band.channels[1].process(intentDryR)-intentDryR;
        }
        l=intentDryL+intentMix*intentDeltaL;
        if(right)r=intentDryR+intentMix*intentDeltaR;
        // EQ MATCH is a real, automatable 6-filter correction with
        // bounded gain and smoothed A/B. The original EQ remains intact.
        const float matchMix=matchWet.getNextValue()*matchDose.getNextValue();
        const float matchDryL=l,matchDryR=r;
        float matchDeltaL=0.f,matchDeltaR=0.f;
        for(int mi=0;mi<loveeq::match::bands;++mi) {
            auto& band=matchRuntime[(size_t)mi];
            const double db=band.gain.getNextValue();
            if(--band.countdown<=0) {
                const auto coeff=loveeq::design(
                    loveeq::Type::Bell,rate,loveeq::match::centers[(size_t)mi],db,0.85);
                for(auto& c:band.channels)c.set(coeff);
                band.countdown=32;
            }
            matchDeltaL+=band.channels[0].process(matchDryL)-matchDryL;
            if(right)matchDeltaR+=band.channels[1].process(matchDryR)-matchDryR;
        }
        l=matchDryL+matchMix*matchDeltaL;
        if(right)r=matchDryR+matchMix*matchDeltaR;
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
    timeLensPhase.store((int)timeDetector.currentPhase(),std::memory_order_relaxed);
    timeLensEnvelope.store(lastEnvelope,std::memory_order_relaxed);
    relationPreviousLevels=relationMeter.take();
    loveeq::relationship::registry().publish(relationSlot,relationGroup,relationRole,relationPreviousLevels);
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
    set("relationship_active",0.f);
    set("time_lens_enabled",0.f);
    set("intent_enabled",0.f);
    set("match_active",0.f);
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
    set("relationship_active",0.f);
    set("time_lens_enabled",0.f);
    set("intent_enabled",0.f);
    set("match_active",0.f);
    set("match_amount",75.f);
    for(int i=0;i<loveeq::match::bands;++i)
        set("match_gain_"+juce::String(i+1),0.f);
    set("intent_amount",75.f);
    for(int i=0;i<loveeq::intent::bandCount;++i) {
        const auto id="intent_"+juce::String(i+1)+"_";
        set(id+"gain",0.f);
        set(id+"q",1.f);
    }
    set("time_lens_attack_ms",35.f);
    set("time_lens_tail_ms",400.f);
    set("time_lens_threshold_db",-38.f);
    for(int i=0;i<timeStageCount;++i) {
        set(timeStageId(i,"hz"),std::array<float,3>{3900.f,280.f,1100.f}[(size_t)i]);
        set(timeStageId(i,"gain"),0.f);
        set(timeStageId(i,"q"),std::array<float,3>{2.1f,0.85f,1.1f}[(size_t)i]);
    }
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
juce::AudioProcessorEditor* LoveEQAudioProcessor::createEditor() {
#if JUCE_IOS
    return new LoveEQMobileEditor(*this);
#else
    return new LoveEQAudioProcessorEditor(*this);
#endif
}
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
