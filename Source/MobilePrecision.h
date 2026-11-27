#pragma once
#include <array>
#include <cmath>
#include <cstddef>
namespace loveeq::mobile {
constexpr int activeHz=24, idleHz=8, hiddenHz=4;
inline constexpr int refreshHz(bool showing,bool browsing,bool capturing,bool frozen,bool freshAudio) noexcept {
    if(capturing)return activeHz; // EQ Match retains its original 48 frames / 2s.
    if(!showing)return hiddenHz;
    if(browsing || frozen || !freshAudio)return idleHz;
    return activeHz;
}
template<std::size_t N> const std::array<float,N>& hannWindow() {
    // Shared immutable table, initialized on the UI thread, never the audio callback.
    static const auto table=[] {
        std::array<float,N> result{};
        for(std::size_t j=0;j<N;++j)
            result[j]=0.5f-0.5f*std::cos(2.f*3.14159265358979323846f*(float)j/(float)(N-1));
        return result;
    }();
    return table;
}
}
