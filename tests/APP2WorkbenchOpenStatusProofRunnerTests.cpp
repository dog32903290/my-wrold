#include "APP2WorkbenchOpenStatusProofRunner.h"
#include "WorkbenchSessionOpenStatus.h"

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
    myworld::WorkbenchSessionOpenStatusRequest openRequest;
    openRequest.candidateRoots = { std::filesystem::current_path() };
    openRequest.proofStatus = "g1-ready";
    const auto opened = myworld::openCurrentWorkbenchSession (openRequest);
    expect (opened.ok, opened.error);

    const auto outputDirectory = std::filesystem::temp_directory_path()
                                 / "my-world-app2-workbench-open-status-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::APP2WorkbenchOpenStatusProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.snapshot = opened.snapshot;

    const auto result = myworld::runAPP2WorkbenchOpenStatusProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.reportPath == outputDirectory / "workbench_open_status_report.json", "report path");
    expect (std::filesystem::exists (result.reportPath), "report exists");
    expect (result.artifactPaths.size() == 1, "artifact count");

    const auto report = readTextFile (result.reportPath);
    expectContains (report, "\"kind\": \"workbenchSessionReport\"", "report kind");
    expectContains (report, "\"ok\": true", "report ok");
    expectContains (report, "\"workSource\": \"fixture\"", "work source");
    expectContains (report, "\"documentId\": \"patch.c2-main\"", "document id");
    expectContains (report, "\"graphIOMappingCount\": 1", "mapping count");
    expectContains (report, "\"graphIOMappingStatus\": \"valid\"", "mapping status");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "app2 workbench open status proof runner ok\n";
    return 0;
}
