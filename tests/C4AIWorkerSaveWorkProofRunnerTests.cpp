#include "C4AIWorkerSaveWorkProofRunner.h"

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
                                 / "my-world-c4-ai-worker-save-work-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::C4AIWorkerSaveWorkProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    const auto result = myworld::runC4AIWorkerSaveWorkProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.outputDirectory == outputDirectory, "output directory");
    expect (result.reportPath == outputDirectory / "ai_worker_save_work_report.json", "report path");
    expect (std::filesystem::exists (result.reportPath), "report exists");
    expect (std::filesystem::exists (outputDirectory / "work" / "myworld.work.json"), "work manifest copied");
    expect (std::filesystem::exists (outputDirectory / "work" / "patches" / "main.patch.json"), "patch copied");
    expect (result.artifactPaths.size() == 1, "artifact count");
    expect (result.artifactPaths.front() == result.reportPath, "artifact path");

    const auto report = readTextFile (result.reportPath);
    expect (report.find ("\"kind\": \"c4AIWorkerSaveWorkProof\"") != std::string::npos, "report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "report ok");
    expect (report.find ("\"allowedSaveWork\": true") != std::string::npos, "save_work allowed");
    expect (report.find ("\"allowedMoveNode\": true") != std::string::npos, "move_node allowed");
    expect (report.find ("\"operation\": \"save_work\"") != std::string::npos, "operation");
    expect (report.find ("\"status\": \"save-ok commit-pending\"") != std::string::npos, "save status");
    expect (report.find ("\"moveStatus\": \"ok\"") != std::string::npos, "move status");
    expect (report.find ("\"graphCommandLogStatus\": \"move_node\"") != std::string::npos,
            "graph command log");
    expect (report.find ("\"graphMutationApplied\": true") != std::string::npos,
            "graph mutation applied");
    expect (report.find ("\"storageCommandLogStatus\": \"save_work:save-ok commit-pending\"") != std::string::npos,
            "storage command log");
    expect (report.find ("\"aiCommandLogStatus\": \"ai_worker:save_work:save-ok commit-pending\"") != std::string::npos,
            "AI command log");
    expect (report.find ("\"patchReloaded\": true") != std::string::npos, "patch reloaded");
    expect (report.find ("\"saveLogOk\": true") != std::string::npos, "save log ok");
    expect (report.find ("\"saveLogStatus\": \"save-ok commit-pending\"") != std::string::npos,
            "save log status");
    expect (report.find ("\"collaborationLogEntries\": 4") != std::string::npos,
            "collaboration log entries");
    expect (report.find ("\"publicInputEdge\": true") != std::string::npos, "public input edge");
    expect (report.find ("\"publicOutputEdge\": true") != std::string::npos, "public output edge");
    expect (report.find ("\"error\": \"\"") != std::string::npos, "empty error");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "c4 ai worker save_work proof runner ok\n";
    return 0;
}
