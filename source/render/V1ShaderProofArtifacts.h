#pragma once

#include "CompoundPatch.h"
#include "GraphContract.h"
#include "RuntimeRegistry.h"

#include <juce_gui_extra/juce_gui_extra.h>

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct V1ShaderProofArtifactRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
    GraphContract graph;
    juce::Image frameImage;
    int viewportWidth = 0;
    int viewportHeight = 0;
    double timeSeconds = 0.0;
    unsigned int frameIndex = 0;
    std::string backendName;
    std::string backendStatus;
    juce::Image quietFrameImage;
    juce::Image loudFrameImage;
    float quietLoudness = 0.0f;
    float loudLoudness = 0.0f;
    CompoundPatchSpec loudnessCompound;
    std::vector<RuntimeOpModuleDiagnostic> runtimeOpDiagnostics;
};

struct V1ShaderProofArtifactResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> artifactPaths;
};

V1ShaderProofArtifactResult writeV1ShaderProofArtifacts (const V1ShaderProofArtifactRequest& request);
}
