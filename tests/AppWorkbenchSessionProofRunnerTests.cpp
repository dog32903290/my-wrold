#include "AppWorkbenchSessionProofRunner.h"
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
                                 / "my-world-app-workbench-session-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::AppWorkbenchSessionProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.snapshot = opened.snapshot;

    const auto result = myworld::runAppWorkbenchSessionProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.reportPath == outputDirectory / "workbench_open_status_report.json", "report path");
    expect (std::filesystem::exists (result.reportPath), "report exists");
    expect (std::string (myworld::appWorkbenchSessionProofDirectoryName()) == "app-workbench-session-proof",
            "stable proof directory");

    const auto report = readTextFile (result.reportPath);
    expectContains (report, "\"kind\": \"workbenchSessionReport\"", "report kind");
    expectContains (report, "\"workSourceStatus\": \"fixture-fallback-no-active-request\"", "source status");
    expectContains (report, "\"graphIOMappingStatus\": \"valid\"", "mapping status");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "app workbench session proof runner ok\n";
    return 0;
}
