#include <cassert>
#include <cmath>
#include <array>
#include "EqCore.h"

int main() {
    constexpr std::array<double,16> hz {25.,40.,63.,100.,160.,250.,400.,630.,
         1000.,1600.,2500.,4000.,6300.,10000.,16000.,20000.};
    for(double sr:{44100.,48000.,96000.}) {
        for(double f:hz) {
            const auto flat=loveeq::design(loveeq::Type::Bell,sr,f,0.0,1.8);
            const auto boost=loveeq::design(loveeq::Type::Bell,sr,f,6.0,1.8);
            const auto cut=loveeq::design(loveeq::Type::Bell,sr,f,-6.0,1.8);
            for(double probe:{50.,100.,500.,1000.,10000.}) {
                assert(std::isfinite(loveeq::magnitudeDb(flat,sr,probe)));
                assert(std::isfinite(loveeq::magnitudeDb(boost,sr,probe)));
                assert(std::isfinite(loveeq::magnitudeDb(cut,sr,probe)));
                assert(std::abs(loveeq::magnitudeDb(flat,sr,probe))<0.01);
            }
            // At the centre: a 6dB boost and cut must be non-trivial and opposite.
            const double cf=std::min(f,sr*0.475);
            const double b=loveeq::magnitudeDb(boost,sr,cf);
            const double c=loveeq::magnitudeDb(cut,sr,cf);
            assert(b>1.0 && c< -1.0);
            assert(std::abs(b+c)<0.5);
        }
    }
    return 0;
}
