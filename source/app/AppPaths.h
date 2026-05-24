#pragma once

#include <juce_core/juce_core.h>

#include <filesystem>
#include <string>
#include <vector>

namespace myworld
{
juce::File projectDirectory();
juce::File proofDumpDirectory (const juce::String& directoryName);
juce::File c5VisibleModulePublishDirectory();
juce::File defaultActiveWorkManifestFile();
juce::File activeWorkManifestFile();
std::vector<std::string> repoCandidatePaths (const juce::String& relativePath);
std::vector<std::filesystem::path> proofCandidateRoots();
}
