#include "C6AIRepairLoopProofRunner.h"

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
                                 / "my-world-c6-ai-repair-loop-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::C6AIRepairLoopProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    const auto result = myworld::runC6AIRepairLoopProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.outputDirectory == outputDirectory, "output directory");
    expect (result.reportPath == outputDirectory / "ai_repair_loop_report.json", "report path");
    expect (std::filesystem::exists (result.reportPath), "report exists");
    expect (result.artifactPaths.size() == 1, "artifact count");
    expect (result.artifactPaths.front() == result.reportPath, "artifact path");

    const auto report = readTextFile (result.reportPath);
    expect (report.find ("\"kind\": \"c6AIRepairLoopProof\"") != std::string::npos, "report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "report ok");
    expect (report.find ("\"operation\": \"ai_repair_loop\"") != std::string::npos, "report operation");
    expect (report.find ("\"repairId\": \"c6.2-ai-repair-loop\"") != std::string::npos, "repair id");
    expect (report.find ("\"status\": \"repaired\"") != std::string::npos, "repair status");
    expect (report.find ("\"attemptsRun\": 2") != std::string::npos, "attempts run");
    expect (report.find ("\"maxAttempts\": 3") != std::string::npos, "max attempts");
    expect (report.find ("\"firstAttemptStatus\": \"failed\"") != std::string::npos,
            "first attempt failed");
    expect (report.find ("\"successfulAttemptIndex\": 2") != std::string::npos,
            "successful attempt index");
    expect (report.find ("\"finalCommandLogStatus\": \"ai_worker_repair_loop:repaired\"") != std::string::npos,
            "final command log");
    expect (report.find ("\"graphMutationApplied\": true") != std::string::npos,
            "graph mutation applied");
    expect (report.find ("\"collaborationLogEntries\": 7") != std::string::npos,
            "collaboration log entries");
    expect (report.find ("\"error\": \"\"") != std::string::npos, "empty error");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "c6 ai repair loop proof runner ok\n";
    return 0;
}
