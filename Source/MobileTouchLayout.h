#pragma once
#include <algorithm>
namespace loveeq::mobile {
// Logical points, not physical device pixels. A smaller host is always
// allowed to fall back to the original zoomable editor.
inline constexpr bool supportsTouch(int w,int h) noexcept {
    return (w>=280 && h>=350) || (w>=600 && h>=300);
}
inline constexpr bool preferTouch(int w,int h) noexcept {
    return supportsTouch(w,h) && (w<760 || h<620);
}
inline constexpr bool sideBySide(int w,int h) noexcept {
    return w>=600 && h<500;
}
struct ZoomDomain { float start, span; };
inline ZoomDomain pinchDomain(float anchor,float centre,float initialSpan,float initialDistance,float currentDistance) noexcept {
    const float span=std::clamp(initialSpan*std::max(8.f,initialDistance)/std::max(8.f,currentDistance),.2f,1.f);
    return {std::clamp(anchor-span*std::clamp(centre,0.f,1.f),0.f,1.f-span),span};
}
struct Rect {int x,y,w,h;};
inline Rect cell(Rect r,int columns,int index,int gap=3) noexcept {
    const int a=r.w*index/columns,b=r.w*(index+1)/columns;
    return {r.x+a,r.y,std::max(0,b-a-gap),r.h};
}
}
