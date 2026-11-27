#include "PluginProcessor.h"
#include <iostream>
#include <chrono>
#include <cmath>
#include <memory>
#include <vector>
static bool set(LoveEQAudioProcessor& p,const juce::String& id,float value){
    auto* param=p.parameters.getParameter(id);
    if(!param)return false;
    param->setValueNotifyingHost(param->convertTo0to1(value));
    return true;
}
int main(){
    juce::ScopedJuceInitialiser_GUI init;
    for(double sr:{44100.,48000.,96000.,192000.})for(int n:{64,128,512})for(int count:{1,4,8}){
        std::vector<std::unique_ptr<LoveEQAudioProcessor>> ps;
        for(int i=0;i<count;++i){
            auto p=std::make_unique<LoveEQAudioProcessor>();
            if(!set(*p,"band1_hz",1000.f)||!set(*p,"band1_gain",6.f)||
               !set(*p,"band1_channel",3.f)||!set(*p,"relationship_role",i==0?1.f:3.f)||
               !set(*p,"relationship_group",1.f))return 1;
            if(i==1){
                p->getBus(true,1)->enable(true);
                if(!p->getBus(true,1)->isEnabled())return 2;
                if(!set(*p,"external_sc",1.f)||!set(*p,"band1_dynamic",1.f))return 3;
            }
            p->prepareToPlay(sr,n);
            ps.push_back(std::move(p));
        }
        juce::AudioBuffer<float> buf(4,n);
        juce::MidiBuffer midi;
        double checksum=0.,elapsed=0.;
        const int blocks=std::max(8,(int)(sr*0.08/n));
        for(int b=0;b<blocks;++b)for(int i=0;i<count;++i){
            auto& p=*ps[(size_t)i];
            if(!set(p,"band1_gain",b<blocks/2?6.f:-3.f))return 4;
            for(int j=0;j<n;++j){
                double t=(b*n+j)/sr;
                float v=(float)(0.1*std::sin(6.283185307179586*1000*t));
                buf.setSample(0,j,v);buf.setSample(1,j,i%2?-v:v);
                buf.setSample(2,j,v*2.f);buf.setSample(3,j,v*2.f);
            }
            const auto start=std::chrono::steady_clock::now();
            p.processBlock(buf,midi);
            elapsed+=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
            for(int ch=0;ch<2;++ch)for(int j=0;j<n;++j){
                float v=buf.getSample(ch,j);
                if(!std::isfinite(v)||std::abs(v)>20.f)return 5;
                checksum+=v*v;
            }
        }
        if(count>1&&!ps[1]->relationshipPriorityFrame().valid)return 6;
        juce::MemoryBlock state;
        ps[0]->getStateInformation(state);
        LoveEQAudioProcessor restored;
        restored.setStateInformation(state.getData(),(int)state.getSize());
        if(std::abs(restored.parameters.getRawParameterValue("band1_gain")->load()+3.f)>0.02f)return 7;
        std::cout<<"sr="<<sr<<" block="<<n<<" instances="<<count<<" cpu_ms="<<elapsed*1000.
                 <<" energy="<<checksum<<" state=PASS\n";
    }
    // Native host-lifecycle simulation, not a claim of real iOS host validation.
    LoveEQAudioProcessor original, restored;
    original.loadFactoryPreset(42);
    set(original,"band1_gain",5.25f);
    set(original,"band1_channel",4.f);
    juce::MemoryBlock saved;
    original.getStateInformation(saved);
    restored.setStateInformation(saved.getData(),(int)saved.getSize());
    auto& a=original.getParameters();auto& b=restored.getParameters();
    if(a.size()!=b.size())return 8;
    for(int i=0;i<a.size();++i)if(std::abs(a[i]->getValue()-b[i]->getValue())>1.e-6f)return 9;
    for(double sr:{48000.,44100.,96000.,48000.}){
        constexpr int n=128;
        original.releaseResources();restored.releaseResources();
        original.setRateAndBufferSizeDetails(sr,n);restored.setRateAndBufferSizeDetails(sr,n);
        original.prepareToPlay(sr,n);restored.prepareToPlay(sr,n);
        juce::AudioBuffer<float> x(2,n),y(2,n);
        juce::MidiBuffer midi;
        double maxError=0.;
        for(int block=0;block<256;++block){
            for(int j=0;j<n;++j){
                float v=(float)(0.15*std::sin(6.283185307179586*440*(block*n+j)/sr));
                x.setSample(0,j,v);x.setSample(1,j,-v);
            }
            y.makeCopyOf(x);
            original.processBlock(x,midi);restored.processBlock(y,midi);
            for(int ch=0;ch<2;++ch)for(int j=0;j<n;++j){
                if(!std::isfinite(x.getSample(ch,j))||!std::isfinite(y.getSample(ch,j)))return 10;
                maxError=std::max(maxError,(double)std::abs(x.getSample(ch,j)-y.getSample(ch,j)));
            }
        }
        if(maxError>1.e-6)return 11;
        std::cout<<"Lifecycle sample_rate="<<sr<<" restored_max_error="<<maxError<<" PASS\n";
    }
    original.releaseResources();restored.releaseResources();
    return 0;
}
