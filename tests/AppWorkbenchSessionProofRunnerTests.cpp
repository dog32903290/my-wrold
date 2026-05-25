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
    expect (opened.snapshot.diagnostics.empty(), "work diagnostics should not block session");
    expect (! opened.snapshot.workDiagnostics.empty(), "work diagnostics present");

    const auto outputDirectory = std::filesystem::temp_directory_path()
                                 / "my-world-app-workbench-session-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::AppWorkbenchSessionProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.snapshot = opened.snapshot;
    request.activeWorkPreparation.ok = true;
    request.activeWorkPreparation.status = "default-active-work-prepared";
    request.activeWorkPreparation.workManifestPath = "/tmp/my-world-active-work/myworld.work.json";
    request.activeWorkPreparation.diagnostics = {
        "activeWorkPreparationStatus=default-active-work-prepared",
        "activeWorkPreparationManifestPath=/tmp/my-world-active-work/myworld.work.json"
    };

    const auto result = myworld::runAppWorkbenchSessionProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.reportPath == outputDirectory / "workbench_open_status_report.json", "report path");
    expect (std::filesystem::exists (result.reportPath), "report exists");
    const auto preparationReportPath = outputDirectory / "active_work_preparation_report.json";
    expect (std::filesystem::exists (preparationReportPath), "preparation report exists");
    expect (std::string (myworld::appWorkbenchSessionProofDirectoryName()) == "app-workbench-session-proof",
            "stable proof directory");

    const auto report = readTextFile (result.reportPath);
    expectContains (report, "\"kind\": \"workbenchSessionReport\"", "report kind");
    expectContains (report, "\"workSourceStatus\": \"fixture-fallback-no-active-request\"", "source status");
    expectContains (report, "\"workDiagnostics\": [", "work diagnostics field");
    expectContains (report,
                    "workSourceStatus=fixture-fallback-no-active-request",
                    "work diagnostics source status");
    expectContains (report, "\"graphIOMappingStatus\": \"valid\"", "mapping status");

    const auto preparationReport = readTextFile (preparationReportPath);
    expectContains (preparationReport, "\"kind\": \"activeWorkPreparationReport\"", "preparation report kind");
    expectContains (preparationReport, "\"ok\": true", "preparation report ok");
    expectContains (preparationReport,
                    "\"status\": \"default-active-work-prepared\"",
                    "preparation report status");
    expectContains (preparationReport,
                    "\"workManifestPath\": \"/tmp/my-world-active-work/myworld.work.json\"",
                    "preparation report manifest path");
    expectContains (preparationReport,
                    "activeWorkPreparationStatus=default-active-work-prepared",
                    "preparation diagnostics status");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "app workbench session proof runner ok\n";
    return 0;
}
