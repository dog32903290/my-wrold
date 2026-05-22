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

Tooll3SkinPolicy makeTooll3SkinPolicy();
Tooll3SkinLayout makeTooll3SkinLayout (double width, double height);
}
