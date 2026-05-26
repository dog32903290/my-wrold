#include "WorkbenchSessionOpenStatus.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
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
    const auto activeWorkRoot = std::filesystem::temp_directory_path() / "my-world-app4-active-work";
    std::filesystem::remove_all (activeWorkRoot);
    copyC2WorkFixture (activeWorkRoot);

    myworld::WorkbenchSessionOpenStatusRequest activeRequest;
    activeRequest.activeWorkManifestPath = activeWorkRoot / "myworld.work.json";
    activeRequest.candidateRoots = { std::filesystem::current_path() };
    activeRequest.saveStatus = "clean";
    activeRequest.proofStatus = "g1-ready";
    activeRequest.previewStatus = "preview-ready";

    const auto activeResult = myworld::openCurrentWorkbenchSession (activeRequest);
    expect (activeResult.ok, activeResult.error);
    expect (activeResult.snapshot.workSource == "active-work", "active work source");
    expect (activeResult.snapshot.workSourceStatus == "active-work-opened", "active work source status");
    expect (activeResult.snapshot.activeWorkManifestPath == activeRequest.activeWorkManifestPath.string(),
            "active work manifest path");
    expect (activeResult.snapshot.workManifestPath == activeRequest.activeWorkManifestPath.string(),
            "opened active work manifest");

    myworld::ExplicitWorkbenchOpenRequest explicitRequest;
    explicitRequest.workManifestPath = activeWorkRoot / "myworld.work.json";
    explicitRequest.candidateRoots = { std::filesystem::current_path() };
    explicitRequest.saveStatus = "clean";
    explicitRequest.proofStatus = "explicit-ready";
    explicitRequest.previewStatus = "preview-ready";

    const auto explicitResult = myworld::openExplicitWorkbenchSession (explicitRequest);
    expect (explicitResult.ok, explicitResult.error);
    expect (explicitResult.status == "explicit-opened", "explicit open status");
    expect (explicitResult.snapshot.workSource == "explicit-work", "explicit work source");
    expect (explicitResult.snapshot.workSourceStatus == "explicit-work-opened", "explicit source status");
    expect (explicitResult.snapshot.workManifestPath == explicitRequest.workManifestPath.string(),
            "explicit opened manifest");
    expect (explicitResult.snapshot.activeWorkManifestPath.empty(), "explicit open does not claim active manifest");
    expect (explicitResult.snapshot.documentId == "patch.c2-main", "explicit document id");
    expect (explicitResult.statusText == "explicit open ready: patch.c2-main source explicit-work-opened mappings 1/1",
            "explicit status text");

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
    expect (result.snapshot.workSourceStatus == "fixture-fallback-active-missing", "fixture fallback source status");
    expect (result.snapshot.activeWorkManifestPath == request.activeWorkManifestPath.string(),
            "missing active work manifest path");
    expect (result.snapshot.documentId == "patch.c2-main", "document id");
    expect (result.snapshot.activeOutputNodeId == "out1", "active output");
    expect (result.snapshot.graphIOMappingCount == 1, "mapping count");
    expect (result.snapshot.graphIOMappingStatus == "valid", "mapping status");
    expect (! result.snapshot.graphIOMappingSourcePath.empty(), "mapping source path");

    const auto statusText = myworld::makeWorkbenchSessionStatusText (result.snapshot);
    expectContains (statusText, "workbench ready", "status text ready");
    expectContains (statusText, "patch.c2-main", "status text document");
    expectContains (statusText, "source fixture-fallback-active-missing", "status text source");
    expectContains (statusText, "mappings 1/1", "status text mapping count");

    const auto brokenWorkRoot = std::filesystem::temp_directory_path() / "my-world-app4-broken-active-work";
    std::filesystem::remove_all (brokenWorkRoot);
    std::filesystem::create_directories (brokenWorkRoot);
    {
        std::ofstream output (brokenWorkRoot / "myworld.work.json");
        output << "{ \"not\": \"a work manifest\" }\n";
    }

    myworld::WorkbenchSessionOpenStatusRequest brokenRequest;
    brokenRequest.activeWorkManifestPath = brokenWorkRoot / "myworld.work.json";
    brokenRequest.candidateRoots = { std::filesystem::current_path() };

    const auto brokenResult = myworld::openCurrentWorkbenchSession (brokenRequest);
    expect (! brokenResult.ok, "broken active work should block");
    expect (brokenResult.status == "failed", "broken status");
    expect (brokenResult.snapshot.status == "blocked", "broken snapshot status");
    expect (brokenResult.snapshot.workSource == "active-work", "broken source");
    expect (brokenResult.snapshot.workSourceStatus == "active-work-blocked", "broken source status");
    expect (brokenResult.snapshot.activeWorkManifestPath == brokenRequest.activeWorkManifestPath.string(),
            "broken active work manifest path");
    expect (brokenResult.snapshot.workManifestPath == brokenRequest.activeWorkManifestPath.string(),
            "broken work manifest path");

    const auto brokenStatusText = myworld::makeWorkbenchSessionStatusText (brokenResult.snapshot);
    expectContains (brokenStatusText, "workbench blocked", "broken status text");
    expectContains (brokenStatusText, "source active-work-blocked", "broken source text");

    myworld::ExplicitWorkbenchOpenRequest missingExplicitRequest;
    missingExplicitRequest.workManifestPath = std::filesystem::temp_directory_path()
                                              / "my-world-missing-explicit-work"
                                              / "myworld.work.json";
    missingExplicitRequest.candidateRoots = { std::filesystem::current_path() };

    const auto missingExplicit = myworld::openExplicitWorkbenchSession (missingExplicitRequest);
    expect (! missingExplicit.ok, "missing explicit work should fail");
    expect (missingExplicit.status == "explicit-open-blocked", "missing explicit status");
    expect (missingExplicit.snapshot.workSource == "explicit-work", "missing explicit source");
    expect (missingExplicit.snapshot.workSourceStatus == "explicit-work-blocked", "missing explicit source status");
    expect (missingExplicit.statusText.find ("explicit open failed:") == 0, "missing explicit status text");

    std::filesystem::remove_all (activeWorkRoot);
    std::filesystem::remove_all (brokenWorkRoot);
    std::cout << "workbench session open status ok\n";
    return 0;
}
