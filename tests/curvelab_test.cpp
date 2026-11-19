#include "CurveLab.h"
#include <array>
#include <cmath>
#include <iostream>
#include <vector>

static int fail(const char* why) { std::cerr<<why<<"\n";return 1; }
int main() {
    using namespace loveeq::curvelab;
    if(std::abs(noteFrequency(442.f)-440.f)>0.01f)return fail("Pitch snapping A4 failed");
    if(noteName(440.)!="La4" || noteName(261.625565)!="Do4")
        return fail("Musical note labeling failed");
    for(double hz : {20.,63.,220.,1000.,4000.,18000.}) {
        const auto inverse=hzFromPosition(positionFromHz(hz));
        if(std::abs(inverse/hz-1.0)>0.0001)return fail("Log frequency mapping failed");
    }
    if(fitSketch({}).front().enabled)return fail("Empty sketch should remain empty");
    std::vector<SketchPoint> path;
    for(int k=0;k<=80;++k) {
        const double x=(double)k/80.0;
        const double bump=7.0*std::exp(-std::pow((x-0.57)/0.08,2.0));
        const double dip=-4.0*std::exp(-std::pow((x-0.28)/0.07,2.0));
        path.push_back({x,bump+dip});
    }
    const auto fitted=fitSketch(path,48000.0);
    int n=0;bool hasBoost=false,hasCut=false;
    for(const auto& p:fitted) {
        if(!p.enabled)continue;
        ++n;
        if(p.gain>1.f)hasBoost=true;
        if(p.gain< -1.f)hasCut=true;
        if(p.hz<20.f || p.hz>20000.f || !std::isfinite(p.q) ||
           p.q<0.15f || p.q>18.f)return fail("Invalid generated band");
    }
    if(n<2 || !hasBoost || !hasCut)return fail("Sketch needs actual positive and negative filters");
    std::array<float,1025> spectrum{};
    spectrum.fill(-75.f);
    for(int k=0;k<1025;++k) {
        const double d=(k-170)/4.0;
        spectrum[(size_t)k]+=(float)(44.0*std::exp(-d*d));
    }
    const auto peak=locateSpectrumPeak(spectrum.data(),(int)spectrum.size(),48000.0);
    if(!peak.valid || std::abs(peak.hz-170.f*48000.f/2048.f)>200.f)
        return fail("Spectrum peak capture failed");
    spectrum.fill(-90.f);
    if(locateSpectrumPeak(spectrum.data(),(int)spectrum.size(),48000.0).valid)
        return fail("Silence must not be treated as resonance");
    std::cout<<"LOVE EQ Curve Lab checks passed\n";
    return 0;
}
