#pragma once
// Liberty LOVE EQ - Time Lens envelope segmentation (C++17, no JUCE).
// First-generation monophonic / mixed-bus event detector. Phases describe
// the AMPLITUDE ENVELOPE of the whole input; they are not per-MIDI-note states.
// Audio callback: fixed storage, no allocation, locks or file I/O.
#include <algorithm>
#include <array>
#include <cmath>

namespace loveeq::timelens {
enum class Phase : int { Silence=0, Attack=1, Body=2, Tail=3 };
struct Weights {
    std::array<float,3> stage {};
    Phase phase=Phase::Silence;
    float envelope=0.f;
};
class Detector {
public:
    void prepare(double sampleRate) noexcept {
        sampleRate=std::clamp(sampleRate,8000.0,384000.0);
        sr=sampleRate;
        fastRise=1.0-std::exp(-1.0/(sr*0.0015));
        fastFall=1.0-std::exp(-1.0/(sr*0.010));
        slowRise=1.0-std::exp(-1.0/(sr*0.025));
        slowFall=1.0-std::exp(-1.0/(sr*0.16));
        reset();
    }
    void configure(float attackMilliseconds,float tailMilliseconds,float thresholdDb) noexcept {
        attackCount=std::max(1,(int)(sr*0.001*std::clamp((double)attackMilliseconds,5.0,150.0)));
        tailCount=std::max(1,(int)(sr*0.001*std::clamp((double)tailMilliseconds,60.0,1200.0)));
        const auto clamped=std::clamp((double)thresholdDb,-65.0,-12.0);
        threshold=(float)std::pow(10.0,clamped/20.0);
    }
    void reset() noexcept {
        fast=slow=0.;
        phase=Phase::Silence;
        age=0;
        samplesSinceAttack=100000000;
    }
    Weights process(float input) noexcept {
        const double level=std::isfinite(input)?
            std::min(4.0,std::abs((double)input)):0.0;
        fast+=(level>fast?fastRise:fastFall)*(level-fast);
        slow+=(level>slow?slowRise:slowFall)*(level-slow);
        const bool clearlyLoud=fast>threshold*1.2;
        const bool newOnset=clearlyLoud &&
            (slow<0.001 || fast>slow*1.60) &&
            samplesSinceAttack>std::max(32,attackCount/2);
        if(samplesSinceAttack<100000000)++samplesSinceAttack;

        switch(phase) {
            case Phase::Silence:
                if(clearlyLoud || (newOnset && fast>threshold))startAttack();
                break;
            case Phase::Attack:
                if(++age>=attackCount) {
                    age=0;
                    phase=Phase::Body;
                } else if(fast<threshold*0.20 && age>attackCount/4) {
                    age=0;
                    phase=Phase::Tail;
                }
                break;
            case Phase::Body:
                if(newOnset)startAttack();
                else if(fast<std::max((double)threshold*0.78,slow*0.56) &&
                        slow>threshold*0.30) {
                    age=0;
                    phase=Phase::Tail;
                } else if(fast<threshold*0.18 && slow<threshold*0.20) {
                    phase=Phase::Silence;
                    age=0;
                }
                break;
            case Phase::Tail:
                if(newOnset)startAttack();
                else if(++age>=tailCount || (age>tailCount/3 && fast<threshold*0.10)) {
                    phase=Phase::Silence;
                    age=0;
                }
                break;
        }
        Weights result;
        result.phase=phase;
        result.envelope=(float)fast;
        if(phase==Phase::Attack)result.stage[0]=1.f;
        if(phase==Phase::Body)result.stage[1]=1.f;
        if(phase==Phase::Tail)
            result.stage[2]=std::clamp(1.f-(float)age/(float)tailCount,0.f,1.f);
        return result;
    }
    Phase currentPhase() const noexcept {return phase;}
private:
    void startAttack() noexcept {
        phase=Phase::Attack;
        age=0;
        samplesSinceAttack=0;
    }
    double sr=48000.0, fast=0.0,slow=0.0;
    double fastRise=0.01,fastFall=0.002,slowRise=0.001,slowFall=0.0002;
    float threshold=0.012f;
    int attackCount=1920,tailCount=14400,age=0,samplesSinceAttack=100000000;
    Phase phase=Phase::Silence;
};
} // namespace loveeq::timelens
