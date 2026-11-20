#include "HarmonicDNA.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

static int fail(const char* msg) { std::cerr<<msg<<"\n";return 1; }
static void addTone(std::vector<float>& spectrum,double hz,double amplitude,
                    double sampleRate=48000.0,int size=8192) {
    const double bin=hz*size/sampleRate;
    for(int delta=-7;delta<=7;++delta) {
        const int i=(int)std::round(bin)+delta;
        if(i<1||i>=(int)spectrum.size()-1)continue;
        const double d=((double)i-bin)/0.78;
        spectrum[(size_t)i]+=(float)(amplitude*std::exp(-0.5*d*d));
    }
}
static std::vector<float> harmonicSource(double f0,int count,double emphasis=1.0) {
    std::vector<float> bins(4097,0.00003f);
    for(int n=1;n<=count;++n)
        addTone(bins,f0*n,(0.86/std::pow((double)n,emphasis))*(n==1?0.62:1.0));
    return bins;
}
int main() {
    using loveeq::dna::analyze;
    {
        auto guitar=harmonicSource(82.4069,10);
        addTone(guitar,950.0,0.33);
        const auto a=analyze(guitar.data(),(int)guitar.size(),48000.0,8192);
        if(!a.reliable)return fail("Guitar E2 fundamental not detected");
        if(std::abs(a.fundamentalHz-82.4069)>5.0)return fail("Wrong guitar fundamental");
        if(a.harmonicCount<5 || a.confidence<0.57f)return fail("Harmonic family not protected");
        if(a.inspectCount<1)return fail("Inharmonic peak should be flagged for inspection");
        bool present=false;
        for(int n=0;n<a.inspectCount;++n)
            if(std::abs(a.inspect[(size_t)n].hz-950.f)<22.f)present=true;
        if(!present)return fail("Nonharmonic peak missing from inspection list");
    }
    {
        auto voice=harmonicSource(196.0,9,0.68);
        addTone(voice,392.0,0.8);
        addTone(voice,588.0,0.7);
        const auto a=analyze(voice.data(),(int)voice.size(),48000.0,8192);
        if(!a.reliable || std::abs(a.fundamentalHz-196.f)>8.f)
            return fail("Voiced pitch should not lock onto a stronger overtone");
        if(a.harmonicCount<3)return fail("Expected voice harmonics");
    }
    {
        auto synth=harmonicSource(440.0,8,0.48);
        const auto a=analyze(synth.data(),(int)synth.size(),48000.0,8192);
        if(!a.reliable || std::abs(a.fundamentalHz-440.f)>10.f)
            return fail("Synth A4 pitch detection failed");
    }
    {
        std::vector<float> silence(4097,0.f);
        if(analyze(silence.data(),(int)silence.size(),48000.0,8192).reliable)
            return fail("Silence must never be classified");
        std::vector<float> noise(4097,0.01f);
        if(analyze(noise.data(),(int)noise.size(),48000.0,8192).reliable)
            return fail("Flat broadband spectrum must not be classified");
    }
    {
        std::vector<float> sine(4097,0.00003f);
        addTone(sine,440.0,0.8);
        if(analyze(sine.data(),(int)sine.size(),48000.0,8192).reliable)
            return fail("Pure sine must not be labelled a complete harmonic family");
    }
    std::cout<<"Harmonic DNA detection and protection checks passed\n";
    return 0;
}
