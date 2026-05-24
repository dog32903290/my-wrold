#include "V1ShaderProofArtifacts.h"

#include "CompoundPatch.h"
#include "GraphContract.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}

std::string readTextFile (const std::filesystem::path& path)
{
    std::ifstream input (path, std::ios::binary);
    return { std::istreambuf_iterator<char> (input), std::istreambuf_iterator<char>() };
}
}

int main()
{
    const auto outputDirectory = std::filesystem::temp_directory_path()
                                 / "my-world-v1-shader-proof-artifacts-test";
    std::filesystem::remove_all (outputDirectory);

    juce::Image frame (juce::Image::ARGB, 2, 2, true);
    {
        juce::Graphics g (frame);
        g.fillAll (juce::Colours::black);
        g.setColour (juce::Colours::white);
        g.fillRect (0, 0, 1, 1);
    }

    myworld::V1ShaderProofArtifactRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };
    request.graph = myworld::makeDefaultShaderOutputGraph();
    request.frameImage = frame;
    request.viewportWidth = 2;
    request.viewportHeight = 2;
    request.timeSeconds = 1.25;
    request.frameIndex = 7;
    request.backendName = "test-backend";
    request.backendStatus = "ok";
    request.loudnessCompound = myworld::makeLoudnessCompoundPatchSpec();

    const auto result = myworld::writeV1ShaderProofArtifacts (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.outputDirectory == outputDirectory, "output directory");
    expect (result.artifactPaths.size() == 14, "artifact count");
    expect (std::filesystem::exists (outputDirectory / "frame.png"), "frame exists");
    expect (std::filesystem::file_size (outputDirectory / "frame.png") > 0, "frame nonempty");

    for (const auto& fileName : {
             "cook_order.json",
             "node_stats.json",
             "loudness_compound.json",
             "runtime_registry.json",
             "runtime_op_catalog.json",
             "runtime_op_coverage.json",
             "runtime_ui_diagnostics.json",
             "runtime_dry_run.json",
             "runtime_execution.json",
             "runtime_missing_runtimeop_registry.json",
             "runtime_missing_runtimeop_coverage.json",
             "runtime_missing_runtimeop_dry_run.json",
             "runtime_missing_runtimeop_execution.json" })
    {
        expect (std::filesystem::exists (outputDirectory / fileName), std::string (fileName) + " exists");
    }

    const auto nodeStats = readTextFile (outputDirectory / "node_stats.json");
    expect (nodeStats.find ("\"version\": 1") != std::string::npos, "node stats version");
    expect (nodeStats.find ("\"frameIndex\": 7") != std::string::npos, "frame index");
    expect (nodeStats.find ("\"renderer\": \"test-backend\"") != std::string::npos, "renderer");

    const auto runtimeExecution = readTextFile (outputDirectory / "runtime_execution.json");
    expect (runtimeExecution.find ("\"kind\": \"runtimeExecution\"") != std::string::npos,
            "runtime execution kind");
    expect (runtimeExecution.find ("\"nodeType\": \"compound.loudness\"") != std::string::npos,
            "runtime execution compound");
    expect (runtimeExecution.find ("\"status\": \"computed\"") != std::string::npos,
            "runtime execution computed");

    const auto missingExecution = readTextFile (outputDirectory / "runtime_missing_runtimeop_execution.json");
    expect (missingExecution.find ("\"status\": \"missing-runtime-op\"") != std::string::npos,
            "missing runtime-op execution");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "v1 shader proof artifacts ok\n";
    return 0;
}
