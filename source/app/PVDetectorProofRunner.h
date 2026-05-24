#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
enum class PVDetectorProofKind
{
    attack,
    density,
    silence,
    sustain,
    residue,
    aggregatePressure
};

struct PVDetectorProofRunRequest
{
    PVDetectorProofKind kind = PVDetectorProofKind::attack;
    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> candidateRoots;
};

struct PVDetectorProofRunResult
{
    bool ok = false;
    std::string status;
    std::string error;
    std::filesystem::path outputDirectory;
    std::filesystem::path reportPath;
    std::vector<std::filesystem::path> artifactPaths;
};

const char* pvDetectorProofDisplayName (PVDetectorProofKind kind);
const char* pvDetectorProofDirectoryName (PVDetectorProofKind kind);

PVDetectorProofRunResult runPVDetectorProof (const PVDetectorProofRunRequest& request);
}
