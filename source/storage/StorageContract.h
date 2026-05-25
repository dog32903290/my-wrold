#pragma once

#include "GraphContract.h"
#include "OutputViewState.h"
#include "TimelineState.h"

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

struct PatchDocument
{
    std::string id;
    std::string title;
    int version = 1;
    GraphContract graph;
    OutputViewState outputView;
    TimelineState timeline;
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

struct WorkProjectLoadResult
{
    bool ok = false;
    WorkProjectManifest manifest;
    std::string error;
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

struct PatchDocumentLoadResult
{
    bool ok = false;
    PatchDocument document;
    std::string error;
};

struct PatchDocumentSaveResult
{
    bool ok = false;
    std::string status;
    std::string path;
    std::string error;
};

WorkProjectManifest makeMinimalWorkProject (const std::string& id, const std::string& title);
PatchDocumentManifest makeMinimalPatchDocument (const std::string& id, const std::string& title);
PatchDocument makePatchDocument (const std::string& id, const std::string& title, const GraphContract& graph);
PatchDocument makePatchDocument (const std::string& id,
                                 const std::string& title,
                                 const GraphContract& graph,
                                 const OutputViewState& outputView);
PatchDocument makePatchDocument (const std::string& id,
                                 const std::string& title,
                                 const GraphContract& graph,
                                 const OutputViewState& outputView,
                                 const TimelineState& timeline);
ModulePackageManifest makeModulePackage (const std::string& id, const std::string& title, const std::string& patchPath);
ModuleLibraryManifest makeModuleLibrary (const std::string& id,
                                         const std::string& title,
                                         const std::vector<std::string>& modulePackages);

std::string toJson (const WorkProjectManifest& manifest);
std::string toJson (const PatchDocumentManifest& manifest);
std::string toJson (const PatchDocument& document);
std::string toJson (const ModulePackageManifest& manifest);
std::string toJson (const ModuleLibraryManifest& manifest);
WorkProjectLoadResult parseWorkProjectManifest (const std::string& text);
WorkProjectLoadResult loadWorkProjectManifest (const std::string& path);
PatchDocumentLoadResult parsePatchDocument (const std::string& text);
PatchDocumentLoadResult loadPatchDocument (const std::string& path);
PatchDocumentLoadResult loadMainPatchDocumentForWork (const std::string& workManifestPath);
PatchDocumentSaveResult savePatchDocument (const std::string& path, const PatchDocument& document);
ModulePackageLoadResult parseModulePackageManifest (const std::string& text);
ModulePackageLoadResult loadModulePackageManifest (const std::string& path);
ModuleLibraryLoadResult parseModuleLibraryManifest (const std::string& text);
ModuleLibraryLoadResult loadModuleLibraryManifest (const std::string& path);
bool isKnownSaveStatus (const std::string& status);
}
