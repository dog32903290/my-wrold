#include "WorkbenchRunProofRunner.h"

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
                                 / "my-world-run2-workbench-run-proof";
    std::filesystem::remove_all (outputDirectory);

    myworld::WorkbenchRunProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    const auto result = myworld::runWorkbenchRunProof (request);
    expect (result.ok, result.error);
    expect (result.status == "dumped", "run proof status");
    expect (result.statusText == "workbench run ready: patch.run2-main headless const1 -> out1",
            "run proof status text");
    expect (result.reportPath == outputDirectory / "run_report.json", "run report path");
    expect (std::filesystem::exists (result.reportPath), "run report exists");

    const auto report = readTextFile (result.reportPath);
    expect (report.find ("\"kind\": \"workbenchRunReport\"") != std::string::npos,
            "report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "report ok");
    expect (report.find ("\"headline\": \"run ready patch.run2-main target out1\"")
                != std::string::npos,
            "surface headline");
    expect (report.find ("\"rowCount\": 6") != std::string::npos, "surface row count");
    expect (report.find ("\"documentId\": \"patch.run2-main\"") != std::string::npos,
            "document id");
    expect (report.find ("\"runtimeNodeCount\": 2") != std::string::npos,
            "runtime node count");
    expect (report.find ("\"runtimeEdgeCount\": 1") != std::string::npos,
            "runtime edge count");
    expect (report.find ("\"activeOutputNodeId\": \"out1\"") != std::string::npos,
            "active output");
    expect (report.find ("\"runOrder\": [\"const1\", \"out1\"]") != std::string::npos,
            "run order array");
    expect (report.find ("\"runOrderText\": \"const1 -> out1\"") != std::string::npos,
            "run order text");
    expect (report.find ("\"targetNodeId\": \"out1\"") != std::string::npos,
            "target node");
    expect (report.find ("\"readiness\": \"ready\"") != std::string::npos, "readiness");
    expect (report.find ("\"executionStatus\": \"ran\"") != std::string::npos,
            "execution status");
    expect (report.find ("\"headlessTextureSummaryPath\":") != std::string::npos,
            "texture summary path");
    expect (report.find ("\"headlessCookOrderPath\":") != std::string::npos,
            "cook order path");
    expect (report.find ("\"headlessNodeStatsPath\":") != std::string::npos,
            "node stats path");
    expect (report.find ("\"headlessThumbnailPath\":") != std::string::npos,
            "thumbnail path");
    expect (report.find ("\"headlessThumbnailStatsPath\":") != std::string::npos,
            "thumbnail stats path");
    expect (report.find ("\"headlessErrorsPath\":") != std::string::npos,
            "errors path");
    expect (report.find ("\"run patch.run2-main\"") != std::string::npos,
            "run row text");
    expect (report.find ("\"target out1\"") != std::string::npos, "target row text");
    expect (report.find ("\"graph 2 nodes/1 edges\"") != std::string::npos,
            "graph row text");
    expect (report.find ("\"adapter headless-render\"") != std::string::npos,
            "adapter row text");
    expect (report.find ("\"readiness ready\"") != std::string::npos,
            "readiness row text");
    expect (report.find ("\"execution ran\"") != std::string::npos,
            "execution row text");
    expect (report.find ("\"error\": \"\"") != std::string::npos, "success error is empty");

    const auto textureSummaryPath = outputDirectory / "texture_summary.json";
    const auto cookOrderPath = outputDirectory / "cook_order.json";
    expect (std::filesystem::exists (textureSummaryPath), "texture summary exists");
    expect (std::filesystem::exists (cookOrderPath), "cook order exists");
    expect (std::filesystem::exists (outputDirectory / "node_stats.json"), "node stats exists");
    expect (std::filesystem::exists (outputDirectory / "thumbnail.png"), "thumbnail exists");
    expect (std::filesystem::exists (outputDirectory / "thumbnail_stats.json"), "thumbnail stats exists");
    expect (std::filesystem::exists (outputDirectory / "errors.json"), "errors exists");

    const auto textureSummary = readTextFile (textureSummaryPath);
    expect (textureSummary.find ("\"sourceNodeId\": \"const1\"") != std::string::npos,
            "texture summary source");
    const auto cookOrder = readTextFile (cookOrderPath);
    expect (cookOrder.find ("\"cookOrder\": [\"const1\", \"out1\"]") != std::string::npos,
            "headless cook order");

    std::filesystem::remove_all (outputDirectory);

    std::cout << "workbench run proof runner ok\n";
    return 0;
}
