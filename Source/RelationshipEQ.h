#pragma once
// LOVE EQ Relationship EQ, v1. One-process, same-plugin-format spectral exchange.
// No allocation, locks, string operations or disk access in the audio callback.
// Other formats, host-plugin sandboxes and separate DAW processes do not share this bus.
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>

namespace loveeq::relationship {

constexpr int bandCount=12;
constexpr int maxInstances=32;
constexpr int maxGroup=4;
constexpr std::array<float,bandCount+1> edges {
    65.f,95.f,140.f,210.f,310.f,460.f,680.f,1020.f,
    1500.f,2200.f,3300.f,5000.f,8000.f
};
constexpr std::array<float,bandCount> centres {
    79.f,115.f,172.f,255.f,377.f,559.f,833.f,1237.f,
    1817.f,2694.f,4062.f,6324.f
};
enum Role : int { none=0,voice=1,bass=2,guitar=3,other=4 };

inline int nearestBand(float hz) noexcept {
    if(!std::isfinite(hz))return 0;
    int best=0;float distance=1.e30f;
    for(int i=0;i<bandCount;++i) {
        const auto d=std::abs(std::log2(std::max(hz,1.f)/centres[(size_t)i]));
        if(d<distance){distance=d;best=i;}
    }
    return best;
}

struct Frame {
    std::array<float,bandCount> rms {};
    bool valid=false;
    int count=0;
};
struct Slot {
    std::atomic<bool> allocated{false},ready{false};
    std::atomic<int> role{0},group{0};
    std::atomic<std::uint64_t> stamp{0};
    std::array<std::atomic<float>,bandCount> rms {};
};
class Registry final {
public:
    int reserve() noexcept {
        for(int i=0;i<maxInstances;++i) {
            bool free=false;
            if(slots[(size_t)i].allocated.compare_exchange_strong(
                   free,true,std::memory_order_acq_rel)) {
                auto& s=slots[(size_t)i];
                s.ready.store(false,std::memory_order_release);
                s.group.store(0,std::memory_order_relaxed);
                s.role.store(0,std::memory_order_relaxed);
                s.stamp.store(0,std::memory_order_relaxed);
                return i;
            }
        }
        return -1;
    }
    void release(int id) noexcept {
        if(id<0||id>=maxInstances)return;
        auto& s=slots[(size_t)id];
        s.ready.store(false,std::memory_order_release);
        s.group.store(0,std::memory_order_relaxed);
        s.role.store(0,std::memory_order_relaxed);
        s.allocated.store(false,std::memory_order_release);
    }
    void publish(int id,int group,int role,const std::array<float,bandCount>& levels) noexcept {
        if(id<0||id>=maxInstances)return;
        auto& s=slots[(size_t)id];
        if(!s.allocated.load(std::memory_order_acquire))return;
        s.ready.store(false,std::memory_order_release);
        s.group.store(group,std::memory_order_relaxed);
        s.role.store(role,std::memory_order_relaxed);
        for(int i=0;i<bandCount;++i)
            s.rms[(size_t)i].store(std::isfinite(levels[(size_t)i])?
                 std::clamp(levels[(size_t)i],0.f,8.f):0.f,std::memory_order_relaxed);
        const auto tick=epoch.fetch_add(1,std::memory_order_acq_rel)+1;
        s.stamp.store(tick,std::memory_order_release);
        s.ready.store(group>0&&group<=maxGroup&&role>0&&role<=4,std::memory_order_release);
    }
    Frame snapshot(int id) const noexcept {
        Frame out;
        if(id<0||id>=maxInstances)return out;
        const auto& slot=slots[(size_t)id];
        if(!slot.allocated.load(std::memory_order_acquire) ||
           !slot.ready.load(std::memory_order_acquire))return out;
        for(int i=0;i<bandCount;++i)
            out.rms[(size_t)i]=slot.rms[(size_t)i].load(std::memory_order_relaxed);
        out.valid=true;
        out.count=1;
        return out;
    }
    Frame aggregate(int group,int role,int except=-1) const noexcept {
        Frame sum;
        if(group<1||group>maxGroup||role<1||role>4)return sum;
        const auto now=epoch.load(std::memory_order_acquire);
        for(int i=0;i<maxInstances;++i) {
            if(i==except)continue;
            const auto& s=slots[(size_t)i];
            if(!s.allocated.load(std::memory_order_acquire) ||
                !s.ready.load(std::memory_order_acquire) ||
                s.group.load(std::memory_order_relaxed)!=group ||
                s.role.load(std::memory_order_relaxed)!=role)continue;
            const auto stamp=s.stamp.load(std::memory_order_acquire);
            // Expire a stopped source once other instances have kept processing.
            if(stamp==0||now<stamp||now-stamp>96)continue;
            for(int b=0;b<bandCount;++b) {
                const float v=s.rms[(size_t)b].load(std::memory_order_relaxed);
                sum.rms[(size_t)b]=std::max(sum.rms[(size_t)b],v);
            }
            ++sum.count;
        }
        sum.valid=sum.count>0;
        return sum;
    }
private:
    std::array<Slot,maxInstances> slots {};
    std::atomic<std::uint64_t> epoch{0};
};
inline Registry& registry() noexcept {
    static Registry shared;
    return shared;
}

// Lightweight, approximate spectral filterbank. The resulting bands are not
// FFT-precise; they are measured from the live audio signal each processBlock.
class Meter final {
public:
    void prepare(double sampleRate) noexcept {
        const double sr=std::max(8000.0,sampleRate);
        for(int i=0;i<=bandCount;++i) {
            const double fc=std::min((double)edges[(size_t)i],sr*0.45);
            coeff[(size_t)i]=1.0-std::exp(-6.283185307179586*fc/sr);
        }
        state.fill(0.0);
        energy.fill(0.0);
        count=0;
    }
    void push(float mono) noexcept {
        const double x=std::isfinite(mono)?std::clamp((double)mono,-8.0,8.0):0.0;
        double previous=0.0;
        for(int i=0;i<=bandCount;++i) {
            auto& lp=state[(size_t)i];
            lp+=coeff[(size_t)i]*(x-lp);
            if(i>0) {
                const double band=lp-previous;
                energy[(size_t)i-1]+=band*band;
            }
            previous=lp;
        }
        ++count;
    }
    std::array<float,bandCount> take() noexcept {
        std::array<float,bandCount> result{};
        const double inv=count>0?1.0/(double)count:0.0;
        for(int i=0;i<bandCount;++i) {
            result[(size_t)i]=(float)std::sqrt(std::max(0.0,energy[(size_t)i])*inv);
            energy[(size_t)i]=0.0;
        }
        count=0;
        return result;
    }
private:
    std::array<double,bandCount+1> state {},coeff {};
    std::array<double,bandCount> energy {};
    int count=0;
};
inline bool isProtected(float hz,float fundamental,int maxOrder=6) noexcept {
    if(!std::isfinite(hz)||!std::isfinite(fundamental)||fundamental<45.f)return false;
    for(int n=1;n<=maxOrder;++n) {
        const float partial=n*fundamental;
        if(partial>16000.f)break;
        if(std::abs(hz-partial)<std::max(21.f,0.11f*partial))return true;
    }
    return false;
}
struct Proposal {
    bool valid=false;
    float hz=0.f,score=0.f;
    int bin=-1,protectedBands=0;
};
inline Proposal propose(const Frame& own,const Frame& priority,float fundamental=0.f) noexcept {
    Proposal result;
    if(!own.valid||!priority.valid)return result;
    float best=0.f;
    for(int i=0;i<bandCount;++i) {
        if(isProtected(centres[(size_t)i],fundamental)){
            ++result.protectedBands;
            continue;
        }
        const float ownLevel=own.rms[(size_t)i];
        const float priorityLevel=priority.rms[(size_t)i];
        if(!std::isfinite(ownLevel)||!std::isfinite(priorityLevel)||
           ownLevel<0.0015f||priorityLevel<0.0015f)continue;
        const float relative=std::abs(20.f*std::log10((ownLevel+1.e-8f)/(priorityLevel+1.e-8f)));
        if(relative>19.f)continue;
        const float loudness=std::clamp(std::min(ownLevel,priorityLevel)/0.025f,0.f,1.f);
        const float compatibility=1.f-relative/24.f;
        const float score=loudness*compatibility;
        if(score>best && score>0.085f) {
            best=score;
            result.valid=true;
            result.bin=i;
            result.hz=centres[(size_t)i];
            result.score=score;
        }
    }
    return result;
}
inline float dynamicCut(float ownLevel,float priorityLevel,float maxCutDb) noexcept {
    if(!std::isfinite(ownLevel)||!std::isfinite(priorityLevel)||
       ownLevel<0.0015f||priorityLevel<0.0015f)return 0.f;
    const float relative=std::abs(20.f*std::log10((ownLevel+1.e-8f)/(priorityLevel+1.e-8f)));
    if(relative>=20.f)return 0.f;
    const float activity=std::clamp((std::min(ownLevel,priorityLevel)-0.0015f)/0.025f,0.f,1.f);
    const float overlap=std::clamp((20.f-relative)/20.f,0.f,1.f);
    return -std::clamp(maxCutDb,0.f,6.f)*activity*overlap;
}
} // namespace loveeq::relationship
