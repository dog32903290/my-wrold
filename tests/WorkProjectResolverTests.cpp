#include "WorkProjectResolver.h"

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

void expectLifecycle (const myworld::WorkProjectResolveResult& result,
                      const std::string& expectedSource,
                      const std::string& expectedStatus,
                      bool expectedBlocked)
{
    expect (result.lifecycle.workSource == expectedSource, expectedStatus + " source");
    expect (result.lifecycle.workSourceStatus == expectedStatus, expectedStatus + " status");
    expect (result.lifecycle.blocksSession == expectedBlocked, expectedStatus + " blocked state");
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
    const auto activeWorkRoot = std::filesystem::temp_directory_path() / "my-world-work2-active-work";
    std::filesystem::remove_all (activeWorkRoot);
    copyC2WorkFixture (activeWorkRoot);

    myworld::WorkProjectResolveRequest activeRequest;
    activeRequest.activeWorkManifestPath = activeWorkRoot / "myworld.work.json";
    activeRequest.candidateRoots = { std::filesystem::current_path() };

    const auto activeResult = myworld::resolveWorkProjectForWorkbench (activeRequest);
    expect (activeResult.ok, activeResult.error);
    expectLifecycle (activeResult, "active-work", "active-work-opened", false);
    expect (activeResult.workManifestPath == activeRequest.activeWorkManifestPath.string(), "active manifest path");
    expect (activeResult.activeWorkManifestPath == activeRequest.activeWorkManifestPath.string(),
            "active request path");
    expect (activeResult.document.id == "patch.c2-main", "active document id");

    myworld::WorkProjectResolveRequest missingActiveRequest;
    missingActiveRequest.activeWorkManifestPath = std::filesystem::temp_directory_path()
                                                  / "my-world-work2-missing-active"
                                                  / "myworld.work.json";
    missingActiveRequest.candidateRoots = { std::filesystem::current_path() };

    const auto missingActiveResult = myworld::resolveWorkProjectForWorkbench (missingActiveRequest);
    expect (missingActiveResult.ok, missingActiveResult.error);
    expectLifecycle (missingActiveResult, "fixture", "fixture-fallback-active-missing", false);
    expect (missingActiveResult.activeWorkManifestPath == missingActiveRequest.activeWorkManifestPath.string(),
            "missing active request path");
    expect (missingActiveResult.document.id == "patch.c2-main", "fixture fallback document id");

    const auto brokenWorkRoot = std::filesystem::temp_directory_path() / "my-world-work2-broken-active-work";
    std::filesystem::remove_all (brokenWorkRoot);
    std::filesystem::create_directories (brokenWorkRoot);
    {
        std::ofstream output (brokenWorkRoot / "myworld.work.json");
        output << "{ \"not\": \"a work manifest\" }\n";
    }

    myworld::WorkProjectResolveRequest brokenActiveRequest;
    brokenActiveRequest.activeWorkManifestPath = brokenWorkRoot / "myworld.work.json";
    brokenActiveRequest.candidateRoots = { std::filesystem::current_path() };

    const auto brokenActiveResult = myworld::resolveWorkProjectForWorkbench (brokenActiveRequest);
    expect (! brokenActiveResult.ok, "broken active work should block");
    expectLifecycle (brokenActiveResult, "active-work", "active-work-blocked", true);
    expect (! brokenActiveResult.error.empty(), "broken active work error");
    expect (brokenActiveResult.workManifestPath == brokenActiveRequest.activeWorkManifestPath.string(),
            "broken active manifest path");

    myworld::WorkProjectResolveRequest missingFixtureRequest;
    missingFixtureRequest.fallbackWorkManifestPath = "fixtures/storage/c2-compound-work/missing.work.json";
    missingFixtureRequest.candidateRoots = { std::filesystem::temp_directory_path() / "my-world-work2-empty-root" };

    const auto missingFixtureResult = myworld::resolveWorkProjectForWorkbench (missingFixtureRequest);
    expect (! missingFixtureResult.ok, "missing fixture should block");
    expectLifecycle (missingFixtureResult, "fixture", "fixture-blocked-no-active-request", true);
    expect (! missingFixtureResult.error.empty(), "missing fixture error");

    std::filesystem::remove_all (activeWorkRoot);
    std::filesystem::remove_all (brokenWorkRoot);
    std::cout << "work project resolver ok\n";
    return 0;
}
