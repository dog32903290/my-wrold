#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
std::string writeProofTextFile (const std::filesystem::path& path, const std::string& text);
std::string clearProofDirectoryIfExists (const std::filesystem::path& directory);
std::string createProofDirectoryIfMissing (const std::filesystem::path& directory);

std::vector<std::filesystem::path> proofCandidatePaths (const std::vector<std::filesystem::path>& roots,
                                                        std::filesystem::path relativePath);

bool copyFirstProofCandidate (const std::vector<std::filesystem::path>& roots,
                              std::filesystem::path relativePath,
                              const std::filesystem::path& target,
                              std::string& error);
}
