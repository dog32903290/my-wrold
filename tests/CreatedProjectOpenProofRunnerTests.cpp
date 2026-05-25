#include "CreatedProjectOpenProofRunner.h"

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
                                 / "my-world-created-project-open-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::CreatedProjectOpenProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    const auto result = myworld::runCreatedProjectOpenProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.statusText == "created project open ready: work.project-open1 -> patch.open1-main source active-work-opened mappings 1/1",
            "status text");
    expect (result.reportPath == outputDirectory / "created_project_open_report.json", "report path");
    expect (std::filesystem::exists (result.reportPath), "report exists");
    expect (std::filesystem::exists (outputDirectory / "created-project" / "myworld.work.json"),
            "created manifest exists");
    expect (std::filesystem::exists (outputDirectory / "created-project" / "patches" / "main.patch.json"),
            "created main patch exists");
    expect (std::string (myworld::createdProjectOpenProofDirectoryName()) == "created-project-open-proof",
            "stable proof directory");

    const auto report = readTextFile (result.reportPath);
    expectContains (report, "\"kind\": \"createdProjectOpenReport\"", "report kind");
    expectContains (report, "\"ok\": true", "report ok");
    expectContains (report, "\"creationStatus\": \"created\"", "creation status");
    expectContains (report, "\"openStatus\": \"ready\"", "open status");
    expectContains (report, "\"workSource\": \"active-work\"", "work source");
    expectContains (report, "\"workSourceStatus\": \"active-work-opened\"", "work source status");
    expectContains (report, "\"documentId\": \"patch.open1-main\"", "document id");
    expectContains (report, "\"graphIOMappingStatus\": \"valid\"", "mapping status");
    expectContains (report, "\"graphIOMappingCount\": 1", "mapping count");
    expectContains (report, "\"validGraphIOMappingCount\": 1", "valid mapping count");
    expectContains (report,
                    "\"statusText\": \"created project open ready: work.project-open1 -> patch.open1-main source active-work-opened mappings 1/1\"",
                    "report status text");

    const auto repeated = myworld::runCreatedProjectOpenProof (request);
    expect (repeated.ok, repeated.error);
    expect (std::filesystem::exists (repeated.reportPath), "repeated report exists");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "created project open proof runner ok\n";
    return 0;
}
