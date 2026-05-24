#include "CompoundModule.h"
#include "InteractionContract.h"
#include "RuntimeRegistry.h"
#include "StorageCommand.h"
#include "StorageContract.h"

#include <algorithm>
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

void expectEqual (const std::string& actual, const std::string& expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + expected + " got " + actual);
}

bool containsCommand (const std::vector<std::string>& commands, const std::string& command)
{
    return std::find (commands.begin(), commands.end(), command) != commands.end();
}

std::string readTextFile (const std::filesystem::path& path)
{
    std::ifstream input (path);
    std::string text ((std::istreambuf_iterator<char> (input)), std::istreambuf_iterator<char>());
    return text;
}
}

int main()
{
    const auto workManifestPath = std::string { "fixtures/storage/c2-compound-work/myworld.work.json" };
    const auto loadedMain = myworld::loadMainPatchDocumentForWork (workManifestPath);
    expect (loadedMain.ok, loadedMain.error);

    auto sourceSession = myworld::makeGraphSession (loadedMain.document.graph);
    const auto publishRoot = std::filesystem::temp_directory_path() / "my-world-c5-module-publish";
    std::filesystem::remove_all (publishRoot);

    myworld::PublishModuleRequest request;
    request.workManifestPath = workManifestPath;
    request.sourceNodeId = "library_loud1";
    request.moduleId = "module.published-loudness";
    request.moduleTitle = "Published Loudness";
    request.nodeType = "compound.published-loudness";
    request.packageDirectory = (publishRoot / "modules" / "published-loudness").string();
    request.targetLibraryPath = (publishRoot / "module-libraries" / "published.module-library.json").string();
    request.overwriteExisting = true;

    const auto publish = myworld::publishModule (sourceSession, request);
    expect (publish.ok, publish.error);
    expectEqual (publish.status, "published", "publish status");
    expectEqual (publish.operation, "publish_module", "publish operation");
    expectEqual (publish.sourceNodeType, "compound.loudness", "source node type");
    expectEqual (publish.publishedNodeType, "compound.published-loudness", "published node type");
    expect (containsCommand (sourceSession.commandLog, "publish_module:published"),
            "publish command logged on graph session");
    expect (std::filesystem::exists (publish.moduleManifestPath), "published module manifest exists");
    expect (std::filesystem::exists (publish.compoundPatchPath), "published compound patch exists");
    expect (std::filesystem::exists (publish.targetLibraryPath), "published module library exists");

    const auto module = myworld::loadModulePackageManifest (publish.moduleManifestPath);
    expect (module.ok, module.error);
    expectEqual (module.manifest.id, "module.published-loudness", "published module id");
    expectEqual (module.manifest.title, "Published Loudness", "published module title");
    expectEqual (module.manifest.nodeType, "compound.published-loudness", "published module node type");
    expectEqual (module.manifest.patchPath, "compound.compound.json", "published module patch path is package-local");
    expect (module.manifest.publicPorts.size() == 6, "published module exposes input plus outputs");

    const auto publishedCompound = myworld::loadCompoundPatchSpec (publish.compoundPatchPath);
    expect (publishedCompound.ok, publishedCompound.error);
    expectEqual (publishedCompound.spec.type, "compound.published-loudness", "published compound type");
    expectEqual (publishedCompound.spec.displayName, "Published Loudness", "published compound title");
    expect (publishedCompound.spec.children.size() == 7, "published compound keeps children");
    expect (publishedCompound.spec.publicInputs.size() == 1, "published compound keeps public input");
    expect (publishedCompound.spec.publicOutputs.size() == 5, "published compound keeps public outputs");
    expect (readTextFile (publish.compoundPatchPath).find ("interaction-state-v1") == std::string::npos,
            "published compound does not use temporary interaction serializer");

    const auto library = myworld::loadModuleLibraryManifest (publish.targetLibraryPath);
    expect (library.ok, library.error);
    expect (library.manifest.modulePackages.size() == 1, "published library lists one module");

    const auto loadedSpecs = myworld::loadCompoundModuleNodeSpecsFromLibrary (publish.targetLibraryPath);
    expect (loadedSpecs.ok, loadedSpecs.error);
    expect (loadedSpecs.specs.size() == 1, "published library reloads one node spec");
    expectEqual (loadedSpecs.specs.front().type, "compound.published-loudness", "published visible spec type");

    const auto runtime = myworld::loadRuntimeRegistryFromModuleLibrary (publish.targetLibraryPath);
    expect (runtime.ok, runtime.error);
    expect (runtime.registry.entries.size() == 1, "published runtime registry reloads one entry");
    expectEqual (runtime.registry.entries.front().nodeType,
                 "compound.published-loudness",
                 "published runtime entry node type");

    const auto coverage = myworld::inspectRuntimeOpCoverage (runtime.registry);
    expect (coverage.ok, coverage.error);
    const auto diagnostics = myworld::makeRuntimeOpModuleDiagnostics (coverage.snapshot);
    expect (diagnostics.size() == 1, "published runtime diagnostics count");
    expectEqual (diagnostics.front().status, "runtime-op-ready", "published runtime coverage status");
    expectEqual (diagnostics.front().creationStatus, "create-enabled", "published creation gate status");

    const auto visibleRegistry = myworld::mergeNodeSpecs (myworld::makeSeedNodeSpecs(), loadedSpecs.specs);
    auto reuseSession = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    const auto create = myworld::createNode (reuseSession,
                                             visibleRegistry,
                                             "compound.published-loudness",
                                             "published_loud1",
                                             { 320.0, 260.0 });
    expect (create.ok, create.message);
    expect (containsCommand (reuseSession.commandLog, "create_node"), "published module create command logged");

    std::filesystem::remove_all (publishRoot);

    std::cout << "module publish/reuse proof ok\n";
    return 0;
}
