#pragma once
// LOVE EQ MATCH v1: analysis in six logarithmic bands (C++17 / JUCE-free).
// Captures actual measured spectra; never invents a curve from silence.
// Match the tonal SHAPE after subtracting the overall level offset.
#include <algorithm>
#include <array>
#include <cmath>
namespace loveeq::match {
constexpr int bands=6;
constexpr std::array<float,bands> centers {85.f,220.f,560.f,1450.f,3600.f,8500.f};
constexpr std::array<float,bands+1> limits {42.f,133.f,345.f,899.f,2285.f,5530.f,14000.f};
struct Profile {
    std::array<double,bands> energy {};
    int frames=0;
    void reset() noexcept {energy.fill(0.0);frames=0;}
    void addFrame(const float* db,int count,double sr,int fftSize) noexcept {
        if(!db||count<128||sr<8000.0||fftSize<512)return;
        std::array<double,bands> powers {};
        for(int b=0;b<bands;++b){
            int n=0;
            const int first=std::max(1,(int)std::ceil(limits[(size_t)b]*fftSize/sr));
            const int last=std::min(count-2,(int)std::floor(limits[(size_t)(b+1)]*fftSize/sr));
            for(int k=first;k<=last;++k){
                if(!std::isfinite(db[k]))continue;
                const double a=std::pow(10.0,std::clamp((double)db[k],-110.0,24.0)/20.0);
                powers[(size_t)b]+=a*a;
                ++n;
            }
            if(n>0)powers[(size_t)b]/=n;
        }
        double maxPower=0.0;
        for(auto p:powers)maxPower=std::max(maxPower,p);
        // Silence must not be interpreted as a flat musical response.
        if(maxPower<1e-8)return;
        for(int b=0;b<bands;++b)
            energy[(size_t)b]+=powers[(size_t)b];
        ++frames;
    }
    bool ready() const noexcept {return frames>=8;}
    float db(int band) const noexcept {
        return (float)(10.0*std::log10(std::max(1.0e-11,
            energy[(size_t)band]/std::max(1,frames))));
    }
};
struct Curve {
    std::array<float,bands> gainDb {};
    bool valid=false;
};
inline Curve calculate(const Profile& reference,const Profile& source,
                       float maxBandGain=6.f) noexcept {
    Curve c;
    if(!reference.ready()||!source.ready())return c;
    std::array<float,bands> difference{};
    double mean=0.0;
    for(int i=0;i<bands;++i){
        difference[(size_t)i]=reference.db(i)-source.db(i);
        mean+=difference[(size_t)i];
    }
    // Remove the broadband loudness offset: EQ should not behave like a fader.
    mean/=bands;
    const float limit=std::clamp(maxBandGain,0.f,6.f);
    for(int i=0;i<bands;++i){
        const float target=(float)((double)difference[(size_t)i]-mean);
        const float previous=(float)((double)difference[(size_t)std::max(0,i-1)]-mean);
        const float next=(float)((double)difference[(size_t)std::min(bands-1,i+1)]-mean);
        // Local smoothness prevents a staircase-like six-band approximation.
        const float smoothed=0.75f*target+0.125f*(previous+next);
        c.gainDb[(size_t)i]=std::clamp(smoothed,-limit,limit);
    }
    c.valid=true;
    return c;
}
inline bool finiteAndBounded(const Curve& c) noexcept {
    if(!c.valid)return false;
    for(float d:c.gainDb)if(!std::isfinite(d)||std::abs(d)>6.0001f)return false;
    return true;
}
} // namespace loveeq::match
