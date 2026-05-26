#include "WorkbenchRunSurface.h"

#include "GraphContract.h"
#include "OutputViewState.h"
#include "StorageContract.h"

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

const myworld::WorkbenchRunSurfaceRow& rowAt (const myworld::WorkbenchRunSurface& surface,
                                              std::size_t index)
{
    expect (surface.rows.size() > index, "surface row index exists");
    return surface.rows[index];
}

std::string readTextFile (const std::filesystem::path& path)
{
    std::ifstream input (path);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

myworld::GraphContract makeHeadlessRunGraph()
{
    myworld::GraphNode constant;
    constant.id = "const1";
    constant.type = "image.constant";
    constant.position = { 80.0, 80.0 };
    constant.params = {
        { "color", "[0.02, 0.02, 0.02, 1.0]" },
        { "resolution", "[1280, 720]" }
    };

    myworld::GraphNode output;
    output.id = "out1";
    output.type = "output.texture_summary";
    output.position = { 320.0, 80.0 };

    myworld::GraphEdge edge;
    edge.id = "edge.const1.out.out1.input";
    edge.from = "const1.out";
    edge.to = "out1.input";
    edge.dataType = "texture.rgba";
    edge.streamKind = "continuous";

    return { 1, { { constant, output }, { edge } }, { { constant, output }, { edge } } };
}

myworld::WorkbenchSessionSnapshot snapshotFor (const myworld::GraphContract& graph)
{
    myworld::OutputViewState outputView;
    outputView.followedNodeId = "out1";

    myworld::WorkbenchSessionRequest request;
    request.workManifestPath = "debug/run1-workbench-headless-run/myworld.work.json";
    request.document = myworld::makePatchDocument ("patch.run1-main",
                                                   "RUN1 Main Patch",
                                                   graph,
                                                   outputView);
    request.saveStatus = "clean";
    request.proofStatus = "run1-ready";
    request.previewStatus = "preview-ready";
    return myworld::makeWorkbenchSessionSnapshot (request);
}
}

