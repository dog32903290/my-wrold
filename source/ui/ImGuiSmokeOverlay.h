#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct CompoundPatchSpec;
struct NodeSpec;

class ImGuiSmokeOverlay
{
public:
    void initialise();
    void shutdown();
    void beginFrame (int width, int height, float scale, float deltaSeconds);
    void drawSmokePanel (const std::vector<NodeSpec>& nodeSpecs,
                         const CompoundPatchSpec& loudnessCompound,
                         const std::string& shaderStatus);
    void render();
    bool wantsMouse() const;
    void setMousePosition (float x, float y);
    void setMouseButton (int buttonIndex, bool isDown);
    void addMouseWheel (float deltaY);

private:
    bool initialised = false;
    float smokeValue = 0.35f;
    bool loudnessExpanded = false;
};
}
