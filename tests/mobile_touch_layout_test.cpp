#include "MobileTouchLayout.h"
#include <iostream>
#include <cmath>
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
    int advanced=0;
    const int pages[5][3]={{0,2,0},{3,2,0},{1,1,3},{1,8,0},{0,6,0}};
    for(int w=280;w<=1400;w+=4)for(int h=240;h<=1100;h+=5){
        if(!supportsTouch(w,h))continue;
        for(const auto& p:pages){
            const int cols=w<500?2:3;
            const int required=10+44+42+p[0]*44+((p[1]+cols-1)/cols)*44+
                ((w>=600&&h<340&&p[2]>0)?44:p[2]*44);
            if(required>h)return 5;
            ++advanced;
        }
    }
    int zoom=0;
    for(int a=0;a<=20;++a)for(int c=0;c<=20;++c)for(int d=8;d<=512;d+=8){
        auto z=pinchDomain(a/20.f,c/20.f,1.f,80.f,(float)d);
        if(z.start<0.f||z.span<.2f||z.span>1.f||z.start+z.span>1.000001f)return 6;
        ++zoom;
    }
    const auto neutral=pinchDomain(.5f,.5f,1.f,80.f,80.f);
    const auto twice=pinchDomain(.5f,.5f,1.f,80.f,160.f);
    if(std::abs(neutral.start)>1.e-6f||std::abs(neutral.span-1.f)>1.e-6f||
       std::abs(twice.start-.25f)>1.e-6f||std::abs(twice.span-.5f)>1.e-6f)return 7;
    std::cout<<"Advanced geometry states: "<<advanced<<" PASS; zoom states: "<<zoom<<" PASS\n";
}
