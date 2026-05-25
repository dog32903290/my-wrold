#include "APP1WorkbenchSessionProofRunner.h"

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
                                 / "my-world-app1-workbench-session-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::APP1WorkbenchSessionProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };
    request.previewStatus = "preview-ready";
    request.proofStatus = "g1-report-dumped";
    request.saveStatus = "dirty";
    request.dirty = true;

    const auto result = myworld::runAPP1WorkbenchSessionProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.outputDirectory == outputDirectory, "output directory");
    expect (result.reportPath == outputDirectory / "workbench_session_report.json", "report path");
    expect (std::filesystem::exists (result.reportPath), "report exists");
    expect (result.artifactPaths.size() == 1, "artifact count");
    expect (result.artifactPaths.front() == result.reportPath, "report artifact");

    const auto report = readTextFile (result.reportPath);
    expectContains (report, "\"kind\": \"workbenchSessionReport\"", "report kind");
    expectContains (report, "\"ok\": true", "report ok");
    expectContains (report, "\"status\": \"ready\"", "report status");
    expectContains (report, "\"documentId\": \"patch.c2-main\"", "document id");
    expectContains (report, "\"documentTitle\": \"C2 Main\"", "document title");
    expectContains (report, "\"dirty\": true", "dirty");
    expectContains (report, "\"editorNodeCount\": 5", "editor node count");
    expectContains (report, "\"runtimeNodeCount\": 5", "runtime node count");
    expectContains (report, "\"graphIOMappingCount\": 1", "mapping count");
    expectContains (report, "\"graphIOMappingStatus\": \"valid\"", "mapping status");
    expectContains (report, "\"previewStatus\": \"preview-ready\"", "preview status");
    expectContains (report, "\"proofStatus\": \"g1-report-dumped\"", "proof status");
    expectContains (report, "\"saveStatus\": \"dirty\"", "save status");
    expectContains (report, "\"diagnostics\": []", "diagnostics");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "app1 workbench session proof runner ok\n";
    return 0;
}
