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
    std::string nodeType;
    std::string patchPath;
    std::string humanDocPath;
    std::string category;
    std::string subcategory;
    std::string runtimeDomain;
    std::string previewPolicy;
    std::vector<std::string> publicPorts;
};

struct ModuleLibraryManifest
{
    std::string id;
    std::string title;
    std::vector<std::string> modulePackages;
};

struct ModulePackageLoadResult
{
    bool ok = false;
    ModulePackageManifest manifest;
    std::string error;
};

struct ModuleLibraryLoadResult
{
    bool ok = false;
    ModuleLibraryManifest manifest;
    std::string error;
};

WorkProjectManifest makeMinimalWorkProject (const std::string& id, const std::string& title);
PatchDocumentManifest makeMinimalPatchDocument (const std::string& id, const std::string& title);
ModulePackageManifest makeModulePackage (const std::string& id, const std::string& title, const std::string& patchPath);
ModuleLibraryManifest makeModuleLibrary (const std::string& id,
                                         const std::string& title,
                                         const std::vector<std::string>& modulePackages);

std::string toJson (const WorkProjectManifest& manifest);
std::string toJson (const PatchDocumentManifest& manifest);
std::string toJson (const ModulePackageManifest& manifest);
std::string toJson (const ModuleLibraryManifest& manifest);
ModulePackageLoadResult parseModulePackageManifest (const std::string& text);
ModulePackageLoadResult loadModulePackageManifest (const std::string& path);
ModuleLibraryLoadResult parseModuleLibraryManifest (const std::string& text);
ModuleLibraryLoadResult loadModuleLibraryManifest (const std::string& path);
bool isKnownSaveStatus (const std::string& status);
}
