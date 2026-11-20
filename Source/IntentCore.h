#pragma once
// LOVE INTENT: transparent deterministic suggestions, not AI or perceptual truth.
// No JUCE dependency, no FFT or memory allocation on the audio callback.
#include <algorithm>
#include <array>
#include <cmath>
namespace loveeq::intent {
constexpr int bandCount=3;
enum class Goal { Clarity=0, VocalWarmth=1, Softness=2, BassDefinition=3 };
struct Band {float hz=1000.f,gain=0.f,q=1.f;};
struct Spectrum {float low=-90.f,body=-90.f,presence=-90.f,harshness=-90.f,air=-90.f;bool valid=false;};
struct Suggestion {
    std::array<Band,bandCount> bands{};
    float expectedMaxChangeDb=0.f;
    int goal=0,variant=0;
    bool valid=false;
};
inline float averageDb(const float* values,int count,double sr,int fft,int startHz,int endHz) noexcept {
    if(!values||count<3||sr<8000.0||fft<256)return -90.f;
    const int first=std::max(1,(int)std::ceil(startHz*fft/sr));
    const int last=std::min(count-2,(int)std::floor(endHz*fft/sr));
    if(last<first)return -90.f;
    double sum=0;int n=0;
    for(int i=first;i<=last;++i) {
        const float db=values[i];
        if(!std::isfinite(db))continue;
        const float a=std::pow(10.f,std::clamp(db,-100.f,18.f)/20.f);
        sum+=a*a;
        ++n;
    }
    if(n<1)return -90.f;
    return (float)(10.0*std::log10(std::max(1e-10,sum/n)));
}
inline Spectrum inspect(const float* values,int count,double sr,int fft) noexcept {
    Spectrum s;
    if(!values||count<256||sr<8000.0||fft<512)return s;
    s.low=averageDb(values,count,sr,fft,55,170);
    s.body=averageDb(values,count,sr,fft,170,700);
    s.presence=averageDb(values,count,sr,fft,1400,3800);
    s.harshness=averageDb(values,count,sr,fft,3800,7600);
    s.air=averageDb(values,count,sr,fft,7600,14000);
    const float strongest=std::max({s.low,s.body,s.presence,s.harshness,s.air});
    s.valid=strongest>-72.f;
    return s;
}
inline Suggestion propose(Goal goal,int variant,const Spectrum& s) noexcept {
    Suggestion result;
    result.goal=(int)goal;
    result.variant=std::clamp(variant,0,2);
    if(!s.valid)return result;
    const float strength=std::array<float,3>{0.45f,0.72f,1.f}[(size_t)result.variant];
    // Estimate broad balance only; not a claim of instrument recognition.
    const float excessHarsh=std::clamp((s.harshness-s.presence+4.f)/10.f,0.f,1.f);
    const float excessLow=std::clamp((s.low-s.body+7.f)/14.f,0.f,1.f);
    const float excessBody=std::clamp((s.body-s.presence+8.f)/16.f,0.f,1.f);
    switch(goal) {
        case Goal::Clarity:
            result.bands={{{245.f,-1.2f-0.75f*excessBody,0.85f},
                           {2300.f,1.15f-0.30f*excessHarsh,0.85f},
                           {5100.f,-0.7f-1.3f*excessHarsh,1.35f}}};
            break;
        case Goal::VocalWarmth:
            result.bands={{{165.f,0.85f+0.55f*(1.f-excessLow),0.68f},
                           {390.f,-0.25f-0.6f*excessBody,0.82f},
                           {4500.f,-0.75f-0.75f*excessHarsh,1.1f}}};
            break;
        case Goal::Softness:
            result.bands={{{240.f,0.30f,0.65f},
                           {3100.f,-0.75f-0.5f*excessHarsh,0.8f},
                           {5900.f,-0.85f-1.3f*excessHarsh,1.15f}}};
            break;
        case Goal::BassDefinition:
            result.bands={{{105.f,0.85f-0.6f*excessLow,0.72f},
                           {270.f,-1.3f-0.9f*excessBody,0.92f},
                           {1050.f,0.65f,0.9f}}};
            break;
    }
    for(auto& b:result.bands) {
        b.gain=std::clamp(b.gain*strength,-3.0f,3.0f);
        b.hz=std::clamp(b.hz,30.f,18000.f);
        b.q=std::clamp(b.q,0.35f,8.f);
        result.expectedMaxChangeDb=std::max(result.expectedMaxChangeDb,std::abs(b.gain));
    }
    result.valid=true;
    return result;
}
inline bool bounded(const Suggestion& x) noexcept {
    if(!x.valid)return false;
    for(const auto& b:x.bands)
        if(!std::isfinite(b.hz)||!std::isfinite(b.gain)||!std::isfinite(b.q) ||
           b.hz<30.f||b.hz>18000.f||std::abs(b.gain)>3.001f||b.q<0.35f||b.q>8.f)
            return false;
    return true;
}
} // namespace loveeq::intent
