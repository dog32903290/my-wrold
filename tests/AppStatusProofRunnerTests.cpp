#include "AppStatusProofRunner.h"

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
    const auto outputDirectory = std::filesystem::temp_directory_path() / "my-world-status2-app-status-proof";
    std::filesystem::remove_all (outputDirectory);

    myworld::AppStatusProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    const auto result = myworld::runAppStatusProof (request);
    expect (result.ok, result.error);
    expect (result.status == "dumped", "app status proof status");
    expect (result.statusText == "app status ready: patch.status2-main source active-work-opened save save-ok commit-pending",
            "app status text");
    expect (result.reportPath == outputDirectory / "app_status_report.json", "app status report path");
    expect (std::filesystem::exists (result.reportPath), "app status report exists");

    const auto report = readTextFile (result.reportPath);
    expect (report.find ("\"kind\": \"appStatusReport\"") != std::string::npos, "report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "report ok");
    expect (report.find ("\"initialStatusText\": \"workbench blocked: no session\"") != std::string::npos,
            "initial status text");
    expect (report.find ("\"openedStatusText\": \"workbench ready patch.status2-main source active-work-opened mappings 1/1\"") != std::string::npos,
            "opened status text");
    expect (report.find ("\"savedStatusText\": \"save_work: save-ok commit-pending\"") != std::string::npos,
            "saved status text");
    expect (report.find ("\"documentId\": \"patch.status2-main\"") != std::string::npos,
            "document id");
    expect (report.find ("\"workSourceStatus\": \"active-work-opened\"") != std::string::npos,
            "work source status");
    expect (report.find ("\"saveStatus\": \"save-ok commit-pending\"") != std::string::npos,
            "save status");
    expect (report.find ("\"dirtyAfterSave\": false") != std::string::npos,
            "dirty after save");
    expect (report.find ("\"graphIOMappingStatus\": \"valid\"") != std::string::npos,
            "mapping status");
    expect (report.find ("\"validGraphIOMappingCount\": 1") != std::string::npos,
            "valid mapping count");
    expect (report.find ("\"error\": \"\"") != std::string::npos,
            "success error is empty");

    std::filesystem::remove_all (outputDirectory);

    std::cout << "app status proof runner ok\n";
    return 0;
}
