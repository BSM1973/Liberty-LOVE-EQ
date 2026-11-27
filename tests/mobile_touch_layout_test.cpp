#include "MobileTouchLayout.h"
#include <iostream>
int main(){
    using namespace loveeq::mobile;
    int tested=0;
    for(int w=280;w<=1400;w+=4)for(int h=240;h<=1100;h+=5){
        if(!supportsTouch(w,h))continue;
        const int innerW=w-6,availableH=h-6-44-3;
        int cw=innerW,ch=availableH;
        if(sideBySide(w,h))cw=innerW*49/100;
        else {
            const int reserve=w<402?232:189;
            const int graph=std::max(48,availableH-reserve);
            ch=availableH-graph-3;
            if(graph<48)return 1;
        }
        const int cols=cw<394?4:8, rows=8/cols;
        const int valueH=ch-44-3-rows*46;
        const int valueW=cw-std::min(84,cw/4);
        if(valueH<44||valueW/4-3<40)return 2;
        for(int i=0;i<cols;++i){
            auto a=cell({0,0,cw,44},cols,i);
            if(a.w<40||a.x<0||a.x+a.w>cw)return 3;
            if(i>0){auto b=cell({0,0,cw,44},cols,i-1);if(b.x+b.w>a.x)return 4;}
        }
        ++tested;
    }
    std::cout<<"Mobile geometry states: "<<tested<<" PASS\n";
}
