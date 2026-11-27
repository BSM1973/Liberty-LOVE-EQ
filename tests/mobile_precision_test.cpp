#include "MobilePrecision.h"
#include <iostream>
#include <cmath>
int main(){
    using namespace loveeq::mobile;
    for(int mask=0;mask<32;++mask){
        bool s=mask&1,b=mask&2,c=mask&4,f=mask&8,a=mask&16;
        int expected=c?24:!s?4:(b||f||!a)?8:24;
        if(refreshHz(s,b,c,f,a)!=expected)return 1;
    }
    auto check=[](const auto& w){
        for(std::size_t j=0;j<w.size();++j){
            float expected=0.5f-0.5f*std::cos(2.f*3.14159265358979323846f*(float)j/(float)(w.size()-1));
            if(!std::isfinite(w[j])||std::abs(w[j]-expected)>1.e-7f)return false;
        }
        return true;
    };
    if(!check(hannWindow<2048>())||!check(hannWindow<8192>()))return 2;
    if(&hannWindow<8192>()!=&hannWindow<8192>())return 3;
    std::cout<<"32 refresh states, 10240 Hann coefficients and shared storage: PASS\n";
}
