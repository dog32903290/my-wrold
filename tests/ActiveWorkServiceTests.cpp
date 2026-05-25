#include "ActiveWorkService.h"

#include "WorkProjectResolver.h"
#include "StorageContract.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

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

bool containsCommand (const std::vector<std::string>& commands, const std::string& command)
{
    return std::find (commands.begin(), commands.end(), command) != commands.end();
}

void setEnvironment (const char* name, const std::string& value)
{
    expect (setenv (name, value.c_str(), 1) == 0, std::string ("setenv failed for ") + name);
}

void clearEnvironment (const char* name)
{
    expect (unsetenv (name) == 0, std::string ("unsetenv failed for ") + name);
}
}

int main()
{
    const auto tempRoot = std::filesystem::temp_directory_path() / "my-world-active-work-service";
    const auto workRoot = tempRoot / "work";
    const auto projectRoot = tempRoot / "project";
    std::filesystem::remove_all (tempRoot);
    std::filesystem::create_directories (projectRoot);
    copyC2WorkFixture (workRoot);

    const auto workManifestPath = workRoot / "myworld.work.json";
    setEnvironment ("MY_WORLD_ACTIVE_WORK_MANIFEST", workManifestPath.string());
    setEnvironment ("MY_WORLD_PROJECT_DIR", projectRoot.string());

    const auto sourceFixtureManifest = std::string { "fixtures/storage/c2-compound-work/myworld.work.json" };
    const auto loadedMain = myworld::loadMainPatchDocumentForWork (sourceFixtureManifest);
    expect (loadedMain.ok, loadedMain.error);

    auto saveSession = myworld::makeGraphSession (loadedMain.document.graph);
    expect (myworld::moveNode (saveSession, "library_loud1", 13.0, 7.0).ok,
            "dirty graph session before active work save");

    const auto save = myworld::saveActiveWorkProject (saveSession);
    expect (save.ok, save.message);
    expect (save.message == "save-ok commit-pending", "active work save status");
    expect (! saveSession.dirty, "active work save clears dirty graph state");
    expect (containsCommand (saveSession.commandLog, "save_work:save-ok commit-pending"),
            "active work save records save command");

    setEnvironment ("MY_WORLD_ACTIVE_WORK_MANIFEST", sourceFixtureManifest);

    auto publishSession = myworld::makeGraphSession (loadedMain.document.graph);
    const auto publish = myworld::publishSelectedModuleFromActiveWork (publishSession, "library_loud1");
    expect (publish.ok, publish.error);
    expect (publish.status == "published", "active work publish status");
    expect (publish.moduleId == "module.visible-library_loud1", "active work publish module id");
    expect (publish.publishedNodeType == "compound.visible-library_loud1", "active work publish node type");
    expect (containsCommand (publishSession.commandLog, "publish_module:published"),
            "active work publish records publish command");
    expect (std::filesystem::exists (publish.moduleManifestPath), "active work publish module manifest exists");
    expect (std::filesystem::exists (publish.compoundPatchPath), "active work publish compound patch exists");
    expect (std::filesystem::exists (projectRoot / "debug" / "c5-visible-module-publish"),
            "active work publish uses project debug directory");

    const auto preparedProjectRoot = tempRoot / "prepared-project";
    std::filesystem::create_directories (preparedProjectRoot);
    setEnvironment ("MY_WORLD_PROJECT_DIR", preparedProjectRoot.string());
    clearEnvironment ("MY_WORLD_ACTIVE_WORK_MANIFEST");

    const auto defaultWorkRoot = preparedProjectRoot / "debug" / "c3-active-work";
    const auto defaultManifestPath = defaultWorkRoot / "myworld.work.json";
    expect (! std::filesystem::exists (defaultManifestPath), "default active work starts missing");

    const auto prepared = myworld::prepareActiveWorkProjectForOpen();
    expect (prepared.ok, prepared.error);
    expect (prepared.status == "default-active-work-prepared", "default active work prepared status");
    expect (prepared.workManifestPath == defaultManifestPath.string(), "default active work manifest path");
    expect (containsCommand (prepared.diagnostics,
                             "activeWorkPreparationStatus=default-active-work-prepared"),
            "default active work prepared diagnostics status");
    expect (containsCommand (prepared.diagnostics,
                             "activeWorkPreparationManifestPath=" + defaultManifestPath.string()),
            "default active work prepared diagnostics path");
    expect (std::filesystem::exists (defaultManifestPath), "default active work manifest exists");
    expect (std::filesystem::exists (defaultWorkRoot / "patches" / "main.patch.json"),
            "default active work main patch exists");
    expect (std::filesystem::exists (preparedProjectRoot / "debug" / "module-libraries" / "default.module-library.json"),
            "default active work module library exists");
    expect (std::filesystem::exists (preparedProjectRoot / "debug" / "module-libraries" / "modules" / "loudness" / "module.json"),
            "default active work module manifest exists");

    myworld::WorkProjectResolveRequest preparedRequest;
    preparedRequest.activeWorkManifestPath = prepared.workManifestPath;
    preparedRequest.candidateRoots = { std::filesystem::current_path() };

    const auto resolvedPreparedWork = myworld::resolveWorkProjectForWorkbench (preparedRequest);
    expect (resolvedPreparedWork.ok, resolvedPreparedWork.error);
    expect (resolvedPreparedWork.lifecycle.workSource == "active-work", "prepared work opens as active source");
    expect (resolvedPreparedWork.lifecycle.workSourceStatus == "active-work-opened",
            "prepared work source status");

    const auto preparedAgain = myworld::prepareActiveWorkProjectForOpen();
    expect (preparedAgain.ok, preparedAgain.error);
    expect (preparedAgain.status == "default-active-work-ready", "default active work ready status");
    expect (preparedAgain.workManifestPath == defaultManifestPath.string(), "default active work ready path");
    expect (containsCommand (preparedAgain.diagnostics,
                             "activeWorkPreparationStatus=default-active-work-ready"),
            "default active work ready diagnostics status");

    const auto externalManifestPath = tempRoot / "external-work" / "myworld.work.json";
    setEnvironment ("MY_WORLD_ACTIVE_WORK_MANIFEST", externalManifestPath.string());

    const auto externalPreparation = myworld::prepareActiveWorkProjectForOpen();
    expect (externalPreparation.ok, externalPreparation.error);
    expect (externalPreparation.status == "external-active-work-requested",
            "external active work is not auto-created");
    expect (externalPreparation.workManifestPath == externalManifestPath.string(),
            "external active work manifest path");
    expect (containsCommand (externalPreparation.diagnostics,
                             "activeWorkPreparationStatus=external-active-work-requested"),
            "external active work diagnostics status");
    expect (! std::filesystem::exists (externalManifestPath), "external active work stays caller-owned");

    const auto createdProjectRoot = tempRoot / "created-project";
    myworld::CreateActiveWorkProjectRequest createRequest;
    createRequest.projectDirectory = createdProjectRoot;
    createRequest.workId = "work.project1";
    createRequest.workTitle = "Project One";
    createRequest.patchId = "patch.project1-main";
    createRequest.patchTitle = "Project One Main";

    const auto created = myworld::createActiveWorkProject (createRequest);
    expect (created.ok, created.error);
    expect (created.status == "created", "created project status");
    expect (created.workManifestPath == (createdProjectRoot / "myworld.work.json").string(),
            "created work manifest path");
    expect (created.patchPath == (createdProjectRoot / "patches" / "main.patch.json").string(),
            "created main patch path");
    expect (containsCommand (created.diagnostics, "createActiveWorkProjectStatus=created"),
            "created project diagnostics status");
    expect (std::filesystem::exists (created.workManifestPath), "created work manifest exists");
    expect (std::filesystem::exists (created.patchPath), "created main patch exists");

    const auto createdManifest = myworld::loadWorkProjectManifest (created.workManifestPath);
    expect (createdManifest.ok, createdManifest.error);
    expect (createdManifest.manifest.id == "work.project1", "created work id");
    expect (createdManifest.manifest.title == "Project One", "created work title");
    expect (createdManifest.manifest.mainPatchPath == "patches/main.patch.json",
            "created work main patch path");

    const auto createdPatch = myworld::loadMainPatchDocumentForWork (created.workManifestPath);
    expect (createdPatch.ok, createdPatch.error);
    expect (createdPatch.document.id == "patch.project1-main", "created patch id");
    expect (createdPatch.document.title == "Project One Main", "created patch title");

    const auto duplicate = myworld::createActiveWorkProject (createRequest);
    expect (! duplicate.ok, "duplicate project should not overwrite by default");
    expect (duplicate.status == "already-exists", "duplicate project status");
    expect (! duplicate.error.empty(), "duplicate project error");

    createRequest.overwriteExisting = true;
    createRequest.workTitle = "Project One Overwrite";
    const auto overwritten = myworld::createActiveWorkProject (createRequest);
    expect (overwritten.ok, overwritten.error);
    expect (overwritten.status == "created", "overwrite project status");
    const auto overwrittenManifest = myworld::loadWorkProjectManifest (overwritten.workManifestPath);
    expect (overwrittenManifest.ok, overwrittenManifest.error);
    expect (overwrittenManifest.manifest.title == "Project One Overwrite", "overwrite work title");

    return 0;
}
