#include "RelationshipEQ.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>

static int failed(const char* text){std::cerr<<text<<"\n";return 1;}
int main() {
    using namespace loveeq::relationship;
    if(nearestBand(1000.f)<4 || nearestBand(1000.f)>8)
        return failed("Logarithmic band mapping failed");
    if(!isProtected(200.f,100.f) || !isProtected(602.f,100.f))
        return failed("Known fundamental/harmonics not protected");
    if(isProtected(2694.f,200.f))
        return failed("Nonharmonic band wrongly protected");
    Meter meter;
    meter.prepare(48000.0);
    constexpr int samples=8192;
    for(int i=0;i<samples;++i)
        meter.push((float)(0.7*std::sin(6.283185307179586*440.0*i/48000.0)));
    const auto energy=meter.take();
    const float maximum=*std::max_element(energy.begin(),energy.end());
    if(maximum<0.01f)return failed("Meter did not measure audio");
    const auto zeros=meter.take();
    for(auto f:zeros) if(f!=0.f)return failed("Meter must reset at each block");

    auto& reg=registry();
    const int guitarId=reg.reserve(),voiceId=reg.reserve(),bassId=reg.reserve();
    if(guitarId<0||voiceId<0||bassId<0)return failed("Registry allocation failed");
    std::array<float,bandCount> own{},voice{},bass{};
    own[3]=0.11f;voice[3]=0.09f;
    own[9]=0.12f;voice[9]=0.09f;
    bass[9]=0.2f;
    reg.publish(guitarId,1,guitar,own);
    reg.publish(voiceId,1,Role::voice,voice);
    reg.publish(bassId,2,Role::bass,bass);
    const auto current=reg.snapshot(guitarId);
    if(!current.valid||std::abs(current.rms[9]-0.12f)>0.00001f)
        return failed("Own instance snapshot incorrect");
    const auto priority=reg.aggregate(1,Role::voice,guitarId);
    if(!priority.valid||priority.count!=1||std::abs(priority.rms[3]-0.09f)>0.00001f)
        return failed("Priority role not connected");
    if(reg.aggregate(2,Role::voice,guitarId).valid)
        return failed("Groups leaked across each other");
    if(reg.aggregate(1,Role::guitar,guitarId).valid)
        return failed("Instance may not prioritize itself");

    const auto suggestion=propose(current,priority,200.f);
    if(!suggestion.valid || suggestion.bin!=9 || std::abs(suggestion.hz-2694.f)>1.f)
        return failed("Harmonic guard did not exclude 377Hz partial");
    if(suggestion.protectedBands<1)return failed("No protected harmonic regions");
    if(dynamicCut(0.1f,0.09f,3.f)>=-1.f)
        return failed("Live conflict should generate a dynamic cut");
    if(dynamicCut(0.1f,0.f,3.f)!=0.f ||
       dynamicCut(0.f,0.1f,3.f)!=0.f)
        return failed("A silent instrument must not be attenuated");
    if(dynamicCut(0.1f,0.09f,3.f)<-3.0001f)
        return failed("Exceeded the opt-in maximum reduction");

    // Simulate a stopped voice: new blocks on the other instances expire it.
    for(int i=0;i<100;++i)reg.publish(guitarId,1,guitar,own);
    if(reg.aggregate(1,Role::voice,guitarId).valid)
        return failed("Stale priority activity was not discarded");
    reg.release(guitarId);reg.release(voiceId);reg.release(bassId);
    if(reg.snapshot(guitarId).valid)return failed("Released instances remain visible");
    std::cout<<"LOVE EQ Relationship spectral bus and harmonic guard tests passed\n";
    return 0;
}
