#pragma once

#include "CompoundPatch.h"
#include "NodeSpec.h"
#include "StorageContract.h"

namespace myworld
{
struct CompoundModuleNodeSpecsResult
{
    bool ok = false;
    std::vector<NodeSpec> specs;
    std::string error;
};

NodeSpec makeCompoundModuleNodeSpec (const ModulePackageManifest& manifest, const CompoundPatchSpec& compound);
CompoundModuleNodeSpecsResult loadCompoundModuleNodeSpecs (const std::vector<std::string>& manifestPaths);
CompoundModuleNodeSpecsResult loadCompoundModuleNodeSpecsFromLibrary (const std::string& libraryPath);
std::vector<NodeSpec> mergeNodeSpecs (std::vector<NodeSpec> baseSpecs, const std::vector<NodeSpec>& overrideSpecs);
}
