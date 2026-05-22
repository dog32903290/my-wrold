#include "Tooll3SkinContract.h"

#include <algorithm>

namespace myworld
{
namespace
{
bool startsWith (const std::string& value, const std::string& prefix)
{
    return value.rfind (prefix, 0) == 0;
}

Tooll3SkinColor color (int r, int g, int b, int a)
{
    return { r, g, b, a };
}

Tooll3SkinColor withAlpha (Tooll3SkinColor source, int alpha)
{
    source.a = std::clamp (alpha, 0, 255);
    return source;
}

Tooll3SkinColor brighten (Tooll3SkinColor source, int amount)
{
    source.r = std::clamp (source.r + amount, 0, 255);
    source.g = std::clamp (source.g + amount, 0, 255);
    source.b = std::clamp (source.b + amount, 0, 255);
    return source;
}

std::string skinRoleFor (const std::string& nodeType, const std::string& dataType)
{
    if (startsWith (nodeType, "shader."))
        return "shader";
    if (startsWith (nodeType, "output."))
        return "output";
    if (startsWith (nodeType, "compound."))
        return "compound";
    if (startsWith (nodeType, "audio.") || startsWith (dataType, "audio."))
        return "audio";
    if (startsWith (nodeType, "analyzer.") || startsWith (nodeType, "signal.") || startsWith (dataType, "signal."))
        return "signal";
    if (startsWith (dataType, "texture."))
        return "texture";
    if (startsWith (dataType, "command."))
        return "command";
    if (startsWith (dataType, "text.") || startsWith (nodeType, "string."))
        return "text";
    if (startsWith (dataType, "geometry.") || startsWith (nodeType, "mesh."))
        return "geometry";
    if (startsWith (dataType, "point."))
        return "point";
    if (startsWith (dataType, "field."))
        return "field";
    if (startsWith (dataType, "material.") || startsWith (nodeType, "material."))
        return "material";

    return "value";
}
}

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

Tooll3SkinColor makeTooll3TypeColor (const std::string& dataType, const std::string& nodeType)
{
    const auto role = skinRoleFor (nodeType, dataType);

    if (role == "shader")
        return color (116, 74, 154, 194);
    if (role == "output" || role == "command")
        return color (48, 156, 170, 202);
    if (role == "compound")
        return color (110, 78, 92, 190);
    if (role == "audio")
        return color (35, 132, 148, 194);
    if (role == "signal")
        return color (60, 108, 166, 194);
    if (role == "texture")
        return color (154, 67, 134, 196);
    if (role == "text")
        return color (72, 134, 88, 190);
    if (role == "geometry")
        return color (142, 105, 72, 190);
    if (role == "point" || role == "field")
        return color (140, 78, 92, 190);
    if (role == "material")
        return color (118, 92, 148, 190);

    return color (82, 88, 96, 188);
}

Tooll3NodeSkin makeTooll3NodeSkin (const std::string& nodeType,
                                   const std::string& primaryDataType,
                                   bool selected,
                                   bool hovered)
{
    Tooll3NodeSkin skin;
    skin.role = skinRoleFor (nodeType, primaryDataType);
    skin.fill = makeTooll3TypeColor (primaryDataType, nodeType);
    skin.label = color (236, 240, 248, 255);
    skin.secondaryLabel = color (172, 186, 204, 255);
    skin.border = selected ? color (255, 255, 255, 245)
                           : (hovered ? color (172, 188, 204, 220) : color (0, 0, 0, 210));
    skin.inputStrip = withAlpha (makeTooll3TypeColor (primaryDataType), 230);
    skin.outputStrip = withAlpha (makeTooll3TypeColor (primaryDataType, nodeType), 245);
    skin.cornerRadius = 0.0;
    skin.borderWidth = selected ? 2.0 : 1.0;
    skin.layoutStableOnHover = true;
    skin.layoutStableOnSelection = true;

    if (hovered && ! selected)
        skin.fill = brighten (skin.fill, 12);

    return skin;
}

Tooll3PortSkin makeTooll3PortSkin (const std::string& dataType, bool compatible, bool active)
{
    Tooll3PortSkin skin;
    skin.strip = makeTooll3TypeColor (dataType);
    skin.label = color (202, 214, 228, compatible ? 235 : 135);
    skin.stripWidth = 5.0;
    skin.compatibleHighlight = compatible && active;
    skin.muted = ! compatible;

    if (! compatible)
        skin.strip = withAlpha (skin.strip, static_cast<int> (skin.strip.a * 0.32));
    else if (active)
        skin.strip = withAlpha (brighten (skin.strip, 28), 255);

    return skin;
}

Tooll3ConnectionSkin makeTooll3ConnectionSkin (const std::string& dataType, bool selected, bool compatible)
{
    Tooll3ConnectionSkin skin;
    skin.color = makeTooll3TypeColor (dataType);
    skin.thickness = selected ? skin.selectedThickness : 3.0;
    skin.compatible = compatible;

    if (! compatible)
        skin.color = withAlpha (skin.color, static_cast<int> (skin.color.a * skin.mutedOpacity));
    else if (selected)
        skin.color = withAlpha (brighten (skin.color, 36), 255);

    return skin;
}

Tooll3InspectorPolicy makeTooll3InspectorPolicy (const std::string& nodeType, bool hasSelection)
{
    Tooll3InspectorPolicy policy;
    policy.panelVisible = hasSelection;

    if (! hasSelection)
        return policy;

    if (startsWith (nodeType, "shader."))
    {
        policy.shaderSourceEditorVisible = true;
        policy.compileStatusVisible = true;
        policy.shaderSourceOwner = "selected_shader_node";
    }

    return policy;
}

Tooll3InspectorRowSkin makeTooll3InspectorRowSkin (const std::string& valueState)
{
    Tooll3InspectorRowSkin skin;
    skin.label = color (212, 220, 232, 235);
    skin.value = color (170, 184, 202, 230);
    skin.stateLabel = valueState.empty() ? "default" : valueState;
    skin.layoutStableOnStateChange = true;

    if (skin.stateLabel == "manual")
    {
        skin.stateAccent = color (104, 154, 218, 230);
    }
    else if (skin.stateLabel == "connected")
    {
        skin.stateAccent = color (92, 198, 162, 235);
        skin.connectionVisible = true;
    }
    else if (skin.stateLabel == "animated")
    {
        skin.stateAccent = color (204, 132, 220, 235);
        skin.connectionVisible = true;
    }
    else
    {
        skin.stateLabel = "default";
        skin.stateAccent = color (92, 96, 106, 190);
    }

    return skin;
}
}
