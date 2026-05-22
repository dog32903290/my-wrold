#pragma once

#include <string>

namespace myworld
{
struct Tooll3SkinRect
{
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
};

struct Tooll3SkinLayout
{
    Tooll3SkinRect topStrip;
    Tooll3SkinRect leftRail;
    Tooll3SkinRect outputSurface;
    Tooll3SkinRect bottomStrip;
    double outputAreaRatio = 0.0;
};

struct Tooll3SkinPolicy
{
    bool outputAsWorkspaceBackground = true;
    bool globalShaderSourceVisible = false;
    bool nodeCanvasIsFramedPanel = false;
    std::string shaderSourceVisibility = "selected_shader_node";
    std::string gridWhenOutputActive = "muted";
};

struct Tooll3SkinColor
{
    int r = 0;
    int g = 0;
    int b = 0;
    int a = 255;
};

struct Tooll3NodeSkin
{
    Tooll3SkinColor fill;
    Tooll3SkinColor label;
    Tooll3SkinColor secondaryLabel;
    Tooll3SkinColor border;
    Tooll3SkinColor inputStrip;
    Tooll3SkinColor outputStrip;
    std::string role;
    double cornerRadius = 0.0;
    double borderWidth = 1.0;
    bool layoutStableOnHover = true;
    bool layoutStableOnSelection = true;
};

struct Tooll3PortSkin
{
    Tooll3SkinColor strip;
    Tooll3SkinColor label;
    double stripWidth = 5.0;
    bool compatibleHighlight = false;
    bool muted = false;
};

struct Tooll3ConnectionSkin
{
    Tooll3SkinColor color;
    double thickness = 3.0;
    double selectedThickness = 5.0;
    double mutedOpacity = 0.32;
    bool compatible = true;
};

struct Tooll3InspectorPolicy
{
    bool panelVisible = false;
    bool shaderSourceEditorVisible = false;
    bool compileStatusVisible = false;
    bool globalShaderSourceVisible = false;
    std::string shaderSourceOwner = "none";
};

struct Tooll3InspectorRowSkin
{
    Tooll3SkinColor stateAccent;
    Tooll3SkinColor label;
    Tooll3SkinColor value;
    std::string stateLabel;
    bool connectionVisible = false;
    bool layoutStableOnStateChange = true;
};

Tooll3SkinPolicy makeTooll3SkinPolicy();
Tooll3SkinLayout makeTooll3SkinLayout (double width, double height);
Tooll3SkinColor makeTooll3TypeColor (const std::string& dataType, const std::string& nodeType = {});
Tooll3NodeSkin makeTooll3NodeSkin (const std::string& nodeType,
                                   const std::string& primaryDataType,
                                   bool selected,
                                   bool hovered);
Tooll3PortSkin makeTooll3PortSkin (const std::string& dataType, bool compatible, bool active);
Tooll3ConnectionSkin makeTooll3ConnectionSkin (const std::string& dataType, bool selected, bool compatible);
Tooll3InspectorPolicy makeTooll3InspectorPolicy (const std::string& nodeType, bool hasSelection);
Tooll3InspectorRowSkin makeTooll3InspectorRowSkin (const std::string& valueState);
}
