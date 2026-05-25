#include "InteractionContract.h"
#include "OutputViewState.h"
#include "StorageCommand.h"
#include "StorageContract.h"

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

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}

void writeText (const std::filesystem::path& path, const std::string& text)
{
    std::filesystem::create_directories (path.parent_path());
    std::ofstream output (path, std::ios::trunc);
    expect (static_cast<bool> (output), "open " + path.string());
    output << text;
    expect (static_cast<bool> (output), "write " + path.string());
}
}

int main()
{
    auto session = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    session.selectedNodeIds = { "shader1" };
    myworld::followSelectedOutputNode (session.outputView, session.graph, session.selectedNodeIds);

    expect (! session.outputView.pinned, "output starts unpinned");
    expect (myworld::activeOutputNodeId (session.outputView) == "shader1", "output follows initial selection");

    session.selectedNodeIds = { "out1" };
    myworld::followSelectedOutputNode (session.outputView, session.graph, session.selectedNodeIds);
    expect (myworld::activeOutputNodeId (session.outputView) == "out1", "output follows changed selection");

    session.selectedNodeIds = { "shader1" };
    auto pin = myworld::pinOutputViewToSelection (session.outputView, session.graph, session.selectedNodeIds);
    expect (pin.ok, pin.message);
    expect (session.outputView.pinned, "pin flips pinned flag");
    expect (myworld::activeOutputNodeId (session.outputView) == "shader1", "pin captures selected node");

    session.selectedNodeIds = { "out1" };
    myworld::followSelectedOutputNode (session.outputView, session.graph, session.selectedNodeIds);
    expect (myworld::activeOutputNodeId (session.outputView) == "shader1",
            "selection changes do not move pinned output");

    auto missingPin = myworld::pinOutputViewToNode (session.outputView, session.graph, "missing");
    expect (! missingPin.ok, "missing pin is rejected");
    expect (myworld::activeOutputNodeId (session.outputView) == "shader1",
            "rejected pin keeps previous output");

    myworld::unpinOutputView (session.outputView, session.graph, session.selectedNodeIds);
    expect (! session.outputView.pinned, "unpin clears pinned flag");
    expect (session.outputView.pinnedNodeId.empty(), "unpin clears pinned node id");
    expect (myworld::activeOutputNodeId (session.outputView) == "out1",
            "unpin returns to current selected node");

    auto documentSession = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    documentSession.selectedNodeIds = { "shader1" };
    expect (myworld::pinOutputViewToSelection (documentSession.outputView,
                                               documentSession.graph,
                                               documentSession.selectedNodeIds)
                .ok,
            "document pin");

    const auto document = myworld::makePatchDocument ("patch.output-pin",
                                                      "Output Pin",
                                                      documentSession.graph,
                                                      documentSession.outputView);
    const auto json = myworld::toJson (document);
    expectContains (json, "\"outputView\"", "patch json");
    expectContains (json, "\"pinned\": true", "patch json");
    expectContains (json, "\"pinnedNodeId\": \"shader1\"", "patch json");

    const auto parsed = myworld::parsePatchDocument (json);
    expect (parsed.ok, parsed.error);
    expect (parsed.document.outputView.pinned, "patch document reloads pinned flag");
    expect (parsed.document.outputView.pinnedNodeId == "shader1", "patch document reloads pinned node");
    expect (myworld::activeOutputNodeId (parsed.document.outputView) == "shader1",
            "patch document reloads active output");

    const auto root = std::filesystem::temp_directory_path() / "my-world-output-view-state-tests";
    std::filesystem::remove_all (root);
    const auto manifestPath = root / "myworld.work.json";
    const auto patchPath = root / "patches" / "main.patch.json";

    writeText (manifestPath, myworld::toJson (myworld::makeMinimalWorkProject ("work.output", "Output Work")));
    const auto initialSave = myworld::savePatchDocument (
        patchPath.string(),
        myworld::makePatchDocument ("patch.output-main",
                                    "Output Main",
                                    myworld::makeDefaultShaderOutputGraph()));
    expect (initialSave.ok, initialSave.error);

    const auto loadedMain = myworld::loadMainPatchDocumentForWork (manifestPath.string());
    expect (loadedMain.ok, loadedMain.error);
    auto saveSession = myworld::makeGraphSession (loadedMain.document.graph);
    saveSession.outputView = loadedMain.document.outputView;
    saveSession.selectedNodeIds = { "shader1" };
    expect (myworld::pinOutputViewToSelection (saveSession.outputView,
                                               saveSession.graph,
                                               saveSession.selectedNodeIds)
                .ok,
            "save session pin");
    saveSession.dirty = true;

    const auto saveResult = myworld::saveWork (saveSession, manifestPath.string());
    expect (saveResult.ok, saveResult.error);

    const auto reloadedMain = myworld::loadMainPatchDocumentForWork (manifestPath.string());
    expect (reloadedMain.ok, reloadedMain.error);
    expect (reloadedMain.document.outputView.pinned, "save_work preserves pinned flag");
    expect (reloadedMain.document.outputView.pinnedNodeId == "shader1", "save_work preserves pinned node");

    std::filesystem::remove_all (root);

    std::cout << "output view state ok\n";
    return 0;
}
