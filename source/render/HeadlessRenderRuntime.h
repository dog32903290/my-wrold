#pragma once

#include <string>

namespace myworld
{
struct HeadlessRenderRuntimeResult
{
    bool ok = false;
    std::string error;
    std::string textureSummaryPath;
    std::string cookOrderPath;
    std::string nodeStatsPath;
    std::string errorsPath;
};

HeadlessRenderRuntimeResult runHeadlessRenderRuntimeProof (const std::string& fixturePath,
                                                           const std::string& outputDirectory);

HeadlessRenderRuntimeResult runHeadlessRenderRuntimeProofText (const std::string& fixtureText,
                                                               const std::string& outputDirectory);
}
