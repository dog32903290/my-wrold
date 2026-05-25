#include "AppPaths.h"

namespace myworld
{
namespace
{
bool looksLikeProjectDirectory (const juce::File& directory)
{
    return directory.getChildFile ("CMakeLists.txt").existsAsFile()
           && directory.getChildFile ("source").isDirectory()
           && directory.getChildFile ("fixtures").isDirectory();
}

juce::File parentDirectory (juce::File file, const int levels)
{
    for (int i = 0; i < levels; ++i)
        file = file.getParentDirectory();

    return file;
}

std::vector<std::string> candidatePaths (const juce::String& relativePath)
{
    const auto executableDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
    const auto buildAppRepoRoot = parentDirectory (executableDir, 5);

    return {
        juce::File::getCurrentWorkingDirectory().getChildFile (relativePath).getFullPathName().toStdString(),
        buildAppRepoRoot.getChildFile (relativePath).getFullPathName().toStdString()
    };
}
}

juce::File projectDirectory()
{
    const auto environmentPath = juce::SystemStats::getEnvironmentVariable ("MY_WORLD_PROJECT_DIR", {});

    if (environmentPath.isNotEmpty())
        return juce::File (environmentPath);

    const auto workingDirectory = juce::File::getCurrentWorkingDirectory();

    if (looksLikeProjectDirectory (workingDirectory))
        return workingDirectory;

    return juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
        .getChildFile (juce::String::fromUTF8 ("\xe6\x88\x91\xe7\x9a\x84\xe4\xb8\x96\xe7\x95\x8c"));
}

juce::File proofDumpDirectory (const juce::String& directoryName)
{
    return projectDirectory().getChildFile ("debug").getChildFile (directoryName);
}

juce::File c5VisibleModulePublishDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c5-visible-module-publish");
}

juce::File defaultActiveWorkManifestFile()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c3-active-work").getChildFile ("myworld.work.json");
}

juce::File activeWorkManifestFile()
{
    const auto environmentPath = juce::SystemStats::getEnvironmentVariable ("MY_WORLD_ACTIVE_WORK_MANIFEST", {});

    if (environmentPath.isNotEmpty())
        return juce::File (environmentPath);

    return defaultActiveWorkManifestFile();
}

juce::File performancePreferencesFile()
{
    const auto environmentPath = juce::SystemStats::getEnvironmentVariable ("MY_WORLD_PERFORMANCE_PREFERENCES", {});

    if (environmentPath.isNotEmpty())
        return juce::File (environmentPath);

    return projectDirectory().getChildFile ("debug").getChildFile ("performance-preferences.properties");
}

std::vector<std::string> repoCandidatePaths (const juce::String& relativePath)
{
    return candidatePaths (relativePath);
}

std::vector<std::filesystem::path> proofCandidateRoots()
{
    const auto executableDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();

    return {
        projectDirectory().getFullPathName().toStdString(),
        juce::File::getCurrentWorkingDirectory().getFullPathName().toStdString(),
        parentDirectory (executableDir, 5).getFullPathName().toStdString()
    };
}
}
