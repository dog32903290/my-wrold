#include "Tooll3SkinContract.h"

#include <algorithm>

namespace myworld
{
Tooll3SkinPolicy makeTooll3SkinPolicy()
{
    return {};
}

Tooll3SkinLayout makeTooll3SkinLayout (double width, double height)
{
    const auto safeWidth = std::max (1.0, width);
    const auto safeHeight = std::max (1.0, height);
    const auto topHeight = 30.0;
    const auto bottomHeight = 54.0;
    const auto leftWidth = std::clamp (safeWidth * 0.18, 230.0, 320.0);
    const auto contentTop = topHeight;
    const auto contentHeight = std::max (1.0, safeHeight - topHeight - bottomHeight);
    const auto outputWidth = std::max (1.0, safeWidth - leftWidth);

    Tooll3SkinLayout layout;
    layout.topStrip = { 0.0, 0.0, safeWidth, topHeight };
    layout.leftRail = { 0.0, contentTop, leftWidth, contentHeight };
    layout.outputSurface = { leftWidth, contentTop, outputWidth, contentHeight };
    layout.bottomStrip = { 0.0, safeHeight - bottomHeight, safeWidth, bottomHeight };
    layout.outputAreaRatio = (layout.outputSurface.width * layout.outputSurface.height) / (safeWidth * safeHeight);
    return layout;
}
}
