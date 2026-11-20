#pragma once
// LOVE EQ - Harmonic DNA v1: advisory, mono-pitched-source analysis.
// Uses spectral evidence rather than treating every peak as an unwanted resonance.
// GUI/worker thread only. No file/network I/O or allocations in the audio callback.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace loveeq::dna {
constexpr int maxHarmonics=12;
constexpr int maxCandidates=5;

struct Harmonic {
    float hz=0.f, relativeDb=-96.f;
    int order=0;
};
struct SpectralPeak {
    float hz=0.f, relativeDb=-96.f, prominenceDb=0.f;
};
struct Result {
    bool reliable=false;
    float fundamentalHz=0.f, confidence=0.f, signalDb=-96.f;
    std::array<Harmonic,maxHarmonics> harmonic {};
    int harmonicCount=0;
    std::array<SpectralPeak,maxCandidates> inspect {};
    int inspectCount=0;
};
struct Peak {
    double hz=0.0, amplitude=0.0, prominence=0.0;
};
inline double frequencyFromBin(double bin,double sr,int fftSize) noexcept {
    return bin*sr/(double)fftSize;
}
inline double peakFrequency(const float* bins,int i,int count,double sr,int fftSize) noexcept {
    if(i<=0||i>=count-1)return frequencyFromBin((double)i,sr,fftSize);
    const double l=std::log(std::max((double)bins[i-1],1.0e-15));
    const double c=std::log(std::max((double)bins[i],1.0e-15));
    const double r=std::log(std::max((double)bins[i+1],1.0e-15));
    const double divisor=l-2.0*c+r;
    const double shift=std::abs(divisor)>1.0e-8?
        std::clamp(0.5*(l-r)/divisor,-0.5,0.5):0.0;
    return frequencyFromBin(i+shift,sr,fftSize);
}
inline Result analyze(const float* magnitudes,int count,double sampleRate,int fftSize) {
    Result result;
    if(!magnitudes||count<256||fftSize<512||sampleRate<8000.0)return result;
    const int nyquist=std::min(count-2,fftSize/2-1);
    if(nyquist<128)return result;
    double strongest=0.0;
    for(int i=4;i<=nyquist;++i)
        if(std::isfinite(magnitudes[i]))strongest=std::max(strongest,(double)magnitudes[i]);
    // All silent/very quiet frames are intentionally unclassified.
    result.signalDb=(float)(20.0*std::log10(std::max(strongest,1.e-12)));
    if(strongest<0.00005)return result;

    std::vector<Peak> peaks;
    peaks.reserve(256);
    const double hzPerBin=sampleRate/(double)fftSize;
    const int first=std::max(3,(int)std::ceil(46.0/hzPerBin));
    const int last=std::min(nyquist-9,(int)std::floor(12000.0/hzPerBin));
    for(int i=first;i<=last;++i) {
        const double amp=magnitudes[i];
        if(!std::isfinite(amp)||amp<0.021*strongest||
           amp<magnitudes[i-1]||amp<magnitudes[i+1])continue;
        // Reject broad noise humps: require a measurable local peak.
        double side=0.0;int sideCount=0;
        for(int off : {-11,-10,-9,-8,-7,-6,-5,-4,4,5,6,7,8,9,10,11}) {
            const int index=i+off;
            if(index<1||index>=count)continue;
            side+=std::max(0.0,(double)magnitudes[index]);++sideCount;
        }
        const double background=sideCount?side/sideCount:amp;
        const double prominence=20.0*std::log10((amp+1.e-12)/(background+1.e-12));
        if(prominence<4.2)continue;
        peaks.push_back({peakFrequency(magnitudes,i,count,sampleRate,fftSize),amp,prominence});
    }
    if(peaks.size()<3)return result;
    std::stable_sort(peaks.begin(),peaks.end(),[](const auto& a,const auto& b){
        return a.amplitude>b.amplitude;
    });
    if(peaks.size()>72)peaks.resize(72);

    const auto nearPeak=[&](double hz)->const Peak* {
        const double tolerance=std::max(hzPerBin*1.2,hz*0.018);
        const Peak* best=nullptr;
        for(const auto& peak:peaks) {
            if(std::abs(peak.hz-hz)>tolerance)continue;
            if(!best||peak.amplitude>best->amplitude)best=&peak;
        }
        return best;
    };
    // Try f0 hypotheses derived from observed peaks divided by harmonic number,
    // including missing-fundamental candidates, not only the loudest peak.
    double bestScore=-1.0,bestHz=0.0;
    int bestHits=0;
    const int sourceCount=std::min(36,(int)peaks.size());
    for(int p=0;p<sourceCount;++p) {
        for(int divisor=1;divisor<=8;++divisor) {
            const double f0=peaks[(size_t)p].hz/divisor;
            if(f0<48.0||f0>1300.0)continue;
            double score=0.0;int hits=0,oddHits=0,earlyHits=0;
            for(int n=1;n<=maxHarmonics;++n) {
                if(n*f0>std::min(12000.0,sampleRate*0.46))break;
                const auto* match=nearPeak(n*f0);
                if(!match)continue;
                ++hits;if(n%2)++oddHits;if(n<=4)++earlyHits;
                const double level=std::sqrt(match->amplitude/strongest);
                score+=(0.35+0.65*level)/std::sqrt((double)n);
            }
            // A simple sine wave, unpitched noise, and weak subharmonic matches
            // are not sufficient evidence of an instrument's harmonic identity.
            if(hits<3||earlyHits<2||oddHits<1)continue;
            if(!nearPeak(f0) && oddHits<2)continue;
            score*=1.0+0.055*std::min(6,hits);
            if(nearPeak(f0))score+=0.26;
            if(oddHits>=2)score+=0.18;
            if(score>bestScore) {bestScore=score;bestHz=f0;bestHits=hits;}
        }
    }
    if(bestScore<=0.0)return result;

    // Refine f0 from aligned peak locations. Peaks at higher harmonics
    // can provide more precise frequency than a wide low-frequency FFT bin.
    double weightedHz=0.0,weightSum=0.0;
    for(int n=1;n<=maxHarmonics;++n) {
        const auto* match=nearPeak(n*bestHz);
        if(!match)continue;
        const double w=std::sqrt(match->amplitude/strongest)*std::sqrt((double)n);
        weightedHz+=(match->hz/n)*w;
        weightSum+=w;
    }
    const double fundamental=weightSum>0?weightedHz/weightSum:bestHz;
    if(!std::isfinite(fundamental)||fundamental<48.0||fundamental>1300.0)return result;

    double matchedWeight=0.0,totalWeight=0.0;
    for(const auto& peak:peaks)totalWeight+=std::sqrt(peak.amplitude/strongest);
    for(int n=1;n<=maxHarmonics;++n) {
        if(n*fundamental>std::min(12000.0,sampleRate*0.46))break;
        const auto* match=nearPeak(n*fundamental);
        if(!match)continue;
        result.harmonic[(size_t)result.harmonicCount++] = {
            (float)match->hz,(float)(20.0*std::log10(match->amplitude/strongest)),n
        };
        matchedWeight+=std::sqrt(match->amplitude/strongest);
    }
    if(result.harmonicCount<3)return result;
    const double coverage=matchedWeight/std::max(1.e-9,totalWeight);
    const double strength=(double)std::min(8,result.harmonicCount)/8.0;
    const double accuracy=(double)std::min(1.0,bestScore/4.3);
    result.confidence=(float)std::clamp(0.15+0.38*coverage+0.25*strength+0.22*accuracy,0.0,0.99);
    if(result.confidence<0.57f)return result;
    result.reliable=true;
    result.fundamentalHz=(float)fundamental;

    // An out-of-series peak is only "to inspect", never automatically "bad".
    // It can be a second note, pick noise, a formant, etc.
    for(const auto& peak:peaks) {
        if(peak.amplitude<strongest*0.035||peak.hz<80.0||peak.hz>10000.0)continue;
        bool harmonic=false;
        for(int n=1;n<=maxHarmonics;++n) {
            if(std::abs(peak.hz-n*fundamental)<=std::max(1.45*hzPerBin,n*fundamental*0.022)){
                harmonic=true;break;
            }
        }
        if(harmonic)continue;
        const auto relative=20.0*std::log10(peak.amplitude/strongest);
        if(relative< -27.0||peak.prominence<5.0)continue;
        if(result.inspectCount<maxCandidates) {
            result.inspect[(size_t)result.inspectCount++]={
                (float)peak.hz,(float)relative,(float)peak.prominence
            };
        }
    }
    return result;
}
} // namespace loveeq::dna
