#pragma once

#include "AudioAnalyzerState.h"
#include "PerformancePreferences.h"

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
struct A1AudioProofRunRequest
{
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
    AudioAnalyzerSnapshot snapshot;
    double sampleRate = 0.0;
    int bufferSize = 0;
    PerformancePreferences preferences;
};

struct A1AudioProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* a1AudioProofDisplayName();
const char* a1AudioProofDirectoryName();

A1AudioProofRunResult runA1AudioProof (const A1AudioProofRunRequest& request);
}
