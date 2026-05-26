#include "WorkbenchRuntimeSurfaceProofRunner.h"

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
                                 / "my-world-runtime2-workbench-runtime-surface-proof";
    std::filesystem::remove_all (outputDirectory);

    myworld::WorkbenchRuntimeSurfaceProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    const auto result = myworld::runWorkbenchRuntimeSurfaceProof (request);
    expect (result.ok, result.error);
    expect (result.status == "dumped", "runtime surface proof status");
    expect (result.statusText == "workbench runtime surface ready: patch.runtime2-main runtime 2/1",
            "runtime surface proof status text");
    expect (result.reportPath == outputDirectory / "runtime_surface_report.json",
            "runtime surface report path");
    expect (std::filesystem::exists (result.reportPath), "runtime surface report exists");

    const auto report = readTextFile (result.reportPath);
    expect (report.find ("\"kind\": \"workbenchRuntimeSurfaceReport\"") != std::string::npos,
            "report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "report ok");
    expect (report.find ("\"headline\": \"runtime ready patch.runtime2-main nodes 2 edges 1\"")
                != std::string::npos,
            "surface headline");
    expect (report.find ("\"rowCount\": 6") != std::string::npos, "surface row count");
    expect (report.find ("\"documentId\": \"patch.runtime2-main\"") != std::string::npos,
            "document id");
    expect (report.find ("\"runtimeNodeCount\": 2") != std::string::npos,
            "runtime node count");
    expect (report.find ("\"runtimeEdgeCount\": 1") != std::string::npos,
            "runtime edge count");
    expect (report.find ("\"activeOutputNodeId\": \"out1\"") != std::string::npos,
            "active output");
    expect (report.find ("\"graphIOMappingStatus\": \"valid\"") != std::string::npos,
            "mapping status");
    expect (report.find ("\"graphIOMappingCount\": 1") != std::string::npos,
            "mapping count");
    expect (report.find ("\"validGraphIOMappingCount\": 1") != std::string::npos,
            "valid mapping count");
    expect (report.find ("\"readiness\": \"ready\"") != std::string::npos, "readiness");
    expect (report.find ("\"cookStatus\": \"parked\"") != std::string::npos, "cook status");
    expect (report.find ("\"runtime patch.runtime2-main\"") != std::string::npos,
            "runtime row text");
    expect (report.find ("\"graph 2 nodes/1 edges\"") != std::string::npos,
            "graph row text");
    expect (report.find ("\"output out1\"") != std::string::npos, "output row text");
    expect (report.find ("\"mapping valid 1/1\"") != std::string::npos, "mapping row text");
    expect (report.find ("\"readiness ready\"") != std::string::npos,
            "readiness row text");
    expect (report.find ("\"cook parked\"") != std::string::npos, "cook row text");
    expect (report.find ("\"error\": \"\"") != std::string::npos, "success error is empty");

    std::filesystem::remove_all (outputDirectory);

    std::cout << "workbench runtime surface proof runner ok\n";
    return 0;
}
