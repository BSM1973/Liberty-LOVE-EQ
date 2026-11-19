#pragma once
// LOVE EQ Curve Lab: original logarithmic sketch fitting, note quantization,
// and non-destructive spectrum peak detection. C++17, no JUCE dependency.
#include "EqCore.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace loveeq::curvelab {
struct SketchPoint { double position=0.0, gain=0.0; };
struct FittedBand { float hz=1000.f, gain=0.f, q=1.f; bool enabled=false; };
struct Peak { float hz=0.f, prominence=0.f; bool valid=false; };

inline double hzFromPosition(double x) noexcept {
    return 20.0*std::pow(1000.0,std::clamp(x,0.0,1.0));
}
inline double positionFromHz(double hz) noexcept {
    return std::clamp(std::log(std::clamp(hz,20.0,20000.0)/20.0)/std::log(1000.0),0.0,1.0);
}
inline float noteFrequency(float hz) noexcept {
    if(!std::isfinite(hz)||hz<=0.f)return 440.f;
    const double key=std::clamp(std::round(69.0+12.0*std::log2((double)hz/440.0)),0.0,127.0);
    return (float)(440.0*std::pow(2.0,(key-69.0)/12.0));
}
inline std::string noteName(double hz) {
    static const std::array<const char*,12> names {
        "Do","Do#","Re","Re#","Mi","Fa","Fa#","Sol","Sol#","La","La#","Si"
    };
    if(!(hz>0.0)||!std::isfinite(hz))return "?";
    const int midi=std::clamp((int)std::round(69.0+12.0*std::log2(hz/440.0)),0,127);
    const int octave=midi/12-1;
    return std::string(names[(size_t)(midi%12)])+std::to_string(octave);
}
inline std::array<FittedBand,8> fitSketch(const std::vector<SketchPoint>& drawn,double sampleRate=48000.0) {
    std::array<FittedBand,8> result {};
    if(drawn.size()<3)return result;
    auto points=drawn;
    points.erase(std::remove_if(points.begin(),points.end(),[](const auto& p){
        return !std::isfinite(p.position)||!std::isfinite(p.gain);
    }),points.end());
    if(points.size()<3)return result;
    for(auto& p:points) {
        p.position=std::clamp(p.position,0.0,1.0);
        p.gain=std::clamp(p.gain,-18.0,18.0);
    }
    std::stable_sort(points.begin(),points.end(),[](const auto& a,const auto& b){
        return a.position<b.position;
    });
    if(points.back().position-points.front().position<0.055)return result;

    // Re-sample the hand-drawn target on a logarithmic axis.
    constexpr int bins=128;
    std::array<double,bins> target{},model{},residual{};
    size_t j=0;
    for(int i=0;i<bins;++i) {
        const double x=(i+0.5)/(double)bins;
        if(x<points.front().position || x>points.back().position)continue;
        while(j+1<points.size() && points[j+1].position<x)++j;
        if(j+1==points.size())break;
        const double dx=std::max(1e-8,points[j+1].position-points[j].position);
        const double t=std::clamp((x-points[j].position)/dx,0.0,1.0);
        target[(size_t)i]=points[j].gain+t*(points[j+1].gain-points[j].gain);
    }
    for(size_t band=0;band<result.size();++band) {
        double best=0.85;
        int bestIndex=-1;
        double bestResidual=0.0;
        for(int i=3;i<bins-3;++i) {
            const double r=target[(size_t)i]-model[(size_t)i];
            residual[(size_t)i]=r;
            const double smoothed=(residual[(size_t)i]+
                (target[(size_t)(i-1)]-model[(size_t)(i-1)])+
                (target[(size_t)(i+1)]-model[(size_t)(i+1)]))/3.0;
            if(std::abs(smoothed)>best){best=std::abs(smoothed);bestIndex=i;bestResidual=smoothed;}
        }
        if(bestIndex<0)break;
        const double threshold=std::abs(bestResidual)*0.47;
        int left=bestIndex,right=bestIndex;
        while(left>0 && residual[(size_t)(left-1)]*bestResidual>0.0 &&
              std::abs(residual[(size_t)(left-1)])>threshold)--left;
        while(right<bins-1 && (target[(size_t)(right+1)]-model[(size_t)(right+1)])*bestResidual>0.0 &&
              std::abs(target[(size_t)(right+1)]-model[(size_t)(right+1)])>threshold)++right;
        const double widthX=std::clamp((right-left+1)/(double)bins,0.025,0.38);
        const double octaveWidth=widthX*std::log2(1000.0);
        const double factor=std::pow(2.0,octaveWidth);
        const double q=std::clamp(std::sqrt(factor)/std::max(0.01,factor-1.0),0.3,12.0);
        const double hz=hzFromPosition((bestIndex+0.5)/(double)bins);
        const double gain=std::clamp(bestResidual,-15.0,15.0);
        const auto coeff=loveeq::design(loveeq::Type::Bell,sampleRate,hz,gain,q);
        result[band]={(float)hz,(float)gain,(float)q,true};
        for(int i=0;i<bins;++i)
            model[(size_t)i]+=loveeq::magnitudeDb(coeff,sampleRate,hzFromPosition((i+0.5)/(double)bins));
    }
    return result;
}
inline Peak locateSpectrumPeak(const float* db,int count,double sr=48000.0,int fftSize=2048) noexcept {
    Peak result;
    if(!db || count<64 || fftSize<64 || sr<8000.0)return result;
    double bestScore=3.6;
    for(int i=8;i<std::min(count-16,fftSize/2);++i) {
        const double hz=(double)i*sr/fftSize;
        if(hz<80.0 || hz>16000.0 || !std::isfinite(db[i]) || db[i]<-72.f)continue;
        if(db[i]<db[i-1] || db[i]<db[i+1])continue;
        double neighbor=0.0;int n=0;
        for(int j=std::max(0,i-16);j<=std::min(count-1,i+16);++j) {
            if(std::abs(j-i)<3 || !std::isfinite(db[j]))continue;
            neighbor+=db[j];++n;
        }
        if(n==0)continue;
        const double prominence=(double)db[i]-neighbor/n;
        const double score=prominence + std::min(7.0,std::max(0.0,((double)db[i]+60.0)/6.0));
        if(prominence>3.0 && score>bestScore) {
            bestScore=score;result={(float)hz,(float)prominence,true};
        }
    }
    return result;
}
} // namespace loveeq::curvelab
