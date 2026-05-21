#include "ImGuiSmokeOverlay.h"

#include "NodeSpec.h"

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>

namespace myworld
{
void ImGuiSmokeOverlay::initialise()
{
    if (initialised)
        return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init ("#version 150");
    initialised = true;
}

void ImGuiSmokeOverlay::shutdown()
{
    if (! initialised)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    initialised = false;
}

void ImGuiSmokeOverlay::beginFrame (int width, int height, float scale, float deltaSeconds)
{
    if (! initialised)
        return;

    const auto safeScale = scale > 0.0f ? scale : 1.0f;
    auto& io = ImGui::GetIO();
    io.DisplaySize = ImVec2 (static_cast<float> (width) / safeScale,
                             static_cast<float> (height) / safeScale);
    io.DisplayFramebufferScale = ImVec2 (safeScale, safeScale);
    io.DeltaTime = deltaSeconds > 0.0f ? deltaSeconds : 1.0f / 60.0f;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}

void ImGuiSmokeOverlay::drawSmokePanel (const std::vector<NodeSpec>& nodeSpecs, const std::string& shaderStatus)
{
    if (! initialised)
        return;

    ImGui::SetNextWindowPos (ImVec2 (16.0f, 16.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize (ImVec2 (360.0f, 220.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin ("A0 ImGui Smoke");
    ImGui::TextUnformatted ("Immediate-mode UI is active.");
    ImGui::Text ("Seed node specs: %d", static_cast<int> (nodeSpecs.size()));
    ImGui::SliderFloat ("smoke value", &smokeValue, 0.0f, 1.0f);
    ImGui::Separator();
    ImGui::TextWrapped ("%s", shaderStatus.c_str());
    ImGui::End();
}

void ImGuiSmokeOverlay::render()
{
    if (! initialised)
        return;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData (ImGui::GetDrawData());
}

bool ImGuiSmokeOverlay::wantsMouse() const
{
    return initialised && ImGui::GetIO().WantCaptureMouse;
}

void ImGuiSmokeOverlay::setMousePosition (float x, float y)
{
    if (initialised)
        ImGui::GetIO().AddMousePosEvent (x, y);
}

void ImGuiSmokeOverlay::setMouseButton (int buttonIndex, bool isDown)
{
    if (initialised)
        ImGui::GetIO().AddMouseButtonEvent (buttonIndex, isDown);
}

void ImGuiSmokeOverlay::addMouseWheel (float deltaY)
{
    if (initialised)
        ImGui::GetIO().AddMouseWheelEvent (0.0f, deltaY);
}
}