int main()
{
    myworld::WorkbenchSessionSnapshot blocked;
    blocked.status = "blocked";
    blocked.message = "no current workbench session";

    const auto blockedSurface = myworld::makeWorkbenchRunSurface (blocked);
    expect (! blockedSurface.ok, "blocked surface ok");
    expect (blockedSurface.headline == "run blocked: no current workbench session",
            "blocked headline");
    expect (blockedSurface.runOrder.empty(), "blocked run order");
    expect (blockedSurface.rows.size() == 6, "blocked row count");
    expect (rowAt (blockedSurface, 0).text == "run no workbench session", "blocked run row");
    expect (rowAt (blockedSurface, 1).text == "target none", "blocked target row");
    expect (rowAt (blockedSurface, 2).text == "graph 0 nodes/0 edges", "blocked graph row");
    expect (rowAt (blockedSurface, 3).text == "adapter headless-render", "blocked adapter row");
    expect (rowAt (blockedSurface, 4).text == "readiness blocked", "blocked readiness row");
    expect (rowAt (blockedSurface, 5).text == "execution parked", "blocked execution row");

    const auto unsupported = snapshotFor (myworld::makeDefaultShaderOutputGraph());
    const auto unsupportedSurface = myworld::makeWorkbenchRunSurface (unsupported);
    expect (unsupportedSurface.ok, "unsupported surface remains readable");
    expect (unsupportedSurface.headline
                == "run unsupported patch.run1-main headless unsupported node shader.fragment",
            "unsupported headline");
    expect (unsupportedSurface.runOrder.empty(), "unsupported run order");
    expect (rowAt (unsupportedSurface, 0).text == "run patch.run1-main", "unsupported run row");
    expect (rowAt (unsupportedSurface, 1).text == "target out1", "unsupported target row");
    expect (rowAt (unsupportedSurface, 2).text == "graph 2 nodes/1 edges", "unsupported graph row");
    expect (rowAt (unsupportedSurface, 3).text == "adapter headless-render", "unsupported adapter row");
    expect (rowAt (unsupportedSurface, 4).text == "readiness unsupported", "unsupported readiness row");
    expect (rowAt (unsupportedSurface, 5).text == "execution parked", "unsupported execution row");

    const auto ready = snapshotFor (makeHeadlessRunGraph());
    expect (ready.ok, ready.message);
    expect (ready.runtimeNodes.front().params.size() == 2, "runtime params are summarized");
    expect (ready.runtimeNodes.front().params.front().id == "color", "first runtime param id");
    expect (ready.runtimeNodes.front().params.front().value == "[0.02, 0.02, 0.02, 1.0]",
            "first runtime param value");

    const auto readySurface = myworld::makeWorkbenchRunSurface (ready);
    expect (readySurface.ok, "ready surface ok");
    expect (readySurface.headline == "run ready patch.run1-main target out1", "ready headline");
    expect (readySurface.runOrder.size() == 2, "ready run order count");
    expect (readySurface.runOrder[0] == "const1", "ready run order first");
    expect (readySurface.runOrder[1] == "out1", "ready run order second");
    expect (readySurface.rows.size() == 6, "ready row count");
    expect (rowAt (readySurface, 0).text == "run patch.run1-main", "ready run row");
    expect (rowAt (readySurface, 1).text == "target out1", "ready target row");
    expect (rowAt (readySurface, 2).text == "graph 2 nodes/1 edges", "ready graph row");
    expect (rowAt (readySurface, 3).text == "adapter headless-render", "ready adapter row");
    expect (rowAt (readySurface, 4).text == "readiness ready", "ready readiness row");
    expect (rowAt (readySurface, 5).text == "execution not-run", "ready execution row");

    const auto fixtureText = myworld::makeWorkbenchHeadlessRenderFixtureText (ready);
    expect (fixtureText.find ("\"type\": \"image.constant\"") != std::string::npos,
            "fixture constant node");
    expect (fixtureText.find ("\"type\": \"output.texture_summary\"") != std::string::npos,
            "fixture output node");
    expect (fixtureText.find ("\"color\": [0.020000, 0.020000, 0.020000, 1.000000]")
                != std::string::npos,
            "fixture color");
    expect (fixtureText.find ("\"resolution\": [1280, 720]") != std::string::npos,
            "fixture resolution");

    const auto outputDirectory = std::filesystem::temp_directory_path()
                                 / "my-world-run1-workbench-headless-run";
    std::filesystem::remove_all (outputDirectory);

    const auto result = myworld::runWorkbenchHeadlessRender (ready, outputDirectory);
    expect (result.ok, result.error);
    expect (result.status == "ran", "run result status");
    expect (result.statusText == "workbench run ready: patch.run1-main headless const1 -> out1",
            "run result status text");
    expect (rowAt (result.surface, 5).text == "execution ran", "run result execution row");
    expect (std::filesystem::exists (result.headless.textureSummaryPath), "texture summary exists");
    expect (std::filesystem::exists (result.headless.cookOrderPath), "cook order exists");
    expect (std::filesystem::exists (result.headless.nodeStatsPath), "node stats exists");
    expect (std::filesystem::exists (result.headless.thumbnailPath), "thumbnail exists");
    expect (std::filesystem::exists (result.headless.thumbnailStatsPath), "thumbnail stats exists");
    expect (std::filesystem::exists (result.headless.errorsPath), "errors exists");

    const auto textureSummary = readTextFile (result.headless.textureSummaryPath);
    expect (textureSummary.find ("\"sourceNodeId\": \"const1\"") != std::string::npos,
            "texture summary source");
    expect (textureSummary.find ("\"outputNodeId\": \"out1\"") != std::string::npos,
            "texture summary output");
    const auto cookOrder = readTextFile (result.headless.cookOrderPath);
    expect (cookOrder.find ("\"cookOrder\": [\"const1\", \"out1\"]") != std::string::npos,
            "headless cook order");

    std::filesystem::remove_all (outputDirectory);

    std::cout << "workbench run surface ok\n";
    return 0;
}
