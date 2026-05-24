#include "RuntimeRegistry.h"

#include "CompoundPatch.h"
#include "PathResolution.h"
#include "StorageContract.h"

namespace myworld
{
namespace
{
std::string fallback (const std::string& value, const std::string& fallbackValue)
{
    return value.empty() ? fallbackValue : value;
}

std::vector<std::string> portIds (const std::vector<CompoundPublicPort>& ports)
{
    std::vector<std::string> ids;
    ids.reserve (ports.size());

    for (const auto& port : ports)
        ids.push_back (port.id);

    return ids;
}

std::vector<RuntimeRegistryChild> runtimeChildren (const CompoundPatchSpec& compound)
{
    std::vector<RuntimeRegistryChild> children;
    children.reserve (compound.children.size());

    for (const auto& child : compound.children)
        children.push_back ({ child.id, child.nodeType, child.role });

    return children;
}

std::vector<RuntimeRegistryEdge> runtimeEdges (const CompoundPatchSpec& compound)
{
    std::vector<RuntimeRegistryEdge> edges;
    edges.reserve (compound.internalEdges.size());

    for (const auto& edge : compound.internalEdges)
        edges.push_back ({ edge.from, edge.to, edge.dataType });

    return edges;
}

std::vector<RuntimeRegistryPublicOutputMapping> runtimePublicOutputMappings (const CompoundPatchSpec& compound)
{
    std::vector<RuntimeRegistryPublicOutputMapping> mappings;
    mappings.reserve (compound.publicOutputs.size());

    for (const auto& port : compound.publicOutputs)
        mappings.push_back ({ port.id, port.mapsTo });

    return mappings;
}

RuntimeRegistryEntry makeRuntimeRegistryEntry (const ModulePackageManifest& module, const CompoundPatchSpec& compound)
{
    RuntimeRegistryEntry entry;
    entry.nodeType = fallback (module.nodeType, compound.type);
    entry.displayName = fallback (module.title, compound.displayName);
    entry.runtimeDomain = fallback (module.runtimeDomain, "graph");
    entry.executionKind = "compound.patch";
    entry.previewPolicy = fallback (module.previewPolicy, "none");
    entry.children = runtimeChildren (compound);
    entry.childCount = compound.children.size();
    entry.internalEdgeCount = compound.internalEdges.size();
    entry.internalEdges = runtimeEdges (compound);
    entry.publicInputs = portIds (compound.publicInputs);
    entry.publicOutputs = portIds (compound.publicOutputs);
    entry.publicOutputMappings = runtimePublicOutputMappings (compound);
    entry.cookOrder = makeCompoundCookOrder (compound);
    return entry;
}
}

RuntimeRegistryLoadResult loadRuntimeRegistryFromModuleLibrary (const std::string& libraryPath)
{
    const auto libraryResolution = resolvePathNearWithReport ({}, libraryPath);
    if (! libraryResolution.found)
        return { false, {}, describePathResolutionFailure ("module library", libraryResolution) };

    const auto library = loadModuleLibraryManifest (libraryResolution.resolvedPath);
    if (! library.ok)
        return { false, {}, library.error };

    RuntimeRegistry registry;

    for (const auto& modulePath : library.manifest.modulePackages)
    {
        const auto moduleResolution = resolvePathNearWithReport (libraryResolution.resolvedPath, modulePath);
        if (! moduleResolution.found)
            return { false, {}, describePathResolutionFailure ("module manifest", moduleResolution) };

        const auto module = loadModulePackageManifest (moduleResolution.resolvedPath);
        if (! module.ok)
            return { false, {}, module.error };

        const auto compoundResolution = resolvePathNearWithReport (moduleResolution.resolvedPath, module.manifest.patchPath);
        if (! compoundResolution.found)
            return { false, {}, describePathResolutionFailure ("compound patch", compoundResolution) };

        const auto compound = loadCompoundPatchSpec (compoundResolution.resolvedPath);
        if (! compound.ok)
            return { false, {}, compound.error };

        if (! isValidCompoundPatchSpec (compound.spec))
            return { false, {}, "compound patch is not valid for runtime registry: " + compound.spec.type };

        registry.entries.push_back (makeRuntimeRegistryEntry (module.manifest, compound.spec));
    }

    return { true, registry, {} };
}
}
