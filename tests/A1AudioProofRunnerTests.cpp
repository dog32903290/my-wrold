#include "A1AudioProofRunner.h"

#include "PerformancePreferences.h"

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
                                 / "my-world-a1-audio-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::AudioAnalyzerSnapshot snapshot;
    snapshot.rms = 0.25f;
    snapshot.peak = 0.5f;
    snapshot.loudness = 0.25f;
    snapshot.gate = 1.0f;
    snapshot.confidence = 1.0f;
    snapshot.active = true;
    snapshot.sampleCounter = 512;

    myworld::A1AudioProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };
    request.snapshot = snapshot;
    request.sampleRate = 48000.0;
    request.bufferSize = 512;
    request.preferences = myworld::makeDefaultPerformancePreferences();

    const auto result = myworld::runA1AudioProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.outputDirectory == outputDirectory, "output directory");
    expect (result.artifactPaths.size() == 4, "artifact count");
    expect (std::filesystem::exists (outputDirectory / "audio_stats.json"), "audio stats exists");
    expect (std::filesystem::exists (outputDirectory / "loudness_compound.json"), "compound exists");
    expect (std::filesystem::exists (outputDirectory / "loudness_runtime_execution.json"),
            "runtime execution exists");
    expect (std::filesystem::exists (outputDirectory / "loudness_runtime_bridge.json"),
            "runtime bridge exists");

    const auto stats = readTextFile (outputDirectory / "audio_stats.json");
    expect (stats.find ("\"sampleRate\": 48000") != std::string::npos, "sample rate");
    expect (stats.find ("\"bufferSize\": 512") != std::string::npos, "buffer size");
    expect (stats.find ("\"rms\": 0.250000") != std::string::npos, "rms");
    expect (stats.find ("\"peak\": 0.500000") != std::string::npos, "peak");
    expect (stats.find ("\"loudness\": 0.250000") != std::string::npos, "loudness");
    expect (stats.find ("\"active\": true") != std::string::npos, "active");
    expect (stats.find ("\"analysisGain\": 1.000") != std::string::npos, "analysis gain");
    expect (stats.find ("\"sampleCounter\": 512") != std::string::npos, "sample counter");

    const auto runtimeExecution = readTextFile (outputDirectory / "loudness_runtime_execution.json");
    expect (runtimeExecution.find ("\"kind\": \"runtimeExecution\"") != std::string::npos,
            "runtime execution kind");
    expect (runtimeExecution.find ("\"nodeType\": \"compound.loudness\"") != std::string::npos,
            "compound loudness entry");
    expect (runtimeExecution.find ("\"status\": \"computed\"") != std::string::npos,
            "computed status");
    expect (runtimeExecution.find ("\"childId\": \"mono_mix\"") != std::string::npos,
            "mono mix child");

    const auto bridge = readTextFile (outputDirectory / "loudness_runtime_bridge.json");
    expect (bridge.find ("\"kind\": \"loudnessRuntimeBridge\"") != std::string::npos,
            "runtime bridge kind");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "a1 audio proof runner ok\n";
    return 0;
}
