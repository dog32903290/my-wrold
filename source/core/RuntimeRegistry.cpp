#include "RuntimeRegistry.h"

#include "CompoundPatch.h"
#include "StorageContract.h"

#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace myworld
{
namespace
{
std::string jsonEscaped (const std::string& text)
{
    std::ostringstream out;

    for (const auto character : text)
    {
        switch (character)
        {
            case '"':  out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (static_cast<unsigned char> (character) < 0x20)
                    out << "\\u" << std::hex << std::setw (4) << std::setfill ('0')
                        << static_cast<int> (static_cast<unsigned char> (character));
                else
                    out << character;
                break;
        }
    }

    return out.str();
}

void appendStringArray (std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";

    for (size_t index = 0; index < values.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << "\"" << jsonEscaped (values[index]) << "\"";
    }

    out << "]";
}

std::string fallback (const std::string& value, const std::string& fallbackValue)
{
    return value.empty() ? fallbackValue : value;
}

std::string resolvePathNear (const std::string& anchorPath, const std::string& candidatePath)
{
    namespace fs = std::filesystem;

    const fs::path candidate { candidatePath };
    if (candidate.is_absolute() || fs::exists (candidate))
        return candidate.string();

    auto directory = fs::path { anchorPath };
    directory = directory.has_parent_path() ? directory.parent_path() : fs::current_path();

    for (int depth = 0; depth < 8; ++depth)
    {
        const auto resolved = directory / candidate;
        if (fs::exists (resolved))
            return resolved.string();

        if (! directory.has_parent_path() || directory == directory.parent_path())
            break;

        directory = directory.parent_path();
    }

    return candidate.string();
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

const RuntimeRegistryChild* findRuntimeChild (const RuntimeRegistryEntry& entry, const std::string& childId)
{
    const auto found = std::find_if (entry.children.begin(), entry.children.end(), [&childId] (const auto& child) {
        return child.id == childId;
    });

    return found == entry.children.end() ? nullptr : &(*found);
}

RuntimeRegistryEntry makeRuntimeRegistryEntry (const ModulePackageManifest& module, const CompoundPatchSpec& compound)
{
    return {
        fallback (module.nodeType, compound.type),
        fallback (module.title, compound.displayName),
        fallback (module.runtimeDomain, "graph"),
        "compound.patch",
        fallback (module.previewPolicy, "none"),
        runtimeChildren (compound),
        compound.children.size(),
        compound.internalEdges.size(),
        portIds (compound.publicInputs),
        portIds (compound.publicOutputs),
        makeCompoundCookOrder (compound)
    };
}
}

RuntimeRegistryLoadResult loadRuntimeRegistryFromModuleLibrary (const std::string& libraryPath)
{
    const auto resolvedLibraryPath = resolvePathNear ({}, libraryPath);
    const auto library = loadModuleLibraryManifest (resolvedLibraryPath);
    if (! library.ok)
        return { false, {}, library.error };

    RuntimeRegistry registry;

    for (const auto& modulePath : library.manifest.modulePackages)
    {
        const auto resolvedModulePath = resolvePathNear (resolvedLibraryPath, modulePath);
        const auto module = loadModulePackageManifest (resolvedModulePath);
        if (! module.ok)
            return { false, {}, module.error };

        const auto resolvedCompoundPath = resolvePathNear (resolvedModulePath, module.manifest.patchPath);
        const auto compound = loadCompoundPatchSpec (resolvedCompoundPath);
        if (! compound.ok)
            return { false, {}, compound.error };

        if (! isValidCompoundPatchSpec (compound.spec))
            return { false, {}, "compound patch is not valid for runtime registry: " + compound.spec.type };

        registry.entries.push_back (makeRuntimeRegistryEntry (module.manifest, compound.spec));
    }

    return { true, registry, {} };
}

RuntimeDryRunResult dryRunRuntimeRegistry (const RuntimeRegistry& registry)
{
    RuntimeDryRunSnapshot snapshot;
    snapshot.version = registry.version;

    for (const auto& entry : registry.entries)
    {
        RuntimeEntryDryRunStatus entryStatus;
        entryStatus.nodeType = entry.nodeType;
        entryStatus.executionKind = entry.executionKind;
        entryStatus.status = "dry-run-ready";
        entryStatus.children.reserve (entry.cookOrder.size());

        for (size_t cookIndex = 0; cookIndex < entry.cookOrder.size(); ++cookIndex)
        {
            const auto& childId = entry.cookOrder[cookIndex];
            const auto* child = findRuntimeChild (entry, childId);

            if (child == nullptr)
                return { false, {}, "dry-run missing child metadata for " + entry.nodeType + ":" + childId };

            entryStatus.children.push_back ({ cookIndex,
                                             child->id,
                                             child->nodeType,
                                             child->role,
                                             "dry-run-ready",
                                             "validated child order; RuntimeOp not executed" });
        }

        if (entryStatus.children.size() != entry.childCount)
            return { false, {}, "dry-run child count mismatch for " + entry.nodeType };

        snapshot.entries.push_back (entryStatus);
    }

    return { true, snapshot, {} };
}

std::string makeRuntimeRegistryJson (const RuntimeRegistry& registry)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"runtimeRegistry\",\n";
    out << "  \"version\": " << registry.version << ",\n";
    out << "  \"entries\": [\n";

    for (size_t index = 0; index < registry.entries.size(); ++index)
    {
        const auto& entry = registry.entries[index];
        out << "    {\n";
        out << "      \"nodeType\": \"" << jsonEscaped (entry.nodeType) << "\",\n";
        out << "      \"displayName\": \"" << jsonEscaped (entry.displayName) << "\",\n";
        out << "      \"runtimeDomain\": \"" << jsonEscaped (entry.runtimeDomain) << "\",\n";
        out << "      \"executionKind\": \"" << jsonEscaped (entry.executionKind) << "\",\n";
        out << "      \"previewPolicy\": \"" << jsonEscaped (entry.previewPolicy) << "\",\n";
        out << "      \"childCount\": " << entry.childCount << ",\n";
        out << "      \"internalEdgeCount\": " << entry.internalEdgeCount << ",\n";
        out << "      \"children\": [\n";

        for (size_t childIndex = 0; childIndex < entry.children.size(); ++childIndex)
        {
            const auto& child = entry.children[childIndex];
            out << "        { \"id\": \"" << jsonEscaped (child.id)
                << "\", \"nodeType\": \"" << jsonEscaped (child.nodeType)
                << "\", \"role\": \"" << jsonEscaped (child.role) << "\" }";

            if (childIndex + 1 < entry.children.size())
                out << ",";

            out << "\n";
        }

        out << "      ],\n";
        out << "      \"publicInputs\": ";
        appendStringArray (out, entry.publicInputs);
        out << ",\n";
        out << "      \"publicOutputs\": ";
        appendStringArray (out, entry.publicOutputs);
        out << ",\n";
        out << "      \"cookOrder\": ";
        appendStringArray (out, entry.cookOrder);
        out << "\n";
        out << "    }";

        if (index + 1 < registry.entries.size())
            out << ",";

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
    return out.str();
}

std::string makeRuntimeDryRunJson (const RuntimeDryRunSnapshot& snapshot)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"runtimeDryRun\",\n";
    out << "  \"version\": " << snapshot.version << ",\n";
    out << "  \"mode\": \"" << jsonEscaped (snapshot.mode) << "\",\n";
    out << "  \"entries\": [\n";

    for (size_t entryIndex = 0; entryIndex < snapshot.entries.size(); ++entryIndex)
    {
        const auto& entry = snapshot.entries[entryIndex];
        out << "    {\n";
        out << "      \"nodeType\": \"" << jsonEscaped (entry.nodeType) << "\",\n";
        out << "      \"executionKind\": \"" << jsonEscaped (entry.executionKind) << "\",\n";
        out << "      \"status\": \"" << jsonEscaped (entry.status) << "\",\n";
        out << "      \"children\": [\n";

        for (size_t childIndex = 0; childIndex < entry.children.size(); ++childIndex)
        {
            const auto& child = entry.children[childIndex];
            out << "        {\n";
            out << "          \"cookIndex\": " << child.cookIndex << ",\n";
            out << "          \"childId\": \"" << jsonEscaped (child.childId) << "\",\n";
            out << "          \"nodeType\": \"" << jsonEscaped (child.nodeType) << "\",\n";
            out << "          \"role\": \"" << jsonEscaped (child.role) << "\",\n";
            out << "          \"status\": \"" << jsonEscaped (child.status) << "\",\n";
            out << "          \"reason\": \"" << jsonEscaped (child.reason) << "\"\n";
            out << "        }";

            if (childIndex + 1 < entry.children.size())
                out << ",";

            out << "\n";
        }

        out << "      ]\n";
        out << "    }";

        if (entryIndex + 1 < snapshot.entries.size())
            out << ",";

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
    return out.str();
}
}
