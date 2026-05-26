#include "WorkbenchGraphSurfaceProofRunner.h"

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
                                 / "my-world-graph2-workbench-graph-surface-proof";
    std::filesystem::remove_all (outputDirectory);

    myworld::WorkbenchGraphSurfaceProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    const auto result = myworld::runWorkbenchGraphSurfaceProof (request);
    expect (result.ok, result.error);
    expect (result.status == "dumped", "graph surface proof status");
    expect (result.statusText == "workbench graph surface ready: patch.graph2-main editor 2/1",
            "graph surface proof status text");
    expect (result.reportPath == outputDirectory / "graph_surface_report.json",
            "graph surface report path");
    expect (std::filesystem::exists (result.reportPath), "graph surface report exists");

    const auto report = readTextFile (result.reportPath);
    expect (report.find ("\"kind\": \"workbenchGraphSurfaceReport\"") != std::string::npos,
            "report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "report ok");
    expect (report.find ("\"headline\": \"graph ready patch.graph2-main editor 2/1 runtime 2/1\"")
                != std::string::npos,
            "surface headline");
    expect (report.find ("\"rowCount\": 6") != std::string::npos, "surface row count");
    expect (report.find ("\"documentId\": \"patch.graph2-main\"") != std::string::npos,
            "document id");
    expect (report.find ("\"editorNodeCount\": 2") != std::string::npos,
            "editor node count");
    expect (report.find ("\"editorEdgeCount\": 1") != std::string::npos,
            "editor edge count");
    expect (report.find ("\"runtimeNodeCount\": 2") != std::string::npos,
            "runtime node count");
    expect (report.find ("\"runtimeEdgeCount\": 1") != std::string::npos,
            "runtime edge count");
    expect (report.find ("\"firstNodeId\": \"shader1\"") != std::string::npos,
            "first node id");
    expect (report.find ("\"firstNodeType\": \"shader.fragment\"") != std::string::npos,
            "first node type");
    expect (report.find ("\"firstEdgeFrom\": \"shader1.output\"") != std::string::npos,
            "first edge from");
    expect (report.find ("\"firstEdgeTo\": \"out1.input\"") != std::string::npos,
            "first edge to");
    expect (report.find ("\"graph patch.graph2-main\"") != std::string::npos,
            "graph row text");
    expect (report.find ("\"editor 2 nodes/1 edges\"") != std::string::npos,
            "editor row text");
    expect (report.find ("\"runtime 2 nodes/1 edges\"") != std::string::npos,
            "runtime row text");
    expect (report.find ("\"activeOutputNodeId\": \"out1\"") != std::string::npos,
            "active output");
    expect (report.find ("\"output out1\"") != std::string::npos, "output row text");
    expect (report.find ("\"node shader1 shader.fragment\"") != std::string::npos,
            "node row text");
    expect (report.find ("\"edge shader1.output -> out1.input\"") != std::string::npos,
            "edge row text");
    expect (report.find ("\"error\": \"\"") != std::string::npos, "success error is empty");

    std::filesystem::remove_all (outputDirectory);

    std::cout << "workbench graph surface proof runner ok\n";
    return 0;
}
