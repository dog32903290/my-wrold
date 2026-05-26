#include "WorkbenchCanvasSurfaceProofRunner.h"

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
                                 / "my-world-canvas2-workbench-canvas-surface-proof";
    std::filesystem::remove_all (outputDirectory);

    myworld::WorkbenchCanvasSurfaceProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    const auto result = myworld::runWorkbenchCanvasSurfaceProof (request);
    expect (result.ok, result.error);
    expect (result.status == "dumped", "canvas surface proof status");
    expect (result.statusText == "workbench canvas surface ready: patch.canvas2-main nodes 2 routes 1",
            "canvas surface proof status text");
    expect (result.reportPath == outputDirectory / "canvas_surface_report.json",
            "canvas surface report path");
    expect (std::filesystem::exists (result.reportPath), "canvas surface report exists");

    const auto report = readTextFile (result.reportPath);
    expect (report.find ("\"kind\": \"workbenchCanvasSurfaceReport\"") != std::string::npos,
            "report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "report ok");
    expect (report.find ("\"headline\": \"canvas ready patch.canvas2-main nodes 2 routes 1\"")
                != std::string::npos,
            "surface headline");
    expect (report.find ("\"rowCount\": 6") != std::string::npos, "surface row count");
    expect (report.find ("\"documentId\": \"patch.canvas2-main\"") != std::string::npos,
            "document id");
    expect (report.find ("\"nodeSurfaceCount\": 2") != std::string::npos,
            "node surface count");
    expect (report.find ("\"edgeRouteCount\": 1") != std::string::npos,
            "edge route count");
    expect (report.find ("\"firstNodeId\": \"shader1\"") != std::string::npos,
            "first node id");
    expect (report.find ("\"firstNodeX\": 80") != std::string::npos, "first node x");
    expect (report.find ("\"firstNodeY\": 80") != std::string::npos, "first node y");
    expect (report.find ("\"firstNodeWidth\": 140") != std::string::npos,
            "first node width");
    expect (report.find ("\"firstNodeHeight\": 60") != std::string::npos,
            "first node height");
    expect (report.find ("\"firstRouteFrom\": \"shader1.output\"") != std::string::npos,
            "first route from");
    expect (report.find ("\"firstRouteTo\": \"out1.input\"") != std::string::npos,
            "first route to");
    expect (report.find ("\"firstRouteFromX\": 220") != std::string::npos,
            "first route from x");
    expect (report.find ("\"firstRouteFromY\": 110") != std::string::npos,
            "first route from y");
    expect (report.find ("\"firstRouteToX\": 320") != std::string::npos,
            "first route to x");
    expect (report.find ("\"firstRouteToY\": 110") != std::string::npos,
            "first route to y");
    expect (report.find ("\"canvas patch.canvas2-main\"") != std::string::npos,
            "canvas row text");
    expect (report.find ("\"nodes 2\"") != std::string::npos, "nodes row text");
    expect (report.find ("\"routes 1\"") != std::string::npos, "routes row text");
    expect (report.find ("\"bounds shader1 80,80 140x60\"") != std::string::npos,
            "bounds row text");
    expect (report.find ("\"route shader1.output -> out1.input\"") != std::string::npos,
            "route row text");
    expect (report.find ("\"points 220,110 -> 320,110\"") != std::string::npos,
            "points row text");
    expect (report.find ("\"error\": \"\"") != std::string::npos, "success error is empty");

    std::filesystem::remove_all (outputDirectory);

    std::cout << "workbench canvas surface proof runner ok\n";
    return 0;
}
