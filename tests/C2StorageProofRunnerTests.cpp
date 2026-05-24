#include "C2StorageProofRunner.h"

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
                                 / "my-world-c2-storage-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::C2StorageProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    const auto result = myworld::runC2StorageProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.outputDirectory == outputDirectory, "output directory");
    expect (result.reportPath == outputDirectory / "reload_report.json", "report path");
    expect (std::filesystem::exists (result.reportPath), "report exists");
    expect (std::filesystem::exists (outputDirectory / "saved_main.patch.json"), "saved patch exists");
    expect (result.artifactPaths.size() == 2, "artifact count");
    expect (result.artifactPaths.at (0) == result.reportPath, "report artifact");
    expect (result.artifactPaths.at (1) == outputDirectory / "saved_main.patch.json", "patch artifact");

    const auto report = readTextFile (result.reportPath);
    expect (report.find ("\"kind\": \"c2StorageProof\"") != std::string::npos, "report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "report ok");
    expect (report.find ("\"source\": \"PatchDocument\"") != std::string::npos, "source");
    expect (report.find ("\"usesInteractionState\": false") != std::string::npos,
            "no interaction state dependency");
    expect (report.find ("\"saveStatus\": \"save-ok commit-pending\"") != std::string::npos,
            "save status");
    expect (report.find ("\"editorNodeCount\": 5") != std::string::npos, "editor node count");
    expect (report.find ("\"editorEdgeCount\": 3") != std::string::npos, "editor edge count");
    expect (report.find ("\"runtimeNodeCount\": 5") != std::string::npos, "runtime node count");
    expect (report.find ("\"runtimeEdgeCount\": 3") != std::string::npos, "runtime edge count");
    expect (report.find ("\"publicInputEdge\": true") != std::string::npos, "public input edge");
    expect (report.find ("\"publicOutputEdge\": true") != std::string::npos, "public output edge");
    expect (report.find ("\"matches\": true") != std::string::npos, "expanded layout");
    expect (report.find ("\"error\": \"\"") != std::string::npos, "empty error");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "c2 storage proof runner ok\n";
    return 0;
}
