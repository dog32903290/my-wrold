#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace myworld
{
struct RuntimeRegistryEntry
{
    std::string nodeType;
    std::string displayName;
    std::string runtimeDomain;
    std::string executionKind;
    std::string previewPolicy;
    size_t childCount = 0;
    size_t internalEdgeCount = 0;
    std::vector<std::string> publicInputs;
    std::vector<std::string> publicOutputs;
    std::vector<std::string> cookOrder;
};

struct RuntimeRegistry
{
    int version = 1;
    std::vector<RuntimeRegistryEntry> entries;
};

struct RuntimeRegistryLoadResult
{
    bool ok = false;
    RuntimeRegistry registry;
    std::string error;
};

RuntimeRegistryLoadResult loadRuntimeRegistryFromModuleLibrary (const std::string& libraryPath);
std::string makeRuntimeRegistryJson (const RuntimeRegistry& registry);
}
