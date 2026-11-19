#include "../Source/EqCore.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main() {
    using namespace loveeq;
    constexpr double sr=48000.0;
    auto bell=design(Type::Bell,sr,1000.0,6.0,1.0);
    assert(std::abs(magnitudeDb(bell,sr,1000.0)-6.0)<0.02);
    auto shelf=design(Type::LowShelf,sr,150.0,6.0,0.707);
    assert(magnitudeDb(shelf,sr,30.0)>4.5);
    auto hp=design(Type::HighPass,sr,100.0,0.0,0.707);
    assert(magnitudeDb(hp,sr,30.0)<-18.0);
    auto lp=design(Type::LowPass,sr,5000.0,0.0,0.707);
    assert(magnitudeDb(lp,sr,15000.0)<-15.0);
    auto one=design(Type::HighPass,sr,100.0,0.0,0.707,true);
    assert(magnitudeDb(one,sr,20.0)<-10.0);
    Biquad b; b.set(bell);
    for(int i=0;i<100000;i++) assert(std::isfinite(b.process(i%13==0 ? 1.0f : 0.0f)));
    std::cout << "LOVE EQ core tests OK\n";
}
