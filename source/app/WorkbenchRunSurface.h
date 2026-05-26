#pragma once

#include "HeadlessRenderRuntime.h"
#include "WorkbenchSession.h"

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct WorkbenchRunSurfaceRow
{
    std::string id;
    std::string label;
    std::string value;
    std::string text;
    std::string tone;
};

struct WorkbenchRunSurface
{
    bool ok = false;
    std::string headline;
    std::vector<std::string> runOrder;
    std::vector<WorkbenchRunSurfaceRow> rows;
};

struct WorkbenchHeadlessRunResult
{
    bool ok = false;
    std::string status;
    std::string statusText;
    std::string error;
    std::string fixtureText;
    WorkbenchRunSurface surface;
    HeadlessRenderRuntimeResult headless;
    std::vector<std::filesystem::path> artifactPaths;
};

WorkbenchRunSurface makeWorkbenchRunSurface (const WorkbenchSessionSnapshot& snapshot);
std::string makeWorkbenchHeadlessRenderFixtureText (const WorkbenchSessionSnapshot& snapshot);
WorkbenchHeadlessRunResult runWorkbenchHeadlessRender (const WorkbenchSessionSnapshot& snapshot,
                                                       const std::filesystem::path& outputDirectory);
}
