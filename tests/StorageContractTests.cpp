#include "StorageContract.h"

#include <cstdlib>
#include <filesystem>
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
}

int main()
{
    const auto work = myworld::makeMinimalWorkProject ("work.city-0001", "City Study");
    expect (work.id == "work.city-0001", "work id");
    expect (work.mainPatchPath == "patches/main.patch.json", "main patch path");
    expect (work.savePolicy == "commandS.localCommit", "save policy");

    const auto workJson = myworld::toJson (work);
    expectContains (workJson, "\"kind\": \"workProject\"", "work json");
    expectContains (workJson, "\"mainPatchPath\": \"patches/main.patch.json\"", "work json");
    expectContains (workJson, "\"moduleLibraries\"", "work json");

    const auto patch = myworld::makeMinimalPatchDocument ("patch.main", "Main Patch");
    const auto patchJson = myworld::toJson (patch);
    expectContains (patchJson, "\"kind\": \"patchDocument\"", "patch json");
    expectContains (patchJson, "\"editorGraph\"", "patch json");
    expectContains (patchJson, "\"runtimeGraph\"", "patch json");
    expectContains (patchJson, "\"portBindings\"", "patch json");

    const auto module = myworld::makeModulePackage ("module.loudness", "Loudness Module", "patches/loudness.patch.json");
    const auto moduleJson = myworld::toJson (module);
    expectContains (moduleJson, "\"kind\": \"modulePackage\"", "module json");
    expectContains (moduleJson, "\"nodeType\"", "module json");
    expectContains (moduleJson, "\"category\"", "module json");
    expectContains (moduleJson, "\"publicPorts\"", "module json");
    expectContains (moduleJson, "\"humanDocPath\"", "module json");

    const auto parsedModule = myworld::parseModulePackageManifest (moduleJson);
    expect (parsedModule.ok, parsedModule.error);
    expect (parsedModule.manifest.id == module.id, "module json parses id");
    expect (parsedModule.manifest.patchPath == module.patchPath, "module json parses patch path");

    const auto library = myworld::makeModuleLibrary ("library.default",
                                                     "Default Module Library",
                                                     { "modules/loudness/module.json" });
    const auto libraryJson = myworld::toJson (library);
    expectContains (libraryJson, "\"kind\": \"moduleLibrary\"", "module library json");
    expectContains (libraryJson, "\"modulePackages\"", "module library json");

    const auto parsedLibrary = myworld::parseModuleLibraryManifest (libraryJson);
    expect (parsedLibrary.ok, parsedLibrary.error);
    expect (parsedLibrary.manifest.id == library.id, "module library json parses id");
    expect (parsedLibrary.manifest.modulePackages.size() == 1, "module library json parses package count");
    expect (parsedLibrary.manifest.modulePackages.front() == "modules/loudness/module.json",
            "module library json parses package path");

    const auto loadedLibrary = myworld::loadModuleLibraryManifest ("fixtures/module-libraries/default.module-library.json");
    expect (loadedLibrary.ok, loadedLibrary.error);
    expect (loadedLibrary.manifest.modulePackages.size() == 1, "module library fixture package count");
    expect (loadedLibrary.manifest.modulePackages.front() == "modules/loudness/module.json",
            "module library fixture stores relative package path");

    const auto loadedModule = myworld::loadModulePackageManifest ("fixtures/modules/loudness/module.json");
    expect (loadedModule.ok, loadedModule.error);
    expect (loadedModule.manifest.nodeType == "compound.loudness", "module fixture node type");
    expect (loadedModule.manifest.publicPorts.size() == 6, "module fixture public ports");

    expect (myworld::isKnownSaveStatus ("saved-and-committed"), "saved-and-committed status");
    expect (myworld::isKnownSaveStatus ("save-ok commit-pending"), "commit-pending status");
    expect (myworld::isKnownSaveStatus ("save-ok commit-failed"), "commit-failed status");
    expect (! myworld::isKnownSaveStatus ("probably-saved"), "unknown status rejected");

    expect (std::filesystem::exists ("fixtures/storage/minimal-work/myworld.work.json"), "minimal work fixture");
    expect (std::filesystem::exists ("fixtures/storage/minimal-work/patches/main.patch.json"), "minimal patch fixture");

    std::cout << "storage contract ok\n";
    return 0;
}
