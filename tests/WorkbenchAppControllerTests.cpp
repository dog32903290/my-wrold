#include "WorkbenchAppController.h"

#include "InteractionContract.h"
#include "StorageContract.h"

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

void copyC2WorkFixture (const std::filesystem::path& workRoot)
{
    std::filesystem::create_directories (workRoot / "patches");
    std::filesystem::copy_file ("fixtures/storage/c2-compound-work/myworld.work.json",
                                workRoot / "myworld.work.json",
                                std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy_file ("fixtures/storage/c2-compound-work/patches/main.patch.json",
                                workRoot / "patches" / "main.patch.json",
                                std::filesystem::copy_options::overwrite_existing);
}
}

int main()
{
    const auto workRoot = std::filesystem::temp_directory_path() / "my-world-save1-controller-work";
    std::filesystem::remove_all (workRoot);
    copyC2WorkFixture (workRoot);

    myworld::WorkbenchAppController controller;

    const auto initialStatus = controller.appStatusSnapshot();
    expect (! initialStatus.ok, "initial app status should be blocked");
    expect (initialStatus.status == "blocked", "initial app status value");
    expect (initialStatus.statusText == "workbench blocked: no session", "initial app status text");
    expect (initialStatus.message == "no current workbench session", "initial app status message");

    myworld::WorkbenchAppControllerOpenRequest request;
    request.activeWorkManifestPath = workRoot / "myworld.work.json";
    request.candidateRoots = { std::filesystem::current_path() };
    request.proofStatus = "g1-ready";

    const auto opened = controller.openCurrentSession (request);
    expect (opened.ok, opened.error);

    const auto& snapshot = controller.currentSession();
    expect (snapshot.ok, snapshot.message);
    expect (snapshot.workSource == "active-work", "active work source");
    expect (snapshot.documentId == "patch.c2-main", "document id");
    expect (snapshot.graphIOMappingStatus == "valid", "mapping status");

    const auto openedStatus = controller.appStatusSnapshot();
    expect (openedStatus.ok, openedStatus.message);
    expect (openedStatus.status == "ready", "opened app status");
    expect (openedStatus.statusText == controller.statusText(), "opened app status text");
    expect (openedStatus.workSource == "active-work", "opened app status work source");
    expect (openedStatus.workSourceStatus == "active-work-opened", "opened app status source status");
    expect (openedStatus.documentId == "patch.c2-main", "opened app status document id");
    expect (openedStatus.graphIOMappingStatus == "valid", "opened app status mapping status");
    expect (openedStatus.validGraphIOMappingCount == snapshot.validGraphIOMappingCount,
            "opened app status valid mapping count");

    expectContains (controller.statusText(), "workbench ready", "controller status");
    expectContains (controller.statusText(),
                    "source active-work-opened",
                    "controller source status");

    const auto loadedMain = myworld::loadMainPatchDocumentForWork (snapshot.workManifestPath);
    expect (loadedMain.ok, loadedMain.error);
    auto saveSession = myworld::makeGraphSession (loadedMain.document.graph);
    expect (myworld::moveNode (saveSession, "library_loud1", 13.0, 7.0).ok,
            "dirty graph session before controller save");

    const auto saved = controller.saveCurrentSession (saveSession);
    expect (saved.ok, saved.message);
    expect (saved.message == "save-ok commit-pending", "controller save status");
    expect (! saveSession.dirty, "controller save clears dirty graph state");
    expectContains (controller.statusText(), "save_work: save-ok commit-pending", "controller save status text");
    const auto savedStatus = controller.appStatusSnapshot();
    expect (savedStatus.ok, savedStatus.message);
    expect (savedStatus.statusText == "save_work: save-ok commit-pending", "saved app status text");
    expect (savedStatus.saveStatus == "save-ok commit-pending", "saved app save status");
    expect (! savedStatus.dirty, "saved app dirty status");
    expect (savedStatus.documentId == snapshot.documentId, "saved app document id");

    const auto proofRequest = controller.makeOpenStatusProofRequest (
        std::filesystem::temp_directory_path() / "my-world-app3-proof");
    expect (proofRequest.snapshot.documentId == snapshot.documentId, "proof uses held snapshot");
    expect (proofRequest.snapshot.workSource == snapshot.workSource, "proof uses held work source");

    myworld::WorkbenchAppController emptyController;
    const auto blockedSave = emptyController.saveCurrentSession (saveSession);
    expect (! blockedSave.ok, "empty controller save should fail");
    expectContains (blockedSave.message, "no current workbench session", "empty controller save message");
    expectContains (emptyController.statusText(), "save_work failed:", "empty controller save status text");

    std::filesystem::remove_all (workRoot);

    std::cout << "workbench app controller ok\n";
    return 0;
}
