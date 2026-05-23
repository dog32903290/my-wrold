#include "CompoundModule.h"

#include <algorithm>

namespace myworld
{
namespace
{
PortSpec toPortSpec (const CompoundPublicPort& port)
{
    return { port.id, port.label, port.dataType, port.direction };
}

const CompoundPublicPort* findPort (const std::vector<CompoundPublicPort>& ports, const std::string& id)
{
    for (const auto& port : ports)
        if (port.id == id)
            return &port;

    return nullptr;
}

std::vector<PortSpec> toInputPortSpecs (const ModulePackageManifest& manifest, const CompoundPatchSpec& compound)
{
    std::vector<PortSpec> result;
    result.reserve (compound.publicInputs.size());

    for (const auto& portId : manifest.publicPorts)
        if (const auto* port = findPort (compound.publicInputs, portId))
            result.push_back (toPortSpec (*port));

    return result;
}

std::vector<PortSpec> toOutputPortSpecs (const ModulePackageManifest& manifest, const CompoundPatchSpec& compound)
{
    std::vector<PortSpec> result;
    result.reserve (compound.publicOutputs.size());

    for (const auto& portId : manifest.publicPorts)
        if (const auto* port = findPort (compound.publicOutputs, portId))
            result.push_back (toPortSpec (*port));

    return result;
}

std::string fallback (const std::string& value, const std::string& fallbackValue)
{
    return value.empty() ? fallbackValue : value;
}
}

NodeSpec makeCompoundModuleNodeSpec (const ModulePackageManifest& manifest, const CompoundPatchSpec& compound)
{
    return {
        fallback (manifest.nodeType, compound.type),
        fallback (manifest.title, compound.displayName),
        fallback (manifest.category, "compound"),
        fallback (manifest.subcategory, "feature"),
        fallback (manifest.runtimeDomain, "graph"),
        fallback (manifest.previewPolicy, "none"),
        manifest.humanDocPath,
        1,
        toInputPortSpecs (manifest, compound),
        toOutputPortSpecs (manifest, compound),
        {}
    };
}

CompoundModuleNodeSpecsResult loadCompoundModuleNodeSpecs (const std::vector<std::string>& manifestPaths)
{
    CompoundModuleNodeSpecsResult result;

    for (const auto& manifestPath : manifestPaths)
    {
        const auto module = loadModulePackageManifest (manifestPath);
        if (! module.ok)
            return { false, {}, module.error };

        const auto compound = loadCompoundPatchSpec (module.manifest.patchPath);
        if (! compound.ok)
            return { false, {}, compound.error };

        result.specs.push_back (makeCompoundModuleNodeSpec (module.manifest, compound.spec));
    }

    result.ok = true;
    return result;
}

std::vector<NodeSpec> mergeNodeSpecs (std::vector<NodeSpec> baseSpecs, const std::vector<NodeSpec>& overrideSpecs)
{
    for (const auto& spec : overrideSpecs)
    {
        const auto found = std::find_if (baseSpecs.begin(), baseSpecs.end(), [&spec] (const auto& existing) {
            return existing.type == spec.type;
        });

        if (found == baseSpecs.end())
        {
            baseSpecs.push_back (spec);
        }
        else
        {
            *found = spec;
        }
    }

    return baseSpecs;
}
}
