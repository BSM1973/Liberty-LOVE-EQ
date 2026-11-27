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
struct Rect {int x,y,w,h;};
inline Rect cell(Rect r,int columns,int index,int gap=3) noexcept {
    const int a=r.w*index/columns,b=r.w*(index+1)/columns;
    return {r.x+a,r.y,std::max(0,b-a-gap),r.h};
}
}
