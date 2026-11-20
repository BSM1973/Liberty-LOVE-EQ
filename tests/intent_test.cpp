#include "IntentCore.h"
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
static int fail(const char* why){std::cerr<<why<<"\n";return 1;}
int main() {
    using namespace loveeq::intent;
    std::array<float,1025> spec{};
    spec.fill(-66.f);
    for(int i=1;i<1025;++i) {
        const double hz=i*48000.0/2048.;
        const float center=(hz>180. && hz<800.)?-26.f:
                           (hz>1500. && hz<3800.)?-30.f:
                           (hz>3800. && hz<7800.)?-22.f:-59.f;
        spec[(size_t)i]=center;
    }
    auto s=inspect(spec.data(),(int)spec.size(),48000.,2048);
    if(!s.valid || s.harshness<s.presence)return fail("Measured harshness profile invalid");
    for(int g=0;g<=3;++g) {
        const auto a=propose(static_cast<Goal>(g),0,s);
        const auto b=propose(static_cast<Goal>(g),1,s);
        const auto c=propose(static_cast<Goal>(g),2,s);
        if(!bounded(a)||!bounded(b)||!bounded(c))return fail("A suggestion exceeded safe bounds");
        if(!(a.expectedMaxChangeDb<=b.expectedMaxChangeDb &&
             b.expectedMaxChangeDb<=c.expectedMaxChangeDb))
            return fail("Three variants must increase gently in intensity");
    }
    const auto clarity=propose(Goal::Clarity,2,s);
    if(clarity.bands[1].gain<=0.f || clarity.bands[2].gain>=0.f)
        return fail("Clarity must brighten presence while taming harshness");
    const auto warm=propose(Goal::VocalWarmth,1,s);
    if(warm.bands[0].gain<=0.f||warm.bands[2].gain>=0.f)
        return fail("Vocal warmth must protect body without adding harshness");
    const auto bass=propose(Goal::BassDefinition,2,s);
    if(bass.bands[1].gain>=0.f)return fail("Bass definition should reduce mud");
    std::array<float,1025> silent{};silent.fill(-96.f);
    if(inspect(silent.data(),1025,48000.,2048).valid)
        return fail("No recommendation must be made from silence");
    if(propose(Goal::Clarity,1,inspect(silent.data(),1025,48000.,2048)).valid)
        return fail("Silent source should not produce fake musical suggestions");
    silent[250]=std::numeric_limits<float>::quiet_NaN();
    if(inspect(silent.data(),1025,48000.,2048).valid)
        return fail("NaN bins must not create suggestions");
    std::cout<<"LOVE INTENT safe deterministic goal/three-variant analysis passed\n";
    return 0;
}
