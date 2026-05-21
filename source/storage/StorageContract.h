#pragma once

#include <string>
#include <vector>

namespace myworld
{
struct WorkProjectManifest
{
    std::string id;
    std::string title;
    std::string mainPatchPath;
    std::string savePolicy;
    std::vector<std::string> moduleLibraries;
    std::vector<std::string> workLibraries;
};

struct PatchDocumentManifest
{
    std::string id;
    std::string title;
    std::string editorGraphKind;
    std::string runtimeGraphKind;
    std::string portBindingsKind;
};

struct ModulePackageManifest
{
    std::string id;
    std::string title;
    std::string patchPath;
    std::string humanDocPath;
    std::vector<std::string> publicPorts;
};

WorkProjectManifest makeMinimalWorkProject (const std::string& id, const std::string& title);
PatchDocumentManifest makeMinimalPatchDocument (const std::string& id, const std::string& title);
ModulePackageManifest makeModulePackage (const std::string& id, const std::string& title, const std::string& patchPath);

std::string toJson (const WorkProjectManifest& manifest);
std::string toJson (const PatchDocumentManifest& manifest);
std::string toJson (const ModulePackageManifest& manifest);
bool isKnownSaveStatus (const std::string& status);
}
