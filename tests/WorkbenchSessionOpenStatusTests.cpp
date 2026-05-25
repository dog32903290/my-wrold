#include "WorkbenchSessionOpenStatus.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (condition)
        return;

    std::cerr << "FAIL: " << message << '\n';
    std::exit (1);
}

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}
}

int main()
{
    myworld::WorkbenchSessionOpenStatusRequest request;
    request.activeWorkManifestPath = std::filesystem::temp_directory_path()
                                     / "my-world-missing-active-work"
                                     / "myworld.work.json";
    request.candidateRoots = { std::filesystem::current_path() };
    request.saveStatus = "clean";
    request.proofStatus = "g1-ready";
    request.previewStatus = "preview-ready";

    const auto result = myworld::openCurrentWorkbenchSession (request);

    expect (result.ok, result.error);
    expect (result.status == "ready", "open status");
    expect (result.snapshot.ok, result.snapshot.message);
    expect (result.snapshot.workSource == "fixture", "fixture source");
    expect (result.snapshot.documentId == "patch.c2-main", "document id");
    expect (result.snapshot.activeOutputNodeId == "out1", "active output");
    expect (result.snapshot.graphIOMappingCount == 1, "mapping count");
    expect (result.snapshot.graphIOMappingStatus == "valid", "mapping status");
    expect (! result.snapshot.graphIOMappingSourcePath.empty(), "mapping source path");

    const auto statusText = myworld::makeWorkbenchSessionStatusText (result.snapshot);
    expectContains (statusText, "workbench ready", "status text ready");
    expectContains (statusText, "patch.c2-main", "status text document");
    expectContains (statusText, "mappings 1/1", "status text mapping count");

    std::cout << "workbench session open status ok\n";
    return 0;
}
