#pragma once

#include "CompoundPatch.h"
#include "NodeSpec.h"
#include "StorageContract.h"

namespace myworld
{
NodeSpec makeCompoundModuleNodeSpec (const ModulePackageManifest& manifest, const CompoundPatchSpec& compound);
}
