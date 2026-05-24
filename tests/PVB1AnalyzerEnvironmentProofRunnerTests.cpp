#include "PVB1AnalyzerEnvironmentProofRunner.h"

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
                                 / "my-world-pv-b1-analyzer-environment-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::PVB1AnalyzerEnvironmentProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    const auto result = myworld::runPVB1AnalyzerEnvironmentProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.outputDirectory == outputDirectory, "output directory");
    expect (result.reportPath == outputDirectory / "analyzer_environment_report.json", "report path");
    expect (std::filesystem::exists (result.reportPath), "report exists");
    expect (result.artifactPaths.size() == 1, "artifact count");
    expect (result.artifactPaths.front() == result.reportPath, "artifact path");

    const auto report = readTextFile (result.reportPath);
    expect (report.find ("\"kind\": \"pvB1AnalyzerEnvironmentProof\"") != std::string::npos, "report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "report ok");
    expect (report.find ("\"operation\": \"pv_b1_analyzer_environment_promotion\"") != std::string::npos,
            "report operation");
    expect (report.find ("\"loadedModuleNodeCount\": 8") != std::string::npos, "loaded node count");
    expect (report.find ("\"visibleCatalogContainsAllRequired\": true") != std::string::npos,
            "visible catalog proof");
    expect (report.find ("\"runtimeDiagnosticsReadyForAllRequired\": true") != std::string::npos,
            "runtime diagnostics proof");
    expect (report.find ("\"createdNodeCount\": 8") != std::string::npos, "created node count");
    expect (report.find ("\"error\": \"\"") != std::string::npos, "empty error");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "pv b1 analyzer environment proof runner ok\n";
    return 0;
}
