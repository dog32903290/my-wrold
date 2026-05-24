#include "C3SaveWorkProofRunner.h"

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
                                 / "my-world-c3-save-work-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::C3SaveWorkProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    const auto result = myworld::runC3SaveWorkProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.outputDirectory == outputDirectory, "output directory");
    expect (result.reportPath == outputDirectory / "save_work_report.json", "report path");
    expect (std::filesystem::exists (result.reportPath), "report exists");
    expect (std::filesystem::exists (outputDirectory / "work" / "myworld.work.json"), "work manifest copied");
    expect (std::filesystem::exists (outputDirectory / "work" / "patches" / "main.patch.json"), "patch copied");
    expect (result.artifactPaths.size() == 1, "artifact count");
    expect (result.artifactPaths.front() == result.reportPath, "artifact path");

    const auto report = readTextFile (result.reportPath);
    expect (report.find ("\"kind\": \"c3SaveWorkProof\"") != std::string::npos, "report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "report ok");
    expect (report.find ("\"source\": \"PatchDocument\"") != std::string::npos, "source");
    expect (report.find ("\"usesInteractionState\": false") != std::string::npos,
            "no interaction state dependency");
    expect (report.find ("\"saveStatus\": \"save-ok commit-pending\"") != std::string::npos,
            "save status");
    expect (report.find ("\"commandLogStatus\": \"save_work:save-ok commit-pending\"") != std::string::npos,
            "command log status");
    expect (report.find ("\"saveLogOk\": true") != std::string::npos, "save log ok");
    expect (report.find ("\"saveLogEntries\": 1") != std::string::npos, "save log entries");
    expect (report.find ("\"saveLogStatus\": \"save-ok commit-pending\"") != std::string::npos,
            "save log status");
    expect (report.find ("\"commitStatus\": \"not-started\"") != std::string::npos,
            "commit status");
    expect (report.find ("\"editorNodeCount\": 5") != std::string::npos, "editor node count");
    expect (report.find ("\"runtimeNodeCount\": 5") != std::string::npos, "runtime node count");
    expect (report.find ("\"publicInputEdge\": true") != std::string::npos, "public input edge");
    expect (report.find ("\"publicOutputEdge\": true") != std::string::npos, "public output edge");
    expect (report.find ("\"matches\": true") != std::string::npos, "expanded layout");
    expect (report.find ("\"error\": \"\"") != std::string::npos, "empty error");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "c3 save_work proof runner ok\n";
    return 0;
}
