#include "WorkbenchStatusSurfaceProofRunner.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
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
    std::ifstream input (path);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}
}

int main()
{
    const auto outputDirectory = std::filesystem::temp_directory_path()
                                 / "my-world-ui2-workbench-status-surface-proof";
    std::filesystem::remove_all (outputDirectory);

    myworld::WorkbenchStatusSurfaceProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    const auto result = myworld::runWorkbenchStatusSurfaceProof (request);
    expect (result.ok, result.error);
    expect (result.status == "dumped", "status surface proof status");
    expect (result.statusText == "workbench status surface ready: patch.ui2-main rows 6",
            "status surface proof status text");
    expect (result.reportPath == outputDirectory / "workbench_status_surface_report.json",
            "status surface report path");
    expect (std::filesystem::exists (result.reportPath), "status surface report exists");

    const auto report = readTextFile (result.reportPath);
    expect (report.find ("\"kind\": \"workbenchStatusSurfaceReport\"") != std::string::npos,
            "report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "report ok");
    expect (report.find ("\"statusText\": \"workbench status surface ready: patch.ui2-main rows 6\"")
                != std::string::npos,
            "report status text");
    expect (report.find ("\"headline\": \"save_work: save-ok commit-pending\"") != std::string::npos,
            "surface headline");
    expect (report.find ("\"rowCount\": 6") != std::string::npos, "surface row count");
    expect (report.find ("\"documentId\": \"patch.ui2-main\"") != std::string::npos,
            "document id");
    expect (report.find ("\"workSourceStatus\": \"active-work-opened\"") != std::string::npos,
            "work source status");
    expect (report.find ("\"saveStatus\": \"save-ok commit-pending\"") != std::string::npos,
            "save status");
    expect (report.find ("\"graphIOMappingStatus\": \"valid\"") != std::string::npos,
            "mapping status");
    expect (report.find ("\"validGraphIOMappingCount\": 1") != std::string::npos,
            "valid mapping count");
    expect (report.find ("\"commandLogContainsSaveWork\": true") != std::string::npos,
            "save work command evidence");
    expect (report.find ("\"work patch.ui2-main\"") != std::string::npos, "work row text");
    expect (report.find ("\"source active-work-opened\"") != std::string::npos, "source row text");
    expect (report.find ("\"save save-ok commit-pending\"") != std::string::npos, "save row text");
    expect (report.find ("\"mapping valid 1/1\"") != std::string::npos, "mapping row text");
    expect (report.find ("\"proof ui2-ready\"") != std::string::npos, "proof row text");
    expect (report.find ("\"preview preview-ready\"") != std::string::npos, "preview row text");
    expect (report.find ("\"error\": \"\"") != std::string::npos, "success error is empty");

    std::filesystem::remove_all (outputDirectory);

    std::cout << "workbench status surface proof runner ok\n";
    return 0;
}
