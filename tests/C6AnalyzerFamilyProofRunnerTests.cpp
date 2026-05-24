#include "C6AnalyzerFamilyProofRunner.h"

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
                                 / "my-world-c6-analyzer-family-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::C6AnalyzerFamilyProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    const auto result = myworld::runC6AnalyzerFamilyProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.outputDirectory == outputDirectory, "output directory");
    expect (result.reportPath == outputDirectory / "analyzer_family_report.json", "report path");
    expect (std::filesystem::exists (result.reportPath), "report exists");
    expect (result.artifactPaths.size() == 1, "artifact count");
    expect (result.artifactPaths.front() == result.reportPath, "artifact path");

    const auto report = readTextFile (result.reportPath);
    expect (report.find ("\"kind\": \"c6AnalyzerFamilyProof\"") != std::string::npos, "report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "report ok");
    expect (report.find ("\"operation\": \"analyzer_compound_family_seed\"") != std::string::npos,
            "report operation");
    expect (report.find ("\"familyEntryCount\": 2") != std::string::npos, "family entry count");
    expect (report.find ("\"visibleRegistryContainsRawEnergy\": true") != std::string::npos,
            "visible raw energy");
    expect (report.find ("\"runtimeRegistryContainsRawEnergy\": true") != std::string::npos,
            "runtime raw energy");
    expect (report.find ("\"runtimeCoverageStatus\": \"ready\"") != std::string::npos,
            "runtime coverage");
    expect (report.find ("\"createdRawEnergyNode\": true") != std::string::npos,
            "created raw energy node");
    expect (report.find ("\"loudnessStillPresent\": true") != std::string::npos,
            "loudness still present");
    expect (report.find ("\"error\": \"\"") != std::string::npos, "empty error");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "c6 analyzer family proof runner ok\n";
    return 0;
}
