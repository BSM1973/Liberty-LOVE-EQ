#include "MatchCore.h"
#include <array>
#include <cmath>
#include <iostream>
static int fail(const char* message){std::cerr<<message<<"\n";return 1;}
int main(){
    using namespace loveeq::match;
    Profile ref,src;
    if(calculate(ref,src).valid)return fail("No match without captures");
    std::array<float,4097> reference{},source{};
    reference.fill(-80.f);source.fill(-80.f);
    constexpr double sr=48000.0;
    constexpr int fft=8192;
    for(int i=1;i<4097;++i){
        const auto hz=i*sr/fft;
        const float refDb=hz<345?-22.f:hz<2285?-34.f:-39.f;
        reference[(size_t)i]=refDb;
        source[(size_t)i]=hz<345?-39.f:hz<2285?-29.f:-27.f;
    }
    for(int k=0;k<36;++k){ref.addFrame(reference.data(),4097,sr,fft);
                           src.addFrame(source.data(),4097,sr,fft);}
    if(!ref.ready()||!src.ready())return fail("Actual profiles did not accumulate");
    const auto curve=calculate(ref,src);
    if(!finiteAndBounded(curve))return fail("Curve invalid or over +/-6 dB");
    if(curve.gainDb[0]<=0.f || curve.gainDb[1]<=0.f)
        return fail("Low-range target must add body when reference has more");
    if(curve.gainDb[4]>=0.f || curve.gainDb[5]>=0.f)
        return fail("High-range match should cut excess treble");
    Profile identical=ref;
    const auto identity=calculate(ref,identical);
    if(!finiteAndBounded(identity))return fail("Identical profile failure");
    for(float d:identity.gainDb)if(std::abs(d)>0.0001f)return fail("Identical tonal profile must be flat");
    Profile shifted=ref;
    for(auto& power:shifted.energy)power*=4.0;
    const auto offset=calculate(ref,shifted);
    for(float d:offset.gainDb)if(std::abs(d)>0.0005f)
        return fail("Loudness-only shift must not produce EQ changes");
    Profile silent;
    std::array<float,4097> silence{};silence.fill(-105.f);
    for(int i=0;i<40;++i)silent.addFrame(silence.data(),4097,sr,fft);
    if(silent.ready())return fail("Silence incorrectly accepted as a reference");
    std::cout<<"Real spectral EQ MATCH safe capture and bounded tonal matching passed\n";
    return 0;
}
