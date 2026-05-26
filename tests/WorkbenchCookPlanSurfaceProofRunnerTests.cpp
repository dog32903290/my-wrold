#include "WorkbenchCookPlanSurfaceProofRunner.h"

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
                                 / "my-world-cook2-workbench-cook-plan-proof";
    std::filesystem::remove_all (outputDirectory);

    myworld::WorkbenchCookPlanSurfaceProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    const auto result = myworld::runWorkbenchCookPlanSurfaceProof (request);
    expect (result.ok, result.error);
    expect (result.status == "dumped", "cook plan proof status");
    expect (result.statusText == "workbench cook plan ready: patch.cook2-main order shader1 -> out1",
            "cook plan proof status text");
    expect (result.reportPath == outputDirectory / "cook_plan_report.json",
            "cook plan report path");
    expect (std::filesystem::exists (result.reportPath), "cook plan report exists");

    const auto report = readTextFile (result.reportPath);
    expect (report.find ("\"kind\": \"workbenchCookPlanReport\"") != std::string::npos,
            "report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "report ok");
    expect (report.find ("\"headline\": \"cook ready patch.cook2-main order shader1 -> out1\"")
                != std::string::npos,
            "surface headline");
    expect (report.find ("\"rowCount\": 6") != std::string::npos, "surface row count");
    expect (report.find ("\"documentId\": \"patch.cook2-main\"") != std::string::npos,
            "document id");
    expect (report.find ("\"runtimeNodeCount\": 2") != std::string::npos,
            "runtime node count");
    expect (report.find ("\"runtimeEdgeCount\": 1") != std::string::npos,
            "runtime edge count");
    expect (report.find ("\"activeOutputNodeId\": \"out1\"") != std::string::npos,
            "active output");
    expect (report.find ("\"cookOrder\": [\"shader1\", \"out1\"]") != std::string::npos,
            "cook order array");
    expect (report.find ("\"cookOrderText\": \"shader1 -> out1\"") != std::string::npos,
            "cook order text");
    expect (report.find ("\"targetNodeId\": \"out1\"") != std::string::npos,
            "target node");
    expect (report.find ("\"readiness\": \"ready\"") != std::string::npos, "readiness");
    expect (report.find ("\"executionStatus\": \"parked\"") != std::string::npos,
            "execution status");
    expect (report.find ("\"cook patch.cook2-main\"") != std::string::npos,
            "cook row text");
    expect (report.find ("\"order shader1 -> out1\"") != std::string::npos,
            "order row text");
    expect (report.find ("\"target out1\"") != std::string::npos, "target row text");
    expect (report.find ("\"graph 2 nodes/1 edges\"") != std::string::npos,
            "graph row text");
    expect (report.find ("\"readiness ready\"") != std::string::npos,
            "readiness row text");
    expect (report.find ("\"execution parked\"") != std::string::npos,
            "execution row text");
    expect (report.find ("\"error\": \"\"") != std::string::npos, "success error is empty");

    std::filesystem::remove_all (outputDirectory);

    std::cout << "workbench cook plan surface proof runner ok\n";
    return 0;
}
