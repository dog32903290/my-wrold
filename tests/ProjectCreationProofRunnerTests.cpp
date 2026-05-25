#include "ProjectCreationProofRunner.h"

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
                                 / "my-world-project-creation-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::ProjectCreationProofRunRequest request;
    request.outputDirectory = outputDirectory;

    const auto result = myworld::runProjectCreationProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.reportPath == outputDirectory / "project_creation_report.json", "report path");
    expect (std::filesystem::exists (result.reportPath), "report exists");
    expect (std::filesystem::exists (outputDirectory / "created-project" / "myworld.work.json"),
            "created manifest exists");
    expect (std::filesystem::exists (outputDirectory / "created-project" / "patches" / "main.patch.json"),
            "created main patch exists");
    expect (std::string (myworld::projectCreationProofDirectoryName()) == "project-creation-proof",
            "stable proof directory");

    const auto report = readTextFile (result.reportPath);
    expectContains (report, "\"kind\": \"projectCreationReport\"", "report kind");
    expectContains (report, "\"ok\": true", "report ok");
    expectContains (report, "\"status\": \"created\"", "service status");
    expectContains (report, "\"manifestExists\": true", "manifest existence");
    expectContains (report, "\"patchExists\": true", "patch existence");
    expectContains (report, "\"workId\": \"work.project2-proof\"", "work id");
    expectContains (report, "\"workTitle\": \"PROJECT2 Proof Work\"", "work title");
    expectContains (report, "\"patchId\": \"patch.project2-main\"", "patch id");
    expectContains (report, "\"patchTitle\": \"PROJECT2 Main Patch\"", "patch title");
    expectContains (report, "\"duplicateStatus\": \"already-exists\"", "duplicate status");
    expectContains (report, "createActiveWorkProjectStatus=created", "creation diagnostics status");

    const auto repeated = myworld::runProjectCreationProof (request);
    expect (repeated.ok, repeated.error);
    expect (std::filesystem::exists (repeated.reportPath), "repeated report exists");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "project creation proof runner ok\n";
    return 0;
}
