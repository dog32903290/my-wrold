#include "WorkbenchAppController.h"

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
    myworld::WorkbenchAppController controller;

    myworld::WorkbenchAppControllerOpenRequest request;
    request.activeWorkManifestPath = std::filesystem::temp_directory_path()
                                     / "my-world-app3-missing-active-work"
                                     / "myworld.work.json";
    request.candidateRoots = { std::filesystem::current_path() };
    request.proofStatus = "g1-ready";

    const auto opened = controller.openCurrentSession (request);
    expect (opened.ok, opened.error);

    const auto& snapshot = controller.currentSession();
    expect (snapshot.ok, snapshot.message);
    expect (snapshot.workSource == "fixture", "fallback fixture source");
    expect (snapshot.documentId == "patch.c2-main", "document id");
    expect (snapshot.graphIOMappingStatus == "valid", "mapping status");

    expectContains (controller.statusText(), "workbench ready", "controller status");

    const auto proofRequest = controller.makeOpenStatusProofRequest (
        std::filesystem::temp_directory_path() / "my-world-app3-proof");
    expect (proofRequest.snapshot.documentId == snapshot.documentId, "proof uses held snapshot");
    expect (proofRequest.snapshot.workSource == snapshot.workSource, "proof uses held work source");

    std::cout << "workbench app controller ok\n";
    return 0;
}
