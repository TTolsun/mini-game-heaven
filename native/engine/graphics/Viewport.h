#pragma once
#include <algorithm>
#include <cmath>
#include "engine/math/Rect.h"
namespace engine {
// Top-left screen coordinates, shared by presentation and touch mapping.
inline Rect fitViewport(int width, int height, int left, int top, int right, int bottom) {
    const int availableWidth=std::max(1,width-left-right);
    const int availableHeight=std::max(1,height-top-bottom);
    const float scale=std::min(availableWidth/1280.0f,availableHeight/720.0f);
    const float w=std::floor(1280*scale),h=std::floor(720*scale);
    return {left+std::floor((availableWidth-w)/2),top+std::floor((availableHeight-h)/2),w,h};
}
inline Vec2 viewportPoint(Rect viewport,Vec2 screen) {
    return {(screen.x-viewport.x)*1280/std::max(1.0f,viewport.w),
            (screen.y-viewport.y)*720/std::max(1.0f,viewport.h)};
}
}
