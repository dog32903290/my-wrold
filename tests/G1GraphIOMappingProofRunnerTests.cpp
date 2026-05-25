#include "G1GraphIOMappingProofRunner.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (condition)
        return;

    std::cerr << "FAIL: " << message << '\n';
    std::exit (1);
}

std::string readTextFile (const std::filesystem::path& path)
{
    std::ifstream input (path, std::ios::binary);
    return { std::istreambuf_iterator<char> (input), std::istreambuf_iterator<char>() };
}

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}
}

int main()
{
    const auto outputDirectory = std::filesystem::temp_directory_path()
                                 / "my-world-g1-graph-io-mapping-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::G1GraphIOMappingProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };
    request.sourceValue = 0.72;

    const auto result = myworld::runG1GraphIOMappingProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.outputDirectory == outputDirectory, "output directory");
    expect (result.reportPath == outputDirectory / "graph_io_mapping_report.json", "report path");
    expect (std::filesystem::exists (result.reportPath), "report exists");
    expect (result.artifactPaths.size() == 1, "artifact count");
    expect (result.artifactPaths.front() == result.reportPath, "report artifact");

    const auto report = readTextFile (result.reportPath);
    expectContains (report, "\"kind\": \"graphIOMappingReport\"", "report kind");
    expectContains (report, "\"ok\": true", "report ok");
    expectContains (report, "\"status\": \"mapped\"", "report status");
    expectContains (report, "\"mappingId\": \"uniform.loudness\"", "mapping id");
    expectContains (report, "\"sourceEndpoint\": \"compound.loudness.out\"", "source endpoint");
    expectContains (report, "\"targetKind\": \"shader.uniform\"", "target kind");
    expectContains (report, "\"uniformName\": \"u_loudness\"", "uniform");
    expectContains (report, "\"inputValue\": 0.720000", "input value");
    expectContains (report, "\"normalizedValue\": 0.720000", "normalized value");
    expectContains (report, "\"diagnostics\": []", "no diagnostics");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "g1 graph io mapping proof runner ok\n";
    return 0;
}
